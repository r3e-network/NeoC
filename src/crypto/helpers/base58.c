/**
 * @file base58.c
 * @brief Base58 encoding/decoding implementation
 */

#include "neoc/crypto/helpers/base58.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/sha256.h"
#include <stdlib.h>
#include <string.h>

static const char BASE58_ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static int base58_decode_char(char c) {
    const char *p = strchr(BASE58_ALPHABET, c);
    if (!p) return -1;
    return p - BASE58_ALPHABET;
}

neoc_error_t neoc_base58_encode(const uint8_t *data, size_t len, char **encoded) {
    if (!data || !encoded) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Count leading zeros
    size_t zeros = 0;
    while (zeros < len && data[zeros] == 0) {
        zeros++;
    }
    
    // Allocate working buffer
    size_t size = len * 138 / 100 + 1;
    uint8_t *buf = neoc_calloc(size, 1);
    if (!buf) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    size_t length = 0;
    
    // Process bytes
    for (size_t i = zeros; i < len; i++) {
        int carry = data[i];
        for (size_t j = 0; j < length || carry; j++) {
            carry += 256 * buf[j];
            buf[j] = carry % 58;
            carry /= 58;
            if (j >= length) length = j + 1;
        }
    }
    
    // Skip leading zeros in base58 result
    size_t j = 0;
    while (j < length && buf[j] == 0) {
        j++;
    }
    
    // Calculate result length
    size_t result_len = zeros + length - j;
    
    // Allocate output buffer
    *encoded = neoc_malloc(result_len + 1);
    if (!*encoded) {
        neoc_free(buf);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate output buffer");
    }
    
    // Translate to base58 alphabet
    for (size_t i = 0; i < zeros; i++) {
        (*encoded)[i] = '1';
    }
    
    for (size_t i = 0; i < length - j; i++) {
        (*encoded)[zeros + i] = BASE58_ALPHABET[buf[length - 1 - i]];
    }
    
    (*encoded)[result_len] = '\0';
    
    neoc_free(buf);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_base58_decode(const char *encoded, uint8_t **data, size_t *len) {
    if (!encoded || !data || !len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    size_t encoded_len = strlen(encoded);
    
    // Count leading ones (zeros in output)
    size_t zeros = 0;
    while (zeros < encoded_len && encoded[zeros] == '1') {
        zeros++;
    }
    
    // Allocate working buffer
    size_t size = encoded_len * 733 / 1000 + 1;
    uint8_t *buf = neoc_calloc(size, 1);
    if (!buf) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    size_t length = 0;
    
    // Process characters
    for (size_t i = zeros; i < encoded_len; i++) {
        int carry = base58_decode_char(encoded[i]);
        if (carry < 0) {
            neoc_free(buf);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid base58 character");
        }
        
        for (size_t j = 0; j < length || carry; j++) {
            carry += 58 * buf[j];
            buf[j] = carry % 256;
            carry /= 256;
            if (j >= length) length = j + 1;
        }
    }
    
    // Skip leading zeros
    size_t j = 0;
    while (j < length && buf[length - 1 - j] == 0) {
        j++;
    }
    
    // Calculate result length
    size_t result_len = zeros + length - j;
    
    // Allocate output buffer
    *data = neoc_malloc(result_len);
    if (!*data) {
        neoc_free(buf);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate output buffer");
    }
    
    memset(*data, 0, zeros);
    for (size_t i = 0; i < length - j; i++) {
        (*data)[zeros + i] = buf[length - 1 - i];
    }
    
    *len = result_len;
    
    neoc_free(buf);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_base58_encode_check(const uint8_t *data, size_t len, char **encoded) {
    if (!data || !encoded) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Calculate checksum
    uint8_t hash1[32], hash2[32];
    neoc_error_t hash_err = neoc_sha256(data, len, hash1);
    if (hash_err != NEOC_SUCCESS) {
        return hash_err;
    }
    hash_err = neoc_sha256(hash1, 32, hash2);
    if (hash_err != NEOC_SUCCESS) {
        return hash_err;
    }
    
    // Append checksum
    uint8_t *data_with_checksum = neoc_malloc(len + 4);
    if (!data_with_checksum) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    memcpy(data_with_checksum, data, len);
    memcpy(data_with_checksum + len, hash2, 4);
    
    // Base58 encode
    neoc_error_t err = neoc_base58_encode(data_with_checksum, len + 4, encoded);
    
    neoc_free(data_with_checksum);
    return err;
}

neoc_error_t neoc_base58_decode_check(const char *encoded, uint8_t **data, size_t *len) {
    if (!encoded || !data || !len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Base58 decode
    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base58_decode(encoded, &decoded, &decoded_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    if (decoded_len < 4) {
        neoc_free(decoded);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid base58check format");
    }
    
    // Verify checksum
    size_t payload_len = decoded_len - 4;
    uint8_t hash1[32], hash2[32];
    neoc_error_t hash_err = neoc_sha256(decoded, payload_len, hash1);
    if (hash_err != NEOC_SUCCESS) {
        neoc_free(decoded);
        return hash_err;
    }
    hash_err = neoc_sha256(hash1, 32, hash2);
    if (hash_err != NEOC_SUCCESS) {
        neoc_free(decoded);
        return hash_err;
    }
    
    if (memcmp(hash2, decoded + payload_len, 4) != 0) {
        neoc_free(decoded);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid checksum");
    }
    
    // Allocate output buffer
    *data = neoc_malloc(payload_len);
    if (!*data) {
        neoc_free(decoded);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate output buffer");
    }
    
    memcpy(*data, decoded, payload_len);
    *len = payload_len;
    
    neoc_free(decoded);
    return NEOC_SUCCESS;
}

bool neoc_base58_is_valid(const char *str) {
    if (!str) {
        return false;
    }
    
    size_t len = strlen(str);
    if (len == 0) {
        return false;
    }
    
    // Check each character is valid base58
    for (size_t i = 0; i < len; i++) {
        if (base58_decode_char(str[i]) < 0) {
            return false;
        }
    }
    
    return true;
}
