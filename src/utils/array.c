/**
 * @file array.c
 * @brief Array utility functions implementation
 * 
 * Based on Swift source: utils/Array.swift
 * Implements dynamic array management and utility operations
 */

#include "../../include/neoc/neoc_error.h"
#include "../../include/neoc/neoc_memory.h"
#include "../../include/neoc/utils/array.h"
#include <string.h>
#include <stdio.h>

#define DEFAULT_ARRAY_CAPACITY 16
#define DEFAULT_BYTE_ARRAY_CAPACITY 64

/**
 * @brief Create a new dynamic array
 */
neoc_error_t neoc_array_create(size_t element_size,
                                size_t initial_capacity,
                                neoc_array_t **array) {
    if (!array || element_size == 0) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *array = NULL;
    
    neoc_array_t *new_array = neoc_malloc(sizeof(neoc_array_t));
    if (!new_array) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    size_t capacity = initial_capacity > 0 ? initial_capacity : DEFAULT_ARRAY_CAPACITY;
    
    new_array->elements = neoc_malloc(capacity * sizeof(void*));
    if (!new_array->elements) {
        neoc_free(new_array);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_array->count = 0;
    new_array->capacity = capacity;
    new_array->element_size = element_size;
    
    *array = new_array;
    return NEOC_SUCCESS;
}

/**
 * @brief Helper function to resize array capacity
 */
static neoc_error_t neoc_array_resize(neoc_array_t *array, size_t new_capacity) {
    if (!array) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    void **new_elements = neoc_realloc(array->elements, new_capacity * sizeof(void*));
    if (!new_elements) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    array->elements = new_elements;
    array->capacity = new_capacity;
    return NEOC_SUCCESS;
}

/**
 * @brief Add an element to the end of the array
 */
neoc_error_t neoc_array_append(neoc_array_t *array, const void *element) {
    if (!array || !element) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Resize if needed
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        neoc_error_t error = neoc_array_resize(array, new_capacity);
        if (error != NEOC_SUCCESS) {
            return error;
        }
    }
    
    // Allocate and copy element
    void *new_element = neoc_malloc(array->element_size);
    if (!new_element) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(new_element, element, array->element_size);
    array->elements[array->count] = new_element;
    array->count++;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Insert an element at the beginning of the array
 */
neoc_error_t neoc_array_prepend(neoc_array_t *array, const void *element) {
    return neoc_array_insert(array, 0, element);
}

/**
 * @brief Insert an element at a specific index
 */
neoc_error_t neoc_array_insert(neoc_array_t *array, size_t index, const void *element) {
    if (!array || !element || index > array->count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Resize if needed
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        neoc_error_t error = neoc_array_resize(array, new_capacity);
        if (error != NEOC_SUCCESS) {
            return error;
        }
    }
    
    // Shift elements right
    for (size_t i = array->count; i > index; i--) {
        array->elements[i] = array->elements[i - 1];
    }
    
    // Allocate and copy new element
    void *new_element = neoc_malloc(array->element_size);
    if (!new_element) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(new_element, element, array->element_size);
    array->elements[index] = new_element;
    array->count++;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Remove an element at a specific index
 */
neoc_error_t neoc_array_remove(neoc_array_t *array, size_t index) {
    if (!array || index >= array->count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free the element
    neoc_free(array->elements[index]);
    
    // Shift elements left
    for (size_t i = index; i < array->count - 1; i++) {
        array->elements[i] = array->elements[i + 1];
    }
    
    array->count--;
    return NEOC_SUCCESS;
}

/**
 * @brief Get an element at a specific index
 */
neoc_error_t neoc_array_get(const neoc_array_t *array, size_t index, void **element) {
    if (!array || !element || index >= array->count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *element = array->elements[index];
    return NEOC_SUCCESS;
}

/**
 * @brief Set an element at a specific index
 */
neoc_error_t neoc_array_set(neoc_array_t *array, size_t index, const void *element) {
    if (!array || !element || index >= array->count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free old element and allocate new one
    neoc_free(array->elements[index]);
    
    void *new_element = neoc_malloc(array->element_size);
    if (!new_element) {
        array->elements[index] = NULL; // Mark as empty
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(new_element, element, array->element_size);
    array->elements[index] = new_element;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get the number of elements in the array
 */
size_t neoc_array_count(const neoc_array_t *array) {
    return array ? array->count : 0;
}

/**
 * @brief Check if the array is empty
 */
bool neoc_array_is_empty(const neoc_array_t *array) {
    return !array || array->count == 0;
}

/**
 * @brief Clear all elements from the array
 */
void neoc_array_clear(neoc_array_t *array) {
    if (!array) {
        return;
    }
    
    // Free all elements
    for (size_t i = 0; i < array->count; i++) {
        neoc_free(array->elements[i]);
    }
    
    array->count = 0;
}

/**
 * @brief Free an array and all its elements
 */
void neoc_array_free(neoc_array_t *array) {
    if (!array) {
        return;
    }
    
    // Clear all elements first
    neoc_array_clear(array);
    
    // Free the elements array
    neoc_free(array->elements);
    
    // Free the array structure
    neoc_free(array);
}

// Byte array functions

/**
 * @brief Create a new byte array
 */
neoc_error_t neoc_byte_array_create(size_t initial_capacity,
                                     neoc_byte_array_t **byte_array) {
    if (!byte_array) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *byte_array = NULL;
    
    neoc_byte_array_t *new_array = neoc_malloc(sizeof(neoc_byte_array_t));
    if (!new_array) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    size_t capacity = initial_capacity > 0 ? initial_capacity : DEFAULT_BYTE_ARRAY_CAPACITY;
    
    new_array->data = neoc_malloc(capacity);
    if (!new_array->data) {
        neoc_free(new_array);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_array->length = 0;
    new_array->capacity = capacity;
    
    *byte_array = new_array;
    return NEOC_SUCCESS;
}

/**
 * @brief Create byte array from existing data
 */
neoc_error_t neoc_byte_array_from_data(const uint8_t *data,
                                        size_t length,
                                        neoc_byte_array_t **byte_array) {
    if (!byte_array) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *byte_array = NULL;
    
    if (!data && length > 0) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    neoc_error_t error = neoc_byte_array_create(length, byte_array);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    if (length > 0) {
        memcpy((*byte_array)->data, data, length);
        (*byte_array)->length = length;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Helper function to resize byte array capacity
 */
static neoc_error_t neoc_byte_array_resize(neoc_byte_array_t *byte_array, size_t new_capacity) {
    if (!byte_array) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    uint8_t *new_data = neoc_realloc(byte_array->data, new_capacity);
    if (!new_data) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    byte_array->data = new_data;
    byte_array->capacity = new_capacity;
    return NEOC_SUCCESS;
}

/**
 * @brief Append bytes to byte array
 */
neoc_error_t neoc_byte_array_append_bytes(neoc_byte_array_t *byte_array,
                                           const uint8_t *data,
                                           size_t length) {
    if (!byte_array || (!data && length > 0)) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (length == 0) {
        return NEOC_SUCCESS;
    }
    
    // Resize if needed
    size_t required_capacity = byte_array->length + length;
    if (required_capacity > byte_array->capacity) {
        size_t new_capacity = byte_array->capacity;
        while (new_capacity < required_capacity) {
            new_capacity *= 2;
        }
        
        neoc_error_t error = neoc_byte_array_resize(byte_array, new_capacity);
        if (error != NEOC_SUCCESS) {
            return error;
        }
    }
    
    // Append data
    memcpy(byte_array->data + byte_array->length, data, length);
    byte_array->length += length;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Append single byte to byte array
 */
neoc_error_t neoc_byte_array_append_byte(neoc_byte_array_t *byte_array, uint8_t byte) {
    return neoc_byte_array_append_bytes(byte_array, &byte, 1);
}

/**
 * @brief Concatenate two byte arrays
 * 
 * Equivalent to Swift Array + Array operator
 */
neoc_error_t neoc_byte_array_concat(const neoc_byte_array_t *left,
                                     const neoc_byte_array_t *right,
                                     neoc_byte_array_t **result) {
    if (!left || !right || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    size_t total_length = left->length + right->length;
    neoc_error_t error = neoc_byte_array_create(total_length, result);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Copy left array
    if (left->length > 0) {
        error = neoc_byte_array_append_bytes(*result, left->data, left->length);
        if (error != NEOC_SUCCESS) {
            neoc_byte_array_free(*result);
            *result = NULL;
            return error;
        }
    }
    
    // Copy right array
    if (right->length > 0) {
        error = neoc_byte_array_append_bytes(*result, right->data, right->length);
        if (error != NEOC_SUCCESS) {
            neoc_byte_array_free(*result);
            *result = NULL;
            return error;
        }
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Concatenate byte array with single byte
 * 
 * Equivalent to Swift Array + Element operator
 */
neoc_error_t neoc_byte_array_concat_byte(const neoc_byte_array_t *byte_array,
                                          uint8_t byte,
                                          neoc_byte_array_t **result) {
    if (!byte_array || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    size_t total_length = byte_array->length + 1;
    neoc_error_t error = neoc_byte_array_create(total_length, result);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Copy array
    if (byte_array->length > 0) {
        error = neoc_byte_array_append_bytes(*result, byte_array->data, byte_array->length);
        if (error != NEOC_SUCCESS) {
            neoc_byte_array_free(*result);
            *result = NULL;
            return error;
        }
    }
    
    // Append byte
    error = neoc_byte_array_append_byte(*result, byte);
    if (error != NEOC_SUCCESS) {
        neoc_byte_array_free(*result);
        *result = NULL;
        return error;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Prepend single byte to byte array
 * 
 * Equivalent to Swift Element + Array operator
 */
neoc_error_t neoc_byte_array_prepend_byte(uint8_t byte,
                                           const neoc_byte_array_t *byte_array,
                                           neoc_byte_array_t **result) {
    if (!byte_array || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    size_t total_length = 1 + byte_array->length;
    neoc_error_t error = neoc_byte_array_create(total_length, result);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Append byte first
    error = neoc_byte_array_append_byte(*result, byte);
    if (error != NEOC_SUCCESS) {
        neoc_byte_array_free(*result);
        *result = NULL;
        return error;
    }
    
    // Append array
    if (byte_array->length > 0) {
        error = neoc_byte_array_append_bytes(*result, byte_array->data, byte_array->length);
        if (error != NEOC_SUCCESS) {
            neoc_byte_array_free(*result);
            *result = NULL;
            return error;
        }
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get a slice of the byte array
 */
neoc_error_t neoc_byte_array_slice(const neoc_byte_array_t *byte_array,
                                    size_t start,
                                    size_t end,
                                    neoc_byte_array_t **slice) {
    if (!byte_array || !slice || start > end || end > byte_array->length) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *slice = NULL;
    
    size_t slice_length = end - start;
    neoc_error_t error = neoc_byte_array_create(slice_length, slice);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    if (slice_length > 0) {
        error = neoc_byte_array_append_bytes(*slice, byte_array->data + start, slice_length);
        if (error != NEOC_SUCCESS) {
            neoc_byte_array_free(*slice);
            *slice = NULL;
            return error;
        }
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Free a byte array
 */
void neoc_byte_array_free(neoc_byte_array_t *byte_array) {
    if (!byte_array) {
        return;
    }
    
    neoc_free(byte_array->data);
    neoc_free(byte_array);
}

