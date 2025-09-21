/**
 * @file bip39_account.h
 * @brief BIP-39 compatible NEO account implementation
 * 
 * Based on Swift source: wallet/Bip39Account.swift
 * Class encapsulating a BIP-39 compatible NEO account with mnemonic phrase support
 */

#ifndef NEOC_WALLET_BIP39_ACCOUNT_H
#define NEOC_WALLET_BIP39_ACCOUNT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/wallet/account.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/bip39.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// BIP-39 constants
#define NEOC_BIP39_MAX_MNEMONIC_LENGTH 512
#define NEOC_BIP39_MAX_PASSWORD_LENGTH 256
#define NEOC_BIP39_WORD_COUNT_12 12
#define NEOC_BIP39_WORD_COUNT_15 15
#define NEOC_BIP39_WORD_COUNT_18 18
#define NEOC_BIP39_WORD_COUNT_21 21
#define NEOC_BIP39_WORD_COUNT_24 24

/**
 * @brief BIP-39 account structure
 * 
 * Extends the basic account with BIP-39 mnemonic functionality
 */
typedef struct {
    neoc_account_t *base_account;    /**< Base account functionality */
    char *mnemonic;                  /**< Generated BIP-39 mnemonic phrase */
    size_t mnemonic_length;          /**< Length of mnemonic string */
    int word_count;                  /**< Number of words in mnemonic (12, 15, 18, 21, or 24) */
    bool is_password_protected;      /**< True if created with password */
} neoc_bip39_account_t;

/**
 * @brief Create a new BIP-39 compatible NEO account
 * 
 * Equivalent to Swift Bip39Account.create(_ password:)
 * Generates a BIP-39 compatible NEO account. The private key is calculated using:
 * Key = SHA-256(BIP_39_SEED(mnemonic, password))
 * 
 * @param password The passphrase used for BIP-39 seed generation
 * @param word_count Number of words in mnemonic (12, 15, 18, 21, or 24)
 * @param bip39_account Output BIP-39 account (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_create(const char *password,
                                        int word_count,
                                        neoc_bip39_account_t **bip39_account);

/**
 * @brief Create BIP-39 account with random mnemonic (no password)
 * 
 * @param word_count Number of words in mnemonic (12, 15, 18, 21, or 24)
 * @param bip39_account Output BIP-39 account (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_create_random(int word_count,
                                               neoc_bip39_account_t **bip39_account);

/**
 * @brief Recover BIP-39 account from existing mnemonic and password
 * 
 * Equivalent to Swift Bip39Account.fromBip39Mneumonic(_ password:, _ mnemonic:)
 * 
 * @param password The passphrase used when the account was generated
 * @param mnemonic The BIP-39 mnemonic phrase
 * @param bip39_account Output recovered BIP-39 account (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_from_mnemonic(const char *password,
                                               const char *mnemonic,
                                               neoc_bip39_account_t **bip39_account);

/**
 * @brief Recover BIP-39 account from mnemonic without password
 * 
 * @param mnemonic The BIP-39 mnemonic phrase
 * @param bip39_account Output recovered BIP-39 account (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_from_mnemonic_no_password(const char *mnemonic,
                                                           neoc_bip39_account_t **bip39_account);

/**
 * @brief Get the mnemonic phrase from BIP-39 account
 * 
 * Equivalent to Swift Bip39Account.mnemonic
 * 
 * @param bip39_account The BIP-39 account
 * @return Mnemonic phrase string (do not free)
 */
const char *neoc_bip39_account_get_mnemonic(const neoc_bip39_account_t *bip39_account);

/**
 * @brief Get the base account from BIP-39 account
 * 
 * @param bip39_account The BIP-39 account
 * @return Base account (do not free separately)
 */
const neoc_account_t *neoc_bip39_account_get_base_account(const neoc_bip39_account_t *bip39_account);

/**
 * @brief Get word count from BIP-39 account
 * 
 * @param bip39_account The BIP-39 account
 * @return Number of words in mnemonic
 */
int neoc_bip39_account_get_word_count(const neoc_bip39_account_t *bip39_account);

/**
 * @brief Check if BIP-39 account was created with password
 * 
 * @param bip39_account The BIP-39 account
 * @return True if password was used
 */
bool neoc_bip39_account_is_password_protected(const neoc_bip39_account_t *bip39_account);

/**
 * @brief Validate a BIP-39 mnemonic phrase
 * 
 * @param mnemonic The mnemonic phrase to validate
 * @return True if valid BIP-39 mnemonic
 */
bool neoc_bip39_account_validate_mnemonic(const char *mnemonic);

/**
 * @brief Generate a random BIP-39 mnemonic phrase
 * 
 * @param word_count Number of words (12, 15, 18, 21, or 24)
 * @param mnemonic Output mnemonic phrase (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_generate_mnemonic(int word_count, char **mnemonic);

/**
 * @brief Convert mnemonic to seed bytes
 * 
 * @param mnemonic The mnemonic phrase
 * @param password Optional password (can be NULL)
 * @param seed Output seed bytes (64 bytes, caller must free)
 * @param seed_length Output: always 64
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_mnemonic_to_seed(const char *mnemonic,
                                                  const char *password,
                                                  uint8_t **seed,
                                                  size_t *seed_length);

/**
 * @brief Get private key from BIP-39 account as bytes
 * 
 * @param bip39_account The BIP-39 account
 * @param private_key Output private key bytes (32 bytes)
 * @param key_length Input/output: buffer size/actual size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_get_private_key(const neoc_bip39_account_t *bip39_account,
                                                 uint8_t *private_key,
                                                 size_t *key_length);

/**
 * @brief Get public key from BIP-39 account
 * 
 * @param bip39_account The BIP-39 account
 * @param compressed Whether to get compressed format
 * @param public_key Output public key bytes (caller must free)
 * @param key_length Output: public key length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_get_public_key(const neoc_bip39_account_t *bip39_account,
                                                bool compressed,
                                                uint8_t **public_key,
                                                size_t *key_length);

/**
 * @brief Get address from BIP-39 account
 * 
 * @param bip39_account The BIP-39 account
 * @param address Output address string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_get_address(const neoc_bip39_account_t *bip39_account,
                                             char **address);

/**
 * @brief Export BIP-39 account to WIF format
 * 
 * @param bip39_account The BIP-39 account
 * @param wif Output WIF string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_export_wif(const neoc_bip39_account_t *bip39_account,
                                            char **wif);

/**
 * @brief Copy a BIP-39 account
 * 
 * @param source Source BIP-39 account
 * @param copy Output copied account (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_account_copy(const neoc_bip39_account_t *source,
                                      neoc_bip39_account_t **copy);

/**
 * @brief Check if two BIP-39 accounts are equal
 * 
 * @param account1 First BIP-39 account
 * @param account2 Second BIP-39 account
 * @return True if accounts are equal (same mnemonic and address)
 */
bool neoc_bip39_account_equals(const neoc_bip39_account_t *account1,
                                const neoc_bip39_account_t *account2);

/**
 * @brief Free a BIP-39 account
 * 
 * @param bip39_account The BIP-39 account to free
 */
void neoc_bip39_account_free(neoc_bip39_account_t *bip39_account);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_WALLET_BIP39_ACCOUNT_H */
