/**
 * @file stack_item.h
 * @brief Neo VM Stack Item types and operations
 * 
 * Complete implementation of Neo VM stack items including:
 * - All stack item types (Any, Boolean, Integer, ByteString, Array, Map, etc.)
 * - Type conversion and validation
 * - Serialization/deserialization
 * - Deep cloning and equality comparison
 */

#ifndef NEOC_STACK_ITEM_H
#define NEOC_STACK_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_types.h"

/**
 * Stack item types in Neo VM
 */
typedef enum {
    STACK_ITEM_TYPE_ANY = 0x00,
    STACK_ITEM_TYPE_POINTER = 0x10,
    STACK_ITEM_TYPE_BOOLEAN = 0x20,
    STACK_ITEM_TYPE_INTEGER = 0x21,
    STACK_ITEM_TYPE_BYTE_STRING = 0x28,
    STACK_ITEM_TYPE_BUFFER = 0x30,
    STACK_ITEM_TYPE_ARRAY = 0x40,
    STACK_ITEM_TYPE_STRUCT = 0x41,
    STACK_ITEM_TYPE_MAP = 0x48,
    STACK_ITEM_TYPE_INTEROP_INTERFACE = 0x60
} stack_item_type_t;

/**
 * Forward declaration of stack item
 */
typedef struct stack_item stack_item_t;
typedef stack_item_t neoc_stack_item_t; // Alias for API consistency

/**
 * Map entry for Map stack items
 */
typedef struct {
    stack_item_t* key;
    stack_item_t* value;
} stack_item_map_entry_t;

/**
 * Stack item structure
 */
struct stack_item {
    stack_item_type_t type;
    size_t ref_count;
    
    union {
        // Boolean value
        bool boolean_value;
        
        // Integer value (big integer)
        struct {
            uint8_t* bytes;
            size_t length;
            bool is_negative;
        } integer;
        
        // Byte string or buffer
        struct {
            uint8_t* data;
            size_t length;
        } byte_string;
        
        // Array or struct
        struct {
            stack_item_t** items;
            size_t count;
            size_t capacity;
        } array;
        
        // Map
        struct {
            stack_item_map_entry_t* entries;
            size_t count;
            size_t capacity;
        } map;
        
        // Pointer
        struct {
            void* ptr;
            size_t position;
        } pointer;
        
        // Interop interface
        void* interop_interface;
    } value;
};

/**
 * @brief Create an Any stack item (null)
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_any(void);

/**
 * @brief Create a Boolean stack item
 * @param value Boolean value
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_boolean(bool value);

/**
 * @brief Create an Integer stack item from int64
 * @param value Integer value
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_integer(int64_t value);

/**
 * @brief Create an Integer stack item from big integer bytes
 * @param bytes Big integer bytes (little-endian)
 * @param length Number of bytes
 * @param is_negative True if negative
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_big_integer(const uint8_t* bytes, size_t length, bool is_negative);

/**
 * @brief Create a ByteString stack item
 * @param data Byte data
 * @param length Data length
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_byte_string(const uint8_t* data, size_t length);

/**
 * @brief Create a Buffer stack item
 * @param data Initial data (can be NULL)
 * @param length Data length
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_buffer(const uint8_t* data, size_t length);

/**
 * @brief Create an Array stack item
 * @param initial_capacity Initial capacity (0 for default)
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_array(size_t initial_capacity);

/**
 * @brief Create a Struct stack item
 * @param initial_capacity Initial capacity (0 for default)
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_struct(size_t initial_capacity);

/**
 * @brief Create a Map stack item
 * @param initial_capacity Initial capacity (0 for default)
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_map(size_t initial_capacity);

/**
 * @brief Create a Pointer stack item
 * @param ptr Pointer value
 * @param position Position value
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_pointer(void* ptr, size_t position);

/**
 * @brief Create an InteropInterface stack item
 * @param interface Interface pointer
 * @return New stack item or NULL on error
 */
stack_item_t* stack_item_create_interop_interface(void* interface);

/**
 * @brief Increase reference count
 * @param item Stack item
 */
void stack_item_ref(stack_item_t* item);

/**
 * @brief Decrease reference count and free if zero
 * @param item Stack item
 */
void stack_item_unref(stack_item_t* item);

/**
 * @brief Free a stack item (decreases ref count)
 * @param item Stack item to free
 */
void stack_item_free(stack_item_t* item);

/**
 * @brief Get the type of a stack item
 * @param item Stack item
 * @return Stack item type
 */
stack_item_type_t stack_item_get_type(const stack_item_t* item);

/**
 * @brief Check if stack item is null (Any type)
 * @param item Stack item
 * @return True if null
 */
bool stack_item_is_null(const stack_item_t* item);

/**
 * @brief Convert stack item to boolean
 * @param item Stack item
 * @param result Output boolean value
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_to_boolean(const stack_item_t* item, bool* result);

/**
 * @brief Convert stack item to integer
 * @param item Stack item
 * @param result Output integer value
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_to_integer(const stack_item_t* item, int64_t* result);

/**
 * @brief Convert stack item to big integer bytes
 * @param item Stack item
 * @param bytes Output buffer for bytes (little-endian)
 * @param length Buffer size on input, bytes written on output
 * @param is_negative Output flag for negative value
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_to_big_integer(const stack_item_t* item, 
                                       uint8_t* bytes, size_t* length,
                                       bool* is_negative);

/**
 * @brief Convert stack item to byte array
 * @param item Stack item
 * @param data Output buffer
 * @param length Buffer size on input, bytes written on output
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_to_byte_array(const stack_item_t* item, 
                                      uint8_t* data, size_t* length);

/**
 * @brief Get array item count
 * @param item Array or Struct stack item
 * @return Item count or 0 if not array/struct
 */
