/**
 * @file neoc_base58.c
 * @brief Base58 encoding/decoding utilities
 */

#include "neoc/utils/neoc_base58.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/neoc_hash.h"
#include <string.h>

const char NEOC_BASE58_ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static int neoc_base58_decode_char(char c) {
    const char *position = strchr(NEOC_BASE58_ALPHABET, c);
    if (!position) {
        return -1;
    }
    return (int)(position - NEOC_BASE58_ALPHABET);
}

bool neoc_base58_is_valid_char(char c) {
    return strchr(NEOC_BASE58_ALPHABET, c) != NULL;
}

bool neoc_base58_is_valid_string(const char *str) {
    if (!str) {
        return false;
    }
    
    for (const char *p = str; *p; ++p) {
        if (!neoc_base58_is_valid_char(*p)) {
            return false;
        }
    }
    return *str != '\0';
}

size_t neoc_base58_encode_buffer_size(size_t data_length) {
    /* Worst-case expansion taken from Bitcoin implementation */
    if (data_length == 0) {
        return 2; /* "1" + null terminator */
    }
    size_t estimated = (data_length * 138) / 100 + 2;
    return estimated;
}

size_t neoc_base58_decode_buffer_size(const char *base58_string) {
    if (!base58_string || base58_string[0] == '\0') {
        return 0;
    }
    
    size_t len = strlen(base58_string);
    size_t zeros = 0;
    while (zeros < len && base58_string[zeros] == '1') {
        ++zeros;
    }
    size_t estimated = ((len - zeros) * 733) / 1000 + 1;
    return zeros + estimated;
}

char* neoc_base58_encode_alloc(const uint8_t *data, size_t data_length) {
    if (!data) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
        return NULL;
    }
    
    size_t buffer_size = neoc_base58_encode_buffer_size(data_length);
    char *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate Base58 buffer");
        return NULL;
    }
    
    neoc_error_t err = neoc_base58_encode(data, data_length, buffer, buffer_size);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return NULL;
    }
    
    size_t actual_length = strlen(buffer) + 1;
    if (actual_length < buffer_size) {
        char *resized = neoc_realloc(buffer, actual_length);
        if (resized) {
            buffer = resized;
        }
    }
    
    return buffer;
}

uint8_t* neoc_base58_decode_alloc(const char *base58_string, size_t *decoded_length) {
    if (!base58_string) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
        return NULL;
    }
    
    size_t buffer_size = neoc_base58_decode_buffer_size(base58_string);
    if (buffer_size == 0) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid Base58 input");
        return NULL;
    }
    
    uint8_t *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate Base58 decode buffer");
        return NULL;
    }
    
    size_t actual_length = 0;
    neoc_error_t err = neoc_base58_decode(base58_string, buffer, buffer_size, &actual_length);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return NULL;
    }
    
    if (decoded_length) {
        *decoded_length = actual_length;
    }
    
    if (actual_length > 0 && actual_length < buffer_size) {
        uint8_t *resized = neoc_realloc(buffer, actual_length);
        if (resized) {
            buffer = resized;
        }
    }
    
    return buffer;
}

size_t neoc_base58_check_encode_buffer_size(size_t data_length) {
    return neoc_base58_encode_buffer_size(data_length + NEOC_BASE58_CHECKSUM_LENGTH);
}

size_t neoc_base58_check_decode_buffer_size(const char *base58_string) {
    size_t raw_size = neoc_base58_decode_buffer_size(base58_string);
    if (raw_size < NEOC_BASE58_CHECKSUM_LENGTH) {
        return 0;
    }
    return raw_size - NEOC_BASE58_CHECKSUM_LENGTH;
}

char* neoc_base58_check_encode_alloc(const uint8_t *data, size_t data_length) {
    if (!data) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
        return NULL;
    }
    
    size_t buffer_size = neoc_base58_check_encode_buffer_size(data_length);
    char *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate Base58Check buffer");
        return NULL;
    }
    
    neoc_error_t err = neoc_base58_check_encode(data, data_length, buffer, buffer_size);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return NULL;
    }
    
    size_t actual_length = strlen(buffer) + 1;
    if (actual_length < buffer_size) {
        char *resized = neoc_realloc(buffer, actual_length);
        if (resized) {
            buffer = resized;
        }
    }
    
    return buffer;
}

uint8_t* neoc_base58_check_decode_alloc(const char *base58_string, size_t *decoded_length) {
    if (!base58_string) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
        return NULL;
    }
    
    size_t buffer_size = neoc_base58_decode_buffer_size(base58_string);
    if (buffer_size == 0) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid Base58 input");
        return NULL;
    }
    
    uint8_t *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate Base58Check buffer");
        return NULL;
    }
    
    size_t actual_length = 0;
    neoc_error_t err = neoc_base58_check_decode(base58_string, buffer, buffer_size, &actual_length);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return NULL;
    }
    
    if (actual_length < NEOC_BASE58_CHECKSUM_LENGTH) {
        neoc_free(buffer);
        neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Decoded Base58Check payload too short");
        return NULL;
    }
    
    if (decoded_length) {
        *decoded_length = actual_length;
    }
    
    if (actual_length > 0 && actual_length < buffer_size) {
        uint8_t *resized = neoc_realloc(buffer, actual_length);
        if (resized) {
            buffer = resized;
        }
    }
    
    return buffer;
}

