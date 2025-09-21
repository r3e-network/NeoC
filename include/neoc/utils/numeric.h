/**
 * @file numeric.h
 * @brief Numeric utility functions and extensions
 * 
 * Based on Swift source: utils/Numeric.swift
 * Provides numeric conversion, variable size encoding, and byte manipulation utilities
 */

#ifndef NEOC_UTILS_NUMERIC_H
#define NEOC_UTILS_NUMERIC_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert integer to power of another integer
 * 
 * @param base Base number
 * @param power Power to raise to
 * @return Result of base^power
 */
int neoc_numeric_power(int base, int power);

/**
 * @brief Get variable size encoding length for an integer
 * 
 * Variable size encoding rules:
 * - < 0xFD: 1 byte
 * - <= 0xFFFF: 3 bytes (0xFD + 2 bytes)
 * - <= 0xFFFFFFFF: 5 bytes (0xFE + 4 bytes)
 * - > 0xFFFFFFFF: 9 bytes (0xFF + 8 bytes)
 * 
 * @param value Integer value
 * @return Number of bytes needed for variable size encoding
 */
size_t neoc_numeric_var_size(uint64_t value);

/**
 * @brief Encode integer as variable size bytes
 * 
 * @param value Integer value to encode
 * @param buffer Output buffer (caller must provide sufficient space)
 * @param buffer_size Size of output buffer
 * @param encoded_size Output: actual bytes written
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_encode_var_size(uint64_t value,
                                           uint8_t *buffer,
                                           size_t buffer_size,
                                           size_t *encoded_size);

/**
 * @brief Decode variable size encoded integer
 * 
 * @param buffer Input buffer containing variable size encoded integer
 * @param buffer_size Size of input buffer
 * @param value Output: decoded integer value
 * @param bytes_consumed Output: number of bytes consumed from buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_decode_var_size(const uint8_t *buffer,
                                           size_t buffer_size,
                                           uint64_t *value,
                                           size_t *bytes_consumed);

/**
 * @brief Convert integer to unsigned (mask to 32-bit)
 * 
 * @param value Signed integer
 * @return Unsigned 32-bit representation
 */
uint32_t neoc_numeric_to_unsigned(int32_t value);

/**
 * @brief Convert numeric value to little-endian bytes
 * 
 * @param value Pointer to numeric value
 * @param value_size Size of the numeric value in bytes
 * @param bytes Output buffer for bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_to_bytes_le(const void *value,
                                       size_t value_size,
                                       uint8_t **bytes,
                                       size_t *bytes_len);

/**
 * @brief Convert numeric value to big-endian bytes
 * 
 * @param value Pointer to numeric value
 * @param value_size Size of the numeric value in bytes
 * @param bytes Output buffer for bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_to_bytes_be(const void *value,
                                       size_t value_size,
                                       uint8_t **bytes,
                                       size_t *bytes_len);

/**
 * @brief Convert little-endian bytes to numeric value
 * 
 * @param bytes Input bytes
 * @param bytes_len Length of input bytes
 * @param value Output buffer for numeric value
 * @param value_size Size of output value buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_from_bytes_le(const uint8_t *bytes,
                                         size_t bytes_len,
                                         void *value,
                                         size_t value_size);

/**
 * @brief Convert big-endian bytes to numeric value
 * 
 * @param bytes Input bytes
 * @param bytes_len Length of input bytes
 * @param value Output buffer for numeric value
 * @param value_size Size of output value buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_from_bytes_be(const uint8_t *bytes,
                                         size_t bytes_len,
                                         void *value,
                                         size_t value_size);

/**
 * @brief Convert BigInt-like large number to padded bytes
 * 
 * @param big_int_hex Hex string representation of large number
 * @param target_length Target byte length (will pad with zeros if needed)
 * @param bytes Output padded bytes (caller must free)
 * @param bytes_len Output: length of bytes
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_numeric_bigint_to_padded_bytes(const char *big_int_hex,
                                                  size_t target_length,
                                                  uint8_t **bytes,
                                                  size_t *bytes_len);

/**
 * @brief Get current timestamp in milliseconds since epoch
 * 
 * Equivalent to Swift Date().millisecondsSince1970
 * 
 * @return Milliseconds since Unix epoch
 */
int64_t neoc_numeric_current_time_millis(void);

/**
 * @brief Get decimal scale from a decimal number representation
 * 
 * @param decimal_str String representation of decimal number
 * @return Scale (number of decimal places)
 */
int neoc_numeric_get_decimal_scale(const char *decimal_str);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_NUMERIC_H */
