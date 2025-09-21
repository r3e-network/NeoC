/**
 * @file neoc_types.h
 * @brief Basic type definitions for NeoC SDK
 * 
 * Defines fundamental types used throughout the SDK.
 */

#ifndef NEOC_TYPES_H
#define NEOC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

/**
 * @brief Byte type definition
 */
typedef uint8_t neoc_byte_t;

/**
 * @brief Byte array structure
 * 
 * Represents a dynamically allocated array of bytes with length tracking.
 * Includes memory management functions for safe allocation/deallocation.
 */
typedef struct {
    neoc_byte_t* data;      ///< Pointer to byte data
    size_t length;          ///< Length of the data in bytes
    size_t capacity;        ///< Allocated capacity (for reallocation)
} neoc_bytes_t;

/**
 * @brief Create a new byte array
 * 
 * @param length Initial length of the array
 * @return Pointer to new byte array, or NULL on failure
 */
neoc_bytes_t* neoc_bytes_create(size_t length);

/**
 * @brief Create a byte array from existing data
 * 
 * @param data Source data to copy
 * @param length Length of source data
 * @return Pointer to new byte array, or NULL on failure
 */
neoc_bytes_t* neoc_bytes_from_data(const neoc_byte_t* data, size_t length);

/**
 * @brief Create a byte array from hexadecimal string
 * 
 * @param hex_string Hexadecimal string (with or without "0x" prefix)
 * @return Pointer to new byte array, or NULL on failure
 */
neoc_bytes_t* neoc_bytes_from_hex(const char* hex_string);

/**
 * @brief Duplicate a byte array
 * 
 * @param bytes Source byte array
 * @return Pointer to new byte array, or NULL on failure
 */
neoc_bytes_t* neoc_bytes_duplicate(const neoc_bytes_t* bytes);

/**
 * @brief Resize a byte array
 * 
 * @param bytes Byte array to resize
 * @param new_length New length for the array
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_resize(neoc_bytes_t* bytes, size_t new_length);

/**
 * @brief Append data to a byte array
 * 
 * @param bytes Target byte array
 * @param data Data to append
 * @param length Length of data to append
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_append(neoc_bytes_t* bytes, const neoc_byte_t* data, size_t length);

/**
 * @brief Reverse the byte order in a byte array
 * 
 * @param bytes Byte array to reverse (modified in place)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_reverse(neoc_bytes_t* bytes);

/**
 * @brief Create a reversed copy of a byte array
 * 
 * @param bytes Source byte array
 * @return Pointer to new reversed byte array, or NULL on failure
 */
neoc_bytes_t* neoc_bytes_reversed(const neoc_bytes_t* bytes);

/**
 * @brief Compare two byte arrays
 * 
 * @param a First byte array
 * @param b Second byte array
 * @return 0 if equal, negative if a < b, positive if a > b
 */
int neoc_bytes_compare(const neoc_bytes_t* a, const neoc_bytes_t* b);

/**
 * @brief Check if two byte arrays are equal
 * 
 * @param a First byte array
 * @param b Second byte array
 * @return true if equal, false otherwise
 */
bool neoc_bytes_equal(const neoc_bytes_t* a, const neoc_bytes_t* b);

/**
 * @brief Convert byte array to hexadecimal string
 * 
 * @param bytes Source byte array
 * @param uppercase Use uppercase letters if true
 * @return Allocated hexadecimal string, or NULL on failure (caller must free)
 */
char* neoc_bytes_to_hex(const neoc_bytes_t* bytes, bool uppercase);

/**
 * @brief Clear byte array data (set all bytes to zero)
 * 
 * @param bytes Byte array to clear
 */
void neoc_bytes_clear(neoc_bytes_t* bytes);

/**
 * @brief Free a byte array and its data
 * 
 * @param bytes Byte array to free (can be NULL)
 */
void neoc_bytes_free(neoc_bytes_t* bytes);

/**
 * @brief Securely free a byte array (clear data before freeing)
 * 
 * @param bytes Byte array to free (can be NULL)
 */
void neoc_bytes_secure_free(neoc_bytes_t* bytes);

/* Convenience functions for common operations */

/**
 * @brief Create empty byte array
 */
#define neoc_bytes_empty() neoc_bytes_create(0)

/**
 * @brief Get byte at specific index
 */
#define neoc_bytes_at(bytes, index) \
    (((bytes) && (index) < (bytes)->length) ? (bytes)->data[index] : 0)

/**
 * @brief Set byte at specific index
 */
#define neoc_bytes_set(bytes, index, value) \
    do { \
        if ((bytes) && (index) < (bytes)->length) { \
            (bytes)->data[index] = (value); \
        } \
    } while(0)

/**
 * @brief Check if byte array is empty
 */
#define neoc_bytes_is_empty(bytes) \
    (!(bytes) || (bytes)->length == 0)

/**
 * @brief Get length of byte array
 */
#define neoc_bytes_length(bytes) \
    ((bytes) ? (bytes)->length : 0)

#ifdef __cplusplus
}
#endif

#endif /* NEOC_TYPES_H */
