#ifndef NEOC_WALLET_H
#define NEOC_WALLET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/wallet/account.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wallet structure for managing multiple accounts
 */
typedef struct {
    char *name;                  // Wallet name
    char *version;               // Wallet version
    neoc_account_t **accounts;   // Array of accounts
    size_t account_count;        // Number of accounts
    size_t account_capacity;     // Capacity of accounts array
    neoc_account_t *default_account; // Default account
    void *extra;                 // Extra data for extensions
} neoc_wallet_t;

/**
 * @brief Create a new empty wallet
 * 
 * @param name Wallet name
 * @param wallet Output wallet (caller must free with neoc_wallet_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_create(const char *name, neoc_wallet_t **wallet);

/**
 * @brief Load wallet from NEP-6 JSON file
 * 
 * @param path Path to wallet file
 * @param wallet Output wallet (caller must free with neoc_wallet_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_load(const char *path, neoc_wallet_t **wallet);

/**
 * @brief Save wallet to NEP-6 JSON file
 * 
 * @param wallet The wallet to save
 * @param path Path to save wallet
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_save(const neoc_wallet_t *wallet, const char *path);

/**
 * @brief Add an account to the wallet
 * 
 * @param wallet The wallet
 * @param account The account to add (wallet takes ownership)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_add_account(neoc_wallet_t *wallet, neoc_account_t *account);

/**
 * @brief Remove an account from the wallet
 * 
 * @param wallet The wallet
 * @param address Address of account to remove
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_remove_account(neoc_wallet_t *wallet, const char *address);

/**
 * @brief Get an account by address
 * 
 * @param wallet The wallet
 * @param address The address to search for
 * @return Account if found, NULL otherwise (do not free)
 */
neoc_account_t* neoc_wallet_get_account(const neoc_wallet_t *wallet, const char *address);

/**
 * @brief Get wallet name
 * 
 * @param wallet The wallet
 * @return Wallet name (do not free)
 */
const char* neoc_wallet_get_name(const neoc_wallet_t *wallet);

/**
 * @brief Get account by script hash
 * 
 * @param wallet The wallet
 * @param script_hash The script hash to search for
 * @return Account if found, NULL otherwise (do not free)
 */
neoc_account_t* neoc_wallet_get_account_by_script_hash(const neoc_wallet_t *wallet,
                                                        const neoc_hash160_t *script_hash);

/**
 * @brief Get all accounts in the wallet
 * 
 * @param wallet The wallet
 * @param count Output number of accounts
 * @return Array of accounts (do not free individual accounts)
 */
neoc_account_t** neoc_wallet_get_accounts(const neoc_wallet_t *wallet, size_t *count);

/**
 * @brief Get the default account
 * 
 * @param wallet The wallet
 * @return Default account or NULL (do not free)
 */
neoc_account_t* neoc_wallet_get_default_account(const neoc_wallet_t *wallet);

/**
 * @brief Set the default account
 * 
 * @param wallet The wallet
 * @param address Address of account to set as default
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_set_default_account(neoc_wallet_t *wallet, const char *address);

/**
 * @brief Create a new account and add to wallet
 * 
 * @param wallet The wallet
 * @param label Account label
 * @return Newly created account (do not free separately)
 */
neoc_account_t* neoc_wallet_create_account(neoc_wallet_t *wallet, const char *label);

/**
 * @brief Import account from WIF and add to wallet
 * 
 * @param wallet The wallet
 * @param wif WIF string
 * @param label Account label
 * @return Imported account (do not free separately)
 */
neoc_account_t* neoc_wallet_import_from_wif(neoc_wallet_t *wallet,
                                             const char *wif,
                                             const char *label);

/**
 * @brief Import account from NEP-2 and add to wallet
 * 
 * @param wallet The wallet
 * @param nep2 NEP-2 encrypted key
 * @param passphrase Passphrase to decrypt
 * @param label Account label
 * @return Imported account (do not free separately)
 */
neoc_account_t* neoc_wallet_import_from_nep2(neoc_wallet_t *wallet,
                                              const char *nep2,
                                              const char *passphrase,
                                              const char *label);

/**
 * @brief Check if wallet contains an address
 * 
 * @param wallet The wallet
 * @param address The address to check
 * @return true if wallet contains the address
 */
bool neoc_wallet_contains(const neoc_wallet_t *wallet, const char *address);

/**
 * @brief Get the number of accounts in the wallet
 * 
 * @param wallet The wallet
 * @return Number of accounts
 */
size_t neoc_wallet_get_account_count(const neoc_wallet_t *wallet);

/**
 * @brief Lock all accounts in the wallet
 * 
 * @param wallet The wallet
 * @param passphrase Passphrase for encryption
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_lock_all(neoc_wallet_t *wallet, const char *passphrase);

/**
 * @brief Unlock all accounts in the wallet
 * 
 * @param wallet The wallet
 * @param passphrase Passphrase for decryption
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_unlock_all(neoc_wallet_t *wallet, const char *passphrase);

/**
 * @brief Free a wallet and all its accounts
 * 
 * @param wallet The wallet to free
 */
void neoc_wallet_free(neoc_wallet_t *wallet);

#ifdef __cplusplus
}
#endif

#endif // NEOC_WALLET_H
