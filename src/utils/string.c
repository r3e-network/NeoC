/**
 * @file string.c
 * @brief String utility functions implementation
 * 
 * Based on Swift source: utils/String.swift
 * Implements string manipulation, encoding/decoding, and validation utilities
 */

#include "../../include/neoc/neoc_error.h"
#include "../../include/neoc/neoc_memory.h"
#include "../../include/neoc/utils/string.h"
#include "../../include/neoc/utils/neoc_base58.h"
#include "../../include/neoc/utils/neoc_base64.h"
#include "../../include/neoc/utils/neoc_hex.h"
#include "../../include/neoc/crypto/neoc_hash.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

// Additional error codes
#define NEOC_ERROR_INVALID_ADDRESS NEOC_ERROR_INVALID_PARAM

/**
 * @brief Convert hex string to bytes
 * 
 * Equivalent to Swift String.bytesFromHex
 */
neoc_error_t neoc_string_hex_to_bytes(const char *hex_string,
                                       uint8_t **bytes,
                                       size_t *bytes_len) {
    if (!hex_string || !bytes || !bytes_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *bytes = NULL;
    *bytes_len = 0;
    
    // Clean hex prefix first
    char *cleaned_hex = NULL;
    neoc_error_t error = neoc_string_clean_hex_prefix(hex_string, &cleaned_hex);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Use neoc_hex_decode_alloc for conversion
    *bytes = neoc_hex_decode_alloc(cleaned_hex, bytes_len);
    neoc_free(cleaned_hex);
    
    return *bytes ? NEOC_SUCCESS : NEOC_ERROR_INVALID_HEX;
}

/**
 * @brief Clean hex string by removing "0x" prefix if present
 * 
 * Equivalent to Swift String.cleanedHexPrefix
 */
neoc_error_t neoc_string_clean_hex_prefix(const char *hex_string,
                                           char **cleaned_hex) {
    if (!hex_string || !cleaned_hex) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *cleaned_hex = NULL;
    
    const char *start = hex_string;
    
    // Check for "0x" or "0X" prefix
    if (neoc_string_starts_with(hex_string, "0x") || 
        neoc_string_starts_with(hex_string, "0X")) {
        start = hex_string + 2;
    }
    
    // Duplicate the cleaned string
    size_t len = strlen(start);
    *cleaned_hex = neoc_malloc(len + 1);
    if (!*cleaned_hex) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    strcpy(*cleaned_hex, start);
    return NEOC_SUCCESS;
}

/**
 * @brief Decode base64 string to bytes
 * 
 * Equivalent to Swift String.base64Decoded
 */
neoc_error_t neoc_string_base64_decode(const char *base64_string,
                                        uint8_t **bytes,
                                        size_t *bytes_len) {
    if (!base64_string || !bytes || !bytes_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *bytes = NULL;
    *bytes_len = 0;
    
    // Use neoc_base64_decode_alloc for decoding
    *bytes = neoc_base64_decode_alloc(base64_string, bytes_len);
    return *bytes ? NEOC_SUCCESS : NEOC_ERROR_INVALID_FORMAT;
}

/**
 * @brief Encode hex string as base64
 * 
 * Equivalent to Swift String.base64Encoded
 */
neoc_error_t neoc_string_hex_to_base64(const char *hex_string,
                                        char **base64_string) {
    if (!hex_string || !base64_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *base64_string = NULL;
    
    // Convert hex to bytes first
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    neoc_error_t error = neoc_string_hex_to_bytes(hex_string, &bytes, &bytes_len);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Encode bytes as base64
    *base64_string = neoc_base64_encode_alloc(bytes, bytes_len);
    
    neoc_free(bytes);
    return *base64_string ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

/**
 * @brief Decode base58 string to bytes
 * 
 * Equivalent to Swift String.base58Decoded
 */
neoc_error_t neoc_string_base58_decode(const char *base58_string,
                                        uint8_t **bytes,
                                        size_t *bytes_len) {
    if (!base58_string || !bytes || !bytes_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *bytes = NULL;
    *bytes_len = 0;
    
    // Use neoc_base58_decode_alloc for decoding
    *bytes = neoc_base58_decode_alloc(base58_string, bytes_len);
    return *bytes ? NEOC_SUCCESS : NEOC_ERROR_INVALID_FORMAT;
}

/**
 * @brief Decode base58 string with checksum validation
 * 
 * Equivalent to Swift String.base58CheckDecoded
 */
neoc_error_t neoc_string_base58_check_decode(const char *base58_string,
                                              uint8_t **bytes,
                                              size_t *bytes_len) {
    if (!base58_string || !bytes || !bytes_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *bytes = NULL;
    *bytes_len = 0;
    
    // Use neoc_base58_check_decode_alloc for checksum decoding
    *bytes = neoc_base58_check_decode_alloc(base58_string, bytes_len);
    return *bytes ? NEOC_SUCCESS : NEOC_ERROR_INVALID_FORMAT;
}

/**
 * @brief Encode string as base58
 * 
 * Equivalent to Swift String.base58Encoded
 */
neoc_error_t neoc_string_to_base58(const char *input_string,
                                    char **base58_string) {
    if (!input_string || !base58_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *base58_string = NULL;
    
    // Convert string to bytes
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    neoc_error_t error = neoc_string_to_bytes(input_string, &bytes, &bytes_len);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Encode bytes as base58
    *base58_string = neoc_base58_encode_alloc(bytes, bytes_len);
    
    neoc_free(bytes);
    return *base58_string ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

/**
 * @brief Get variable size encoding length for string
 * 
 * Equivalent to Swift String.varSize
 */
size_t neoc_string_var_size(const char *string) {
    if (!string) {
        return 0;
    }
    
    size_t len = strlen(string);
    
    // Variable size encoding:
    // - 1 byte for length < 0xFD (253)
    // - 3 bytes for length < 0x10000 (65536) 
    // - 5 bytes for length < 0x100000000
    // - 9 bytes for larger lengths
    
    if (len < 0xFD) {
        return 1 + len;
    } else if (len < 0x10000) {
        return 3 + len;
    } else if (len < 0x100000000ULL) {
        return 5 + len;
    } else {
        return 9 + len;
    }
}

/**
 * @brief Validate if string is a valid NEO address
 * 
 * Equivalent to Swift String.isValidAddress
 */
bool neoc_string_is_valid_address(const char *address) {
    if (!address) {
        return false;
    }
    
    // Decode base58
    uint8_t *data = NULL;
    size_t data_len = 0;
    if (neoc_string_base58_decode(address, &data, &data_len) != NEOC_SUCCESS) {
        return false;
    }
    
    bool is_valid = false;
    
    // Check length (should be 25 bytes)
    if (data_len == 25) {
        // Check address version (first byte should be 0x35 for N3)
        if (data[0] == 0x35) { // NeoSwiftConfig.DEFAULT_ADDRESS_VERSION
            // Verify checksum: hash256 of first 21 bytes should match last 4 bytes
            uint8_t hash_result[32];
            if (neoc_hash256(data, 21, hash_result) == NEOC_SUCCESS) {
                if (memcmp(hash_result, data + 21, 4) == 0) {
                    is_valid = true;
                }
            }
        }
    }
    
    neoc_free(data);
    return is_valid;
}

/**
 * @brief Validate if string is valid hex
 * 
 * Equivalent to Swift String.isValidHex
 */
bool neoc_string_is_valid_hex(const char *hex_string) {
    if (!hex_string) {
        return false;
    }
    
    // Clean hex prefix
    char *cleaned_hex = NULL;
    if (neoc_string_clean_hex_prefix(hex_string, &cleaned_hex) != NEOC_SUCCESS) {
        return false;
    }
    
    size_t len = strlen(cleaned_hex);
    bool is_valid = true;
    
    // Must have even length
    if (len % 2 != 0) {
        is_valid = false;
    } else {
        // All characters must be hex digits
        for (size_t i = 0; i < len && is_valid; i++) {
            if (!neoc_string_is_hex_digit(cleaned_hex[i])) {
                is_valid = false;
            }
        }
    }
    
    neoc_free(cleaned_hex);
    return is_valid;
}

/**
 * @brief Convert NEO address to script hash bytes
 * 
 * Equivalent to Swift String.addressToScriptHash()
 */
neoc_error_t neoc_string_address_to_script_hash(const char *address,
                                                 uint8_t **script_hash,
                                                 size_t *script_hash_len) {
    if (!address || !script_hash || !script_hash_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *script_hash = NULL;
    *script_hash_len = 0;
    
    // Validate address first
    if (!neoc_string_is_valid_address(address)) {
        return NEOC_ERROR_INVALID_ADDRESS;
    }
    
    // Decode base58
    uint8_t *data = NULL;
    size_t data_len = 0;
    neoc_error_t error = neoc_string_base58_decode(address, &data, &data_len);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Extract script hash (bytes 1-20, reversed)
    *script_hash_len = 20;
    *script_hash = neoc_malloc(20);
    if (!*script_hash) {
        neoc_free(data);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Copy bytes 1-20 and reverse them
    for (size_t i = 0; i < 20; i++) {
        (*script_hash)[i] = data[20 - i]; // Reverse order
    }
    
    neoc_free(data);
    return NEOC_SUCCESS;
}

/**
 * @brief Reverse hex string byte order
 * 
 * Equivalent to Swift String.reversedHex
 */
neoc_error_t neoc_string_reverse_hex(const char *hex_string,
                                      char **reversed_hex) {
    if (!hex_string || !reversed_hex) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *reversed_hex = NULL;
    
    // Convert to bytes
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    neoc_error_t error = neoc_string_hex_to_bytes(hex_string, &bytes, &bytes_len);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Reverse bytes
    for (size_t i = 0; i < bytes_len / 2; i++) {
        uint8_t temp = bytes[i];
        bytes[i] = bytes[bytes_len - 1 - i];
        bytes[bytes_len - 1 - i] = temp;
    }
    
    // Convert back to hex (without prefix)
    *reversed_hex = neoc_hex_encode_alloc(bytes, bytes_len, false, false);
    
    neoc_free(bytes);
    return *reversed_hex ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

/**
 * @brief Check if character is a hex digit
 */
bool neoc_string_is_hex_digit(char c) {
    return isxdigit((unsigned char)c);
}

/**
 * @brief Get UTF-8 byte representation of string
 */
neoc_error_t neoc_string_to_bytes(const char *string,
                                   uint8_t **bytes,
                                   size_t *bytes_len) {
    if (!string || !bytes || !bytes_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *bytes_len = strlen(string);
    *bytes = neoc_malloc(*bytes_len);
    if (!*bytes) {
        *bytes_len = 0;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(*bytes, string, *bytes_len);
    return NEOC_SUCCESS;
}

/**
 * @brief Convert string to lowercase
 */
neoc_error_t neoc_string_to_lowercase(const char *string,
                                       char **lowercase_string) {
    if (!string || !lowercase_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    size_t len = strlen(string);
    *lowercase_string = neoc_malloc(len + 1);
    if (!*lowercase_string) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < len; i++) {
        (*lowercase_string)[i] = tolower((unsigned char)string[i]);
    }
    (*lowercase_string)[len] = '\0';
    
    return NEOC_SUCCESS;
}

/**
 * @brief Convert string to uppercase
 */
neoc_error_t neoc_string_to_uppercase(const char *string,
                                       char **uppercase_string) {
    if (!string || !uppercase_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    size_t len = strlen(string);
    *uppercase_string = neoc_malloc(len + 1);
    if (!*uppercase_string) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < len; i++) {
        (*uppercase_string)[i] = toupper((unsigned char)string[i]);
    }
    (*uppercase_string)[len] = '\0';
    
    return NEOC_SUCCESS;
}

/**
 * @brief Trim whitespace from string
 */
neoc_error_t neoc_string_trim(const char *string,
                               char **trimmed_string) {
    if (!string || !trimmed_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *trimmed_string = NULL;
    
    // Find start (skip leading whitespace)
    const char *start = string;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Find end (skip trailing whitespace)
    const char *end = string + strlen(string) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    
    // Calculate trimmed length
    size_t len = end - start + 1;
    *trimmed_string = neoc_malloc(len + 1);
    if (!*trimmed_string) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(*trimmed_string, start, len);
    (*trimmed_string)[len] = '\0';
    
    return NEOC_SUCCESS;
}

/**
 * @brief Check if string starts with prefix
 */
bool neoc_string_starts_with(const char *string, const char *prefix) {
    if (!string || !prefix) {
        return false;
    }
    
    size_t string_len = strlen(string);
    size_t prefix_len = strlen(prefix);
    
    if (prefix_len > string_len) {
        return false;
    }
    
    return strncmp(string, prefix, prefix_len) == 0;
}

/**
 * @brief Check if string ends with suffix
 */
bool neoc_string_ends_with(const char *string, const char *suffix) {
    if (!string || !suffix) {
        return false;
    }
    
    size_t string_len = strlen(string);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > string_len) {
        return false;
    }
    
    return strcmp(string + string_len - suffix_len, suffix) == 0;
}

/**
 * @brief Split string by delimiter
 */
neoc_error_t neoc_string_split(const char *string,
                                char delimiter,
                                char ***parts,
                                size_t *parts_count) {
    if (!string || !parts || !parts_count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *parts = NULL;
    *parts_count = 0;
    
    // Count delimiters to estimate parts count
    size_t delimiter_count = 0;
    for (const char *p = string; *p; p++) {
        if (*p == delimiter) {
            delimiter_count++;
        }
    }
    
    size_t max_parts = delimiter_count + 1;
    *parts = neoc_malloc(max_parts * sizeof(char*));
    if (!*parts) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Split the string
    const char *start = string;
    size_t part_index = 0;
    
    for (const char *p = string; *p; p++) {
        if (*p == delimiter) {
            size_t part_len = p - start;
            (*parts)[part_index] = neoc_malloc(part_len + 1);
            if (!(*parts)[part_index]) {
                // Cleanup on failure
                neoc_string_split_free(*parts, part_index);
                *parts = NULL;
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            
            memcpy((*parts)[part_index], start, part_len);
            (*parts)[part_index][part_len] = '\0';
            part_index++;
            start = p + 1;
        }
    }
    
    // Add the final part
    size_t final_len = strlen(start);
    (*parts)[part_index] = neoc_malloc(final_len + 1);
    if (!(*parts)[part_index]) {
        neoc_string_split_free(*parts, part_index);
        *parts = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    strcpy((*parts)[part_index], start);
    part_index++;
    
    *parts_count = part_index;
    return NEOC_SUCCESS;
}

/**
 * @brief Free array of strings returned by neoc_string_split
 */
void neoc_string_split_free(char **parts, size_t parts_count) {
    if (!parts) {
        return;
    }
    
    for (size_t i = 0; i < parts_count; i++) {
        neoc_free(parts[i]);
    }
    neoc_free(parts);
}

