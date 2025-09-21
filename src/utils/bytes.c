/**
 * @file bytes.c
 * @brief Unified byte array utilities implementation
 * 
 * Based on Swift source: utils/Bytes.swift and Array.swift extensions
 * Provides unified interface to all byte array operations in NeoC
 */

#include "../../include/neoc/neoc_error.h"
#include "../../include/neoc/neoc_memory.h"
#include "../../include/neoc/utils/bytes.h"
#include "../../include/neoc/types/neoc_types.h"
#include "../../include/neoc/utils/array.h"
#include "../../include/neoc/utils/neoc_hex.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * @brief Convert neoc_byte_array_t to neoc_bytes_t
 */
neoc_bytes_t* neoc_bytes_from_byte_array(const neoc_byte_array_t *byte_array) {
    if (!byte_array || !byte_array->data) {
        return NULL;
    }
    
    neoc_bytes_t *bytes = neoc_bytes_create(byte_array->length);
    if (!bytes) {
        return NULL;
    }
    
    if (byte_array->length > 0) {
        memcpy(bytes->data, byte_array->data, byte_array->length);
    }
    
    return bytes;
}

/**
 * @brief Convert neoc_bytes_t to neoc_byte_array_t
 */
neoc_error_t neoc_byte_array_from_bytes(const neoc_bytes_t *bytes,
                                         neoc_byte_array_t **result) {
    if (!bytes || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    neoc_error_t error = neoc_byte_array_from_data(bytes->data, bytes->length, result);
    return error;
}

/**
 * @brief Convert hex string to byte array
 */
neoc_error_t neoc_byte_array_from_hex_string(const char *hex_string,
                                              neoc_byte_array_t **result) {
    if (!hex_string || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    // Remove "0x" prefix if present
    const char *clean_hex = hex_string;
    if (strncmp(hex_string, "0x", 2) == 0 || strncmp(hex_string, "0X", 2) == 0) {
        clean_hex = hex_string + 2;
    }
    
    size_t hex_len = strlen(clean_hex);
    if (hex_len == 0) {
        // Empty hex string -> empty byte array
        return neoc_byte_array_create(0, result);
    }
    
    // Hex string must have even length
    if (hex_len % 2 != 0) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Validate hex characters
    for (size_t i = 0; i < hex_len; i++) {
        if (!isxdigit(clean_hex[i])) {
            return NEOC_ERROR_INVALID_HEX;
        }
    }
    
    size_t byte_len = hex_len / 2;
    neoc_error_t error = neoc_byte_array_create(byte_len, result);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Convert hex pairs to bytes
    for (size_t i = 0; i < byte_len; i++) {
        char hex_pair[3] = {clean_hex[i * 2], clean_hex[i * 2 + 1], '\0'};
        unsigned int byte_val;
        
        if (sscanf(hex_pair, "%2x", &byte_val) != 1) {
            neoc_byte_array_free(*result);
            *result = NULL;
            return NEOC_ERROR_INVALID_HEX;
        }
        
        (*result)->data[i] = (uint8_t)byte_val;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Convert byte array to hex string
 */
char* neoc_byte_array_to_hex_string(const neoc_byte_array_t *byte_array,
                                     bool uppercase,
                                     bool with_prefix) {
    if (!byte_array || !byte_array->data) {
        return NULL;
    }
    
    size_t prefix_len = with_prefix ? 2 : 0;
    size_t hex_len = byte_array->length * 2;
    size_t total_len = prefix_len + hex_len + 1; // +1 for null terminator
    
    char *hex_string = neoc_malloc(total_len);
    if (!hex_string) {
        return NULL;
    }
    
    char *pos = hex_string;
    
    // Add prefix if requested
    if (with_prefix) {
        strcpy(pos, "0x");
        pos += 2;
    }
    
    // Convert bytes to hex
    const char *format = uppercase ? "%02X" : "%02x";
    for (size_t i = 0; i < byte_array->length; i++) {
        sprintf(pos, format, byte_array->data[i]);
        pos += 2;
    }
    
    *pos = '\0';
    return hex_string;
}

/**
 * @brief Check if byte array contains specific value
 */
size_t neoc_byte_array_find_byte(const neoc_byte_array_t *byte_array, uint8_t value) {
    if (!byte_array || !byte_array->data) {
        return SIZE_MAX;
    }
    
    for (size_t i = 0; i < byte_array->length; i++) {
        if (byte_array->data[i] == value) {
            return i;
        }
    }
    
    return SIZE_MAX;
}

/**
 * @brief Check if byte array starts with specific prefix
 */
bool neoc_byte_array_starts_with(const neoc_byte_array_t *byte_array,
                                  const neoc_byte_array_t *prefix) {
    if (!byte_array || !prefix || !byte_array->data || !prefix->data) {
        return false;
    }
    
    if (prefix->length == 0) {
        return true;  // Empty prefix matches everything
    }
    
    if (byte_array->length < prefix->length) {
        return false;
    }
    
    return memcmp(byte_array->data, prefix->data, prefix->length) == 0;
}

/**
 * @brief Check if byte array ends with specific suffix
 */
bool neoc_byte_array_ends_with(const neoc_byte_array_t *byte_array,
                                const neoc_byte_array_t *suffix) {
    if (!byte_array || !suffix || !byte_array->data || !suffix->data) {
        return false;
    }
    
    if (suffix->length == 0) {
        return true;  // Empty suffix matches everything
    }
    
    if (byte_array->length < suffix->length) {
        return false;
    }
    
    size_t offset = byte_array->length - suffix->length;
    return memcmp(byte_array->data + offset, suffix->data, suffix->length) == 0;
}

/**
 * @brief Helper function to reverse byte array in place
 */
void neoc_byte_array_reverse_inplace(neoc_byte_array_t *byte_array) {
    if (!byte_array || !byte_array->data || byte_array->length <= 1) {
        return;
    }
    
    size_t left = 0;
    size_t right = byte_array->length - 1;
    
    while (left < right) {
        uint8_t temp = byte_array->data[left];
        byte_array->data[left] = byte_array->data[right];
        byte_array->data[right] = temp;
        left++;
        right--;
    }
}

/**
 * @brief Create reversed copy of byte array
 */
neoc_error_t neoc_byte_array_reverse_copy(const neoc_byte_array_t *byte_array,
                                           neoc_byte_array_t **result) {
    if (!byte_array || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    neoc_error_t error = neoc_byte_array_from_data(byte_array->data, byte_array->length, result);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    neoc_byte_array_reverse_inplace(*result);
    return NEOC_SUCCESS;
}

/**
 * @brief Pad byte array to specified length
 */
neoc_error_t neoc_byte_array_pad(const neoc_byte_array_t *byte_array,
                                  size_t target_length,
                                  uint8_t pad_value,
                                  bool pad_left,
                                  neoc_byte_array_t **result) {
    if (!byte_array || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    if (target_length <= byte_array->length) {
        // No padding needed, just copy
        return neoc_byte_array_from_data(byte_array->data, byte_array->length, result);
    }
    
    neoc_error_t error = neoc_byte_array_create(target_length, result);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    size_t pad_count = target_length - byte_array->length;
    
    if (pad_left) {
        // Pad on the left
        memset((*result)->data, pad_value, pad_count);
        memcpy((*result)->data + pad_count, byte_array->data, byte_array->length);
    } else {
        // Pad on the right
        memcpy((*result)->data, byte_array->data, byte_array->length);
        memset((*result)->data + byte_array->length, pad_value, pad_count);
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Compare two byte arrays
 */
int neoc_byte_array_compare(const neoc_byte_array_t *a, const neoc_byte_array_t *b) {
    if (!a && !b) {
        return 0;
    }
    if (!a) {
        return -1;
    }
    if (!b) {
        return 1;
    }
    
    size_t min_len = (a->length < b->length) ? a->length : b->length;
    
    int cmp = memcmp(a->data, b->data, min_len);
    if (cmp != 0) {
        return cmp;
    }
    
    // If prefixes are equal, compare lengths
    if (a->length < b->length) {
        return -1;
    } else if (a->length > b->length) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Check if two byte arrays are equal
 */
bool neoc_byte_array_equal(const neoc_byte_array_t *a, const neoc_byte_array_t *b) {
    return neoc_byte_array_compare(a, b) == 0;
}

