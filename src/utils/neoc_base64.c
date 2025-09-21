/**
 * @file neoc_base64.c
 * @brief Base64 encoding/decoding utilities
 */

#include "neoc/utils/neoc_base64.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>

static const char BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const int BASE64_DECODE_TABLE[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

neoc_error_t neoc_base64_encode(const uint8_t *data, size_t data_len, 
                               char *buffer, size_t buffer_size) {
    if (!data || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    size_t encoded_len = ((data_len + 2) / 3) * 4;
    if (buffer_size < encoded_len + 1) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    size_t i, j;
    for (i = 0, j = 0; i < data_len; i += 3, j += 4) {
        uint32_t chunk = (uint32_t)data[i] << 16;
        if (i + 1 < data_len) chunk |= (uint32_t)data[i + 1] << 8;
        if (i + 2 < data_len) chunk |= (uint32_t)data[i + 2];
        
        buffer[j] = BASE64_CHARS[(chunk >> 18) & 0x3F];
        buffer[j + 1] = BASE64_CHARS[(chunk >> 12) & 0x3F];
        buffer[j + 2] = (i + 1 < data_len) ? BASE64_CHARS[(chunk >> 6) & 0x3F] : '=';
        buffer[j + 3] = (i + 2 < data_len) ? BASE64_CHARS[chunk & 0x3F] : '=';
    }
    
    buffer[encoded_len] = '\0';
    return NEOC_SUCCESS;
}

neoc_error_t neoc_base64_decode(const char *encoded, uint8_t *buffer, 
                               size_t buffer_size, size_t *decoded_length) {
    if (!encoded || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    size_t encoded_len = strlen(encoded);
    if (encoded_len % 4 != 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid base64 string length");
    }
    
    // Calculate output length
    size_t output_len = (encoded_len / 4) * 3;
    if (encoded_len > 0 && encoded[encoded_len - 1] == '=') output_len--;
    if (encoded_len > 1 && encoded[encoded_len - 2] == '=') output_len--;
    
    if (buffer_size < output_len) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    size_t i, j;
    for (i = 0, j = 0; i < encoded_len; i += 4, j += 3) {
        int val[4];
        for (int k = 0; k < 4; k++) {
            if (encoded[i + k] == '=') {
                val[k] = 0;
            } else {
                val[k] = BASE64_DECODE_TABLE[(unsigned char)encoded[i + k]];
                if (val[k] == -1) {
                    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid base64 character");
                }
            }
        }
        
        uint32_t chunk = ((uint32_t)val[0] << 18) | ((uint32_t)val[1] << 12) | 
                        ((uint32_t)val[2] << 6) | (uint32_t)val[3];
        
        if (j < output_len) buffer[j] = (chunk >> 16) & 0xFF;
        if (j + 1 < output_len) buffer[j + 1] = (chunk >> 8) & 0xFF;
        if (j + 2 < output_len) buffer[j + 2] = chunk & 0xFF;
    }
    
    if (decoded_length) {
        *decoded_length = output_len;
    }
    
    return NEOC_SUCCESS;
}
