/**
 * @file stack_item.c
 * @brief Implementation of Neo VM Stack Items
 */

#include "neoc/protocol/stack_item.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEFAULT_ARRAY_CAPACITY 16
#define DEFAULT_MAP_CAPACITY 16

// Helper function to allocate stack item
static stack_item_t* stack_item_alloc(stack_item_type_t type) {
    stack_item_t* item = calloc(1, sizeof(stack_item_t));
    if (!item) return NULL;
    
    item->type = type;
    item->ref_count = 1;
    return item;
}

// Create Any (null) stack item
stack_item_t* stack_item_create_any(void) {
    return stack_item_alloc(STACK_ITEM_TYPE_ANY);
}

// Create Boolean stack item
stack_item_t* stack_item_create_boolean(bool value) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_BOOLEAN);
    if (!item) return NULL;
    
    item->value.boolean_value = value;
    return item;
}

// Create Integer stack item from int64
stack_item_t* stack_item_create_integer(int64_t value) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_INTEGER);
    if (!item) return NULL;
    
    // Convert int64 to big integer bytes (little-endian)
    size_t size = sizeof(int64_t);
    item->value.integer.bytes = malloc(size);
    if (!item->value.integer.bytes) {
        free(item);
        return NULL;
    }
    
    item->value.integer.is_negative = (value < 0);
    uint64_t abs_value = item->value.integer.is_negative ? -value : value;
    
    // Store as little-endian
    for (size_t i = 0; i < size; i++) {
        item->value.integer.bytes[i] = (abs_value >> (i * 8)) & 0xFF;
    }
    
    // Trim leading zeros
    while (size > 1 && item->value.integer.bytes[size - 1] == 0) {
        size--;
    }
    item->value.integer.length = size;
    
    return item;
}

// Create Integer stack item from big integer bytes
stack_item_t* stack_item_create_big_integer(const uint8_t* bytes, size_t length, bool is_negative) {
    if (!bytes || length == 0) return NULL;
    
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_INTEGER);
    if (!item) return NULL;
    
    item->value.integer.bytes = malloc(length);
    if (!item->value.integer.bytes) {
        free(item);
        return NULL;
    }
    
    memcpy(item->value.integer.bytes, bytes, length);
    item->value.integer.length = length;
    item->value.integer.is_negative = is_negative;
    
    return item;
}

// Create ByteString stack item
stack_item_t* stack_item_create_byte_string(const uint8_t* data, size_t length) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_BYTE_STRING);
    if (!item) return NULL;
    
    if (length > 0) {
        item->value.byte_string.data = malloc(length);
        if (!item->value.byte_string.data) {
            free(item);
            return NULL;
        }
        if (data) {
            memcpy(item->value.byte_string.data, data, length);
        } else {
            memset(item->value.byte_string.data, 0, length);
        }
        item->value.byte_string.length = length;
    }
    
    return item;
}

// Create Buffer stack item
stack_item_t* stack_item_create_buffer(const uint8_t* data, size_t length) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_BUFFER);
    if (!item) return NULL;
    
    if (length > 0) {
        item->value.byte_string.data = malloc(length);
        if (!item->value.byte_string.data) {
            free(item);
            return NULL;
        }
        if (data) {
            memcpy(item->value.byte_string.data, data, length);
        } else {
            memset(item->value.byte_string.data, 0, length);
        }
        item->value.byte_string.length = length;
    }
    
    return item;
}

// Create Array stack item
stack_item_t* stack_item_create_array(size_t initial_capacity) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_ARRAY);
    if (!item) return NULL;
    
    size_t capacity = initial_capacity ? initial_capacity : DEFAULT_ARRAY_CAPACITY;
    item->value.array.items = calloc(capacity, sizeof(stack_item_t*));
    if (!item->value.array.items) {
        free(item);
        return NULL;
    }
    
    item->value.array.capacity = capacity;
    item->value.array.count = 0;
    
    return item;
}

// Create Struct stack item
stack_item_t* stack_item_create_struct(size_t initial_capacity) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_STRUCT);
    if (!item) return NULL;
    
    size_t capacity = initial_capacity ? initial_capacity : DEFAULT_ARRAY_CAPACITY;
    item->value.array.items = calloc(capacity, sizeof(stack_item_t*));
    if (!item->value.array.items) {
        free(item);
        return NULL;
    }
    
    item->value.array.capacity = capacity;
    item->value.array.count = 0;
    
    return item;
}

