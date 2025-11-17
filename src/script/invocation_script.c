#include "neoc/script/invocation_script.h"
#include "neoc/script/opcode.h"
#include "neoc/neo_constants.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/crypto/hash.h"
#include <string.h>

/**
 * Compute the size of a Neo varint encoding.
 */
static size_t neoc_invocation_var_int_size(uint64_t value) {
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

/**
 * Determine pushdata header length for the provided payload size.
 */
static neoc_error_t neoc_invocation_pushdata_header_size(size_t data_len,
                                                          size_t *header_len) {
    if (!header_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Header length pointer is NULL");
    }

    if (data_len <= 0xFF) {
        *header_len = 2; /* opcode + uint8 length */
    } else if (data_len <= 0xFFFF) {
        *header_len = 3; /* opcode + uint16 length */
    } else if (data_len <= 0xFFFFFFFF) {
        *header_len = 5; /* opcode + uint32 length */
    } else {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Push data length exceeds supported range");
    }

    return NEOC_SUCCESS;
}

/**
 * Write a pushdata header into the provided buffer.
 */
static neoc_error_t neoc_invocation_write_pushdata_header(uint8_t *buffer,
                                                           size_t buffer_len,
                                                           size_t data_len,
                                                           size_t *written) {
    if (!buffer || !written) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid pushdata header arguments");
    }

    size_t required = 0;
    neoc_error_t err = neoc_invocation_pushdata_header_size(data_len, &required);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (buffer_len < required) {
        return neoc_error_set(NEOC_ERROR_BUFFER_OVERFLOW, "Pushdata header buffer too small");
    }

    if (data_len <= 0xFF) {
        buffer[0] = NEOC_OP_PUSHDATA1;
        buffer[1] = (uint8_t)data_len;
    } else if (data_len <= 0xFFFF) {
        buffer[0] = NEOC_OP_PUSHDATA2;
        buffer[1] = (uint8_t)(data_len & 0xFF);
        buffer[2] = (uint8_t)((data_len >> 8) & 0xFF);
    } else {
        buffer[0] = NEOC_OP_PUSHDATA4;
        buffer[1] = (uint8_t)(data_len & 0xFF);
        buffer[2] = (uint8_t)((data_len >> 8) & 0xFF);
        buffer[3] = (uint8_t)((data_len >> 16) & 0xFF);
        buffer[4] = (uint8_t)((data_len >> 24) & 0xFF);
    }

    *written = required;
    return NEOC_SUCCESS;
}

/**
 * Append pushdata encoded payload to the invocation script buffer.
 */
