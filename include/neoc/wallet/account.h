#ifndef NEOC_ACCOUNT_H
#define NEOC_ACCOUNT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum length of a Neo address string
 */
#define NEOC_ADDRESS_LENGTH 64

/**
 * @brief Account structure representing a NEO account
 */
typedef struct {
    char *label;                     // Account label/name
    char *address;                   // NEO address
    neoc_hash160_t script_hash;      // Script hash of the account
    neoc_ec_key_pair_t *key_pair;    // Key pair (may be NULL if encrypted)
    bool is_locked;                  // Whether account is locked
    bool is_default;                 // Whether this is the default account
    uint8_t *encrypted_key;          // NEP-2 encrypted private key
    size_t encrypted_key_len;        // Length of encrypted key
    void *extra;                     // Extra data for extensions
} neoc_account_t;

/**
 * @brief Create a new account with a random key pair
 * 
 * @param label Account label (can be NULL)
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create(const char *label, neoc_account_t **account);

/**
 * @brief Create account from existing key pair
 * 
 * @param label Account label (can be NULL)
 * @param key_pair The key pair to use
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_from_key_pair(const char *label,
                                                const neoc_ec_key_pair_t *key_pair,
                                                neoc_account_t **account);

/**
 * @brief Create account from WIF
 * 
 * @param label Account label (can be NULL)
 * @param wif WIF string
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_from_wif(const char *label,
                                           const char *wif,
                                           neoc_account_t **account);

/**
 * @brief Create account from NEP-2 encrypted key
 * 
 * @param label Account label (can be NULL)
 * @param encrypted_key NEP-2 encrypted key string
 * @param passphrase Passphrase to decrypt the key
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_from_nep2(const char *label,
                                            const char *encrypted_key,
                                            const char *passphrase,
                                            neoc_account_t **account);

/**
 * @brief Create account from address
 * 
 * @param address Neo address string
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_from_address(const char *address, neoc_account_t **account);

/**
 * @brief Create account from public key
 * 
 * @param public_key The public key
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_from_public_key(neoc_ec_public_key_t *public_key,
                                                  neoc_account_t **account);

/**
 * @brief Create account from verification script
 * 
 * @param script Verification script bytes
 * @param script_len Length of script
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_from_verification_script(const uint8_t *script,
                                                          size_t script_len,
                                                          neoc_account_t **account);

/**
 * @brief Create multi-signature account from public keys
 * 
 * @param public_keys Array of public keys
 * @param key_count Number of public keys
 * @param threshold Minimum signatures required
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_multisig_from_public_keys(neoc_ec_public_key_t **public_keys,
                                                           size_t key_count,
                                                           int threshold,
                                                           neoc_account_t **account);

/**
 * @brief Create multi-signature account from address and parameters
 * 
 * @param address Multi-sig address
 * @param threshold Minimum signatures required
 * @param nr_participants Total number of participants
 * @param account Output account (caller must free with neoc_account_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_multisig_with_address(const char *address,
                                                       int threshold,
                                                       int nr_participants,
                                                       neoc_account_t **account);

/**
 * @brief Get the address of an account
 * 
 * @param account The account
 * @param address Output address string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_get_address(const neoc_account_t *account, char **address);

/**
 * @brief Get the script hash of an account
 * 
 * @param account The account
 * @param hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_get_script_hash(const neoc_account_t *account, neoc_hash160_t *hash);

/**
 * @brief Get the label of an account
 * 
 * @param account The account
 * @param label Output label string (caller must free, may be NULL)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_get_label(const neoc_account_t *account, char **label);

/**
 * @brief Get the verification script of an account
 * 
 * @param account The account
 * @param script Output script bytes (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_get_verification_script(const neoc_account_t *account,
                                                   uint8_t **script,
                                                   size_t *script_len);

/**
 * @brief Set the label of an account
 * 
 * @param account The account
 * @param label New label (can be NULL)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_set_label(neoc_account_t *account, const char *label);

/**
 * @brief Check if account is locked
 * 
 * @param account The account
 * @param is_locked Output locked status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_is_locked(const neoc_account_t *account, bool *is_locked);

/**
 * @brief Lock an account
 * 
 * @param account The account to lock
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_lock(neoc_account_t *account);

/**
 * @brief Unlock an account
 * 
 * @param account The account to unlock
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_unlock(neoc_account_t *account);

/**
 * @brief Export account as WIF
 * 
 * @param account The account (must be unlocked)
 * @param wif Output WIF string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_export_wif(const neoc_account_t *account, char **wif);

/**
 * @brief Export account as NEP-2 encrypted key
 * 
 * @param account The account
 * @param passphrase Passphrase for encryption
 * @param nep2 Output NEP-2 string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_export_nep2(const neoc_account_t *account,
                                       const char *passphrase,
                                       char **nep2);

/**
 * @brief Sign data with account's private key
 * 
 * @param account The account (must be unlocked)
 * @param data Data to sign
 * @param data_len Length of data
 * @param signature Output signature (caller must free)
 * @param signature_len Output signature length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_sign(const neoc_account_t *account,
                                const uint8_t *data,
                                size_t data_len,
                                uint8_t **signature,
                                size_t *signature_len);

/**
 * @brief Verify a signature
 * 
 * @param account The account
 * @param data Original data
 * @param data_len Length of data
 * @param signature Signature to verify
 * @param signature_len Length of signature
 * @return true if signature is valid, false otherwise
 */
bool neoc_account_verify(const neoc_account_t *account,
                          const uint8_t *data,
                          size_t data_len,
                          const uint8_t *signature,
                          size_t signature_len);

/**
 * @brief Free an account
 * 
 * @param account The account to free
 */
void neoc_account_free(neoc_account_t *account);

/**
 * @brief Create a multi-signature account
 * 
 * @param label Account label
 * @param threshold Minimum signatures required
 * @param public_keys Array of public keys
 * @param key_count Number of public keys
 * @param account Output account (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_create_multisig(const char *label,
                                           int threshold,
                                           const neoc_ec_public_key_t **public_keys,
                                           size_t key_count,
                                           neoc_account_t **account);

/**
 * @brief Check if account is multi-signature
 * 
 * @param account The account
 * @param is_multisig Output multi-sig status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_is_multisig(const neoc_account_t *account, bool *is_multisig);

/**
 * @brief Get the signing threshold of a multi-sig account
 * 
 * @param account The multi-sig account
 * @param threshold Output signing threshold
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_get_signing_threshold(const neoc_account_t *account, int *threshold);

/**
 * @brief Get the number of participants in a multi-sig account
 * 
 * @param account The multi-sig account
 * @param nr_participants Output number of participants
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_account_get_nr_participants(const neoc_account_t *account, int *nr_participants);

#ifdef __cplusplus
}
#endif

#endif // NEOC_ACCOUNT_H
