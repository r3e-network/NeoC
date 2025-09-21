/**
 * @file array.h
 * @brief Array utility functions and dynamic array implementation
 * 
 * Based on Swift source: utils/Array.swift
 * Provides dynamic array management and utility operations
 */

#ifndef NEOC_UTILS_ARRAY_H
#define NEOC_UTILS_ARRAY_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dynamic array structure for generic data
 */
typedef struct {
    void **elements;        /**< Array of element pointers */
    size_t count;          /**< Current number of elements */
    size_t capacity;       /**< Current capacity */
    size_t element_size;   /**< Size of each element in bytes */
} neoc_array_t;

/**
 * @brief Byte array structure for byte data
 */
typedef struct {
    uint8_t *data;         /**< Array of bytes */
    size_t length;         /**< Number of bytes */
    size_t capacity;       /**< Current capacity */
} neoc_byte_array_t;

/**
 * @brief Create a new dynamic array
 * 
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial capacity (0 for default)
 * @param array Output array (caller must free with neoc_array_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_create(size_t element_size,
                                size_t initial_capacity,
                                neoc_array_t **array);

/**
 * @brief Add an element to the end of the array
 * 
 * @param array The array
 * @param element Pointer to element to add
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_append(neoc_array_t *array, const void *element);

/**
 * @brief Insert an element at the beginning of the array
 * 
 * @param array The array
 * @param element Pointer to element to insert
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_prepend(neoc_array_t *array, const void *element);

/**
 * @brief Insert an element at a specific index
 * 
 * @param array The array
 * @param index Index to insert at
 * @param element Pointer to element to insert
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_insert(neoc_array_t *array, size_t index, const void *element);

/**
 * @brief Remove an element at a specific index
 * 
 * @param array The array
 * @param index Index to remove
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_remove(neoc_array_t *array, size_t index);

/**
 * @brief Get an element at a specific index
 * 
 * @param array The array
 * @param index Index to get
 * @param element Output pointer to element (do not free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_get(const neoc_array_t *array, size_t index, void **element);

/**
 * @brief Set an element at a specific index
 * 
 * @param array The array
 * @param index Index to set
 * @param element Pointer to new element value
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_array_set(neoc_array_t *array, size_t index, const void *element);

/**
 * @brief Get the number of elements in the array
 * 
 * @param array The array
 * @return Number of elements
 */
size_t neoc_array_count(const neoc_array_t *array);

/**
 * @brief Check if the array is empty
 * 
 * @param array The array
 * @return True if empty
 */
bool neoc_array_is_empty(const neoc_array_t *array);

/**
 * @brief Clear all elements from the array
 * 
 * @param array The array
 */
void neoc_array_clear(neoc_array_t *array);

/**
 * @brief Free an array and all its elements
 * 
 * @param array The array to free
 */
void neoc_array_free(neoc_array_t *array);

// Byte array functions

/**
 * @brief Create a new byte array
 * 
 * @param initial_capacity Initial capacity (0 for default)
 * @param byte_array Output byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_create(size_t initial_capacity,
                                     neoc_byte_array_t **byte_array);

/**
 * @brief Create byte array from existing data
 * 
 * @param data Source data (will be copied)
 * @param length Length of data
 * @param byte_array Output byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_from_data(const uint8_t *data,
                                        size_t length,
                                        neoc_byte_array_t **byte_array);

/**
 * @brief Append bytes to byte array
 * 
 * @param byte_array The byte array
 * @param data Data to append
 * @param length Length of data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_append_bytes(neoc_byte_array_t *byte_array,
                                           const uint8_t *data,
                                           size_t length);

/**
 * @brief Append single byte to byte array
 * 
 * @param byte_array The byte array
 * @param byte Byte to append
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_append_byte(neoc_byte_array_t *byte_array, uint8_t byte);

/**
 * @brief Concatenate two byte arrays
 * 
 * Equivalent to Swift Array + Array operator
 * 
 * @param left Left byte array
 * @param right Right byte array
 * @param result Output concatenated array (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_concat(const neoc_byte_array_t *left,
                                     const neoc_byte_array_t *right,
                                     neoc_byte_array_t **result);

/**
 * @brief Concatenate byte array with single byte
 * 
 * Equivalent to Swift Array + Element operator
 * 
 * @param byte_array Source byte array
 * @param byte Byte to append
 * @param result Output concatenated array (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_concat_byte(const neoc_byte_array_t *byte_array,
                                          uint8_t byte,
                                          neoc_byte_array_t **result);

/**
 * @brief Prepend single byte to byte array
 * 
 * Equivalent to Swift Element + Array operator
 * 
 * @param byte Byte to prepend
 * @param byte_array Source byte array
 * @param result Output array with prepended byte (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_prepend_byte(uint8_t byte,
                                           const neoc_byte_array_t *byte_array,
                                           neoc_byte_array_t **result);

/**
 * @brief Get a slice of the byte array
 * 
 * @param byte_array Source byte array
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @param slice Output slice (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_byte_array_slice(const neoc_byte_array_t *byte_array,
                                    size_t start,
                                    size_t end,
                                    neoc_byte_array_t **slice);

/**
 * @brief Free a byte array
 * 
 * @param byte_array The byte array to free
 */
void neoc_byte_array_free(neoc_byte_array_t *byte_array);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_ARRAY_H */