static neoc_error_t neoc_invocation_append_data(neoc_invocation_script_t *invocation_script,
                                                 const uint8_t *data,
                                                 size_t data_len) {
    if (!invocation_script || !data) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid invocation script append arguments");
    }

    uint8_t header[5] = {0};
    size_t header_len = 0;
    neoc_error_t err = neoc_invocation_write_pushdata_header(header,
                                                             sizeof(header),
                                                             data_len,
                                                             &header_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    size_t new_len = invocation_script->script_length + header_len + data_len;
    uint8_t *new_buffer = neoc_realloc(invocation_script->script, new_len);
    if (!new_buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to grow invocation script buffer");
    }

    size_t offset = invocation_script->script_length;
    memcpy(new_buffer + offset, header, header_len);
    offset += header_len;
    memcpy(new_buffer + offset, data, data_len);

    invocation_script->script = new_buffer;
    invocation_script->script_length = new_len;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_invocation_script_allocate(const uint8_t *script,
                                                     size_t script_length,
                                                     neoc_invocation_script_t **invocation_script) {
    if (!invocation_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invocation script output pointer is NULL");
    }

    *invocation_script = neoc_calloc(1, sizeof(neoc_invocation_script_t));
    if (!*invocation_script) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate invocation script");
    }

    if (script_length > 0) {
        if (!script) {
            neoc_invocation_script_free(*invocation_script);
            *invocation_script = NULL;
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Script data is NULL but length is non-zero");
        }

        (*invocation_script)->script = neoc_memdup(script, script_length);
        if (!(*invocation_script)->script) {
            neoc_invocation_script_free(*invocation_script);
            *invocation_script = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy invocation script data");
        }

        (*invocation_script)->script_length = script_length;
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_invocation_script_create(const uint8_t *script,
                                            size_t script_length,
                                            neoc_invocation_script_t **invocation_script) {
    return neoc_invocation_script_allocate(script, script_length, invocation_script);
}

neoc_error_t neoc_invocation_script_create_empty(neoc_invocation_script_t **invocation_script) {
    return neoc_invocation_script_allocate(NULL, 0, invocation_script);
}

neoc_error_t neoc_invocation_script_create_single_sig(const uint8_t *signature,
                                                       size_t signature_length,
                                                       neoc_invocation_script_t **invocation_script) {
    if (!signature || !invocation_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid single signature invocation arguments");
    }

    if (signature_length != NEOC_SIGNATURE_SIZE) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Signature must be 64 bytes");
    }

    neoc_invocation_script_t *script_obj = NULL;
    neoc_error_t err = neoc_invocation_script_allocate(NULL, 0, &script_obj);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_invocation_append_data(script_obj, signature, signature_length);
    if (err != NEOC_SUCCESS) {
        neoc_invocation_script_free(script_obj);
        return err;
    }

    *invocation_script = script_obj;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_invocation_script_create_multi_sig(const uint8_t **signatures,
                                                      const size_t *signature_lengths,
                                                      size_t signature_count,
                                                      neoc_invocation_script_t **invocation_script) {
    if (!invocation_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invocation script output pointer is NULL");
    }

    if (signature_count > 0 && (!signatures || !signature_lengths)) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Signature arrays are NULL");
    }

    neoc_invocation_script_t *script_obj = NULL;
    neoc_error_t err = neoc_invocation_script_allocate(NULL, 0, &script_obj);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    for (size_t i = 0; i < signature_count; i++) {
        if (!signatures[i]) {
            neoc_invocation_script_free(script_obj);
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Signature pointer is NULL");
        }
        if (signature_lengths[i] == 0) {
            neoc_invocation_script_free(script_obj);
            return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Signature length must be greater than zero");
        }

        err = neoc_invocation_append_data(script_obj, signatures[i], signature_lengths[i]);
        if (err != NEOC_SUCCESS) {
            neoc_invocation_script_free(script_obj);
            return err;
        }
    }

    *invocation_script = script_obj;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_invocation_script_add_signature(neoc_invocation_script_t *invocation_script,
                                                   const uint8_t *signature,
                                                   size_t signature_length) {
    if (!invocation_script || !signature) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid add signature arguments");
    }

    if (signature_length == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Signature length must be greater than zero");
    }

    return neoc_invocation_append_data(invocation_script, signature, signature_length);
}

neoc_error_t neoc_invocation_script_get_script(const neoc_invocation_script_t *invocation_script,
                                                const uint8_t **script,
                                                size_t *script_length) {
    if (!invocation_script || !script || !script_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid get script arguments");
    }

    *script = invocation_script->script;
    *script_length = invocation_script->script_length;
    return NEOC_SUCCESS;
}

size_t neoc_invocation_script_get_size(const neoc_invocation_script_t *invocation_script) {
    if (!invocation_script) {
        return 0;
    }

    return neoc_invocation_var_int_size(invocation_script->script_length) +
           invocation_script->script_length;
}

bool neoc_invocation_script_is_empty(const neoc_invocation_script_t *invocation_script) {
    return !invocation_script || invocation_script->script_length == 0;
}

neoc_error_t neoc_invocation_script_extract_signatures(const neoc_invocation_script_t *invocation_script,
                                                        uint8_t ***signatures,
                                                        size_t **signature_lengths,
                                                        size_t *signature_count) {
    if (!invocation_script || !signatures || !signature_lengths || !signature_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid extract signatures arguments");
    }

    *signatures = NULL;
    *signature_lengths = NULL;
    *signature_count = 0;

    if (neoc_invocation_script_is_empty(invocation_script)) {
        return NEOC_SUCCESS;
    }

    const uint8_t *buffer = invocation_script->script;
    size_t length = invocation_script->script_length;
    size_t offset = 0;
    size_t count = 0;

    /* First pass: count valid signatures */
    while (offset < length) {
        uint8_t opcode = buffer[offset++];
        size_t payload_len = 0;

        if (opcode == NEOC_OP_PUSHDATA1) {
            if (offset >= length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected end of script while reading PUSHDATA1 length");
            }
            payload_len = buffer[offset++];
        } else if (opcode == NEOC_OP_PUSHDATA2) {
            if (offset + 1 >= length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected end of script while reading PUSHDATA2 length");
            }
            payload_len = (size_t)buffer[offset] | ((size_t)buffer[offset + 1] << 8);
            offset += 2;
        } else if (opcode == NEOC_OP_PUSHDATA4) {
            if (offset + 3 >= length) {
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected end of script while reading PUSHDATA4 length");
            }
            payload_len = (size_t)buffer[offset] |
                          ((size_t)buffer[offset + 1] << 8) |
                          ((size_t)buffer[offset + 2] << 16) |
                          ((size_t)buffer[offset + 3] << 24);
            offset += 4;
        } else {
            break; /* Non signature related opcode encountered */
        }

        if (offset + payload_len > length) {
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invocation script payload exceeds buffer length");
        }

        if (payload_len == NEOC_SIGNATURE_SIZE) {
            count++;
        }

        offset += payload_len;
    }

    if (count == 0) {
        return NEOC_SUCCESS;
    }

    uint8_t **sig_array = neoc_calloc(count, sizeof(uint8_t *));
    size_t *len_array = neoc_calloc(count, sizeof(size_t));
    if (!sig_array || !len_array) {
        neoc_free(sig_array);
        neoc_free(len_array);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signature arrays");
    }

    offset = 0;
    size_t index = 0;
    while (offset < length && index < count) {
        uint8_t opcode = buffer[offset++];
        size_t payload_len = 0;

        if (opcode == NEOC_OP_PUSHDATA1) {
            payload_len = buffer[offset++];
        } else if (opcode == NEOC_OP_PUSHDATA2) {
            payload_len = (size_t)buffer[offset] | ((size_t)buffer[offset + 1] << 8);
            offset += 2;
        } else if (opcode == NEOC_OP_PUSHDATA4) {
            payload_len = (size_t)buffer[offset] |
                          ((size_t)buffer[offset + 1] << 8) |
                          ((size_t)buffer[offset + 2] << 16) |
                          ((size_t)buffer[offset + 3] << 24);
            offset += 4;
        } else {
            break;
        }

        if (offset + payload_len > length) {
            neoc_invocation_script_free_signatures(sig_array, len_array, index);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invocation script payload exceeds buffer length");
        }

        if (payload_len == NEOC_SIGNATURE_SIZE) {
            sig_array[index] = neoc_memdup(buffer + offset, payload_len);
            if (!sig_array[index]) {
                neoc_invocation_script_free_signatures(sig_array, len_array, index);
                return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy signature");
            }
            len_array[index] = payload_len;
            index++;
        }

        offset += payload_len;
    }

    *signatures = sig_array;
    *signature_lengths = len_array;
    *signature_count = index;
    return NEOC_SUCCESS;
}

void neoc_invocation_script_free_signatures(uint8_t **signatures,
                                             size_t *signature_lengths,
                                             size_t signature_count) {
    if (signatures) {
        for (size_t i = 0; i < signature_count; i++) {
            neoc_free(signatures[i]);
        }
        neoc_free(signatures);
    }

    neoc_free(signature_lengths);
}

neoc_error_t neoc_invocation_script_copy(const neoc_invocation_script_t *source,
                                          neoc_invocation_script_t **copy) {
    if (!source || !copy) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid copy invocation script arguments");
    }

    return neoc_invocation_script_allocate(source->script,
                                           source->script_length,
                                           copy);
}