size_t stack_item_array_count(const stack_item_t* item);

/**
 * @brief Get array item at index
 * @param item Array or Struct stack item
 * @param index Item index
 * @return Stack item at index or NULL if invalid
 */
stack_item_t* stack_item_array_get(const stack_item_t* item, size_t index);

/**
 * @brief Add item to array
 * @param array Array or Struct stack item
 * @param item Item to add (reference count increased)
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_array_add(stack_item_t* array, stack_item_t* item);

/**
 * @brief Set array item at index
 * @param array Array or Struct stack item
 * @param index Item index
 * @param item New item (reference count increased)
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_array_set(stack_item_t* array, size_t index, stack_item_t* item);

/**
 * @brief Remove array item at index
 * @param array Array or Struct stack item
 * @param index Item index
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_array_remove(stack_item_t* array, size_t index);

/**
 * @brief Clear all array items
 * @param array Array or Struct stack item
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_array_clear(stack_item_t* array);

/**
 * @brief Get map entry count
 * @param item Map stack item
 * @return Entry count or 0 if not map
 */
size_t stack_item_map_count(const stack_item_t* item);

/**
 * @brief Get value for key in map
 * @param map Map stack item
 * @param key Key stack item
 * @return Value stack item or NULL if not found
 */
stack_item_t* stack_item_map_get(const stack_item_t* map, const stack_item_t* key);

/**
 * @brief Set key-value pair in map
 * @param map Map stack item
 * @param key Key stack item (reference count increased)
 * @param value Value stack item (reference count increased)
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_map_set(stack_item_t* map, stack_item_t* key, stack_item_t* value);

/**
 * @brief Remove key from map
 * @param map Map stack item
 * @param key Key stack item
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_map_remove(stack_item_t* map, const stack_item_t* key);

/**
 * @brief Clear all map entries
 * @param map Map stack item
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_map_clear(stack_item_t* map);

/**
 * @brief Check if map contains key
 * @param map Map stack item
 * @param key Key stack item
 * @return True if key exists
 */
bool stack_item_map_contains(const stack_item_t* map, const stack_item_t* key);

/**
 * @brief Get all map keys
 * @param map Map stack item
 * @param keys Output array for keys (must be freed by caller)
 * @param count Output key count
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_map_get_keys(const stack_item_t* map, 
                                     stack_item_t*** keys, size_t* count);

/**
 * @brief Get all map values
 * @param map Map stack item
 * @param values Output array for values (must be freed by caller)
 * @param count Output value count
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_map_get_values(const stack_item_t* map,
                                       stack_item_t*** values, size_t* count);

/**
 * @brief Deep clone a stack item
 * @param item Stack item to clone
 * @return Cloned stack item or NULL on error
 */
stack_item_t* stack_item_clone(const stack_item_t* item);

/**
 * @brief Check equality of two stack items
 * @param a First stack item
 * @param b Second stack item
 * @return True if equal
 */
bool stack_item_equals(const stack_item_t* a, const stack_item_t* b);

/**
 * @brief Serialize stack item to bytes
 * @param item Stack item
 * @param data Output buffer
 * @param length Buffer size on input, bytes written on output
 * @return NEOC_SUCCESS or error code
 */
neoc_error_t stack_item_serialize(const stack_item_t* item, 
                                  uint8_t* data, size_t* length);

/**
 * @brief Deserialize stack item from bytes
 * @param data Input data
 * @param length Data length
 * @return Stack item or NULL on error
 */
stack_item_t* stack_item_deserialize(const uint8_t* data, size_t length);

/**
 * @brief Convert stack item to JSON string
 * @param item Stack item
 * @return JSON string (must be freed by caller) or NULL on error
 */
char* stack_item_to_json(const stack_item_t* item);

/**
 * @brief Create stack item from JSON string
 * @param json JSON string
 * @return Stack item or NULL on error
 */
stack_item_t* stack_item_from_json(const char* json);

/**
 * @brief Get human-readable type name
 * @param type Stack item type
 * @return Type name string
 */
const char* stack_item_type_name(stack_item_type_t type);

// Aliases for API consistency
#define neoc_stack_item_free stack_item_free
#define neoc_stack_item_clone stack_item_clone
#define neoc_stack_item_equals stack_item_equals
#define neoc_stack_item_to_json stack_item_to_json
#define neoc_stack_item_from_json stack_item_from_json
#define neoc_stack_item_create_any stack_item_create_any
#define neoc_stack_item_create_boolean stack_item_create_boolean
#define neoc_stack_item_create_integer stack_item_create_integer
#define neoc_stack_item_create_byte_string stack_item_create_byte_string
#define neoc_stack_item_create_array stack_item_create_array

#ifdef __cplusplus
}
#endif

#endif // NEOC_STACK_ITEM_H
