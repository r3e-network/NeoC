/**
 * @file stack_item.h
 * @brief Neo VM stack item implementation for C
 * 
 * Based on Swift source: protocol/core/stackitem/StackItem.swift
 * Provides complete stack item types and manipulation functions
 */

#ifndef NEOC_PROTOCOL_CORE_STACKITEM_STACK_ITEM_H
#define NEOC_PROTOCOL_CORE_STACKITEM_STACK_ITEM_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_types.h"
#include "neoc/types/hash160.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Stack item type constants
 */
#define NEOC_STACK_ITEM_ANY_VALUE "Any"
#define NEOC_STACK_ITEM_POINTER_VALUE "Pointer"
#define NEOC_STACK_ITEM_BOOLEAN_VALUE "Boolean"
#define NEOC_STACK_ITEM_INTEGER_VALUE "Integer"
#define NEOC_STACK_ITEM_BYTE_STRING_VALUE "ByteString"
#define NEOC_STACK_ITEM_BUFFER_VALUE "Buffer"
#define NEOC_STACK_ITEM_ARRAY_VALUE "Array"
#define NEOC_STACK_ITEM_STRUCT_VALUE "Struct"
#define NEOC_STACK_ITEM_MAP_VALUE "Map"
#define NEOC_STACK_ITEM_INTEROP_INTERFACE_VALUE "InteropInterface"

/**
 * @brief Stack item type bytes
 */
#define NEOC_STACK_ITEM_ANY_BYTE 0x00
#define NEOC_STACK_ITEM_POINTER_BYTE 0x10
#define NEOC_STACK_ITEM_BOOLEAN_BYTE 0x20
#define NEOC_STACK_ITEM_INTEGER_BYTE 0x21
#define NEOC_STACK_ITEM_BYTE_STRING_BYTE 0x28
#define NEOC_STACK_ITEM_BUFFER_BYTE 0x30
#define NEOC_STACK_ITEM_ARRAY_BYTE 0x40
#define NEOC_STACK_ITEM_STRUCT_BYTE 0x41
#define NEOC_STACK_ITEM_MAP_BYTE 0x48
#define NEOC_STACK_ITEM_INTEROP_INTERFACE_BYTE 0x60

/**
 * @brief Stack item type enumeration
 */
typedef enum {
    NEOC_STACK_ITEM_TYPE_ANY,               /**< Any type */
    NEOC_STACK_ITEM_TYPE_POINTER,           /**< Pointer type */
    NEOC_STACK_ITEM_TYPE_BOOLEAN,           /**< Boolean type */
    NEOC_STACK_ITEM_TYPE_INTEGER,           /**< Integer type */
    NEOC_STACK_ITEM_TYPE_BYTE_STRING,       /**< Byte string type */
    NEOC_STACK_ITEM_TYPE_BUFFER,            /**< Buffer type */
    NEOC_STACK_ITEM_TYPE_ARRAY,             /**< Array type */
    NEOC_STACK_ITEM_TYPE_STRUCT,            /**< Struct type */
    NEOC_STACK_ITEM_TYPE_MAP,               /**< Map type */
    NEOC_STACK_ITEM_TYPE_INTEROP_INTERFACE  /**< Interop interface type */
} neoc_stack_item_type_t;

/**
 * @brief Forward declarations
 */
typedef struct neoc_stack_item_t neoc_stack_item_t;
typedef struct neoc_stack_item_map_entry_t neoc_stack_item_map_entry_t;

/**
 * @brief Map entry structure for stack item maps
 */
struct neoc_stack_item_map_entry_t {
    neoc_stack_item_t *key;                 /**< Map key */
    neoc_stack_item_t *value;               /**< Map value */
};

/**
 * @brief Interop interface data
 */
typedef struct {
    char *iterator_id;                      /**< Iterator ID */
    char *interface_name;                   /**< Interface name */
} neoc_stack_item_interop_t;

/**
 * @brief Stack item structure
 * 
 * Based on Swift StackItem indirect enum
 * Represents all possible Neo VM stack item types
 */
struct neoc_stack_item_t {
    neoc_stack_item_type_t type;            /**< Item type */
    union {
        void *any_value;                    /**< Any value pointer */
        int64_t pointer_value;              /**< Pointer value */
        bool boolean_value;                 /**< Boolean value */
        int64_t integer_value;              /**< Integer value (can be big) */
        neoc_bytes_t byte_string_value;     /**< Byte string value */
        neoc_bytes_t buffer_value;          /**< Buffer value */
        struct {
            neoc_stack_item_t **items;      /**< Array items */
            size_t count;                   /**< Array count */
        } array_value;
        struct {
            neoc_stack_item_t **items;      /**< Struct items */
            size_t count;                   /**< Struct count */
        } struct_value;
        struct {
            neoc_stack_item_map_entry_t *entries; /**< Map entries */
            size_t count;                   /**< Map entry count */
        } map_value;
        neoc_stack_item_interop_t interop_value; /**< Interop interface value */
    } value;
};

