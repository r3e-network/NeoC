#ifndef NEOC_EC_KEY_PAIR_H
#define NEOC_EC_KEY_PAIR_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_types.h"
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
struct neoc_ecdsa_signature_t;

/**
 * @brief EC Private Key structure
 */
typedef struct {
    EVP_PKEY *pkey;           // OpenSSL EVP_PKEY containing private key
    EC_KEY *ec_key;           // OpenSSL EC_KEY for low-level operations
    uint8_t bytes[32];        // Private key as 32-byte array
} neoc_ec_private_key_t;

/**
 * @brief EC Public Key structure
 */
typedef struct neoc_ec_public_key {
    EC_POINT *point;          // OpenSSL EC_POINT
    EC_GROUP *group;          // OpenSSL EC_GROUP for secp256r1
    uint8_t compressed[33];   // Compressed public key (33 bytes)
    uint8_t uncompressed[65]; // Uncompressed public key (65 bytes)
    bool is_compressed;       // Whether to use compressed format by default
} neoc_ec_public_key_t;

/**
 * @brief EC Key Pair structure
 */
typedef struct {
    neoc_ec_private_key_t *private_key;
    neoc_ec_public_key_t *public_key;
} neoc_ec_key_pair_t;

/**
 * @brief Create a new EC key pair from a private key
 * 
 * @param private_key_bytes 32-byte private key
 * @param key_pair Output key pair (caller must free with neoc_ec_key_pair_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_create_from_private_key(const uint8_t *private_key_bytes, 
                                                       neoc_ec_key_pair_t **key_pair);

/**
 * @brief Create a fresh secp256r1 EC key pair
 * 
 * @param key_pair Output key pair (caller must free with neoc_ec_key_pair_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_create_random(neoc_ec_key_pair_t **key_pair);

/**
 * @brief Create EC key pair from private key bytes (compatibility alias)
 * 
 * @param private_key_bytes Private key bytes
 * @param key_size Size of private key (must be 32)
 * @param key_pair Output key pair (caller must free with neoc_ec_key_pair_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_from_private_key(const uint8_t *private_key_bytes,
                                                size_t key_size,
                                                neoc_ec_key_pair_t **key_pair);

/**
 * @brief Get the NEO address from this key pair's public key
 * 
 * @param key_pair The key pair
 * @param address Output address string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_get_address(const neoc_ec_key_pair_t *key_pair, char **address);

/**
 * @brief Get the script hash from this key pair's public key
 * 
 * @param key_pair The key pair
 * @param script_hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_get_script_hash(const neoc_ec_key_pair_t *key_pair, 
                                               neoc_hash160_t *script_hash);

/**
 * @brief Sign a message hash with the private key
 * 
 * @param key_pair The key pair containing the private key
 * @param message_hash The 32-byte message hash to sign
 * @param signature Output signature (caller must free with neoc_ecdsa_signature_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_sign(const neoc_ec_key_pair_t *key_pair,
                                    const uint8_t *message_hash,
                                    struct neoc_ecdsa_signature_t **signature);

/**
 * @brief Export the private key as WIF (Wallet Import Format)
 * 
 * @param key_pair The key pair
 * @param wif Output WIF string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_export_as_wif(const neoc_ec_key_pair_t *key_pair, char **wif);

/**
 * @brief Import a key pair from WIF (Wallet Import Format)
 * 
 * @param wif The WIF string
 * @param key_pair Output key pair (caller must free with neoc_ec_key_pair_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_import_from_wif(const char *wif, neoc_ec_key_pair_t **key_pair);

/**
 * @brief Free an EC key pair
 * 
 * @param key_pair The key pair to free
 */
void neoc_ec_key_pair_free(neoc_ec_key_pair_t *key_pair);

/**
 * @brief Get public key from private key
 * 
 * @param private_key_bytes 32-byte private key
 * @param public_key Output public key (caller must free with neoc_ec_public_key_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_from_private(const uint8_t *private_key_bytes,
                                              neoc_ec_public_key_t **public_key);

/**
 * @brief Create public key from encoded bytes
 * 
 * @param encoded The encoded public key (33 or 65 bytes)
 * @param encoded_len Length of encoded data (33 or 65)
 * @param public_key Output public key (caller must free with neoc_ec_public_key_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_from_bytes(const uint8_t *encoded, size_t encoded_len,
                                            neoc_ec_public_key_t **public_key);

/**
 * @brief Get encoded public key
 * 
 * @param public_key The public key
 * @param compressed Whether to get compressed format
 * @param encoded Output encoded bytes (caller must free)
 * @param encoded_len Output length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_get_encoded(const neoc_ec_public_key_t *public_key,
                                             bool compressed,
                                             uint8_t **encoded,
                                             size_t *encoded_len);

/**
 * @brief Free a public key
 * 
 * @param public_key The public key to free
 */
void neoc_ec_public_key_free(neoc_ec_public_key_t *public_key);

/**
 * @brief Free a private key
 * 
 * @param private_key The private key to free
 */
void neoc_ec_private_key_free(neoc_ec_private_key_t *private_key);

/**
 * @brief Get the private key from a key pair
 * 
 * @param key_pair The key pair
 * @param private_key Output buffer for private key
 * @param key_len Input/output length of key buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_get_private_key(const neoc_ec_key_pair_t *key_pair,
                                               uint8_t *private_key,
                                               size_t *key_len);

/**
 * @brief Get the public key from a key pair
 * 
 * @param key_pair The key pair
 * @param public_key Output buffer for public key
 * @param key_len Input/output length of key buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_key_pair_get_public_key(const neoc_ec_key_pair_t *key_pair,
                                              uint8_t *public_key,
                                              size_t *key_len);

#ifdef __cplusplus
}
#endif

#endif // NEOC_EC_KEY_PAIR_H
