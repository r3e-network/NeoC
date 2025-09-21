/**
 * @file neoc_bytes_utils.h
 * @brief Extended byte manipulation utilities for NeoC SDK
 * 
 * Provides additional utility functions for byte arrays beyond the basic
 * neoc_bytes_t operations, including encoding, padding, and numeric conversions.
 */

#ifndef NEOC_BYTES_UTILS_H
#define NEOC_BYTES_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "neoc/types/neoc_types.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/neoc_error.h"
#include <stdint.h>

/**
 * @brief Convert bytes to Base64 encoded string
 * 
 * @param bytes Input byte array
 * @param buffer Output buffer for Base64 string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_to_base64(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size);

/**
 * @brief Convert bytes to Base58 encoded string
 * 
 * @param bytes Input byte array
 * @param buffer Output buffer for Base58 string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_to_base58(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size);

/**
 * @brief Convert bytes to Base58Check encoded string
 * 
 * @param bytes Input byte array
 * @param buffer Output buffer for Base58Check string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_to_base58_check(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size);

/**
 * @brief Convert bytes to hex string without 0x prefix
 * 
 * @param bytes Input byte array
 * @param buffer Output buffer for hex string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_to_hex_no_prefix(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size);

/**
 * @brief Calculate variable-length integer size of byte array
 * 
 * Returns the total size needed to encode this byte array with varint length prefix
 * 
 * @param bytes Input byte array
 * @return Size in bytes including varint prefix
 */
size_t neoc_bytes_var_size(const neoc_bytes_t* bytes);

/**
 * @brief Convert script hash to Neo address
 * 
 * @param script_hash Input script hash bytes
 * @param address_buffer Output buffer for address string
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_script_hash_to_address(const neoc_bytes_t* script_hash, char* address_buffer, size_t buffer_size);

/**
 * @brief Pad byte array to specified length
 * 
 * @param bytes Input byte array
 * @param length Target length
 * @param trailing If true, pad at end; if false, pad at beginning
 * @param result Output padded byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_pad(const neoc_bytes_t* bytes, size_t length, bool trailing, neoc_bytes_t** result);

/**
 * @brief Trim trailing bytes of specified value
 * 
 * @param bytes Input byte array
 * @param byte_value Value to trim from end
 * @param result Output trimmed byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_trim_trailing(const neoc_bytes_t* bytes, uint8_t byte_value, neoc_bytes_t** result);

/**
 * @brief Convert bytes to numeric value (little or big endian)
 * 
 * @param bytes Input byte array
 * @param little_endian If true, interpret as little endian
 * @param result Output numeric value
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_to_uint32(const neoc_bytes_t* bytes, bool little_endian, uint32_t* result);

/**
 * @brief Convert bytes to 64-bit numeric value
 * 
 * @param bytes Input byte array
 * @param little_endian If true, interpret as little endian
 * @param result Output numeric value
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_to_uint64(const neoc_bytes_t* bytes, bool little_endian, uint64_t* result);

/**
 * @brief XOR two byte arrays
 * 
 * Arrays must be the same length
 * 
 * @param lhs First byte array
 * @param rhs Second byte array
 * @param result Output XOR result (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_xor(const neoc_bytes_t* lhs, const neoc_bytes_t* rhs, neoc_bytes_t** result);

/**
 * @brief Check if byte value is between two OpCode values
 * 
 * @param byte_value The byte value to check
 * @param opcode1 First OpCode value (lower bound)
 * @param opcode2 Second OpCode value (upper bound)
 * @return true if byte is between the OpCodes, false otherwise
 */
bool neoc_byte_is_between_opcodes(uint8_t byte_value, uint8_t opcode1, uint8_t opcode2);

/**
 * @brief Reverse byte array in place
 * 
 * @param bytes Byte array to reverse
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_reverse_inplace(neoc_bytes_t* bytes);

/**
 * @brief Create reversed copy of byte array
 * 
 * @param bytes Input byte array
 * @param result Output reversed byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_bytes_reverse_copy(const neoc_bytes_t* bytes, neoc_bytes_t** result);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_BYTES_UTILS_H */
