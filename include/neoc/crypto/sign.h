#ifndef NEOC_SIGN_H
#define NEOC_SIGN_H

#include <stdint.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/ecdsa_signature.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sign a message with the private key
 * 
 * The message will be hashed with SHA256 before signing.
 * 
 * @param message The message to sign
 * @param message_len Length of the message
 * @param key_pair The key pair containing the private key
 * @param sig_data Output signature data with recovery ID (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_sign_message(const uint8_t *message, size_t message_len,
                                const neoc_ec_key_pair_t *key_pair,
                                neoc_signature_data_t **sig_data);

/**
 * @brief Sign a hex-encoded message
 * 
 * @param hex_message The hex-encoded message string
 * @param key_pair The key pair containing the private key
 * @param sig_data Output signature data with recovery ID (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_sign_hex_message(const char *hex_message,
                                    const neoc_ec_key_pair_t *key_pair,
                                    neoc_signature_data_t **sig_data);

/**
 * @brief Recover public key from signature
 * 
 * Given a signature and the message that was signed, recover the public key
 * that was used to create the signature.
 * 
 * @param rec_id Recovery ID (0-3)
 * @param signature The signature
 * @param message_hash The 32-byte message hash that was signed
 * @param public_key Output recovered public key (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_recover_from_signature(int rec_id,
                                          const neoc_ecdsa_signature_t *signature,
                                          const uint8_t *message_hash,
                                          neoc_ec_public_key_t **public_key);

/**
 * @brief Recover public key from signed message
 * 
 * @param message The original message
 * @param message_len Length of the message
 * @param sig_data The signature data with recovery ID
 * @param public_key Output recovered public key (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signed_message_to_key(const uint8_t *message, size_t message_len,
                                         const neoc_signature_data_t *sig_data,
                                         neoc_ec_public_key_t **public_key);

/**
 * @brief Recover signing script hash from signature
 * 
 * @param message The message that was signed
 * @param message_len Length of the message
 * @param sig_data The signature data
 * @param script_hash Output script hash of the signer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_recover_signing_script_hash(const uint8_t *message, size_t message_len,
                                               const neoc_signature_data_t *sig_data,
                                               neoc_hash160_t *script_hash);

/**
 * @brief Verify a signature
 * 
 * @param message The original message
 * @param message_len Length of the message
 * @param sig_data The signature to verify
 * @param public_key The public key to verify against
 * @return true if signature is valid, false otherwise
 */
bool neoc_verify_signature(const uint8_t *message, size_t message_len,
                            const neoc_signature_data_t *sig_data,
                            const neoc_ec_public_key_t *public_key);

/**
 * @brief Get public key from private key
 * 
 * @param private_key The private key
 * @param public_key Output public key (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_public_key_from_private_key(const neoc_ec_private_key_t *private_key,
                                               neoc_ec_public_key_t **public_key);

/**
 * @brief Verify a message signature (convenience wrapper)
 * 
 * @param message The original message
 * @param message_len Length of the message
 * @param signature The signature to verify
 * @param key_pair The key pair containing the public key
 * @param verified Output true if valid, false otherwise
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verify_message(const uint8_t *message, size_t message_len,
                                  const neoc_signature_data_t *signature,
                                  const neoc_ec_key_pair_t *key_pair,
                                  bool *verified);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SIGN_H
