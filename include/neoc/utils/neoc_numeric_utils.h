#ifndef NEOC_NUMERIC_UTILS_H
#define NEOC_NUMERIC_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file neoc_numeric_utils.h
 * @brief Numeric utility functions for Neo C SDK
 * 
 * This header provides utility functions for numeric operations commonly used
 * in the Neo protocol, including variable-length integer encoding/decoding,
 * byte order conversion, big integer operations, and other numeric helpers.
 * 
 * The functions are optimized for the Neo protocol requirements and handle
 * edge cases and error conditions appropriately.
 * 
 * @author NeoC SDK Team
 * @version 1.0.0
 * @date 2024
 */

/**
 * @brief Get the variable-length size of an integer
 * 
 * Calculates how many bytes would be needed to encode an integer
 * using Neo's variable-length encoding scheme.
 * 
 * @param value Value to measure
 * @return Size in bytes (1, 3, 5, or 9)
 */
size_t neoc_var_int_size(uint64_t value);

/**
 * @brief Encode integer to variable-length format
 * 
 * Encodes an integer using Neo's variable-length encoding scheme:
 * - Values < 0xFD: 1 byte
 * - Values <= 0xFFFF: 0xFD + 2 bytes (little-endian)
 * - Values <= 0xFFFFFFFF: 0xFE + 4 bytes (little-endian)
 * - Values > 0xFFFFFFFF: 0xFF + 8 bytes (little-endian)
 * 
 * @param value Value to encode
 * @param buffer Output buffer (must be at least neoc_var_int_size(value) bytes)
 * @param buffer_size Size of output buffer
 * @param bytes_written Number of bytes written to buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_encode_var_int(uint64_t value, 
                                  uint8_t *buffer, 
                                  size_t buffer_size, 
                                  size_t *bytes_written);

/**
 * @brief Decode variable-length integer
 * 
 * Decodes an integer from Neo's variable-length encoding format.
 * 
 * @param buffer Input buffer
 * @param buffer_size Size of input buffer
 * @param value Decoded value
 * @param bytes_consumed Number of bytes consumed from buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_decode_var_int(const uint8_t *buffer, 
                                  size_t buffer_size, 
                                  uint64_t *value, 
                                  size_t *bytes_consumed);

/**
 * @brief Convert bytes to little-endian uint16
 * 
 * @param bytes Input bytes (at least 2 bytes, little-endian)
 * @return uint16 value
 */
uint16_t neoc_bytes_to_uint16_le(const uint8_t *bytes);

/**
 * @brief Convert bytes to little-endian uint32
 * 
 * @param bytes Input bytes (at least 4 bytes, little-endian)
 * @return uint32 value
 */
uint32_t neoc_bytes_to_uint32_le(const uint8_t *bytes);

/**
 * @brief Convert bytes to little-endian uint64
 * 
 * @param bytes Input bytes (at least 8 bytes, little-endian)
 * @return uint64 value
 */
uint64_t neoc_bytes_to_uint64_le(const uint8_t *bytes);

/**
 * @brief Convert uint16 to little-endian bytes
 * 
 * @param value Value to convert
 * @param bytes Output bytes (must be at least 2 bytes)
 */
void neoc_uint16_to_bytes_le(uint16_t value, uint8_t *bytes);

/**
 * @brief Convert uint32 to little-endian bytes
 * 
 * @param value Value to convert
 * @param bytes Output bytes (must be at least 4 bytes)
 */
void neoc_uint32_to_bytes_le(uint32_t value, uint8_t *bytes);

/**
 * @brief Convert uint64 to little-endian bytes
 * 
 * @param value Value to convert
 * @param bytes Output bytes (must be at least 8 bytes)
 */
void neoc_uint64_to_bytes_le(uint64_t value, uint8_t *bytes);

/**
 * @brief Convert bytes to big-endian uint16
 * 
 * @param bytes Input bytes (at least 2 bytes, big-endian)
 * @return uint16 value
 */
uint16_t neoc_bytes_to_uint16_be(const uint8_t *bytes);

/**
 * @brief Convert bytes to big-endian uint32
 * 
 * @param bytes Input bytes (at least 4 bytes, big-endian)
 * @return uint32 value
 */
uint32_t neoc_bytes_to_uint32_be(const uint8_t *bytes);

/**
 * @brief Convert bytes to big-endian uint64
 * 
 * @param bytes Input bytes (at least 8 bytes, big-endian)
 * @return uint64 value
 */
uint64_t neoc_bytes_to_uint64_be(const uint8_t *bytes);

/**
 * @brief Convert uint16 to big-endian bytes
 * 
 * @param value Value to convert
 * @param bytes Output bytes (must be at least 2 bytes)
 */
void neoc_uint16_to_bytes_be(uint16_t value, uint8_t *bytes);

/**
 * @brief Convert uint32 to big-endian bytes
 * 
 * @param value Value to convert
 * @param bytes Output bytes (must be at least 4 bytes)
 */
void neoc_uint32_to_bytes_be(uint32_t value, uint8_t *bytes);

/**
 * @brief Convert uint64 to big-endian bytes
 * 
 * @param value Value to convert
 * @param bytes Output bytes (must be at least 8 bytes)
 */
void neoc_uint64_to_bytes_be(uint64_t value, uint8_t *bytes);

/**
 * @brief Reverse byte order in place
 * 
 * @param bytes Bytes to reverse
 * @param length Number of bytes
 */
void neoc_reverse_bytes(uint8_t *bytes, size_t length);