neoc_error_t neoc_base58_encode(const uint8_t *data, size_t data_len,
                               char *buffer, size_t buffer_size) {
    if (!data || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    if (data_len == 0) {
        if (buffer_size < 1) {
            return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
        }
        buffer[0] = '\0';
        return NEOC_SUCCESS;
    }
    
    size_t zeros = 0;
    while (zeros < data_len && data[zeros] == 0) {
        ++zeros;
    }
    
    size_t size = data_len * 138 / 100 + 1;
    uint8_t *buf = neoc_calloc(size, 1);
    if (!buf) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate Base58 buffer");
    }

    size_t length = 0;
    for (size_t i = zeros; i < data_len; ++i) {
        int carry = data[i];
        size_t j = 0;
        for (; j < length || carry != 0; ++j) {
            carry += 256 * buf[j];
            buf[j] = (uint8_t)(carry % 58);
            carry /= 58;
            if (j >= length) {
                length = j + 1;
            }
        }
    }

    size_t result_len = zeros + length;
    if (result_len + 1 > buffer_size) {
        neoc_free(buf);
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }

    size_t out_index = 0;
    for (; out_index < zeros; ++out_index) {
        buffer[out_index] = '1';
    }

    for (size_t k = 0; k < length; ++k) {
        buffer[out_index + k] = NEOC_BASE58_ALPHABET[buf[length - 1 - k]];
    }
    out_index += length;
    buffer[out_index] = '\0';
    neoc_free(buf);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_base58_decode(const char *encoded, uint8_t *buffer, 
                               size_t buffer_size, size_t *decoded_length) {
    if (!encoded || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }

    size_t encoded_len = strlen(encoded);
    size_t zeros = 0;
    while (zeros < encoded_len && encoded[zeros] == '1') {
        ++zeros;
    }

    size_t size = encoded_len * 733 / 1000 + 1;
    uint8_t *buf = neoc_calloc(size, 1);
    if (!buf) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate Base58 buffer");
    }

    size_t length = 0;
    for (size_t i = zeros; i < encoded_len; ++i) {
        int carry = neoc_base58_decode_char(encoded[i]);
        if (carry < 0) {
            neoc_free(buf);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid base58 character");
        }

        size_t j = 0;
        for (; j < length || carry != 0; ++j) {
            carry += 58 * buf[j];
            buf[j] = (uint8_t)(carry % 256);
            carry >>= 8;
            if (j >= length) {
                length = j + 1;
            }
        }
    }

    size_t skip = 0;
    while (skip < length && buf[length - 1 - skip] == 0) {
        ++skip;
    }

    size_t result_len = zeros + (length - skip);
    if (buffer_size < result_len) {
        neoc_free(buf);
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }

    memset(buffer, 0, zeros);
    for (size_t k = 0; k < length - skip; ++k) {
        buffer[zeros + k] = buf[length - 1 - k];
    }

    if (decoded_length) {
        *decoded_length = result_len;
    }

    neoc_free(buf);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_base58_check_encode(const uint8_t *data, size_t data_len, 
                                     char *buffer, size_t buffer_size) {
    if (!data || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Add checksum
    uint8_t hash[32];
    neoc_error_t err = neoc_sha256_double(data, data_len, hash);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Append checksum
    uint8_t *data_with_checksum = neoc_malloc(data_len + 4);
    if (!data_with_checksum) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate data with checksum");
    }
    
    memcpy(data_with_checksum, data, data_len);
    memcpy(data_with_checksum + data_len, hash, 4);
    
    // Encode with checksum
    err = neoc_base58_encode(data_with_checksum, data_len + 4, buffer, buffer_size);
    neoc_free(data_with_checksum);
    
    return err;
}

neoc_error_t neoc_base58_check_decode(const char *encoded, uint8_t *buffer, 
                                     size_t buffer_size, size_t *decoded_length) {
    if (!encoded || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    size_t temp_size = neoc_base58_decode_buffer_size(encoded);
    if (temp_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid Base58 input");
    }

    uint8_t *temp_buffer = neoc_malloc(temp_size);
    if (!temp_buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate temp buffer");
    }
    
    size_t temp_len = 0;
    neoc_error_t err = neoc_base58_decode(encoded, temp_buffer, temp_size, &temp_len);
    if (err != NEOC_SUCCESS) {
        neoc_free(temp_buffer);
        return err;
    }
    
    if (temp_len < 4) {
        neoc_free(temp_buffer);
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Decoded data too short for checksum");
    }
    
    // Verify checksum
    uint8_t hash[32];
    err = neoc_sha256_double(temp_buffer, temp_len - 4, hash);
    if (err != NEOC_SUCCESS) {
        neoc_free(temp_buffer);
        return err;
    }
    
    if (memcmp(temp_buffer + temp_len - 4, hash, 4) != 0) {
        neoc_free(temp_buffer);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Checksum verification failed");
    }
    
    // Check buffer size
    size_t result_len = temp_len - 4;
    if (buffer_size < result_len) {
        neoc_free(temp_buffer);
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    // Copy data without checksum
    memcpy(buffer, temp_buffer, result_len);
    neoc_free(temp_buffer);
    
    if (decoded_length) {
        *decoded_length = result_len;
    }
    
    return NEOC_SUCCESS;
}
