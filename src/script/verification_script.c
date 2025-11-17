#include "neoc/script/verification_script.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/script/interop_service.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/hash.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint8_t *data;
    size_t length;
} neoc_key_entry_t;

static size_t neoc_verification_var_int_size(uint64_t value) {
    if (value < 0xFD) {
        return 1;
    }
    if (value <= 0xFFFF) {
        return 3;
    }
    if (value <= 0xFFFFFFFF) {
        return 5;
    }
    return 9;
}

static neoc_error_t neoc_verification_script_allocate(const uint8_t *script,
                                                       size_t script_length,
                                                       neoc_verification_script_t **verification_script) {
    if (!verification_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Verification script output pointer is NULL");
    }

    *verification_script = neoc_calloc(1, sizeof(neoc_verification_script_t));
    if (!*verification_script) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate verification script");
    }

    if (script_length > 0) {
        if (!script) {
            neoc_verification_script_free(*verification_script);
            *verification_script = NULL;
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Script data pointer is NULL");
        }

        (*verification_script)->script = neoc_memdup(script, script_length);
        if (!(*verification_script)->script) {
            neoc_verification_script_free(*verification_script);
            *verification_script = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy verification script data");
        }

        (*verification_script)->script_length = script_length;
    }

    return NEOC_SUCCESS;
}

static int neoc_key_entry_compare(const void *lhs, const void *rhs) {
    const neoc_key_entry_t *a = lhs;
    const neoc_key_entry_t *b = rhs;
    size_t min_len = a->length < b->length ? a->length : b->length;
    int cmp = memcmp(a->data, b->data, min_len);
    if (cmp != 0) {
        return cmp;
    }
    if (a->length == b->length) {
        return 0;
    }
    return (a->length < b->length) ? -1 : 1;
}

static neoc_error_t neoc_verification_script_parse_push_int(const uint8_t *script,
                                                             size_t script_length,
                                                             size_t *offset,
                                                             int64_t *value) {
    if (!script || !offset || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for push-int parsing");
    }
    if (*offset >= script_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected end of script while reading integer");
    }

    uint8_t opcode = script[(*offset)++];
    switch (opcode) {
        case NEOC_OP_PUSHM1:
            *value = -1;
            return NEOC_SUCCESS;
        case NEOC_OP_PUSH0:
            *value = 0;
            return NEOC_SUCCESS;
        case NEOC_OP_PUSH1: case NEOC_OP_PUSH2: case NEOC_OP_PUSH3:
        case NEOC_OP_PUSH4: case NEOC_OP_PUSH5: case NEOC_OP_PUSH6:
        case NEOC_OP_PUSH7: case NEOC_OP_PUSH8: case NEOC_OP_PUSH9:
        case NEOC_OP_PUSH10: case NEOC_OP_PUSH11: case NEOC_OP_PUSH12:
        case NEOC_OP_PUSH13: case NEOC_OP_PUSH14: case NEOC_OP_PUSH15:
        case NEOC_OP_PUSH16:
            *value = (int64_t)(opcode - NEOC_OP_PUSH0);
            return NEOC_SUCCESS;
        case NEOC_OP_PUSHINT8:
            if (*offset + 1 > script_length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid PUSHINT8 encoding");
            }
            *value = (int8_t)script[(*offset)++];
            return NEOC_SUCCESS;
        case NEOC_OP_PUSHINT16:
            if (*offset + 2 > script_length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid PUSHINT16 encoding");
            }
            *value = (int16_t)(script[*offset] | (script[*offset + 1] << 8));
            *offset += 2;
            return NEOC_SUCCESS;
        case NEOC_OP_PUSHINT32:
            if (*offset + 4 > script_length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid PUSHINT32 encoding");
            }
            *value = (int32_t)(script[*offset] |
                               (script[*offset + 1] << 8) |
                               (script[*offset + 2] << 16) |
                               (script[*offset + 3] << 24));
            *offset += 4;
            return NEOC_SUCCESS;
        case NEOC_OP_PUSHINT64: {
            if (*offset + 8 > script_length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid PUSHINT64 encoding");
            }
            int64_t tmp = 0;
            for (int i = 0; i < 8; i++) {
                tmp |= ((int64_t)script[*offset + i]) << (8 * i);
            }
            *value = tmp;
            *offset += 8;
            return NEOC_SUCCESS;
        }
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unsupported integer opcode in script");
    }
}

static uint32_t neoc_verification_script_expected_hash(neoc_interop_service_t service) {
    return neoc_interop_get_hash(service);
}

neoc_error_t neoc_verification_script_create(const uint8_t *script,
                                              size_t script_length,
                                              neoc_verification_script_t **verification_script) {
    return neoc_verification_script_allocate(script, script_length, verification_script);
}

