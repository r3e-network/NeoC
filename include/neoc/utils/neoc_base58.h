/**
 * @file neoc_base58.h
 * @brief Base58 and Base58Check encoding/decoding utilities for NeoC SDK
 * 
 * Provides functions for Base58 encoding used in cryptocurrency addresses
 * and other applications requiring human-readable encoded binary data.
 */

#ifndef NEOC_BASE58_H
#define NEOC_BASE58_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

/**
 * @brief Length of checksum in Base58Check encoding
 */
#define NEOC_BASE58_CHECKSUM_LENGTH 4

/**
 * @brief Base58 alphabet used for encoding
 */
extern const char NEOC_BASE58_ALPHABET[];

/**
 * @brief Check if a character is a valid Base58 character
 * 
 * @param c Character to check
 * @return true if valid Base58 character, false otherwise
 */
bool neoc_base58_is_valid_char(char c);

/**
 * @brief Check if a string is a valid Base58 string
 * 
 * @param str String to check
 * @return true if valid Base58 string, false otherwise
 */
bool neoc_base58_is_valid_string(const char* str);

/**
 * @brief Calculate required buffer size for Base58 encoding
 * 
 * @param data_length Length of binary data
 * @return Required buffer size (including null terminator)
 */
size_t neoc_base58_encode_buffer_size(size_t data_length);

/**
 * @brief Calculate required buffer size for Base58 decoding
 * 
 * @param base58_string Base58 string
 * @return Required buffer size for decoded data
 */
size_t neoc_base58_decode_buffer_size(const char* base58_string);

/**
 * @brief Encode binary data to Base58 string
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @param buffer Output buffer for Base58 string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base58_encode(const uint8_t* data, size_t data_length,
                               char* buffer, size_t buffer_size);

/**
 * @brief Decode Base58 string to binary data
 * 
 * @param base58_string Base58 string to decode
 * @param buffer Output buffer for binary data
 * @param buffer_size Size of output buffer
 * @param decoded_length Pointer to store actual decoded length (can be NULL)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base58_decode(const char* base58_string,
                               uint8_t* buffer, size_t buffer_size,
                               size_t* decoded_length);

/**
 * @brief Allocate and encode binary data to Base58 string
 * 
 * The returned string must be freed with neoc_free().
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @return Allocated Base58 string on success, NULL on failure
 */
char* neoc_base58_encode_alloc(const uint8_t* data, size_t data_length);

/**
 * @brief Allocate and decode Base58 string to binary data
 * 
 * The returned data must be freed with neoc_free().
 * 
 * @param base58_string Base58 string to decode
 * @param decoded_length Pointer to store decoded length (can be NULL)
 * @return Allocated binary data on success, NULL on failure
 */
uint8_t* neoc_base58_decode_alloc(const char* base58_string, size_t* decoded_length);

/* Base58Check functions */

/**
 * @brief Calculate required buffer size for Base58Check encoding
 * 
 * @param data_length Length of binary data
 * @return Required buffer size (including null terminator)
 */
size_t neoc_base58_check_encode_buffer_size(size_t data_length);

/**
 * @brief Calculate required buffer size for Base58Check decoding
 * 
 * @param base58_string Base58Check string
 * @return Required buffer size for decoded data
 */
size_t neoc_base58_check_decode_buffer_size(const char* base58_string);

/**
 * @brief Encode binary data to Base58Check string
 * 
 * Base58Check adds a 4-byte checksum to the data before encoding.
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @param buffer Output buffer for Base58Check string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base58_check_encode(const uint8_t* data, size_t data_length,
                                     char* buffer, size_t buffer_size);

/**
 * @brief Decode Base58Check string to binary data
 * 
 * Base58Check verifies the checksum before returning the decoded data.
 * 
 * @param base58_string Base58Check string to decode
 * @param buffer Output buffer for binary data
 * @param buffer_size Size of output buffer
 * @param decoded_length Pointer to store actual decoded length (can be NULL)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base58_check_decode(const char* base58_string,
                                     uint8_t* buffer, size_t buffer_size,
                                     size_t* decoded_length);

/**
 * @brief Allocate and encode binary data to Base58Check string
 * 
 * The returned string must be freed with neoc_free().
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @return Allocated Base58Check string on success, NULL on failure
 */
char* neoc_base58_check_encode_alloc(const uint8_t* data, size_t data_length);

/**
 * @brief Allocate and decode Base58Check string to binary data
 * 
 * The returned data must be freed with neoc_free().
 * 
 * @param base58_string Base58Check string to decode
 * @param decoded_length Pointer to store decoded length (can be NULL)
 * @return Allocated binary data on success, NULL on failure
 */
uint8_t* neoc_base58_check_decode_alloc(const char* base58_string, size_t* decoded_length);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_BASE58_H */
