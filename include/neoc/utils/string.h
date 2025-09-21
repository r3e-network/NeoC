/**
 * @file string.h
 * @brief String utility functions and extensions
 * 
 * Based on Swift source: utils/String.swift
 * Provides string manipulation, encoding/decoding, and validation utilities
 */

#ifndef NEOC_UTILS_STRING_H
#define NEOC_UTILS_STRING_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert hex string to bytes
 * 
 * Equivalent to Swift String.bytesFromHex
 * 
 * @param hex_string Hex string (with or without "0x" prefix)
 * @param bytes Output bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_hex_to_bytes(const char *hex_string,
                                       uint8_t **bytes,
                                       size_t *bytes_len);

/**
 * @brief Clean hex string by removing "0x" prefix if present
 * 
 * Equivalent to Swift String.cleanedHexPrefix
 * 
 * @param hex_string Input hex string
 * @param cleaned_hex Output cleaned hex string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_clean_hex_prefix(const char *hex_string,
                                           char **cleaned_hex);

/**
 * @brief Decode base64 string to bytes
 * 
 * Equivalent to Swift String.base64Decoded
 * 
 * @param base64_string Base64 encoded string
 * @param bytes Output bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_base64_decode(const char *base64_string,
                                        uint8_t **bytes,
                                        size_t *bytes_len);

/**
 * @brief Encode hex string as base64
 * 
 * Equivalent to Swift String.base64Encoded
 * 
 * @param hex_string Hex string to encode
 * @param base64_string Output base64 string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_hex_to_base64(const char *hex_string,
                                        char **base64_string);

/**
 * @brief Decode base58 string to bytes
 * 
 * Equivalent to Swift String.base58Decoded
 * 
 * @param base58_string Base58 encoded string
 * @param bytes Output bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_base58_decode(const char *base58_string,
                                        uint8_t **bytes,
                                        size_t *bytes_len);

/**
 * @brief Decode base58 string with checksum validation
 * 
 * Equivalent to Swift String.base58CheckDecoded
 * 
 * @param base58_string Base58 encoded string with checksum
 * @param bytes Output bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_base58_check_decode(const char *base58_string,
                                              uint8_t **bytes,
                                              size_t *bytes_len);

/**
 * @brief Encode string as base58
 * 
 * Equivalent to Swift String.base58Encoded
 * 
 * @param input_string Input string
 * @param base58_string Output base58 string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_to_base58(const char *input_string,
                                    char **base58_string);

/**
 * @brief Get variable size encoding length for string
 * 
 * Equivalent to Swift String.varSize
 * 
 * @param string Input string
 * @return Variable size encoding length
 */
size_t neoc_string_var_size(const char *string);

/**
 * @brief Validate if string is a valid NEO address
 * 
 * Equivalent to Swift String.isValidAddress
 * 
 * @param address Address string to validate
 * @return True if valid NEO address
 */
bool neoc_string_is_valid_address(const char *address);

/**
 * @brief Validate if string is valid hex
 * 
 * Equivalent to Swift String.isValidHex
 * 
 * @param hex_string Hex string to validate
 * @return True if valid hex string
 */
bool neoc_string_is_valid_hex(const char *hex_string);

/**
 * @brief Convert NEO address to script hash bytes
 * 
 * Equivalent to Swift String.addressToScriptHash()
 * 
 * @param address NEO address string
 * @param script_hash Output script hash bytes (caller must free)
 * @param script_hash_len Output: length of script hash (should be 20)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_address_to_script_hash(const char *address,
                                                 uint8_t **script_hash,
                                                 size_t *script_hash_len);

/**
 * @brief Reverse hex string byte order
 * 
 * Equivalent to Swift String.reversedHex
 * 
 * @param hex_string Input hex string
 * @param reversed_hex Output reversed hex string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_reverse_hex(const char *hex_string,
                                      char **reversed_hex);

/**
 * @brief Check if character is a hex digit
 * 
 * @param c Character to check
 * @return True if hex digit (0-9, a-f, A-F)
 */
bool neoc_string_is_hex_digit(char c);

/**
 * @brief Get UTF-8 byte representation of string
 * 
 * @param string Input string
 * @param bytes Output bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_to_bytes(const char *string,
                                   uint8_t **bytes,
                                   size_t *bytes_len);

/**
 * @brief Convert string to lowercase
 * 
 * @param string Input string
 * @param lowercase_string Output lowercase string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_to_lowercase(const char *string,
                                       char **lowercase_string);

/**
 * @brief Convert string to uppercase
 * 
 * @param string Input string
 * @param uppercase_string Output uppercase string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_to_uppercase(const char *string,
                                       char **uppercase_string);

/**
 * @brief Trim whitespace from string
 * 
 * @param string Input string
 * @param trimmed_string Output trimmed string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_trim(const char *string,
                               char **trimmed_string);

/**
 * @brief Check if string starts with prefix
 * 
 * @param string Input string
 * @param prefix Prefix to check
 * @return True if string starts with prefix
 */
bool neoc_string_starts_with(const char *string, const char *prefix);

/**
 * @brief Check if string ends with suffix
 * 
 * @param string Input string
 * @param suffix Suffix to check
 * @return True if string ends with suffix
 */
bool neoc_string_ends_with(const char *string, const char *suffix);

/**
 * @brief Split string by delimiter
 * 
 * @param string Input string
 * @param delimiter Delimiter character
 * @param parts Output array of string parts (caller must free array and strings)
 * @param parts_count Output: number of parts
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_string_split(const char *string,
                                char delimiter,
                                char ***parts,
                                size_t *parts_count);

/**
 * @brief Free array of strings returned by neoc_string_split
 * 
 * @param parts Array of string parts
 * @param parts_count Number of parts
 */
void neoc_string_split_free(char **parts, size_t parts_count);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_STRING_H */