/**
 * @brief Create a new stack item of ANY type
 * @param any_value Any value pointer (nullable)
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_any(void *any_value);

/**
 * @brief Create a new pointer stack item
 * @param pointer_value Pointer value
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_pointer(int64_t pointer_value);

/**
 * @brief Create a new boolean stack item
 * @param boolean_value Boolean value
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_boolean(bool boolean_value);

/**
 * @brief Create a new integer stack item
 * @param integer_value Integer value
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_integer(int64_t integer_value);

/**
 * @brief Create a new byte string stack item
 * @param bytes Byte data (takes ownership)
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_byte_string(neoc_bytes_t bytes);

/**
 * @brief Create a new buffer stack item
 * @param bytes Byte data (takes ownership)
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_buffer(neoc_bytes_t bytes);

/**
 * @brief Create a new array stack item
 * @param items Array items (takes ownership)
 * @param count Number of items
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_array(neoc_stack_item_t **items, size_t count);

/**
 * @brief Create a new struct stack item
 * @param items Struct items (takes ownership)
 * @param count Number of items
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_struct(neoc_stack_item_t **items, size_t count);

/**
 * @brief Create a new map stack item
 * @param entries Map entries (takes ownership)
 * @param count Number of entries
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_map(neoc_stack_item_map_entry_t *entries, size_t count);

/**
 * @brief Create a new interop interface stack item
 * @param iterator_id Iterator ID string (makes copy)
 * @param interface_name Interface name string (makes copy)
 * @return Newly allocated stack item or NULL on failure
 */
neoc_stack_item_t *neoc_stack_item_create_interop(const char *iterator_id, const char *interface_name);

/**
 * @brief Get stack item type
 * @param item Stack item
 * @return Item type
 */
neoc_stack_item_type_t neoc_stack_item_get_type(const neoc_stack_item_t *item);

/**
 * @brief Get JSON value string for the type
 * @param item Stack item
 * @return JSON type string (static, do not free)
 */
const char *neoc_stack_item_get_json_value(const neoc_stack_item_t *item);

/**
 * @brief Get type byte for the item
 * @param item Stack item
 * @return Type byte
 */
uint8_t neoc_stack_item_get_type_byte(const neoc_stack_item_t *item);

/**
 * @brief Try to get boolean value from stack item
 * @param item Stack item
 * @param bool_out Output boolean value
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_boolean(const neoc_stack_item_t *item, bool *bool_out);

/**
 * @brief Try to get integer value from stack item
 * @param item Stack item
 * @param int_out Output integer value
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_integer(const neoc_stack_item_t *item, int64_t *int_out);

/**
 * @brief Try to get string value from stack item
 * @param item Stack item
 * @param string_out Output string (caller must free)
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_string(const neoc_stack_item_t *item, char **string_out);

/**
 * @brief Try to get hex string value from stack item
 * @param item Stack item
 * @param hex_out Output hex string (caller must free)
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_hex_string(const neoc_stack_item_t *item, char **hex_out);

/**
 * @brief Try to get byte array from stack item
 * @param item Stack item
 * @param bytes_out Output bytes (caller must free with neoc_bytes_free)
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_byte_array(const neoc_stack_item_t *item, neoc_bytes_t *bytes_out);

/**
 * @brief Try to get array from stack item
 * @param item Stack item
 * @param items_out Output items array (do not free)
 * @param count_out Output item count
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_array(const neoc_stack_item_t *item, 
                                      neoc_stack_item_t ***items_out, size_t *count_out);

/**
 * @brief Try to get address string from stack item
 * @param item Stack item
 * @param address_out Output address string (caller must free)
 * @return NEOC_SUCCESS if convertible, error code otherwise
 */
neoc_error_t neoc_stack_item_get_address(const neoc_stack_item_t *item, char **address_out);

/**
 * @brief Convert stack item to string representation
 * @param item Stack item
 * @param max_length Maximum string length (0 for no limit)
 * @return String representation (caller must free)
 */
char *neoc_stack_item_to_string(const neoc_stack_item_t *item, size_t max_length);

/**
 * @brief Parse stack item from JSON
 * @param json JSON string
 * @param item_out Output stack item
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_stack_item_from_json(const char *json, neoc_stack_item_t **item_out);

/**
 * @brief Serialize stack item to JSON
 * @param item Stack item
 * @param json_out Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_stack_item_to_json(const neoc_stack_item_t *item, char **json_out);

/**
 * @brief Free stack item and all nested items
 * @param item Stack item to free
 */
void neoc_stack_item_free(neoc_stack_item_t *item);

/**
 * @brief Free map entry
 * @param entry Map entry to free
 */
void neoc_stack_item_map_entry_free(neoc_stack_item_map_entry_t *entry);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_STACKITEM_STACK_ITEM_H */
