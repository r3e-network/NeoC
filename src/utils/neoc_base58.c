/**
 * @file neoc_base58.c
 * @brief Base58 encoding/decoding utilities
 */

#include "neoc/utils/neoc_base58.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/neoc_hash.h"
#include <string.h>
#include <stdio.h>

static const char* BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

neoc_error_t neoc_base58_encode(const uint8_t *data, size_t data_len, 
                               char *buffer, size_t buffer_size) {
    if (!data || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Count leading zeros
    size_t zeros = 0;
    for (size_t i = 0; i < data_len && data[i] == 0; i++) {
        zeros++;
    }
    
    // Check buffer size
    size_t min_size = (data_len * 138 / 100) + zeros + 1;
    if (buffer_size < min_size) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    // Convert to base58
    uint8_t *temp_buffer = neoc_calloc(data_len * 2, 1);
    if (!temp_buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    memcpy(temp_buffer, data, data_len);
    
    // Add '1' for each leading zero byte
    size_t idx = 0;
    for (size_t i = 0; i < zeros; i++) {
        buffer[idx++] = '1';
    }
    
    // Convert non-zero data
    if (data_len > zeros) {
        // Algorithm for base58 encoding
        uint8_t *work_buffer = neoc_calloc(data_len, 1);
        if (!work_buffer) {
            neoc_free(temp_buffer);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate work buffer");
        }
        
        memcpy(work_buffer, data + zeros, data_len - zeros);
        size_t work_len = data_len - zeros;
        
        // Convert to base58
        while (work_len > 0) {
            int carry = 0;
            size_t new_len = 0;
            
            for (size_t i = 0; i < work_len; i++) {
                carry = carry * 256 + work_buffer[i];
                if (carry >= 58 || new_len > 0) {
                    work_buffer[new_len++] = carry / 58;
                }
                carry %= 58;
            }
            
            buffer[idx++] = BASE58_ALPHABET[carry];
            work_len = new_len;
        }
        
        neoc_free(work_buffer);
    }
    
    buffer[idx] = '\0';
    
    // Reverse the result (except leading 1s)
    size_t start = zeros;
    size_t end = idx - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
    
    neoc_free(temp_buffer);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_base58_decode(const char *encoded, uint8_t *buffer, 
                               size_t buffer_size, size_t *decoded_length) {
    if (!encoded || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    size_t encoded_len = strlen(encoded);
    
    // Count leading '1's
    size_t zeros = 0;
    for (size_t i = 0; i < encoded_len && encoded[i] == '1'; i++) {
        zeros++;
    }
    
    // Check buffer size
    size_t estimated_size = encoded_len * 733 / 1000 + 1;
    if (buffer_size < estimated_size) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    // Clear buffer
    memset(buffer, 0, buffer_size);
    
    // Decode base58
    for (size_t i = zeros; i < encoded_len; i++) {
        const char *p = strchr(BASE58_ALPHABET, encoded[i]);
        if (!p) {
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid base58 character");
        }
        
        int carry = p - BASE58_ALPHABET;
        for (size_t j = buffer_size - 1; j < buffer_size; j--) {
            carry += buffer[j] * 58;
            buffer[j] = carry & 0xFF;
            carry >>= 8;
        }
    }
    
    // Find actual data start
    size_t data_start = 0;
    while (data_start < buffer_size && buffer[data_start] == 0) {
        data_start++;
    }
    
    // Calculate actual length
    size_t actual_len = zeros + (buffer_size - data_start);
    
    // Move data to beginning if needed
    if (data_start > zeros) {
        memmove(buffer + zeros, buffer + data_start, buffer_size - data_start);
    }
    
    // Set leading zeros
    memset(buffer, 0, zeros);
    
    if (decoded_length) {
        *decoded_length = actual_len;
    }
    
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
    
    // Decode base58 to working buffer for validation
    size_t temp_size = strlen(encoded) * 733 / 1000 + 10;  // Extra space for safety
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
