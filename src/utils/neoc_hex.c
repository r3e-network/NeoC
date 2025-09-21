/**
 * @file neoc_hex.c
 * @brief Hexadecimal encoding/decoding utilities
 */

#include "neoc/utils/neoc_hex.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

neoc_error_t neoc_hex_encode(const uint8_t *data, size_t data_length, 
                            char *buffer, size_t buffer_size,
                            bool uppercase, bool include_prefix) {
    if (!data || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    size_t prefix_len = include_prefix ? 2 : 0;
    size_t required_size = prefix_len + data_length * 2 + 1;
    
    if (buffer_size < required_size) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    size_t offset = 0;
    if (include_prefix) {
        buffer[0] = '0';
        buffer[1] = 'x';
        offset = 2;
    }
    
    const char* format = uppercase ? "%02X" : "%02x";
    for (size_t i = 0; i < data_length; i++) {
        sprintf(buffer + offset + (i * 2), format, data[i]);
    }
    buffer[prefix_len + data_length * 2] = '\0';
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hex_decode(const char *hex_string, uint8_t *buffer, 
                            size_t buffer_size, size_t *decoded_length) {
    if (!hex_string || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Skip "0x" prefix if present
    const char* hex = hex_string;
    if (strlen(hex_string) >= 2 && hex_string[0] == '0' && 
        (hex_string[1] == 'x' || hex_string[1] == 'X')) {
        hex = hex_string + 2;
    }
    
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Hex string must have even length");
    }
    
    size_t output_len = hex_len / 2;
    if (buffer_size < output_len) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    for (size_t i = 0; i < output_len; i++) {
        char high = hex[i * 2];
        char low = hex[i * 2 + 1];
        
        int high_val = (high >= '0' && high <= '9') ? high - '0' :
                      (high >= 'a' && high <= 'f') ? high - 'a' + 10 :
                      (high >= 'A' && high <= 'F') ? high - 'A' + 10 : -1;
        
        int low_val = (low >= '0' && low <= '9') ? low - '0' :
                     (low >= 'a' && low <= 'f') ? low - 'a' + 10 :
                     (low >= 'A' && low <= 'F') ? low - 'A' + 10 : -1;
        
        if (high_val == -1 || low_val == -1) {
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid hex character");
        }
        
        buffer[i] = (uint8_t)((high_val << 4) | low_val);
    }
    
    if (decoded_length) {
        *decoded_length = output_len;
    }
    
    return NEOC_SUCCESS;
}

bool neoc_hex_is_valid_char(char c) {
    return (c >= '0' && c <= '9') || 
           (c >= 'a' && c <= 'f') || 
           (c >= 'A' && c <= 'F');
}

bool neoc_hex_is_valid_string(const char* str, bool allow_0x_prefix) {
    if (!str) {
        return false;
    }
    
    size_t len = strlen(str);
    if (len == 0) {
        return false;
    }
    
    // Handle 0x prefix
    if (allow_0x_prefix && len >= 2 && str[0] == '0' && 
        (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
        len -= 2;
    }
    
    // Must have even length for valid hex
    if (len % 2 != 0) {
        return false;
    }
    
    // Check all characters are valid hex
    for (size_t i = 0; i < len; i++) {
        if (!neoc_hex_is_valid_char(str[i])) {
            return false;
        }
    }
    
    return true;
}

int neoc_hex_char_to_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

char neoc_hex_value_to_char(int value, bool uppercase) {
    if (value < 0 || value > 15) {
        return '\0';
    }
    
    if (value < 10) {
        return '0' + value;
    } else if (uppercase) {
        return 'A' + value - 10;
    } else {
        return 'a' + value - 10;
    }
}

size_t neoc_hex_encode_buffer_size(size_t data_length, bool include_prefix) {
    return (include_prefix ? 2 : 0) + data_length * 2 + 1;
}

size_t neoc_hex_decode_buffer_size(const char* hex_string) {
    if (!hex_string) {
        return 0;
    }
    
    size_t len = strlen(hex_string);
    
    // Handle 0x prefix
    if (len >= 2 && hex_string[0] == '0' && 
        (hex_string[1] == 'x' || hex_string[1] == 'X')) {
        len -= 2;
    }
    
    return len / 2;
}

neoc_error_t neoc_hex_encode_reverse(const uint8_t *data, size_t data_len, char **hex) {
    if (!data || !hex) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Allocate buffer for reversed hex string
    size_t buffer_size = data_len * 2 + 1;
    *hex = neoc_malloc(buffer_size);
    if (!*hex) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate hex string");
    }
    
    // Create reversed data
    uint8_t *reversed_data = neoc_malloc(data_len);
    if (!reversed_data) {
        neoc_free(*hex);
        *hex = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate reversed data");
    }
    
    for (size_t i = 0; i < data_len; i++) {
        reversed_data[i] = data[data_len - 1 - i];
    }
    
    // Encode reversed data
    neoc_error_t err = neoc_hex_encode(reversed_data, data_len, *hex, buffer_size, false, false);
    neoc_free(reversed_data);
    
    if (err != NEOC_SUCCESS) {
        neoc_free(*hex);
        *hex = NULL;
    }
    
    return err;
}

neoc_error_t neoc_hex_decode_reverse(const char *hex, uint8_t **data, size_t *data_len) {
    if (!hex || !data || !data_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Calculate length
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Hex string must have even length");
    }
    
    *data_len = hex_len / 2;
    *data = neoc_malloc(*data_len);
    if (!*data) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate data");
    }
    
    // Decode using buffer-based function
    size_t decoded_len = 0;
    neoc_error_t err = neoc_hex_decode(hex, *data, *data_len, &decoded_len);
    if (err != NEOC_SUCCESS) {
        neoc_free(*data);
        *data = NULL;
        return err;
    }
    
    // Then reverse the bytes
    for (size_t i = 0; i < *data_len / 2; i++) {
        uint8_t temp = (*data)[i];
        (*data)[i] = (*data)[*data_len - 1 - i];
        (*data)[*data_len - 1 - i] = temp;
    }
    
    return NEOC_SUCCESS;
}
