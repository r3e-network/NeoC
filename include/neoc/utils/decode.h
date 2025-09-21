/**
 * @file decode.h
 * @brief Decoding utilities for JSON and string conversions
 * 
 * Converted from Swift source: utils/Decode.swift
 * Provides functionality for safe decoding of JSON strings to various types,
 * equivalent to Swift's StringDecode property wrapper and SafeDecode functionality.
 */

#ifndef NEOC_UTILS_DECODE_H
#define NEOC_UTILS_DECODE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief String to integer conversion with error handling
 * 
 * Equivalent to Swift's Int.init(string:) with StringDecodable protocol.
 * 
 * @param string String representation of integer
 * @param value Pointer to store the decoded integer (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_int_from_string(
    const char *string,
    int *value
);

/**
 * @brief String to int64 conversion with error handling
 * 
 * @param string String representation of int64
 * @param value Pointer to store the decoded int64 (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_int64_from_string(
    const char *string,
    int64_t *value
);

/**
 * @brief String to boolean conversion with error handling
 * 
 * Equivalent to Swift's Bool.init(string:) with StringDecodable protocol.
 * 
 * @param string String representation of boolean ("true"/"false")
 * @param value Pointer to store the decoded boolean (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_bool_from_string(
    const char *string,
    bool *value
);

/**
 * @brief String to double conversion with error handling
 * 
 * @param string String representation of double
 * @param value Pointer to store the decoded double (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_double_from_string(
    const char *string,
    double *value
);

/**
 * @brief Integer to string conversion
 * 
 * @param value Integer value to convert
 * @param string Pointer to store the string representation (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_int_to_string(
    int value,
    char **string
);

/**
 * @brief Int64 to string conversion
 * 
 * @param value Int64 value to convert
 * @param string Pointer to store the string representation (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_int64_to_string(
    int64_t value,
    char **string
);

/**
 * @brief Boolean to string conversion
 * 
 * @param value Boolean value to convert
 * @param string Pointer to store the string representation (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_bool_to_string(
    bool value,
    char **string
);

/**
 * @brief Double to string conversion
 * 
 * @param value Double value to convert
 * @param string Pointer to store the string representation (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_double_to_string(
    double value,
    char **string
);

/**
 * @brief Safe string decoding with fallback
 * 
 * Attempts to decode a value from string, falling back to default if decoding fails.
 * Equivalent to Swift's SafeDecode property wrapper functionality.
 * 
 * @param string String to decode
 * @param default_value Default value to use if decoding fails
 * @param value Pointer to store the decoded or default value (output)
 * @return NEOC_SUCCESS always (uses default on decode failure)
 */
neoc_error_t neoc_decode_safe_int_from_string(
    const char *string,
    int default_value,
    int *value
);

/**
 * @brief Safe boolean decoding with fallback
 * 
 * @param string String to decode
 * @param default_value Default value to use if decoding fails
 * @param value Pointer to store the decoded or default value (output)
 * @return NEOC_SUCCESS always (uses default on decode failure)
 */
neoc_error_t neoc_decode_safe_bool_from_string(
    const char *string,
    bool default_value,
    bool *value
);

/**
 * @brief Parse JSON string value from JSON object field
 * 
 * Helper function to extract string values from JSON, useful for
 * implementing string-decode functionality in JSON parsing.
 * 
 * @param json_str JSON object string
 * @param field_name Field name to extract
 * @param value Pointer to store the extracted string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_json_string_field(
    const char *json_str,
    const char *field_name,
    char **value
);

/**
 * @brief Parse JSON integer value from JSON object field with string fallback
 * 
 * Equivalent to Swift's StringDecode property wrapper functionality.
 * Tries to parse as integer first, then as string-encoded integer.
 * 
 * @param json_str JSON object string
 * @param field_name Field name to extract
 * @param value Pointer to store the decoded integer (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_json_int_field(
    const char *json_str,
    const char *field_name,
    int *value
);

/**
 * @brief Parse JSON boolean value from JSON object field with string fallback
 * 
 * @param json_str JSON object string
 * @param field_name Field name to extract
 * @param value Pointer to store the decoded boolean (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_json_bool_field(
    const char *json_str,
    const char *field_name,
    bool *value
);

/**
 * @brief Validate string is valid integer representation
 * 
 * @param string String to validate
 * @return true if string represents valid integer, false otherwise
 */
bool neoc_decode_is_valid_int_string(
    const char *string
);

/**
 * @brief Validate string is valid boolean representation
 * 
 * @param string String to validate (should be "true" or "false")
 * @return true if string represents valid boolean, false otherwise
 */
bool neoc_decode_is_valid_bool_string(
    const char *string
);

/**
 * @brief Validate string is valid double representation
 * 
 * @param string String to validate
 * @return true if string represents valid double, false otherwise
 */
bool neoc_decode_is_valid_double_string(
    const char *string
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_DECODE_H */
