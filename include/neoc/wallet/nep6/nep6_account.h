/**
 * @file nep6_account.h
 * @brief NEP-6 wallet account structure
 * 
 * Based on Swift source: wallet/nep6/Nep6Account.swift
 * Represents an account in a NEP-6 wallet format
 */

#ifndef NEOC_WALLET_NEP6_NEP6_ACCOUNT_H
#define NEOC_WALLET_NEP6_NEP6_ACCOUNT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/wallet/nep6/nep6_contract.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extra field entry for NEP-6 account
 */
typedef struct {
    char *key;                              /**< Extra field key */
    char *value;                            /**< Extra field value */
} neoc_nep6_account_extra_t;

/**
 * @brief NEP-6 account structure
 * 
 * Represents an account in a NEP-6 standard wallet
 */
typedef struct {
    char *address;                          /**< Account address */
    char *label;                            /**< Account label (nullable) */
    bool is_default;                        /**< Whether this is the default account */
    bool lock;                              /**< Whether the account is locked */
    char *key;                              /**< Encrypted private key (nullable) */
    neoc_nep6_contract_t *contract;         /**< Account contract (nullable) */
    neoc_nep6_account_extra_t *extra;       /**< Extra fields (nullable) */
    size_t extra_count;                     /**< Number of extra fields */
} neoc_nep6_account_t;

/**
 * @brief Create a new NEP-6 account
 * 
 * @param address Account address
 * @param label Account label (can be NULL)
 * @param is_default Whether this is the default account
 * @param lock Whether the account is locked
 * @param key Encrypted private key (can be NULL)
 * @param contract Account contract (can be NULL)
 * @param account Pointer to store the created account
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_account_create(
    const char *address,
    const char *label,
    bool is_default,
    bool lock,
    const char *key,
    neoc_nep6_contract_t *contract,
    neoc_nep6_account_t **account
);

/**
 * @brief Free a NEP-6 account and its resources
 * 
 * @param account Account to free
 */
void neoc_nep6_account_free(
    neoc_nep6_account_t *account
);

/**
 * @brief Parse JSON into NEP-6 account
 * 
 * @param json_str JSON string to parse
 * @param account Pointer to store the parsed account
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_account_from_json(
    const char *json_str,
    neoc_nep6_account_t **account
);

/**
 * @brief Convert NEP-6 account to JSON string
 * 
 * @param account Account to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_account_to_json(
    const neoc_nep6_account_t *account,
    char **json_str
);

/**
 * @brief Compare two NEP-6 accounts for equality
 * 
 * @param account1 First account
 * @param account2 Second account
 * @return true if accounts are equal, false otherwise
 */
bool neoc_nep6_account_equals(
    const neoc_nep6_account_t *account1,
    const neoc_nep6_account_t *account2
);

/**
 * @brief Create a copy of a NEP-6 account
 * 
 * @param src Source account to copy
 * @param dest Pointer to store the copied account
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_account_copy(
    const neoc_nep6_account_t *src,
    neoc_nep6_account_t **dest
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_WALLET_NEP6_NEP6_ACCOUNT_H */
