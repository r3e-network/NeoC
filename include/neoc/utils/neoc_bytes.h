/**
 * @file neoc_bytes.h
 * @brief Byte manipulation utilities for NeoC SDK
 */

#ifndef NEOC_BYTES_H
#define NEOC_BYTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

/**
 * @brief Reverse bytes in place
 * @param data Data to reverse
 * @param length Length of data
 * @return Error code
 */
neoc_error_t neoc_bytes_reverse(uint8_t* data, size_t length);

/**
 * @brief Copy bytes in reversed order
 * @param src Source data
 * @param src_len Source data length
 * @param dst Destination buffer
 * @param dst_len Destination buffer size
 * @return Error code
 */
neoc_error_t neoc_bytes_reverse_copy(const uint8_t* src, size_t src_len,
                                     uint8_t* dst, size_t dst_len);

/**
 * @brief Pad bytes to target length
 * @param src Source data
 * @param src_len Source data length
 * @param target_len Target length
 * @param trailing Whether to pad at trailing end
 * @param result Output result (caller must free)
 * @param result_len Output result length
 * @return Error code
 */
neoc_error_t neoc_bytes_pad(const uint8_t* src, size_t src_len,
                           size_t target_len, bool trailing,
                           uint8_t** result, size_t* result_len);

/**
 * @brief Trim trailing bytes of specific value
 * @param src Source data
 * @param src_len Source data length
 * @param byte_value Byte value to trim
 * @param result Output result (caller must free)
 * @param result_len Output result length
 * @return Error code
 */
neoc_error_t neoc_bytes_trim_trailing(const uint8_t* src, size_t src_len,
                                      uint8_t byte_value,
                                      uint8_t** result, size_t* result_len);

/**
 * @brief XOR two byte arrays
 * @param lhs Left hand side
 * @param rhs Right hand side
 * @param length Length of arrays
 * @param result Output result
 * @return Error code
 */
neoc_error_t neoc_bytes_xor(const uint8_t* lhs, const uint8_t* rhs,
                            size_t length, uint8_t* result);

/**
 * @brief Convert script hash to address
 * @param script_hash Script hash (20 bytes)
 * @param address_version Address version byte
 * @param address Output address buffer
 * @param address_size Address buffer size
 * @return Error code
 */
neoc_error_t neoc_bytes_script_hash_to_address(const uint8_t* script_hash,
                                               uint8_t address_version,
                                               char* address, size_t address_size);

/**
 * @brief Get required size for variable-length integer encoding
 * @param value Value to encode
 * @return Required size in bytes
 */
size_t neoc_bytes_var_size(size_t value);

/**
 * @brief Encode variable-length integer
 * @param value Value to encode
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @param bytes_written Output bytes written
 * @return Error code
 */
neoc_error_t neoc_bytes_encode_var_size(size_t value, uint8_t* buffer,
                                        size_t buffer_size, size_t* bytes_written);

/**
 * @brief Decode variable-length integer
 * @param buffer Input buffer
 * @param buffer_size Buffer size
 * @param value Output value
 * @param bytes_read Output bytes read
 * @return Error code
 */
neoc_error_t neoc_bytes_decode_var_size(const uint8_t* buffer, size_t buffer_size,
                                        size_t* value, size_t* bytes_read);

/**
 * @brief Check if byte is between two opcodes
 * @param byte Byte to check
 * @param op1 First opcode
 * @param op2 Second opcode
 * @return True if between opcodes
 */
bool neoc_byte_is_between_opcodes(uint8_t byte, uint8_t op1, uint8_t op2);

/**
 * @brief Convert bytes to uint64 (little endian)
 * @param bytes Input bytes
 * @param length Number of bytes
 * @param result Output result
 * @return Error code
 */
neoc_error_t neoc_bytes_to_uint64_le(const uint8_t* bytes, size_t length,
                                     uint64_t* result);

/**
 * @brief Convert bytes to uint64 (big endian)
 * @param bytes Input bytes
 * @param length Number of bytes
 * @param result Output result
 * @return Error code
 */
neoc_error_t neoc_bytes_to_uint64_be(const uint8_t* bytes, size_t length,
                                     uint64_t* result);

/**
 * @brief Convert uint64 to bytes (little endian)
 * @param value Input value
 * @param bytes Output bytes
 * @param length Number of bytes
 * @return Error code
 */
neoc_error_t neoc_uint64_to_bytes_le(uint64_t value, uint8_t* bytes, size_t length);

/**
 * @brief Convert uint64 to bytes (big endian)
 * @param value Input value
 * @param bytes Output bytes
 * @param length Number of bytes
 * @return Error code
 */
neoc_error_t neoc_uint64_to_bytes_be(uint64_t value, uint8_t* bytes, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_BYTES_H */