neoc_error_t neoc_verification_script_create_single_sig(const neoc_ec_public_key_t *public_key,
                                                         neoc_verification_script_t **verification_script) {
    if (!public_key || !verification_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for single signature script");
    }

    uint8_t *encoded = NULL;
    size_t encoded_len = 0;
    neoc_error_t err = neoc_ec_public_key_get_encoded(public_key, true, &encoded, &encoded_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_build_verification_script(encoded, encoded_len, &script, &script_len);
    neoc_free(encoded);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_verification_script_allocate(script, script_len, verification_script);
    neoc_free(script);
    return err;
}

neoc_error_t neoc_verification_script_create_multi_sig(neoc_ec_public_key_t **public_keys,
                                                        size_t key_count,
                                                        int signing_threshold,
                                                        neoc_verification_script_t **verification_script) {
    if (!verification_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Verification script output pointer is NULL");
    }
    if (key_count == 0 || signing_threshold <= 0 || signing_threshold > (int)key_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid key count or signing threshold");
    }
    if (key_count > NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Too many public keys for multisig script");
    }
    if (!public_keys) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Public key array is NULL");
    }

    neoc_key_entry_t *entries = neoc_calloc(key_count, sizeof(neoc_key_entry_t));
    if (!entries) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate key entries");
    }

    neoc_error_t err = NEOC_SUCCESS;
    for (size_t i = 0; i < key_count; i++) {
        if (!public_keys[i]) {
            err = neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Public key pointer is NULL");
            goto cleanup_entries;
        }
        uint8_t *encoded = NULL;
        size_t encoded_len = 0;
        err = neoc_ec_public_key_get_encoded(public_keys[i], true, &encoded, &encoded_len);
        if (err != NEOC_SUCCESS) {
            goto cleanup_entries;
        }
        entries[i].data = encoded;
        entries[i].length = encoded_len;
    }

    qsort(entries, key_count, sizeof(neoc_key_entry_t), neoc_key_entry_compare);

    neoc_script_builder_t *builder = NULL;
    err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        goto cleanup_entries;
    }

    err = neoc_script_builder_push_integer(builder, signing_threshold);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        goto cleanup_entries;
    }

    for (size_t i = 0; i < key_count; i++) {
        err = neoc_script_builder_push_data(builder, entries[i].data, entries[i].length);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            goto cleanup_entries;
        }
    }

    err = neoc_script_builder_push_integer(builder, (int64_t)key_count);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        goto cleanup_entries;
    }

    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        goto cleanup_entries;
    }

    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(builder, &script, &script_len);
    neoc_script_builder_free(builder);
    if (err != NEOC_SUCCESS) {
        goto cleanup_entries;
    }

    err = neoc_verification_script_allocate(script, script_len, verification_script);
    neoc_free(script);

cleanup_entries:
    for (size_t i = 0; i < key_count; i++) {
        neoc_free(entries[i].data);
    }
    neoc_free(entries);
    return err;
}

neoc_error_t neoc_verification_script_get_script(const neoc_verification_script_t *verification_script,
                                                  const uint8_t **script,
                                                  size_t *script_length) {
    if (!verification_script || !script || !script_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for get script");
    }
    *script = verification_script->script;
    *script_length = verification_script->script_length;
    return NEOC_SUCCESS;
}

size_t neoc_verification_script_get_size(const neoc_verification_script_t *verification_script) {
    if (!verification_script) {
        return 0;
    }
    return neoc_verification_var_int_size(verification_script->script_length) +
           verification_script->script_length;
}

static bool neoc_verification_script_checksig_match(const uint8_t *script, size_t script_length) {
    if (script_length != 40) {
        return false;
    }
    if (script[0] != NEOC_OP_PUSHDATA1 || script[1] != NEOC_PUBLIC_KEY_SIZE_COMPRESSED) {
        return false;
    }
    if (script[35] != NEOC_OP_SYSCALL) {
        return false;
    }
    uint32_t expected = neoc_verification_script_expected_hash(NEOC_INTEROP_SYSTEM_CRYPTO_CHECKSIG);
    uint32_t actual;
    memcpy(&actual, script + 36, sizeof(uint32_t));
    return expected == actual;
}

bool neoc_verification_script_is_single_sig(const neoc_verification_script_t *verification_script) {
    if (!verification_script || verification_script->script_length != 40) {
        return false;
    }
    return neoc_verification_script_checksig_match(verification_script->script,
                                                   verification_script->script_length);
}

