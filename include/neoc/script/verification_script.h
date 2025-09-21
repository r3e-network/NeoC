/**
 * @file verification_script.h
 * @brief Neo verification script implementation
 * 
 * Based on Swift source: script/VerificationScript.swift
 * A verification script is part of a witness and describes what has to be verified
 * such that the witness is valid (e.g., signature verification).
 */

#ifndef NEOC_SCRIPT_VERIFICATION_SCRIPT_H
#define NEOC_SCRIPT_VERIFICATION_SCRIPT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/types/neoc_hash160.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verification script structure
 * 
 * Contains the Neo VM instructions that describe verification logic
 */
typedef struct {
    uint8_t *script;         /**< The verification script as byte array */
    size_t script_length;    /**< Length of the script in bytes */
} neoc_verification_script_t;

/**
 * @brief Create verification script from byte array
 * 
 * Equivalent to Swift VerificationScript.init(_ script: Bytes)
 * 
 * @param script Script bytes
 * @param script_length Length of script
 * @param verification_script Output verification script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_create(const uint8_t *script,
                                              size_t script_length,
                                              neoc_verification_script_t **verification_script);

/**
 * @brief Create single signature verification script from public key
 * 
 * Equivalent to Swift VerificationScript.init(_ publicKey: ECPublicKey)
 * 
 * @param public_key The public key
 * @param verification_script Output verification script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_create_single_sig(const neoc_ec_public_key_t *public_key,
                                                         neoc_verification_script_t **verification_script);

/**
 * @brief Create multi-signature verification script
 * 
 * Equivalent to Swift VerificationScript.init(_ publicKeys: [ECPublicKey], _ signingThreshold: Int)
 * 
 * @param public_keys Array of public keys
 * @param key_count Number of public keys
 * @param signing_threshold Number of signatures required
 * @param verification_script Output verification script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_create_multi_sig(neoc_ec_public_key_t **public_keys,
                                                        size_t key_count,
                                                        int signing_threshold,
                                                        neoc_verification_script_t **verification_script);

/**
 * @brief Check if verification script is a single signature script
 * 
 * Equivalent to Swift VerificationScript.isSingleSigScript()
 * 
 * @param verification_script The verification script
 * @return True if single signature script
 */
bool neoc_verification_script_is_single_sig(const neoc_verification_script_t *verification_script);

/**
 * @brief Check if verification script is a multi-signature script
 * 
 * Equivalent to Swift VerificationScript.isMultiSigScript()
 * 
 * @param verification_script The verification script
 * @return True if multi-signature script
 */
bool neoc_verification_script_is_multi_sig(const neoc_verification_script_t *verification_script);

/**
 * @brief Get signing threshold from multi-signature script
 * 
 * Equivalent to Swift VerificationScript.getSigningThreshold()
 * 
 * @param verification_script The verification script
 * @param threshold Output signing threshold
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_get_signing_threshold(const neoc_verification_script_t *verification_script,
                                                             int *threshold);

/**
 * @brief Get number of accounts from multi-signature script
 * 
 * Equivalent to Swift VerificationScript.getNrOfAccounts()
 * 
 * @param verification_script The verification script
 * @param nr_accounts Output number of accounts
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_get_nr_accounts(const neoc_verification_script_t *verification_script,
                                                       int *nr_accounts);

/**
 * @brief Extract public keys from verification script
 * 
 * Equivalent to Swift VerificationScript.getPublicKeys()
 * 
 * @param verification_script The verification script
 * @param public_keys Output array of public keys (caller must free)
 * @param key_count Output number of public keys
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_get_public_keys(const neoc_verification_script_t *verification_script,
                                                       neoc_ec_public_key_t ***public_keys,
                                                       size_t *key_count);

/**
 * @brief Get script hash from verification script
 * 
 * Equivalent to Swift VerificationScript.getScriptHash()
 * 
 * @param verification_script The verification script
 * @param script_hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_get_script_hash(const neoc_verification_script_t *verification_script,
                                                       neoc_hash160_t *script_hash);

/**
 * @brief Get address from verification script
 * 
 * Equivalent to Swift VerificationScript.getAddress()
 * 
 * @param verification_script The verification script
 * @param address Output address string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_get_address(const neoc_verification_script_t *verification_script,
                                                   char **address);

/**
 * @brief Get script bytes
 * 
 * @param verification_script The verification script
 * @param script Output pointer to script bytes (do not free)
 * @param script_length Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_get_script(const neoc_verification_script_t *verification_script,
                                                  const uint8_t **script,
                                                  size_t *script_length);

/**
 * @brief Get script size in bytes
 * 
 * Equivalent to Swift VerificationScript.size
 * 
 * @param verification_script The verification script
 * @return Size in bytes
 */
size_t neoc_verification_script_get_size(const neoc_verification_script_t *verification_script);

/**
 * @brief Copy verification script
 * 
 * @param source Source verification script
 * @param copy Output copied script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_copy(const neoc_verification_script_t *source,
                                            neoc_verification_script_t **copy);

/**
 * @brief Compare two verification scripts for equality
 * 
 * Equivalent to Swift VerificationScript.== operator
 * 
 * @param script1 First verification script
 * @param script2 Second verification script
 * @return True if equal
 */
bool neoc_verification_script_equals(const neoc_verification_script_t *script1,
                                      const neoc_verification_script_t *script2);

/**
 * @brief Calculate hash of verification script
 * 
 * Equivalent to Swift VerificationScript.hash(into:)
 * 
 * @param verification_script The verification script
 * @return Hash value
 */
uint32_t neoc_verification_script_hash(const neoc_verification_script_t *verification_script);

/**
 * @brief Serialize verification script
 * 
 * Equivalent to Swift VerificationScript.serialize(_:)
 * 
 * @param verification_script The verification script
 * @param serialized Output serialized data (caller must free)
 * @param serialized_length Output length of serialized data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_serialize(const neoc_verification_script_t *verification_script,
                                                 uint8_t **serialized,
                                                 size_t *serialized_length);

/**
 * @brief Deserialize verification script
 * 
 * Equivalent to Swift VerificationScript.deserialize(_:)
 * 
 * @param serialized Serialized data
 * @param serialized_length Length of serialized data
 * @param verification_script Output verification script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_verification_script_deserialize(const uint8_t *serialized,
                                                   size_t serialized_length,
                                                   neoc_verification_script_t **verification_script);

/**
 * @brief Free verification script
 * 
 * @param verification_script The verification script to free
 */
void neoc_verification_script_free(neoc_verification_script_t *verification_script);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_SCRIPT_VERIFICATION_SCRIPT_H */
