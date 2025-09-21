/**
 * @file neoc_hex.h
 * @brief Hexadecimal encoding and decoding utilities for NeoC SDK
 * 
 * Provides functions for converting between binary data and hexadecimal
 * string representations with proper validation and error handling.
 */

#ifndef NEOC_HEX_H
#define NEOC_HEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

/**
 * @brief Check if a character is a valid hexadecimal digit
 * 
 * @param c Character to check
 * @return true if valid hex digit, false otherwise
 */
bool neoc_hex_is_valid_char(char c);

/**
 * @brief Check if a string is a valid hexadecimal string
 * 
 * @param str String to check
 * @param allow_0x_prefix Allow "0x" prefix if true
 * @return true if valid hex string, false otherwise
 */
bool neoc_hex_is_valid_string(const char* str, bool allow_0x_prefix);

/**
 * @brief Get the value of a hexadecimal digit
 * 
 * @param c Hexadecimal character ('0'-'9', 'A'-'F', 'a'-'f')
 * @return Value (0-15) on success, -1 on invalid character
 */
int neoc_hex_char_to_value(char c);

/**
 * @brief Convert a value to hexadecimal character
 * 
 * @param value Value (0-15)
 * @param uppercase Use uppercase if true
 * @return Hexadecimal character, or '\0' on invalid value
 */
char neoc_hex_value_to_char(int value, bool uppercase);

/**
 * @brief Calculate the required buffer size for hex encoding
 * 
 * @param data_length Length of binary data
 * @param include_prefix Include "0x" prefix in calculation
 * @return Required buffer size (including null terminator)
 */
size_t neoc_hex_encode_buffer_size(size_t data_length, bool include_prefix);

/**
 * @brief Calculate the required buffer size for hex decoding
 * 
 * @param hex_string Hexadecimal string
 * @return Required buffer size for decoded data
 */
size_t neoc_hex_decode_buffer_size(const char* hex_string);

/**
 * @brief Encode binary data to hexadecimal string
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @param buffer Output buffer for hex string
 * @param buffer_size Size of output buffer
 * @param uppercase Use uppercase letters if true
 * @param include_prefix Include "0x" prefix if true
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hex_encode(const uint8_t* data, size_t data_length,
                            char* buffer, size_t buffer_size,
                            bool uppercase, bool include_prefix);

/**
 * @brief Decode hexadecimal string to binary data
 * 
 * @param hex_string Hexadecimal string to decode (with or without "0x" prefix)
 * @param buffer Output buffer for binary data
 * @param buffer_size Size of output buffer
 * @param decoded_length Pointer to store actual decoded length (can be NULL)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hex_decode(const char* hex_string,
                            uint8_t* buffer, size_t buffer_size,
                            size_t* decoded_length);

/**
 * @brief Allocate and encode binary data to hexadecimal string
 * 
 * The returned string must be freed with neoc_free().
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @param uppercase Use uppercase letters if true
 * @param include_prefix Include "0x" prefix if true
 * @return Allocated hex string on success, NULL on failure
 */
char* neoc_hex_encode_alloc(const uint8_t* data, size_t data_length,
                           bool uppercase, bool include_prefix);

/**
 * @brief Allocate and decode hexadecimal string to binary data
 * 
 * The returned data must be freed with neoc_free().
 * 
 * @param hex_string Hexadecimal string to decode
 * @param decoded_length Pointer to store decoded length (can be NULL)
 * @return Allocated binary data on success, NULL on failure
 */
uint8_t* neoc_hex_decode_alloc(const char* hex_string, size_t* decoded_length);

/**
 * @brief Normalize hexadecimal string (remove prefix, convert to lowercase/uppercase)
 * 
 * @param hex_string Input hexadecimal string
 * @param buffer Output buffer for normalized string
 * @param buffer_size Size of output buffer
 * @param uppercase Convert to uppercase if true, lowercase if false
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hex_normalize(const char* hex_string,
                               char* buffer, size_t buffer_size,
                               bool uppercase);

/**
 * @brief Compare two hexadecimal strings (case-insensitive)
 * 
 * Handles "0x" prefix automatically.
 * 
 * @param hex1 First hexadecimal string
 * @param hex2 Second hexadecimal string
 * @return 0 if equal, negative if hex1 < hex2, positive if hex1 > hex2
 */
int neoc_hex_compare(const char* hex1, const char* hex2);

/**
 * @brief Check if two hexadecimal strings are equal (case-insensitive)
 * 
 * @param hex1 First hexadecimal string
 * @param hex2 Second hexadecimal string
 * @return true if equal, false otherwise
 */
bool neoc_hex_equal(const char* hex1, const char* hex2);

/* Convenience macros */

/**
 * @brief Encode data to lowercase hex without prefix
 */
#define neoc_hex_encode_simple(data, len, buf, buf_size) \
    neoc_hex_encode((data), (len), (buf), (buf_size), false, false)

/**
 * @brief Encode data to uppercase hex without prefix
 */
#define neoc_hex_encode_upper(data, len, buf, buf_size) \
    neoc_hex_encode((data), (len), (buf), (buf_size), true, false)

/**
 * @brief Encode data to lowercase hex with "0x" prefix
 */
#define neoc_hex_encode_prefixed(data, len, buf, buf_size) \
    neoc_hex_encode((data), (len), (buf), (buf_size), false, true)

/**
 * @brief Decode hex string (handles prefix automatically)
 */
#define neoc_hex_decode_simple(hex, buf, buf_size) \
    neoc_hex_decode((hex), (buf), (buf_size), NULL)

#ifdef __cplusplus
}
#endif

#endif /* NEOC_HEX_H */
