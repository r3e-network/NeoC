/**
 * @file bytes.h
 * @brief Unified byte array utilities interface
 * 
 * Based on Swift source: utils/Bytes.swift and Array.swift extensions
 * Provides a unified interface to all byte array operations in NeoC
 */

#ifndef NEOC_UTILS_BYTES_H
#define NEOC_UTILS_BYTES_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_types.h"
#include "neoc/utils/array.h"
#include "neoc/utils/neoc_bytes_utils.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This header provides a unified interface to all byte operations.
 * For comprehensive byte utilities, use functions from:
 * 
 * 1. neoc_types.h - Basic neoc_bytes_t operations:
 *    - neoc_bytes_create()
 *    - neoc_bytes_from_data()
 *    - neoc_bytes_duplicate()
 *    - neoc_bytes_reverse()
 *    - neoc_bytes_compare()
 *    - neoc_bytes_equal()
 *    - neoc_bytes_to_hex()
 *    - neoc_bytes_clear()
 *    - neoc_bytes_free()
 *    
 * 2. array.h - Dynamic neoc_byte_array_t operations:
 *    - neoc_byte_array_create()
 *    - neoc_byte_array_from_data()
 *    - neoc_byte_array_append_byte()
 *    - neoc_byte_array_concat()
 *    - neoc_byte_array_slice()
 *    - neoc_byte_array_free()
 *    
 * 3. neoc_bytes_utils.h - Advanced byte operations:
 *    - neoc_bytes_pad()
 *    - neoc_bytes_trim_trailing() 
 *    - neoc_bytes_to_uint32()
 *    - neoc_bytes_xor()
 *    - neoc_bytes_reverse_copy()
 */

/**
 * @brief Convert neoc_byte_array_t to neoc_bytes_t
 * 
 * @param byte_array Source byte array
 * @return New neoc_bytes_t (caller must free)
 */
neoc_bytes_t* neoc_bytes_from_byte_array(const neoc_byte_array_t *byte_array);

/**
 * @brief Convert neoc_bytes_t to neoc_byte_array_t
 * 
 * @param bytes Source bytes
 * @param result Output byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_byte_array_from_bytes(const neoc_bytes_t *bytes,
                                         neoc_byte_array_t **result);

/**
 * @brief Convert hex string to byte array
 * 
 * Convenience wrapper for hex string conversion
 * 
 * @param hex_string Hexadecimal string (with or without "0x" prefix)
 * @param result Output byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_byte_array_from_hex_string(const char *hex_string,
                                              neoc_byte_array_t **result);

/**
 * @brief Convert byte array to hex string
 * 
 * Convenience wrapper for hex string conversion
 * 
 * @param byte_array Source byte array
 * @param uppercase Use uppercase letters if true
 * @param with_prefix Add "0x" prefix if true
 * @return Allocated hex string (caller must free)
 */
char* neoc_byte_array_to_hex_string(const neoc_byte_array_t *byte_array,
                                     bool uppercase,
                                     bool with_prefix);

/**
 * @brief Check if byte array contains specific value
 * 
 * @param byte_array Byte array to search
 * @param value Byte value to find
 * @return Index of first occurrence, or SIZE_MAX if not found
 */
size_t neoc_byte_array_find_byte(const neoc_byte_array_t *byte_array, uint8_t value);

/**
 * @brief Check if byte array starts with specific prefix
 * 
 * @param byte_array Byte array to check
 * @param prefix Prefix to check for
 * @return true if starts with prefix, false otherwise
 */
bool neoc_byte_array_starts_with(const neoc_byte_array_t *byte_array,
                                  const neoc_byte_array_t *prefix);

/**
 * @brief Check if byte array ends with specific suffix
 * 
 * @param byte_array Byte array to check
 * @param suffix Suffix to check for
 * @return true if ends with suffix, false otherwise
 */
bool neoc_byte_array_ends_with(const neoc_byte_array_t *byte_array,
                                const neoc_byte_array_t *suffix);

/* 
 * Convenience macros for common operations
 */

/**
 * @brief Check if byte array is empty
 */
#define neoc_byte_array_is_empty(arr) \
    (!(arr) || (arr)->length == 0)

/**
 * @brief Get byte array length
 */
#define neoc_byte_array_length(arr) \
    ((arr) ? (arr)->length : 0)

/**
 * @brief Get byte at index (safe access)
 */
#define neoc_byte_array_at(arr, idx) \
    (((arr) && (idx) < (arr)->length) ? (arr)->data[idx] : 0)

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_BYTES_H */