// Create Map stack item
stack_item_t* stack_item_create_map(size_t initial_capacity) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_MAP);
    if (!item) return NULL;
    
    size_t capacity = initial_capacity ? initial_capacity : DEFAULT_MAP_CAPACITY;
    item->value.map.entries = calloc(capacity, sizeof(stack_item_map_entry_t));
    if (!item->value.map.entries) {
        free(item);
        return NULL;
    }
    
    item->value.map.capacity = capacity;
    item->value.map.count = 0;
    
    return item;
}

// Create Pointer stack item
stack_item_t* stack_item_create_pointer(void* ptr, size_t position) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_POINTER);
    if (!item) return NULL;
    
    item->value.pointer.ptr = ptr;
    item->value.pointer.position = position;
    
    return item;
}

// Create InteropInterface stack item
stack_item_t* stack_item_create_interop_interface(void* interface) {
    stack_item_t* item = stack_item_alloc(STACK_ITEM_TYPE_INTEROP_INTERFACE);
    if (!item) return NULL;
    
    item->value.interop_interface = interface;
    
    return item;
}

// Increase reference count
void stack_item_ref(stack_item_t* item) {
    if (item) {
        item->ref_count++;
    }
}

// Decrease reference count and free if zero
void stack_item_unref(stack_item_t* item) {
    if (!item) return;
    
    if (--item->ref_count > 0) return;
    
    // Free type-specific data
    switch (item->type) {
        case STACK_ITEM_TYPE_INTEGER:
            free(item->value.integer.bytes);
            break;
            
        case STACK_ITEM_TYPE_BYTE_STRING:
        case STACK_ITEM_TYPE_BUFFER:
            free(item->value.byte_string.data);
            break;
            
        case STACK_ITEM_TYPE_ARRAY:
        case STACK_ITEM_TYPE_STRUCT:
            for (size_t i = 0; i < item->value.array.count; i++) {
                stack_item_unref(item->value.array.items[i]);
            }
            free(item->value.array.items);
            break;
            
        case STACK_ITEM_TYPE_MAP:
            for (size_t i = 0; i < item->value.map.count; i++) {
                stack_item_unref(item->value.map.entries[i].key);
                stack_item_unref(item->value.map.entries[i].value);
            }
            free(item->value.map.entries);
            break;
            
        default:
            break;
    }
    
    free(item);
}

// Free a stack item (alias for unref)
void stack_item_free(stack_item_t* item) {
    stack_item_unref(item);
}

// Get the type of a stack item
stack_item_type_t stack_item_get_type(const stack_item_t* item) {
    return item ? item->type : STACK_ITEM_TYPE_ANY;
}

// Check if stack item is null
bool stack_item_is_null(const stack_item_t* item) {
    return !item || item->type == STACK_ITEM_TYPE_ANY;
}

// Convert stack item to boolean
neoc_error_t stack_item_to_boolean(const stack_item_t* item, bool* result) {
    if (!item || !result) return NEOC_ERROR_INVALID_PARAM;
    
    switch (item->type) {
        case STACK_ITEM_TYPE_ANY:
            *result = false;
            break;
            
        case STACK_ITEM_TYPE_BOOLEAN:
            *result = item->value.boolean_value;
            break;
            
        case STACK_ITEM_TYPE_INTEGER:
            // Non-zero is true
            *result = false;
            for (size_t i = 0; i < item->value.integer.length; i++) {
                if (item->value.integer.bytes[i] != 0) {
                    *result = true;
                    break;
                }
            }
            break;
            
        case STACK_ITEM_TYPE_BYTE_STRING:
        case STACK_ITEM_TYPE_BUFFER:
            // Non-empty is true
            *result = (item->value.byte_string.length > 0);
            break;
            
        default:
            *result = true;  // Other types are considered true
            break;
    }
    
    return NEOC_SUCCESS;
}

