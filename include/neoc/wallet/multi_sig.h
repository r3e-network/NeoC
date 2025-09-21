/**
 * @file multi_sig.h
 * @brief Multi-signature account support for Neo
 */

#ifndef NEOC_MULTI_SIG_H
#define NEOC_MULTI_SIG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/script/script_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Multi-signature account structure
 */
typedef struct {
    uint8_t threshold;              /**< Number of signatures required */
    uint8_t public_key_count;       /**< Total number of public keys */
    neoc_ec_public_key_t **public_keys; /**< Array of public keys */
    neoc_hash160_t script_hash;     /**< Script hash of the multi-sig account */
    uint8_t *verification_script;   /**< Verification script */
    size_t script_size;             /**< Size of the verification script */
} neoc_multi_sig_account_t;

/**
 * @brief Create a multi-signature account
 * 
 * @param threshold Number of signatures required (M in M-of-N)
 * @param public_keys Array of public keys
 * @param key_count Number of public keys (N in M-of-N)
 * @param account Output multi-sig account
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_multi_sig_create(uint8_t threshold,
                                    neoc_ec_public_key_t **public_keys,
                                    uint8_t key_count,
                                    neoc_multi_sig_account_t **account);

/**
 * @brief Get the verification script for a multi-sig account
 * 
 * @param account Multi-sig account
 * @param script Output script buffer
 * @param script_size Size of the script buffer
 * @param actual_size Actual size of the script
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_multi_sig_get_verification_script(const neoc_multi_sig_account_t *account,
                                                     uint8_t *script,
                                                     size_t script_size,
                                                     size_t *actual_size);

/**
 * @brief Get the script hash (address) of a multi-sig account
 * 
 * @param account Multi-sig account
 * @param hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_multi_sig_get_script_hash(const neoc_multi_sig_account_t *account,
                                             neoc_hash160_t *hash);

/**
 * @brief Create a multi-sig witness
 * 
 * @param account Multi-sig account
 * @param signatures Array of signatures
 * @param sig_count Number of signatures (must be >= threshold)
 * @param invocation_script Output invocation script
 * @param invocation_size Size of invocation script buffer
 * @param actual_size Actual size of invocation script
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_multi_sig_create_witness(const neoc_multi_sig_account_t *account,
                                            const uint8_t **signatures,
                                            uint8_t sig_count,
                                            uint8_t *invocation_script,
                                            size_t invocation_size,
                                            size_t *actual_size);

/**
 * @brief Verify if a public key is part of a multi-sig account
 * 
 * @param account Multi-sig account
 * @param public_key Public key to check
 * @return true if the key is part of the account, false otherwise
 */
bool neoc_multi_sig_contains_key(const neoc_multi_sig_account_t *account,
                                  const neoc_ec_public_key_t *public_key);

/**
 * @brief Free a multi-sig account
 * 
 * @param account Multi-sig account to free
 */
void neoc_multi_sig_free(neoc_multi_sig_account_t *account);

#ifdef __cplusplus
}
#endif

#endif // NEOC_MULTI_SIG_H
