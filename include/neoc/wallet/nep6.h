/**
 * @file nep6.h
 * @brief NEP-6 wallet file format support
 */

#ifndef NEOC_NEP6_H
#define NEOC_NEP6_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/wallet/nep6/nep6_account.h"
#include "neoc/wallet/nep6/nep6_contract.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct neoc_nep6_wallet_t neoc_nep6_wallet_t;

/**
 * @brief NEP-6 wallet scrypt parameters
 */
typedef struct {
    uint32_t n;
    uint32_t r;
    uint32_t p;
} neoc_nep6_scrypt_params_t;

/**
 * @brief NEP-6 contract parameter type
 */
typedef enum {
    NEP6_PARAM_SIGNATURE = 0,
    NEP6_PARAM_BOOLEAN = 1,
    NEP6_PARAM_INTEGER = 2,
    NEP6_PARAM_HASH160 = 3,
    NEP6_PARAM_HASH256 = 4,
    NEP6_PARAM_BYTES = 5,
    NEP6_PARAM_PUBLIC_KEY = 6,
    NEP6_PARAM_STRING = 7,
    NEP6_PARAM_ARRAY = 16,
    NEP6_PARAM_MAP = 17,
    NEP6_PARAM_INTEROP_INTERFACE = 18,
    NEP6_PARAM_VOID = 255
} neoc_nep6_param_type_t;

/**
 * @brief Create a new NEP-6 wallet
 * 
 * @param name Wallet name
 * @param version Wallet version (e.g., "1.0")
 * @param wallet Output wallet handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_create(const char *name,
                                      const char *version,
                                      neoc_nep6_wallet_t **wallet);

/**
 * @brief Load a NEP-6 wallet from JSON string
 * 
 * @param json JSON string containing wallet data
 * @param wallet Output wallet handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_from_json(const char *json,
                                         neoc_nep6_wallet_t **wallet);

/**
 * @brief Load a NEP-6 wallet from file
 * 
 * @param filename Path to wallet file
 * @param wallet Output wallet handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_from_file(const char *filename,
                                         neoc_nep6_wallet_t **wallet);

/**
 * @brief Save a NEP-6 wallet to JSON string
 * 
 * @param wallet Wallet handle
 * @param json Output JSON string (caller must free)
 * @param json_len Output JSON string length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_to_json(const neoc_nep6_wallet_t *wallet,
                                       char **json,
                                       size_t *json_len);

/**
 * @brief Save a NEP-6 wallet to file
 * 
 * @param wallet Wallet handle
 * @param filename Path to save wallet file
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_to_file(const neoc_nep6_wallet_t *wallet,
                                       const char *filename);

/**
 * @brief Add an account to a NEP-6 wallet
 * 
 * @param wallet Wallet handle
 * @param private_key Private key (32 bytes)
 * @param password Password for encryption
 * @param label Optional account label
 * @param is_default Set as default account
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_add_account(neoc_nep6_wallet_t *wallet,
                                           const uint8_t *private_key,
                                           const char *password,
                                           const char *label,
                                           bool is_default);

/**
 * @brief Remove an account from a NEP-6 wallet
 * 
 * @param wallet Wallet handle
 * @param address Account address
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_remove_account(neoc_nep6_wallet_t *wallet,
                                              const char *address);

/**
 * @brief Get account by address
 * 
 * @param wallet Wallet handle
 * @param address Account address
 * @param account Output account handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_get_account(const neoc_nep6_wallet_t *wallet,
                                           const char *address,
                                           neoc_nep6_account_t **account);

/**
 * @brief Get default account
 * 
 * @param wallet Wallet handle
 * @param account Output account handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_get_default_account(const neoc_nep6_wallet_t *wallet,
                                                   neoc_nep6_account_t **account);

/**
 * @brief Get number of accounts in wallet
 * 
 * @param wallet Wallet handle
 * @return Number of accounts
 */
size_t neoc_nep6_wallet_get_account_count(const neoc_nep6_wallet_t *wallet);

/**
 * @brief Get account by index
 * 
 * @param wallet Wallet handle
 * @param index Account index
 * @param account Output account handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_wallet_get_account_by_index(const neoc_nep6_wallet_t *wallet,
                                                    size_t index,
                                                    neoc_nep6_account_t **account);

/**
 * @brief Decrypt account private key
 * 
 * @param account Account handle
 * @param password Password for decryption
 * @param private_key Output private key (32 bytes)
 * @param private_key_len Size of output buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep6_account_decrypt_private_key(const neoc_nep6_account_t *account,
                                                    const char *password,
                                                    uint8_t *private_key,
                                                    size_t private_key_len);

/**
 * @brief Get account address
 * 
 * @param account Account handle
 * @return Account address (do not free)
 */
const char* neoc_nep6_account_get_address(const neoc_nep6_account_t *account);

/**
 * @brief Get account label
 * 
 * @param account Account handle
 * @return Account label (do not free), or NULL if no label
 */
const char* neoc_nep6_account_get_label(const neoc_nep6_account_t *account);

/**
 * @brief Check if account is default
 * 
 * @param account Account handle
 * @return true if default account, false otherwise
 */
bool neoc_nep6_account_is_default(const neoc_nep6_account_t *account);

/**
 * @brief Free a NEP-6 wallet
 * 
 * @param wallet Wallet handle
 */
void neoc_nep6_wallet_free(neoc_nep6_wallet_t *wallet);

/**
 * @brief Free a NEP-6 account
 * 
 * @param account Account handle
 */
void neoc_nep6_account_free(neoc_nep6_account_t *account);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEP6_H