// Convert stack item to integer
neoc_error_t stack_item_to_integer(const stack_item_t* item, int64_t* result) {
    if (!item || !result) return NEOC_ERROR_INVALID_PARAM;
    
    if (item->type != STACK_ITEM_TYPE_INTEGER) {
        return NEOC_ERROR_INVALID_TYPE;
    }
    
    if (item->value.integer.length > sizeof(int64_t)) {
        return NEOC_ERROR_OVERFLOW;
    }
    
    *result = 0;
    for (size_t i = 0; i < item->value.integer.length; i++) {
        *result |= ((int64_t)item->value.integer.bytes[i]) << (i * 8);
    }
    
    if (item->value.integer.is_negative) {
        *result = -*result;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t stack_item_to_big_integer(const stack_item_t* item,
                                       uint8_t* bytes,
                                       size_t* length,
                                       bool* is_negative) {
    if (!item || !bytes || !length) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (item->type != STACK_ITEM_TYPE_INTEGER) {
        return NEOC_ERROR_INVALID_TYPE;
    }
    if (*length < item->value.integer.length) {
        *length = item->value.integer.length;
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    if (item->value.integer.length > 0) {
        memcpy(bytes, item->value.integer.bytes, item->value.integer.length);
    }
    *length = item->value.integer.length;
    if (is_negative) {
        *is_negative = item->value.integer.is_negative;
    }
    return NEOC_SUCCESS;
}

neoc_error_t stack_item_to_byte_array(const stack_item_t* item,
                                      uint8_t* data,
                                      size_t* length) {
    if (!item || !data || !length) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    size_t required = 0;
    const uint8_t *source = NULL;
    switch (item->type) {
        case STACK_ITEM_TYPE_BYTE_STRING:
        case STACK_ITEM_TYPE_BUFFER:
            required = item->value.byte_string.length;
            source = item->value.byte_string.data;
            break;
        default:
            return NEOC_ERROR_INVALID_TYPE;
    }
    if (*length < required) {
        *length = required;
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    if (required > 0 && source) {
        memcpy(data, source, required);
    }
    *length = required;
    return NEOC_SUCCESS;
}

// Get array item count
size_t stack_item_array_count(const stack_item_t* item) {
    if (!item) return 0;
    
    if (item->type == STACK_ITEM_TYPE_ARRAY || item->type == STACK_ITEM_TYPE_STRUCT) {
        return item->value.array.count;
    }
    
    return 0;
}

// Get array item at index
stack_item_t* stack_item_array_get(const stack_item_t* item, size_t index) {
    if (!item) return NULL;
    
    if (item->type != STACK_ITEM_TYPE_ARRAY && item->type != STACK_ITEM_TYPE_STRUCT) {
        return NULL;
    }
    
    if (index >= item->value.array.count) {
        return NULL;
    }
    
    return item->value.array.items[index];
}

// Add item to array
neoc_error_t stack_item_array_add(stack_item_t* array, stack_item_t* item) {
    if (!array || !item) return NEOC_ERROR_INVALID_PARAM;
    
    if (array->type != STACK_ITEM_TYPE_ARRAY && array->type != STACK_ITEM_TYPE_STRUCT) {
        return NEOC_ERROR_INVALID_TYPE;
    }
    
    // Resize if needed
    if (array->value.array.count >= array->value.array.capacity) {
        size_t new_capacity = array->value.array.capacity * 2;
        stack_item_t** new_items = realloc(array->value.array.items, 
                                          new_capacity * sizeof(stack_item_t*));
        if (!new_items) return NEOC_ERROR_OUT_OF_MEMORY;
        
        array->value.array.items = new_items;
        array->value.array.capacity = new_capacity;
    }
    
    stack_item_ref(item);
    array->value.array.items[array->value.array.count++] = item;
    
    return NEOC_SUCCESS;
}

// Get map entry count
size_t stack_item_map_count(const stack_item_t* item) {
    if (!item || item->type != STACK_ITEM_TYPE_MAP) return 0;
    return item->value.map.count;
}

// Get value for key in map
stack_item_t* stack_item_map_get(const stack_item_t* map, const stack_item_t* key) {
    if (!map || !key || map->type != STACK_ITEM_TYPE_MAP) return NULL;
    
    for (size_t i = 0; i < map->value.map.count; i++) {
        if (stack_item_equals(map->value.map.entries[i].key, key)) {
            return map->value.map.entries[i].value;
        }
    }
    
    return NULL;
}

// Set key-value pair in map
neoc_error_t stack_item_map_set(stack_item_t* map, stack_item_t* key, stack_item_t* value) {
    if (!map || !key || !value) return NEOC_ERROR_INVALID_PARAM;
    
    if (map->type != STACK_ITEM_TYPE_MAP) {
        return NEOC_ERROR_INVALID_TYPE;
    }
    
    // Check if key exists
    for (size_t i = 0; i < map->value.map.count; i++) {
        if (stack_item_equals(map->value.map.entries[i].key, key)) {
            // Replace value
            stack_item_unref(map->value.map.entries[i].value);
            stack_item_ref(value);
            map->value.map.entries[i].value = value;
            return NEOC_SUCCESS;
        }
    }
    
    // Add new entry
    if (map->value.map.count >= map->value.map.capacity) {
        size_t new_capacity = map->value.map.capacity * 2;
        stack_item_map_entry_t* new_entries = realloc(map->value.map.entries,
                                                      new_capacity * sizeof(stack_item_map_entry_t));
        if (!new_entries) return NEOC_ERROR_OUT_OF_MEMORY;
        
        map->value.map.entries = new_entries;
        map->value.map.capacity = new_capacity;
    }
    
    stack_item_ref(key);
    stack_item_ref(value);
    map->value.map.entries[map->value.map.count].key = key;
    map->value.map.entries[map->value.map.count].value = value;
    map->value.map.count++;
    
    return NEOC_SUCCESS;
}

// Check equality of two stack items
bool stack_item_equals(const stack_item_t* a, const stack_item_t* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->type != b->type) return false;
    
    switch (a->type) {
        case STACK_ITEM_TYPE_ANY:
            return true;
            
        case STACK_ITEM_TYPE_BOOLEAN:
            return a->value.boolean_value == b->value.boolean_value;
            
        case STACK_ITEM_TYPE_INTEGER:
            if (a->value.integer.length != b->value.integer.length) return false;
            if (a->value.integer.is_negative != b->value.integer.is_negative) return false;
            return memcmp(a->value.integer.bytes, b->value.integer.bytes, 
                         a->value.integer.length) == 0;
            
        case STACK_ITEM_TYPE_BYTE_STRING:
        case STACK_ITEM_TYPE_BUFFER:
            if (a->value.byte_string.length != b->value.byte_string.length) return false;
            if (a->value.byte_string.length == 0) return true;
            return memcmp(a->value.byte_string.data, b->value.byte_string.data,
                         a->value.byte_string.length) == 0;
            
        case STACK_ITEM_TYPE_ARRAY:
        case STACK_ITEM_TYPE_STRUCT:
            if (a->value.array.count != b->value.array.count) return false;
            for (size_t i = 0; i < a->value.array.count; i++) {
                if (!stack_item_equals(a->value.array.items[i], b->value.array.items[i])) {
                    return false;
                }
            }
            return true;
            
        case STACK_ITEM_TYPE_MAP:
            if (a->value.map.count != b->value.map.count) return false;
            // Maps need more complex equality check
            for (size_t i = 0; i < a->value.map.count; i++) {
                stack_item_t* value = stack_item_map_get(b, a->value.map.entries[i].key);
                if (!value || !stack_item_equals(a->value.map.entries[i].value, value)) {
                    return false;
                }
            }
            return true;
            
        case STACK_ITEM_TYPE_POINTER:
            return a->value.pointer.ptr == b->value.pointer.ptr &&
                   a->value.pointer.position == b->value.pointer.position;
            
        case STACK_ITEM_TYPE_INTEROP_INTERFACE:
            return a->value.interop_interface == b->value.interop_interface;
            
        default:
            return false;
    }
}

// Get human-readable type name
const char* stack_item_type_name(stack_item_type_t type) {
    switch (type) {
        case STACK_ITEM_TYPE_ANY: return "Any";
        case STACK_ITEM_TYPE_POINTER: return "Pointer";
        case STACK_ITEM_TYPE_BOOLEAN: return "Boolean";
        case STACK_ITEM_TYPE_INTEGER: return "Integer";
        case STACK_ITEM_TYPE_BYTE_STRING: return "ByteString";
        case STACK_ITEM_TYPE_BUFFER: return "Buffer";
        case STACK_ITEM_TYPE_ARRAY: return "Array";
        case STACK_ITEM_TYPE_STRUCT: return "Struct";
        case STACK_ITEM_TYPE_MAP: return "Map";
        case STACK_ITEM_TYPE_INTEROP_INTERFACE: return "InteropInterface";
        default: return "Unknown";
    }
}

// Additional functions would be implemented here...
// For brevity, I'm showing the core implementation pattern
