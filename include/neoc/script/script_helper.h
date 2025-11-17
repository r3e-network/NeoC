/**
 * @file script_helper.h
 * @brief Helper functions for creating verification and invocation scripts
 */

#ifndef NEOC_SCRIPT_HELPER_H
#define NEOC_SCRIPT_HELPER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/crypto/ec_key_pair.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a single signature verification script
 * 
 * @param public_key Public key (33 or 65 bytes)
 * @param public_key_len Length of public key
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_create_single_sig_verification(const uint8_t *public_key,
                                                         size_t public_key_len,
                                                         uint8_t **script,
                                                         size_t *script_len);

/**
 * @brief Create a multi-signature verification script
 * 
 * @param minimum_signatures Minimum required signatures
 * @param public_keys Array of public keys
 * @param public_key_lens Array of public key lengths
 * @param public_key_count Number of public keys
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_create_multisig_verification(uint8_t minimum_signatures,
                                                       const uint8_t **public_keys,
                                                       const size_t *public_key_lens,
                                                       size_t public_key_count,
                                                       uint8_t **script,
                                                       size_t *script_len);

/**
 * @brief Create an invocation script for single signature
 * 
 * @param signature Signature bytes
 * @param signature_len Signature length
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_create_single_sig_invocation(const uint8_t *signature,
                                                       size_t signature_len,
                                                       uint8_t **script,
                                                       size_t *script_len);

/**
 * @brief Create an invocation script for multiple signatures
 * 
 * @param signatures Array of signatures
 * @param signature_lens Array of signature lengths
 * @param signature_count Number of signatures
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_create_multisig_invocation(const uint8_t **signatures,
                                                     const size_t *signature_lens,
                                                     size_t signature_count,
                                                     uint8_t **script,
                                                     size_t *script_len);

/**
 * @brief Create a contract invocation script
 * 
 * @param script_hash Contract script hash
 * @param method Method name
 * @param params Serialized parameters (can be NULL)
 * @param params_len Length of parameters
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_create_contract_invocation(const neoc_hash160_t *script_hash,
                                                     const char *method,
                                                     const uint8_t *params,
                                                     size_t params_len,
                                                     uint8_t **script,
                                                     size_t *script_len);

/**
 * @brief Create an invocation script for an NEP-17 transfer.
 *
 * The resulting script invokes transfer(from, to, amount, data) and appends an ASSERT to
 * ensure the invocation returned true.
 */
neoc_error_t neoc_script_create_nep17_transfer(const neoc_hash160_t *token_hash,
                                               const neoc_hash160_t *from,
                                               const neoc_hash160_t *to,
                                               uint64_t amount,
                                               const uint8_t *data,
                                               size_t data_len,
                                               uint8_t **script,
                                               size_t *script_len);

/**
 * @brief Create a witness script from verification and invocation scripts
 * 
 * @param invocation_script Invocation script
 * @param invocation_len Invocation script length
 * @param verification_script Verification script
 * @param verification_len Verification script length
 * @param witness_script Output witness script (caller must free)
 * @param witness_len Output witness script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_create_witness(const uint8_t *invocation_script,
                                         size_t invocation_len,
                                         const uint8_t *verification_script,
                                         size_t verification_len,
                                         uint8_t **witness_script,
                                         size_t *witness_len);

/**
 * @brief Get script hash from verification script
 * 
 * @param script Verification script
 * @param script_len Script length
 * @param hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_get_hash(const uint8_t *script,
                                   size_t script_len,
                                   neoc_hash160_t *hash);

/**
 * @brief Check if script is a standard contract
 * 
 * @param script Script to check
 * @param script_len Script length
 * @return true if standard contract, false otherwise
 */
bool neoc_script_is_standard_contract(const uint8_t *script, size_t script_len);

/**
 * @brief Check if script is a single signature contract
 * 
 * @param script Script to check
 * @param script_len Script length
 * @return true if single signature contract, false otherwise
 */
bool neoc_script_is_single_sig_contract(const uint8_t *script, size_t script_len);

/**
 * @brief Check if script is a multi-signature contract
 * 
 * @param script Script to check
 * @param script_len Script length
 * @return true if multi-signature contract, false otherwise
 */
bool neoc_script_is_multisig_contract(const uint8_t *script, size_t script_len);

/**
 * @brief Extract public keys from multi-signature script
 * 
 * @param script Multi-signature script
 * @param script_len Script length
 * @param public_keys Output array of public keys (caller must free)
 * @param key_count Output number of keys
 * @param min_signatures Output minimum required signatures
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_extract_multisig_info(const uint8_t *script,
                                                size_t script_len,
                                                uint8_t ***public_keys,
                                                size_t *key_count,
                                                uint8_t *min_signatures);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SCRIPT_HELPER_H
