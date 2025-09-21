/**
 * @file neoc_hash256.h
 * @brief Hash256 type implementation for NeoC SDK
 * 
 * A Hash256 is a 32 bytes long hash created from some data by applying SHA-256.
 * These hashes are mostly used for obtaining transaction or block hashes.
 */

#ifndef NEOC_HASH256_H
#define NEOC_HASH256_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/neo_constants.h"
#include "neoc/types/neoc_types.h"

/* Forward declarations */
#ifndef NEOC_FORWARD_DECLARATIONS
#define NEOC_FORWARD_DECLARATIONS
typedef struct neoc_binary_reader neoc_binary_reader_t;
typedef struct neoc_binary_writer neoc_binary_writer_t;
#endif

/**
 * @brief Hash256 structure
 * 
 * Represents a 256-bit (32-byte) hash value used for transaction and block hashes.
 * The hash is stored in big-endian order internally.
 */
typedef struct {
    uint8_t data[NEOC_HASH256_SIZE];  ///< The 32-byte hash data in big-endian order
} neoc_hash256_t;

/**
 * @brief Create a new Hash256 with zero bytes
 * 
 * @param hash Pointer to Hash256 structure to initialize
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_init_zero(neoc_hash256_t* hash);

/**
 * @brief Create a Hash256 from byte array
 * 
 * @param hash Pointer to Hash256 structure to initialize
 * @param data Byte array (must be exactly 32 bytes, big-endian order)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_from_bytes(neoc_hash256_t* hash, const uint8_t data[NEOC_HASH256_SIZE]);

/**
 * @brief Create a Hash256 from byte array with length check
 * 
 * @param hash Pointer to Hash256 structure to initialize
 * @param data Byte array
 * @param length Length of byte array (must be exactly 32)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_from_data(neoc_hash256_t* hash, const uint8_t* data, size_t length);

/**
 * @brief Create a Hash256 from hexadecimal string
 * 
 * @param hash Pointer to Hash256 structure to initialize
 * @param hex_string Hexadecimal string (64 characters, big-endian order)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_from_hex(neoc_hash256_t* hash, const char* hex_string);

/**
 * @brief Create a Hash256 by hashing input data
 * 
 * Calculates the SHA-256 hash of the input data.
 * 
 * @param hash Pointer to Hash256 structure to initialize
 * @param data Input data to hash
 * @param data_length Length of input data
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_from_data_hash(neoc_hash256_t* hash, const uint8_t* data, size_t data_length);

/**
 * @brief Create a Hash256 by double-hashing input data
 * 
 * Calculates the SHA-256 hash of the SHA-256 hash of the input data.
 * This is commonly used in Bitcoin and Neo for transaction and block hashes.
 * 
 * @param hash Pointer to Hash256 structure to initialize
 * @param data Input data to hash
 * @param data_length Length of input data
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_from_data_double_hash(neoc_hash256_t* hash, const uint8_t* data, size_t data_length);

/**
 * @brief Copy Hash256
 * 
 * @param dest Destination Hash256
 * @param src Source Hash256
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_copy(neoc_hash256_t* dest, const neoc_hash256_t* src);

/**
 * @brief Get Hash256 data as byte array (big-endian)
 * 
 * @param hash Hash256 to get data from
 * @param buffer Buffer to store the data (must be at least 32 bytes)
 * @param buffer_size Size of the buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_to_bytes(const neoc_hash256_t* hash, uint8_t* buffer, size_t buffer_size);

/**
 * @brief Get Hash256 data as byte array (little-endian)
 * 
 * @param hash Hash256 to get data from
 * @param buffer Buffer to store the data (must be at least 32 bytes)
 * @param buffer_size Size of the buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_to_little_endian_bytes(const neoc_hash256_t* hash, uint8_t* buffer, size_t buffer_size);

/**
 * @brief Convert Hash256 to hexadecimal string
 * 
 * @param hash Hash256 to convert
 * @param buffer Buffer to store the hex string (must be at least 65 bytes)
 * @param buffer_size Size of the buffer
 * @param uppercase Use uppercase letters if true
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_to_hex(const neoc_hash256_t* hash, char* buffer, size_t buffer_size, bool uppercase);

/**
 * @brief Compare two Hash256 values
 * 
 * @param a First Hash256
 * @param b Second Hash256
 * @return 0 if equal, negative if a < b, positive if a > b
 */
int neoc_hash256_compare(const neoc_hash256_t* a, const neoc_hash256_t* b);

/**
 * @brief Check if two Hash256 values are equal
 * 
 * @param a First Hash256
 * @param b Second Hash256
 * @return true if equal, false otherwise
 */
bool neoc_hash256_equal(const neoc_hash256_t* a, const neoc_hash256_t* b);

/**
 * @brief Check if Hash256 is zero
 * 
 * @param hash Hash256 to check
 * @return true if zero, false otherwise
 */
bool neoc_hash256_is_zero(const neoc_hash256_t* hash);

/**
 * @brief Create Hash256 from string
 * 
 * @param str Hex string (with or without 0x prefix)
 * @param hash Output Hash256
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_from_string(const char *str, neoc_hash256_t *hash);

/**
 * @brief Convert Hash256 to string
 * 
 * @param hash Hash256 to convert
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_to_string(const neoc_hash256_t *hash, char *buffer, size_t buffer_size);

/**
 * @brief Serialize Hash256 to binary writer
 * 
 * @param hash Hash256 to serialize
 * @param writer Binary writer to write to
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_serialize(const neoc_hash256_t* hash, neoc_binary_writer_t* writer);

/**
 * @brief Deserialize Hash256 from binary reader
 * 
 * @param hash Hash256 to initialize with deserialized data
 * @param reader Binary reader to read from
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash256_deserialize(neoc_hash256_t* hash, neoc_binary_reader_t* reader);

/**
 * @brief Get the size of Hash256 when serialized
 * 
 * @return Size in bytes (always 32)
 */
size_t neoc_hash256_serialized_size(void);

/* Constant zero Hash256 */
extern const neoc_hash256_t NEOC_HASH256_ZERO_VALUE;

#ifdef __cplusplus
}
#endif

#endif /* NEOC_HASH256_H */
