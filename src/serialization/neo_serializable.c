#include "neoc/serialization/neo_serializable.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdlib.h>

/**
 * @file neo_serializable.c
 * @brief Implementation of Neo serialization protocol interface
 * 
 * This file implements the core serialization functionality for the Neo C SDK.
 * It provides a standard interface for serializable objects and common operations
 * for working with serializable data in Neo protocol format.
 * 
 * The implementation focuses on memory safety, proper error handling, and
 * thread safety where possible. All functions validate their inputs and
 * handle edge cases gracefully.
 * 
 * @author NeoC SDK Team
 * @version 1.0.0
 * @date 2024
 */

neoc_error_t neoc_serializable_validate(const neoc_serializable_t *obj) {
    if (!obj) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (!obj->serialize || !obj->deserialize || !obj->get_size || !obj->free_obj) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_serializable_get_size(const neoc_serializable_t *obj, size_t *size) {
    if (!obj || !size) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    neoc_error_t result = neoc_serializable_validate(obj);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    return obj->get_size(obj, size);
}

neoc_error_t neoc_serializable_serialize(const neoc_serializable_t *obj,
                                          neoc_binary_writer_t *writer) {
    if (!obj || !writer) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    neoc_error_t result = neoc_serializable_validate(obj);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    return obj->serialize(obj, writer);
}

neoc_error_t neoc_serializable_to_array(const neoc_serializable_t *obj,
                                         uint8_t **data,
                                         size_t *len) {
    if (!obj || !data || !len) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *data = NULL;
    *len = 0;
    
    // Validate object
    neoc_error_t result = neoc_serializable_validate(obj);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    size_t size = 0;
    result = obj->get_size(obj, &size);
    if (result != NEOC_SUCCESS) {
        return result;
    }

    neoc_binary_writer_t *writer = NULL;
    result = neoc_binary_writer_create(size, true, &writer);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    // Serialize object
    result = obj->serialize(obj, writer);
    if (result != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return result;
    }
    
    // Get serialized data
    result = neoc_binary_writer_to_array(writer, data, len);
    neoc_binary_writer_free(writer);
    
    return result;
}

neoc_error_t neoc_serializable_from_array(const uint8_t *data,
                                           size_t len,
                                           neoc_deserialize_func_t deserialize_func,
                                           neoc_serializable_t **obj) {
    if (!data || !deserialize_func || !obj) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *obj = NULL;
    
    if (len == 0) {
        return NEOC_ERROR_INVALID_DATA;
    }
    
    // Create binary reader
    neoc_binary_reader_t *reader = NULL;
    neoc_error_t result = neoc_binary_reader_create(data, len, &reader);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    // Deserialize object
    result = deserialize_func(reader, obj);
    neoc_binary_reader_free(reader);
    
    if (result != NEOC_SUCCESS && *obj) {
        // Clean up partially deserialized object
        neoc_serializable_free(*obj);
        *obj = NULL;
    }
    
    return result;
}

neoc_error_t neoc_serializable_clone(const neoc_serializable_t *obj,
                                      neoc_serializable_t **clone) {
    if (!obj || !clone) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *clone = NULL;
    
    neoc_error_t result = neoc_serializable_validate(obj);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    if (!obj->clone) {
        return NEOC_ERROR_NOT_SUPPORTED;
    }
    
    return obj->clone(obj, clone);
}

void neoc_serializable_free(neoc_serializable_t *obj) {
    if (!obj) {
        return;
    }
    
    if (obj->free_obj) {
        obj->free_obj((void*)obj);
    }
}

neoc_error_t neoc_serializable_array_serialize(neoc_serializable_t **objects,
                                                size_t count,
                                                neoc_binary_writer_t *writer) {
    if (!writer || (count > 0 && !objects)) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Write array count as variable-length integer
    neoc_error_t result = neoc_binary_writer_write_var_int(writer, count);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    // Serialize each object
    for (size_t i = 0; i < count; i++) {
        if (!objects[i]) {
            return NEOC_ERROR_INVALID_ARGUMENT;
        }
        
        result = neoc_serializable_serialize(objects[i], writer);
        if (result != NEOC_SUCCESS) {
            return result;
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_serializable_array_deserialize(neoc_binary_reader_t *reader,
                                                  neoc_deserialize_func_t deserialize_func,
                                                  neoc_serializable_t ***objects,
                                                  size_t *count,
                                                  size_t max_count) {
    if (!reader || !deserialize_func || !objects || !count) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *objects = NULL;
    *count = 0;
    
    // Read array count
    uint64_t array_count;
    neoc_error_t result = neoc_binary_reader_read_var_int(reader, &array_count);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    // Check maximum count limit
    if (max_count > 0 && array_count > max_count) {
        return NEOC_ERROR_INVALID_DATA;
    }
    
    // Handle empty array
    if (array_count == 0) {
        return NEOC_SUCCESS;
    }
    
    // Check for reasonable limits (prevent excessive memory allocation)
    if (array_count > 1000000) { // 1 million objects maximum
        return NEOC_ERROR_INVALID_DATA;
    }
    
    size_t final_count = (size_t)array_count;
    
    // Allocate array of object pointers
    neoc_serializable_t **obj_array = (neoc_serializable_t**)neoc_malloc(sizeof(neoc_serializable_t*) * final_count);
    if (!obj_array) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize all pointers to NULL for cleanup safety
    memset(obj_array, 0, sizeof(neoc_serializable_t*) * final_count);
    
    // Deserialize each object
    for (size_t i = 0; i < final_count; i++) {
        result = deserialize_func(reader, &obj_array[i]);
        if (result != NEOC_SUCCESS) {
            // Clean up already deserialized objects
            for (size_t j = 0; j < i; j++) {
                neoc_serializable_free(obj_array[j]);
            }
            neoc_free(obj_array);
            return result;
        }
    }
    
    *objects = obj_array;
    *count = final_count;
    return NEOC_SUCCESS;
}

void neoc_serializable_array_free(neoc_serializable_t **objects, size_t count) {
    if (!objects) {
        return;
    }
    
    // Free each object
    for (size_t i = 0; i < count; i++) {
        neoc_serializable_free(objects[i]);
    }
    
    // Free the array itself
    neoc_free(objects);
}

neoc_error_t neoc_serializable_array_get_size(neoc_serializable_t **objects,
                                               size_t count,
                                               size_t *total_size) {
    if (!total_size || (count > 0 && !objects)) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *total_size = 0;
    
    // Calculate size of variable-length count prefix
    size_t count_size = 0;
    if (count < 0xFD) {
        count_size = 1;
    } else if (count <= 0xFFFF) {
        count_size = 3;
    } else if (count <= 0xFFFFFFFF) {
        count_size = 5;
    } else {
        count_size = 9;
    }
    
    *total_size += count_size;
    
    // Add size of each object
    for (size_t i = 0; i < count; i++) {
        if (!objects[i]) {
            return NEOC_ERROR_INVALID_ARGUMENT;
        }
        
        size_t obj_size;
        neoc_error_t result = neoc_serializable_get_size(objects[i], &obj_size);
        if (result != NEOC_SUCCESS) {
            return result;
        }
        
        *total_size += obj_size;
    }
    
    return NEOC_SUCCESS;
}
