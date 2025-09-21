/**
 * @file neoc_hash_extra.c
 * @brief Additional hash conversion functions
 */

#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/utils/neoc_hex.h"
#include <string.h>

neoc_error_t neoc_hash160_from_string(const char *str, neoc_hash160_t *hash) {
    if (!str || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Remove 0x prefix if present
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    
    size_t len = strlen(str);
    if (len != NEOC_HASH160_SIZE * 2) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Invalid hash string length");
    }
    
    uint8_t bytes[NEOC_HASH160_SIZE];
    size_t decoded_length;
    neoc_error_t err = neoc_hex_decode(str, bytes, sizeof(bytes), &decoded_length);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    return neoc_hash160_from_bytes(hash, bytes);
}

neoc_error_t neoc_hash256_from_string(const char *str, neoc_hash256_t *hash) {
    if (!str || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Remove 0x prefix if present
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    
    size_t len = strlen(str);
    if (len != NEOC_HASH256_SIZE * 2) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Invalid hash string length");
    }
    
    uint8_t bytes[NEOC_HASH256_SIZE];
    size_t decoded_length;
    neoc_error_t err = neoc_hex_decode(str, bytes, sizeof(bytes), &decoded_length);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    return neoc_hash256_from_bytes(hash, bytes);
}

neoc_error_t neoc_hash256_to_string(const neoc_hash256_t *hash, char *buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (buffer_size < (NEOC_HASH256_SIZE * 2 + 1)) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    uint8_t bytes[NEOC_HASH256_SIZE];
    neoc_error_t err = neoc_hash256_to_bytes(hash, bytes, sizeof(bytes));
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    return neoc_hex_encode(bytes, sizeof(bytes), buffer, buffer_size, false, false);
}
