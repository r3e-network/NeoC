/**
 * @file ec_public_key.h
 * @brief Elliptic curve public key operations for Neo
 */

#ifndef NEOC_EC_PUBLIC_KEY_H
#define NEOC_EC_PUBLIC_KEY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ecdsa_signature.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief EC public key structure
 */
typedef struct neoc_ec_public_key_t neoc_ec_public_key_t;

/**
 * @brief Create public key from bytes
 * 
 * @param bytes Public key bytes (33 bytes compressed or 65 bytes uncompressed)
 * @param len Length of bytes
 * @param public_key Output public key (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_from_bytes(const uint8_t *bytes,
                                            size_t len,
                                            neoc_ec_public_key_t **public_key);

/**
 * @brief Convert public key to bytes
 * 
 * @param public_key Public key
 * @param bytes Output buffer
 * @param len Input/output buffer length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_to_bytes(const neoc_ec_public_key_t *public_key,
                                          uint8_t *bytes,
                                          size_t *len);

/**
 * @brief Get public key in compressed format
 * 
 * @param public_key Public key
 * @param compressed Output compressed key (33 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_get_compressed(const neoc_ec_public_key_t *public_key,
                                                uint8_t compressed[33]);

/**
 * @brief Get public key in uncompressed format
 * 
 * @param public_key Public key
 * @param uncompressed Output uncompressed key (65 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_get_uncompressed(const neoc_ec_public_key_t *public_key,
                                                  uint8_t uncompressed[65]);

/**
 * @brief Verify signature with public key
 * 
 * @param public_key Public key
 * @param message Message hash (32 bytes)
 * @param signature Signature to verify
 * @param is_valid Output verification result
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_verify(const neoc_ec_public_key_t *public_key,
                                       const uint8_t message[32],
                                       const neoc_ecdsa_signature_t *signature,
                                       bool *is_valid);

/**
 * @brief Compare two public keys
 * 
 * @param key1 First public key
 * @param key2 Second public key
 * @return true if equal, false otherwise
 */
bool neoc_ec_public_key_equals(const neoc_ec_public_key_t *key1,
                               const neoc_ec_public_key_t *key2);

/**
 * @brief Clone public key
 * 
 * @param public_key Public key to clone
 * @param clone Output cloned key (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_public_key_clone(const neoc_ec_public_key_t *public_key,
                                       neoc_ec_public_key_t **clone);

/**
 * @brief Free public key
 * 
 * @param public_key Public key to free
 */
void neoc_ec_public_key_free(neoc_ec_public_key_t *public_key);

#ifdef __cplusplus
}
#endif

#endif // NEOC_EC_PUBLIC_KEY_H