bool neoc_invocation_script_equals(const neoc_invocation_script_t *script1,
                                    const neoc_invocation_script_t *script2) {
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

uint32_t neoc_invocation_script_hash(const neoc_invocation_script_t *invocation_script) {
    if (!invocation_script || invocation_script->script_length == 0) {
        return neoc_hash_murmur32(NULL, 0, 0);
    }
    return neoc_hash_murmur32(invocation_script->script,
                              invocation_script->script_length,
                              0);
}

neoc_error_t neoc_invocation_script_serialize(const neoc_invocation_script_t *invocation_script,
                                               uint8_t **serialized,
                                               size_t *serialized_length) {
    if (!invocation_script || !serialized || !serialized_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid serialize invocation script arguments");
    }

    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(32, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_binary_writer_write_var_bytes(writer,
                                             invocation_script->script,
                                             invocation_script->script_length);
    if (err == NEOC_SUCCESS) {
        err = neoc_binary_writer_to_array(writer, serialized, serialized_length);
    }

    neoc_binary_writer_free(writer);
    return err;
}

neoc_error_t neoc_invocation_script_deserialize(const uint8_t *serialized,
                                                 size_t serialized_length,
                                                 neoc_invocation_script_t **invocation_script) {
    if (!serialized || !invocation_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid deserialize invocation script arguments");
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

    err = neoc_invocation_script_allocate(data, data_len, invocation_script);
    neoc_free(data);
    neoc_binary_reader_free(reader);
    return err;
}

neoc_error_t neoc_invocation_script_clear(neoc_invocation_script_t *invocation_script) {
    if (!invocation_script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid clear invocation script arguments");
    }

    if (invocation_script->script) {
        neoc_free(invocation_script->script);
        invocation_script->script = NULL;
    }
    invocation_script->script_length = 0;
    return NEOC_SUCCESS;
}

void neoc_invocation_script_free(neoc_invocation_script_t *invocation_script) {
    if (!invocation_script) {
        return;
    }

    neoc_invocation_script_clear(invocation_script);
    neoc_free(invocation_script);
}