bool neoc_verification_script_is_multi_sig(const neoc_verification_script_t *verification_script) {
    if (!verification_script || verification_script->script_length < 42) {
        return false;
    }

    size_t offset = 0;
    int64_t threshold = 0;
    if (neoc_verification_script_parse_push_int(verification_script->script,
                                                verification_script->script_length,
                                                &offset,
                                                &threshold) != NEOC_SUCCESS) {
        return false;
    }
    if (threshold < 1 || threshold > NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT) {
        return false;
    }

    size_t key_count = 0;
    const uint8_t *script = verification_script->script;
    size_t script_length = verification_script->script_length;

    while (offset < script_length) {
        if (script[offset] != NEOC_OP_PUSHDATA1) {
            break;
        }
        if (offset + 2 > script_length) {
            return false;
        }
        uint8_t len = script[offset + 1];
        if (len != NEOC_PUBLIC_KEY_SIZE_COMPRESSED) {
            return false;
        }
        if (offset + 2 + len > script_length) {
            return false;
        }
        offset += 2 + len;
        key_count++;
    }

    if (key_count == 0 || key_count > NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT) {
        return false;
    }
    if (threshold > (int64_t)key_count) {
        return false;
    }

    int64_t declared_count = 0;
    if (neoc_verification_script_parse_push_int(script, script_length, &offset, &declared_count) != NEOC_SUCCESS) {
        return false;
    }
    if (declared_count != (int64_t)key_count) {
        return false;
    }
    if (offset >= script_length || script[offset++] != NEOC_OP_SYSCALL) {
        return false;
    }
    if (offset + 4 > script_length) {
        return false;
    }
    uint32_t expected = neoc_verification_script_expected_hash(NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG);
    uint32_t actual;
    memcpy(&actual, script + offset, sizeof(uint32_t));
    offset += 4;
    if (expected != actual) {
        return false;
    }

    return offset == script_length;
}

neoc_error_t neoc_verification_script_get_signing_threshold(const neoc_verification_script_t *verification_script,
                                                             int *threshold) {
    if (!verification_script || !threshold) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for signing threshold");
    }

    if (neoc_verification_script_is_single_sig(verification_script)) {
        *threshold = 1;
        return NEOC_SUCCESS;
    }

    size_t offset = 0;
    int64_t value = 0;
    neoc_error_t err = neoc_verification_script_parse_push_int(verification_script->script,
                                                               verification_script->script_length,
                                                               &offset,
                                                               &value);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (value < 1 || value > NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid signing threshold in script");
    }

    *threshold = (int)value;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_verification_script_get_nr_accounts(const neoc_verification_script_t *verification_script,
                                                       int *nr_accounts) {
    if (!verification_script || !nr_accounts) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for nr accounts");
    }

    neoc_ec_public_key_t **keys = NULL;
    size_t count = 0;
    neoc_error_t err = neoc_verification_script_get_public_keys(verification_script, &keys, &count);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_ec_public_key_free(keys[i]);
    }
    neoc_free(keys);

    *nr_accounts = (int)count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_verification_script_get_public_keys(const neoc_verification_script_t *verification_script,
                                                       neoc_ec_public_key_t ***public_keys,
                                                       size_t *key_count) {
    if (!verification_script || !public_keys || !key_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for get public keys");
    }

    *public_keys = NULL;
    *key_count = 0;

    if (neoc_verification_script_is_single_sig(verification_script)) {
        neoc_ec_public_key_t **keys = neoc_calloc(1, sizeof(neoc_ec_public_key_t *));
        if (!keys) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate key array");
        }
        neoc_error_t err = neoc_ec_public_key_from_bytes(verification_script->script + 2,
                                                         NEOC_PUBLIC_KEY_SIZE_COMPRESSED,
                                                         &keys[0]);
        if (err != NEOC_SUCCESS) {
            neoc_free(keys);
            return err;
        }
        *public_keys = keys;
        *key_count = 1;
        return NEOC_SUCCESS;
    }

    if (!neoc_verification_script_is_multi_sig(verification_script)) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Verification script is not signature-based");
    }

    size_t offset = 0;
    int64_t threshold = 0;
    neoc_error_t err = neoc_verification_script_parse_push_int(verification_script->script,
                                                               verification_script->script_length,
                                                               &offset,
                                                               &threshold);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    (void)threshold; /* threshold already validated in is_multi_sig */

    const uint8_t *script = verification_script->script;
    size_t script_length = verification_script->script_length;

    size_t count = 0;
    size_t tmp_offset = offset;
    while (tmp_offset < script_length && script[tmp_offset] == NEOC_OP_PUSHDATA1) {
        tmp_offset += 2 + NEOC_PUBLIC_KEY_SIZE_COMPRESSED;
        count++;
    }

    if (count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "No public keys found in script");
    }

    neoc_ec_public_key_t **keys = neoc_calloc(count, sizeof(neoc_ec_public_key_t *));
    if (!keys) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate key list");
    }

    for (size_t i = 0; i < count; i++) {
        if (offset + 2 + NEOC_PUBLIC_KEY_SIZE_COMPRESSED > script_length) {
            for (size_t j = 0; j < i; j++) {
                neoc_ec_public_key_free(keys[j]);
            }
            neoc_free(keys);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Malformed public key entry");
        }
        offset += 2; /* skip opcode and length */
        err = neoc_ec_public_key_from_bytes(script + offset,
                                            NEOC_PUBLIC_KEY_SIZE_COMPRESSED,
                                            &keys[i]);
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; j++) {
                neoc_ec_public_key_free(keys[j]);
            }
            neoc_free(keys);
            return err;
        }
        offset += NEOC_PUBLIC_KEY_SIZE_COMPRESSED;
    }

    *public_keys = keys;
    *key_count = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_verification_script_get_script_hash(const neoc_verification_script_t *verification_script,
                                                       neoc_hash160_t *script_hash) {
    if (!verification_script || !script_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for script hash");
    }
    return neoc_hash160_from_script(script_hash,
                                    verification_script->script,
                                    verification_script->script_length);
}

