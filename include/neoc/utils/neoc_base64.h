/**
 * @file neoc_base64.h
 * @brief Base64 encoding and decoding utilities for NeoC SDK
 * 
 * Provides functions for Base64 encoding/decoding commonly used
 * in web protocols and data serialization.
 */

#ifndef NEOC_BASE64_H
#define NEOC_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

/**
 * @brief Base64 alphabet used for encoding
 */
extern const char NEOC_BASE64_ALPHABET[64];

/**
 * @brief Check if a character is a valid Base64 character
 * 
 * @param c Character to check
 * @return true if valid Base64 character, false otherwise
 */
bool neoc_base64_is_valid_char(char c);

/**
 * @brief Check if a string is a valid Base64 string
 * 
 * @param str String to check
 * @return true if valid Base64 string, false otherwise
 */
bool neoc_base64_is_valid_string(const char* str);

/**
 * @brief Calculate required buffer size for Base64 encoding
 * 
 * @param data_length Length of binary data
 * @return Required buffer size (including null terminator)
 */
size_t neoc_base64_encode_buffer_size(size_t data_length);

/**
 * @brief Calculate required buffer size for Base64 decoding
 * 
 * @param base64_string Base64 string
 * @return Required buffer size for decoded data
 */
size_t neoc_base64_decode_buffer_size(const char* base64_string);

/**
 * @brief Encode binary data to Base64 string
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @param buffer Output buffer for Base64 string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base64_encode(const uint8_t* data, size_t data_length,
                               char* buffer, size_t buffer_size);

/**
 * @brief Decode Base64 string to binary data
 * 
 * @param base64_string Base64 string to decode
 * @param buffer Output buffer for binary data
 * @param buffer_size Size of output buffer
 * @param decoded_length Pointer to store actual decoded length (can be NULL)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base64_decode(const char* base64_string,
                               uint8_t* buffer, size_t buffer_size,
                               size_t* decoded_length);

/**
 * @brief Allocate and encode binary data to Base64 string
 * 
 * The returned string must be freed with neoc_free().
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @return Allocated Base64 string on success, NULL on failure
 */
char* neoc_base64_encode_alloc(const uint8_t* data, size_t data_length);

/**
 * @brief Allocate and decode Base64 string to binary data
 * 
 * The returned data must be freed with neoc_free().
 * 
 * @param base64_string Base64 string to decode
 * @param decoded_length Pointer to store decoded length (can be NULL)
 * @return Allocated binary data on success, NULL on failure
 */
uint8_t* neoc_base64_decode_alloc(const char* base64_string, size_t* decoded_length);

/**
 * @brief Encode binary data to Base64 URL-safe string
 * 
 * Uses URL-safe Base64 alphabet (- and _ instead of + and /).
 * 
 * @param data Binary data to encode
 * @param data_length Length of binary data
 * @param buffer Output buffer for Base64 string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base64_url_encode(const uint8_t* data, size_t data_length,
                                   char* buffer, size_t buffer_size);

/**
 * @brief Decode Base64 URL-safe string to binary data
 * 
 * @param base64_string Base64 URL-safe string to decode
 * @param buffer Output buffer for binary data
 * @param buffer_size Size of output buffer
 * @param decoded_length Pointer to store actual decoded length (can be NULL)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_base64_url_decode(const char* base64_string,
                                   uint8_t* buffer, size_t buffer_size,
                                   size_t* decoded_length);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_BASE64_H */
