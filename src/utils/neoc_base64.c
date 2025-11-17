/**
 * @file neoc_base64.c
 * @brief Base64 encoding/decoding utilities
 */

#include "neoc/utils/neoc_base64.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const char NEOC_BASE64_ALPHABET[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

size_t neoc_base64_encode_buffer_size(size_t data_length) {
    return ((data_length + 2) / 3) * 4 + 1;
}

size_t neoc_base64_decode_buffer_size(const char *base64_string) {
    if (!base64_string) {
        return 0;
    }

    size_t len = strlen(base64_string);
    if (len == 0) {
        return 0;
    }

    size_t padding = 0;
    if (len >= 1 && base64_string[len - 1] == '=') padding++;
    if (len >= 2 && base64_string[len - 2] == '=') padding++;

    return (len / 4) * 3 - padding;
}

bool neoc_base64_is_valid_char(char c) {
    return (isalnum((unsigned char)c) || c == '+' || c == '/' || c == '=' || c == '-' || c == '_');
}

bool neoc_base64_is_valid_string(const char *str) {
    if (!str) {
        return false;
    }

    size_t len = strlen(str);
    if (len == 0 || len % 4 != 0) {
        return false;
    }

    for (size_t i = 0; i < len; ++i) {
        if (!neoc_base64_is_valid_char(str[i])) {
            return false;
        }
    }

    return true;
}

char* neoc_base64_encode_alloc(const uint8_t *data, size_t data_length) {
    if (!data && data_length > 0) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neoc_base64_encode_alloc: invalid data");
        return NULL;
    }

    size_t buffer_size = neoc_base64_encode_buffer_size(data_length);
    char *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "neoc_base64_encode_alloc: allocation failed");
        return NULL;
    }

    neoc_error_t err = neoc_base64_encode(data, data_length, buffer, buffer_size);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return NULL;
    }

    return buffer;
}

uint8_t* neoc_base64_decode_alloc(const char *base64_string, size_t *decoded_length) {
    if (!base64_string) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neoc_base64_decode_alloc: base64 string is NULL");
        return NULL;
    }

    size_t buffer_size = neoc_base64_decode_buffer_size(base64_string);
    if (buffer_size == 0) {
        neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "neoc_base64_decode_alloc: invalid base64 string");
        return NULL;
    }

    uint8_t *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "neoc_base64_decode_alloc: allocation failed");
        return NULL;
    }

    size_t actual_length = 0;
    neoc_error_t err = neoc_base64_decode(base64_string, buffer, buffer_size, &actual_length);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return NULL;
    }

    if (decoded_length) {
        *decoded_length = actual_length;
    }

    return buffer;
}

neoc_error_t neoc_base64_url_encode(const uint8_t *data, size_t data_length,
                                    char *buffer, size_t buffer_size) {
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "base64_url_encode: invalid buffer");
    }

    neoc_error_t err = neoc_base64_encode(data, data_length, buffer, buffer_size);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    for (size_t i = 0; buffer[i]; ++i) {
        if (buffer[i] == '+') buffer[i] = '-';
        else if (buffer[i] == '/') buffer[i] = '_';
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_base64_url_decode(const char *base64_string, uint8_t *buffer,
                                    size_t buffer_size, size_t *decoded_length) {
    if (!base64_string) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "base64_url_decode: invalid string");
    }

    size_t len = strlen(base64_string);
    char *normalized = neoc_malloc(len + 1);
    if (!normalized) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "base64_url_decode: allocation failed");
    }

    for (size_t i = 0; i < len; ++i) {
        if (base64_string[i] == '-') normalized[i] = '+';
        else if (base64_string[i] == '_') normalized[i] = '/';
        else normalized[i] = base64_string[i];
    }
    normalized[len] = '\0';

    neoc_error_t err = neoc_base64_decode(normalized, buffer, buffer_size, decoded_length);
    neoc_free(normalized);
    return err;
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