neoc_error_t neoc_verification_script_get_address(const neoc_verification_script_t *verification_script,
                                                   char **address) {
    if (!verification_script || !address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for get address");
    }
    neoc_hash160_t hash;
    neoc_error_t err = neoc_verification_script_get_script_hash(verification_script, &hash);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    size_t buffer_size = neoc_base58_check_encode_buffer_size(1 + NEOC_HASH160_SIZE);
    char *addr = neoc_malloc(buffer_size);
    if (!addr) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address buffer");
    }
    err = neoc_hash160_to_address(&hash, addr, buffer_size);
    if (err != NEOC_SUCCESS) {
        neoc_free(addr);
        return err;
    }
    *address = addr;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_verification_script_copy(const neoc_verification_script_t *source,
                                            neoc_verification_script_t **copy) {
    if (!source || !copy) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for copy");
    }
    return neoc_verification_script_allocate(source->script,
                                             source->script_length,
                                             copy);
}

bool neoc_verification_script_equals(const neoc_verification_script_t *script1,
                                      const neoc_verification_script_t *script2) {
    if (script1 == script2) {
        return true;
    }
    if (!script1 || !script2) {
        return false;
    }
    if (script1->script_length != script2->script_length) {
        return false;
    }
    if (script1->script_length == 0) {
        return true;
    }
    return memcmp(script1->script, script2->script, script1->script_length) == 0;
}

uint32_t neoc_verification_script_hash(const neoc_verification_script_t *verification_script) {
    if (!verification_script || verification_script->script_length == 0) {
        return neoc_hash_murmur32(NULL, 0, 0);
    }
    return neoc_hash_murmur32(verification_script->script,
                              verification_script->script_length,
                              0);
}

neoc_error_t neoc_verification_script_serialize(const neoc_verification_script_t *verification_script,
                                                 uint8_t **serialized,
                                                 size_t *serialized_length) {
    if (!verification_script || !serialized || !serialized_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for serialize");
    }

    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(32, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_binary_writer_write_var_bytes(writer,
                                             verification_script->script,
                                             verification_script->script_length);
    if (err == NEOC_SUCCESS) {
        err = neoc_binary_writer_to_array(writer, serialized, serialized_length);
    }

    neoc_binary_writer_free(writer);
    return err;
}

neoc_error_t neoc_verification_script_deserialize(const uint8_t *serialized,
                                                   size_t serialized_length,
                                                   neoc_verification_script_t **verification_script) {
    if (!serialized || !verification_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments for deserialize");
    }

    neoc_binary_reader_t *reader = NULL;
    neoc_error_t err = neoc_binary_reader_create(serialized, serialized_length, &reader);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_reader_read_var_bytes(reader, &data, &data_len);
    if (err != NEOC_SUCCESS) {
        neoc_binary_reader_free(reader);
        return err;
    }

    err = neoc_verification_script_allocate(data, data_len, verification_script);
    neoc_free(data);
    neoc_binary_reader_free(reader);
    return err;
}

neoc_error_t neoc_verification_script_clear(neoc_verification_script_t *verification_script) {
    if (!verification_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Verification script pointer is NULL");
    }
    if (verification_script->script) {
        neoc_free(verification_script->script);
        verification_script->script = NULL;
    }
    verification_script->script_length = 0;
    return NEOC_SUCCESS;
}

void neoc_verification_script_free(neoc_verification_script_t *verification_script) {
    if (!verification_script) {
        return;
    }
    neoc_verification_script_clear(verification_script);
    neoc_free(verification_script);
}
