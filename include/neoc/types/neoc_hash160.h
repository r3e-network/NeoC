/**
 * @file neoc_hash160.h
 * @brief Hash160 type implementation for NeoC SDK
 * 
 * A Hash160 is a 20 bytes long hash created from some data by first applying 
 * SHA-256 and then RIPEMD-160. These hashes are mostly used for obtaining 
 * the script hash of a smart contract or an account.
 */

#ifndef NEOC_HASH160_H
#define NEOC_HASH160_H

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

/* Forward declaration for EC public key - defined in crypto/ec_key_pair.h */
struct neoc_ec_public_key;

/**
 * @brief Hash160 structure
 * 
 * Represents a 160-bit (20-byte) hash value used for script hashes and addresses.
 * The hash is stored in big-endian order internally.
 */
typedef struct {
    uint8_t data[NEOC_HASH160_SIZE];  ///< The 20-byte hash data in big-endian order
} neoc_hash160_t;

/**
 * @brief Create a new Hash160 with zero bytes
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_init_zero(neoc_hash160_t* hash);

/**
 * @brief Create a Hash160 from byte array
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param data Byte array (must be exactly 20 bytes, big-endian order)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_bytes(neoc_hash160_t* hash, const uint8_t data[NEOC_HASH160_SIZE]);

/**
 * @brief Create a Hash160 from byte array with length check
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param data Byte array
 * @param length Length of byte array (must be exactly 20)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_data(neoc_hash160_t* hash, const uint8_t* data, size_t length);

/**
 * @brief Create a Hash160 from hexadecimal string
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param hex_string Hexadecimal string (40 characters, big-endian order)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_hex(neoc_hash160_t* hash, const char* hex_string);

/**
 * @brief Create a Hash160 from Neo address
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param address Neo address string
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_address(neoc_hash160_t* hash, const char* address);

/**
 * @brief Create a Hash160 from script bytes
 * 
 * Calculates the script hash by applying SHA-256 then RIPEMD-160 to the script.
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param script Script bytes
 * @param script_length Length of script bytes
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_script(neoc_hash160_t* hash, const uint8_t* script, size_t script_length);

/**
 * @brief Create a Hash160 from script hex string
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param script_hex Script as hexadecimal string
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_script_hex(neoc_hash160_t* hash, const char* script_hex);

/**
 * @brief Create a Hash160 from public key
 * 
 * Creates a script hash for a single public key verification script.
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param public_key_data Compressed public key bytes (33 bytes)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_public_key(neoc_hash160_t* hash, const uint8_t public_key_data[NEOC_PUBLIC_KEY_SIZE_COMPRESSED]);

/**
 * @brief Create a Hash160 from multiple public keys (multi-sig)
 * 
 * Creates a script hash for a multi-signature verification script.
 * 
 * @param hash Pointer to Hash160 structure to initialize
 * @param public_keys Array of compressed public key data
 * @param num_keys Number of public keys
 * @param signing_threshold Minimum number of signatures required
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_public_keys(neoc_hash160_t* hash, 
                                           const uint8_t public_keys[][NEOC_PUBLIC_KEY_SIZE_COMPRESSED],
                                           size_t num_keys,
                                           size_t signing_threshold);

/**
 * @brief Copy Hash160
 * 
 * @param dest Destination Hash160
 * @param src Source Hash160
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_copy(neoc_hash160_t* dest, const neoc_hash160_t* src);

/**
 * @brief Get Hash160 data as byte array (big-endian)
 * 
 * @param hash Hash160 to get data from
 * @param buffer Buffer to store the data (must be at least 20 bytes)
 * @param buffer_size Size of the buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_to_bytes(const neoc_hash160_t* hash, uint8_t* buffer, size_t buffer_size);

/**
 * @brief Get Hash160 data as byte array (little-endian)
 * 
 * @param hash Hash160 to get data from
 * @param buffer Buffer to store the data (must be at least 20 bytes)
 * @param buffer_size Size of the buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_to_little_endian_bytes(const neoc_hash160_t* hash, uint8_t* buffer, size_t buffer_size);

/**
 * @brief Create Hash160 from string
 * 
 * @param str Hex string (with or without 0x prefix)
 * @param hash Output Hash160
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_from_string(const char *str, neoc_hash160_t *hash);

/**
 * @brief Convert Hash160 to hexadecimal string
 * 
 * @param hash Hash160 to convert
 * @param buffer Buffer to store the hex string (must be at least 41 bytes)
 * @param buffer_size Size of the buffer
 * @param uppercase Use uppercase letters if true
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_to_hex(const neoc_hash160_t* hash, char* buffer, size_t buffer_size, bool uppercase);

/**
 * @brief Convert Hash160 to string (alias for to_hex with lowercase)
 * 
 * @param hash Hash160 to convert
 * @param buffer Buffer to store the hex string (must be at least 41 bytes)
 * @param buffer_size Size of the buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_to_string(const neoc_hash160_t* hash, char* buffer, size_t buffer_size);

/**
 * @brief Convert Hash160 to Neo address
 * 
 * @param hash Hash160 to convert
 * @param buffer Buffer to store the address (must be at least 64 bytes)
 * @param buffer_size Size of the buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_to_address(const neoc_hash160_t* hash, char* buffer, size_t buffer_size);

/**
 * @brief Compare two Hash160 values
 * 
 * @param a First Hash160
 * @param b Second Hash160
 * @return 0 if equal, negative if a < b, positive if a > b
 */
int neoc_hash160_compare(const neoc_hash160_t* a, const neoc_hash160_t* b);

/**
 * @brief Check if two Hash160 values are equal
 * 
 * @param a First Hash160
 * @param b Second Hash160
 * @return true if equal, false otherwise
 */
bool neoc_hash160_equal(const neoc_hash160_t* a, const neoc_hash160_t* b);

/**
 * @brief Check if Hash160 is zero
 * 
 * @param hash Hash160 to check
 * @return true if zero, false otherwise
 */
bool neoc_hash160_is_zero(const neoc_hash160_t* hash);

/**
 * @brief Serialize Hash160 to binary writer
 * 
 * @param hash Hash160 to serialize
 * @param writer Binary writer to write to
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_serialize(const neoc_hash160_t* hash, neoc_binary_writer_t* writer);

/**
 * @brief Deserialize Hash160 from binary reader
 * 
 * @param hash Hash160 to initialize with deserialized data
 * @param reader Binary reader to read from
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_hash160_deserialize(neoc_hash160_t* hash, neoc_binary_reader_t* reader);

/**
 * @brief Get the size of Hash160 when serialized
 * 
 * @return Size in bytes (always 20)
 */
size_t neoc_hash160_serialized_size(void);

/* Constant zero Hash160 */
extern const neoc_hash160_t NEOC_HASH160_ZERO_VALUE;

#ifdef __cplusplus
}
#endif

#endif /* NEOC_HASH160_H */
