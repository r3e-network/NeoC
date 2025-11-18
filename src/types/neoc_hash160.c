/**
 * @file neoc_hash160.c
 * @brief Implementation of Hash160 type for NeoC SDK
 */

#include "neoc/types/neoc_hash160.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/script/script_helper.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/neoc_memory.h"
#include <string.h>

/* Constant zero Hash160 */
const neoc_hash160_t NEOC_HASH160_ZERO_VALUE = {{0}};

#define NEOC_HASH160_HEX_CHARS (NEOC_HASH160_SIZE * 2)

static const char* skip_hex_prefix(const char* hex) {
    if (!hex) {
        return NULL;
    }
    if (strlen(hex) >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        return hex + 2;
    }
    return hex;
}

static neoc_error_t decode_exact_hex(const char* hex_string,
                                     size_t expected_bytes,
                                     uint8_t* output) {
    if (!hex_string || !output) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    const char* digits = skip_hex_prefix(hex_string);
    if (!digits) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t digits_len = strlen(digits);
    if (digits_len == 0) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    for (size_t i = 0; i < digits_len; ++i) {
        if (!neoc_hex_is_valid_char(digits[i])) {
            return NEOC_ERROR_INVALID_HEX;
        }
    }
    
    size_t required_chars = expected_bytes * 2;
    bool odd_length = (digits_len % 2) != 0;
    size_t padded_len = digits_len + (odd_length ? 1 : 0);
    
    if (padded_len > required_chars) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    if (padded_len < required_chars) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    char normalized[NEOC_HASH160_HEX_CHARS + 1];
    if (odd_length) {
        normalized[0] = '0';
        memcpy(normalized + 1, digits, digits_len + 1);
    } else {
        memcpy(normalized, digits, digits_len + 1);
    }
    
    size_t decoded_length = 0;
    neoc_error_t result = neoc_hex_decode(normalized, output, expected_bytes, &decoded_length);
    if (result == NEOC_ERROR_INVALID_FORMAT) {
        return NEOC_ERROR_INVALID_HEX;
    }
    if (result != NEOC_SUCCESS) {
        return result;
    }
    if (decoded_length != expected_bytes) {
        return (decoded_length < expected_bytes) ? NEOC_ERROR_INVALID_ARGUMENT
                                                 : NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_init_zero(neoc_hash160_t* hash) {
    if (!hash) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memset(hash->data, 0, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_bytes(neoc_hash160_t* hash, const uint8_t data[NEOC_HASH160_SIZE]) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memcpy(hash->data, data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_data(neoc_hash160_t* hash, const uint8_t* data, size_t length) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length != NEOC_HASH160_SIZE) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    memcpy(hash->data, data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_hex(neoc_hash160_t* hash, const char* hex_string) {
    if (!hash || !hex_string) {
        return NEOC_ERROR_NULL_POINTER;
    }
    return decode_exact_hex(hex_string, NEOC_HASH160_SIZE, hash->data);
}

neoc_error_t neoc_hash160_from_address(neoc_hash160_t* hash, const char* address) {
    if (!hash || !address) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Decode Base58Check address */
    size_t decoded_length;
    uint8_t* decoded = neoc_base58_check_decode_alloc(address, &decoded_length);
    if (!decoded) {
        return NEOC_ERROR_INVALID_BASE58;
    }
    
    /* Address should be version byte + 20 byte hash */
    if (decoded_length != 1 + NEOC_HASH160_SIZE) {
        neoc_free(decoded);
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    /* Check version byte */
    if (decoded[0] != NEOC_ADDRESS_VERSION) {
        neoc_free(decoded);
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    /* Extract hash160 (skip version byte) and store in big-endian order */
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        hash->data[i] = decoded[1 + NEOC_HASH160_SIZE - 1 - i];
    }
    
    neoc_secure_free(decoded, decoded_length);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_script(neoc_hash160_t* hash, const uint8_t* script, size_t script_length) {
    if (!hash || !script) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Calculate hash160 of script */
    uint8_t hash_result[NEOC_RIPEMD160_DIGEST_LENGTH];
    neoc_error_t result = neoc_hash160(script, script_length, hash_result);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    /* Neo stores script hash in little-endian format */
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        hash->data[i] = hash_result[NEOC_HASH160_SIZE - 1 - i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_script_hex(neoc_hash160_t* hash, const char* script_hex) {
    if (!hash || !script_hex) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t script_length;
    uint8_t* script = neoc_hex_decode_alloc(script_hex, &script_length);
    if (!script) {
        return NEOC_ERROR_INVALID_HEX;
    }
    
    neoc_error_t result = neoc_hash160_from_script(hash, script, script_length);
    neoc_free(script);
    
    return result;
}

neoc_error_t neoc_hash160_from_public_key(neoc_hash160_t* hash, const uint8_t public_key_data[NEOC_PUBLIC_KEY_SIZE_COMPRESSED]) {
    if (!hash || !public_key_data) {
        return NEOC_ERROR_NULL_POINTER;
    }

    uint8_t *script = NULL;
    size_t script_len = 0;
    neoc_error_t err = neoc_script_create_single_sig_verification(public_key_data,
                                                                  NEOC_PUBLIC_KEY_SIZE_COMPRESSED,
                                                                  &script,
                                                                  &script_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_hash160_from_script(hash, script, script_len);
    neoc_free(script);
    return err;
}

neoc_error_t neoc_hash160_from_public_keys(neoc_hash160_t* hash, 
                                           const uint8_t public_keys[][NEOC_PUBLIC_KEY_SIZE_COMPRESSED],
                                           size_t num_keys,
                                           size_t signing_threshold) {
    if (!hash || !public_keys) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (num_keys == 0 || signing_threshold == 0 || signing_threshold > num_keys) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (num_keys > NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }

    // Reuse script helper to ensure identical script layout to multisig accounts
    const uint8_t *key_ptrs[NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT] = {0};
    size_t key_lens[NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT] = {0};
    for (size_t i = 0; i < num_keys; ++i) {
        key_ptrs[i] = public_keys[i];
        key_lens[i] = NEOC_PUBLIC_KEY_SIZE_COMPRESSED;
    }

    uint8_t *script = NULL;
    size_t script_len = 0;
    neoc_error_t err = neoc_script_create_multisig_verification((uint8_t)signing_threshold,
                                                                key_ptrs,
                                                                key_lens,
                                                                num_keys,
                                                                &script,
                                                                &script_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_hash160_from_script(hash, script, script_len);
    neoc_free(script);
    return err;
}

neoc_error_t neoc_hash160_copy(neoc_hash160_t* dest, const neoc_hash160_t* src) {
    if (!dest || !src) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memcpy(dest->data, src->data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_to_bytes(const neoc_hash160_t* hash, uint8_t* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < NEOC_HASH160_SIZE) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    memcpy(buffer, hash->data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_to_little_endian_bytes(const neoc_hash160_t* hash, uint8_t* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < NEOC_HASH160_SIZE) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    /* Reverse byte order */
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        buffer[i] = hash->data[NEOC_HASH160_SIZE - 1 - i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_to_hex(const neoc_hash160_t* hash, char* buffer, size_t buffer_size, bool uppercase) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t required_size = neoc_hex_encode_buffer_size(NEOC_HASH160_SIZE, false);
    if (buffer_size < required_size) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    return neoc_hex_encode(hash->data, NEOC_HASH160_SIZE, buffer, buffer_size, uppercase, false);
}

neoc_error_t neoc_hash160_to_address(const neoc_hash160_t* hash, char* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Create versioned hash (version byte + little-endian script hash) */
    uint8_t versioned[1 + NEOC_HASH160_SIZE];
    versioned[0] = NEOC_ADDRESS_VERSION;
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        versioned[1 + i] = hash->data[NEOC_HASH160_SIZE - 1 - i];
    }
    
    /* Encode as Base58Check */
    char* address = neoc_base58_check_encode_alloc(versioned, sizeof(versioned));
    if (!address) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    size_t address_len = strlen(address);
    if (buffer_size <= address_len) {
        neoc_free(address);
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    strcpy(buffer, address);
    neoc_free(address);
    
    return NEOC_SUCCESS;
}

int neoc_hash160_compare(const neoc_hash160_t* a, const neoc_hash160_t* b) {
    if (!a || !b) {
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    
    return memcmp(a->data, b->data, NEOC_HASH160_SIZE);
}

bool neoc_hash160_equal(const neoc_hash160_t* a, const neoc_hash160_t* b) {
    return neoc_hash160_compare(a, b) == 0;
}

bool neoc_hash160_is_zero(const neoc_hash160_t* hash) {
    if (!hash) {
        return false;
    }
    
    return neoc_hash160_equal(hash, &NEOC_HASH160_ZERO_VALUE);
}

neoc_error_t neoc_hash160_serialize(const neoc_hash160_t* hash, neoc_binary_writer_t* writer) {
    if (!hash || !writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Write hash bytes directly */
    return neoc_binary_writer_write_bytes(writer, hash->data, NEOC_HASH160_SIZE);
}

neoc_error_t neoc_hash160_deserialize(neoc_hash160_t* hash, neoc_binary_reader_t* reader) {
    if (!hash || !reader) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Read hash bytes directly */
    return neoc_binary_reader_read_bytes(reader, hash->data, NEOC_HASH160_SIZE);
}

size_t neoc_hash160_serialized_size(void) {
    return NEOC_HASH160_SIZE;
}

neoc_error_t neoc_hash160_from_string(const char *str, neoc_hash160_t *hash) {
    if (!str || !hash) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    const char *hex_str = str;
    
    /* Skip '0x' prefix if present */
    if (strlen(str) >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        hex_str = str + 2;
    }
    
    /* Check if length is correct for Hash160 (40 hex characters) */
    size_t hex_len = strlen(hex_str);
    if (hex_len != 40) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return neoc_hash160_from_hex(hash, hex_str);
}

neoc_error_t neoc_hash160_to_string(const neoc_hash160_t* hash, char* buffer, size_t buffer_size) {
    return neoc_hash160_to_hex(hash, buffer, buffer_size, false);
}