/**
 * @brief Copy and reverse bytes
 * 
 * @param src Source bytes
 * @param dst Destination bytes (must be at least length bytes)
 * @param length Number of bytes to copy and reverse
 */
void neoc_copy_reverse_bytes(const uint8_t *src, uint8_t *dst, size_t length);

/**
 * @brief Add two big integers represented as byte arrays
 * 
 * Performs addition on big integers represented as little-endian byte arrays.
 * 
 * @param a First operand (little-endian)
 * @param a_len Length of first operand
 * @param b Second operand (little-endian)
 * @param b_len Length of second operand
 * @param result Output result (caller must free)
 * @param result_len Output result length
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_big_int_add(const uint8_t *a, size_t a_len,
                               const uint8_t *b, size_t b_len,
                               uint8_t **result, size_t *result_len);

/**
 * @brief Subtract two big integers represented as byte arrays
 * 
 * Performs subtraction on big integers represented as little-endian byte arrays.
 * Returns error if result would be negative.
 * 
 * @param a First operand (little-endian)
 * @param a_len Length of first operand
 * @param b Second operand (little-endian)
 * @param b_len Length of second operand
 * @param result Output result (caller must free)
 * @param result_len Output result length
 * @return NEOC_SUCCESS on success, NEOC_UNDERFLOW if result would be negative
 */
neoc_error_t neoc_big_int_subtract(const uint8_t *a, size_t a_len,
                                    const uint8_t *b, size_t b_len,
                                    uint8_t **result, size_t *result_len);

/**
 * @brief Compare two big integers represented as byte arrays
 * 
 * Compares two big integers represented as little-endian byte arrays.
 * 
 * @param a First operand (little-endian)
 * @param a_len Length of first operand
 * @param b Second operand (little-endian)
 * @param b_len Length of second operand
 * @return -1 if a < b, 0 if a == b, 1 if a > b
 */
int neoc_big_int_compare(const uint8_t *a, size_t a_len,
                         const uint8_t *b, size_t b_len);

/**
 * @brief Check if big integer is zero
 * 
 * @param bytes Big integer bytes (little-endian)
 * @param length Length of bytes
 * @return true if zero, false otherwise
 */
bool neoc_big_int_is_zero(const uint8_t *bytes, size_t length);

/**
 * @brief Normalize big integer by removing leading zeros
 * 
 * Creates a new normalized copy of a big integer with leading zeros removed.
 * 
 * @param bytes Input bytes (little-endian)
 * @param length Input length
 * @param normalized Output normalized bytes (caller must free)
 * @param normalized_len Output normalized length
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_big_int_normalize(const uint8_t *bytes, size_t length,
                                     uint8_t **normalized, size_t *normalized_len);

/**
 * @brief Convert big integer to signed int32
 * 
 * @param bytes Big integer bytes (little-endian)
 * @param length Length of bytes
 * @param is_negative Whether the integer is negative
 * @param result Output int32 result
 * @return NEOC_SUCCESS on success, NEOC_OVERFLOW if too large
 */
neoc_error_t neoc_big_int_to_int32(const uint8_t *bytes, size_t length,
                                    bool is_negative, int32_t *result);

/**
 * @brief Convert big integer to signed int64
 * 
 * @param bytes Big integer bytes (little-endian)
 * @param length Length of bytes
 * @param is_negative Whether the integer is negative
 * @param result Output int64 result
 * @return NEOC_SUCCESS on success, NEOC_OVERFLOW if too large
 */
neoc_error_t neoc_big_int_to_int64(const uint8_t *bytes, size_t length,
                                    bool is_negative, int64_t *result);

/**
 * @brief Convert int32 to big integer bytes
 * 
 * @param value Input int32 value
 * @param bytes Output bytes (caller must free)
 * @param length Output length
 * @param is_negative Output sign flag
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_int32_to_big_int(int32_t value,
                                    uint8_t **bytes, size_t *length,
                                    bool *is_negative);

/**
 * @brief Convert int64 to big integer bytes
 * 
 * @param value Input int64 value
 * @param bytes Output bytes (caller must free)
 * @param length Output length
 * @param is_negative Output sign flag
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_int64_to_big_int(int64_t value,
                                    uint8_t **bytes, size_t *length,
                                    bool *is_negative);

/**
 * @brief Calculate power of integer
 * 
 * @param base Base value
 * @param exponent Exponent value (must be >= 0)
 * @param result Output result
 * @return NEOC_SUCCESS on success, NEOC_OVERFLOW if result too large
 */
neoc_error_t neoc_int_power(int64_t base, uint32_t exponent, int64_t *result);

/**
 * @brief Get current timestamp in milliseconds
 * 
 * @return Timestamp in milliseconds since Unix epoch
 */
uint64_t neoc_get_timestamp_ms(void);

/**
 * @brief Check if value is within range
 * 
 * @param value Value to check
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return true if within range, false otherwise
 */
bool neoc_in_range(int64_t value, int64_t min, int64_t max);

/**
 * @brief Safe integer addition with overflow check
 * 
 * @param a First operand
 * @param b Second operand
 * @param result Output result
 * @return NEOC_SUCCESS on success, NEOC_OVERFLOW on overflow
 */
neoc_error_t neoc_safe_add_int64(int64_t a, int64_t b, int64_t *result);

/**
 * @brief Safe integer multiplication with overflow check
 * 
 * @param a First operand
 * @param b Second operand
 * @param result Output result
 * @return NEOC_SUCCESS on success, NEOC_OVERFLOW on overflow
 */
neoc_error_t neoc_safe_mul_int64(int64_t a, int64_t b, int64_t *result);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NUMERIC_UTILS_H
