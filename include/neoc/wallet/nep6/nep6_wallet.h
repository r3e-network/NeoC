/**
 * @file nep6_wallet.h
 * @brief NEP-6 wallet structure and operations
 * 
 * Based on Swift source: wallet/nep6/NEP6Wallet.swift
 * Provides NEP-6 standard wallet format implementation
 */

#ifndef NEOC_WALLET_NEP6_NEP6_WALLET_H
#define NEOC_WALLET_NEP6_NEP6_WALLET_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/wallet/nep6/nep6_account.h"
#include "neoc/wallet/nep6/nep6_contract.h"
#include "neoc/wallet/nep6.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extra field entry for NEP-6 wallet
 */
typedef struct {
    char *key;                              /**< Extra field key */
    char *value;                            /**< Extra field value */
} neoc_nep6_wallet_extra_t;

/**
 * @brief NEP-6 wallet structure (alternative interface)
 * 
 * This structure provides a more direct representation of the
 * Swift NEP6Wallet structure. For most operations, use neoc_nep6_wallet_t
 * from nep6.h which provides the main wallet implementation.
 */
typedef struct {
    char *name;                             /**< Wallet name */
    char *version;                          /**< Wallet version */
    neoc_nep6_scrypt_params_t scrypt;       /**< Scrypt parameters */
    neoc_nep6_account_t **accounts;         /**< Array of accounts */
    size_t account_count;                   /**< Number of accounts */
    neoc_nep6_wallet_extra_t *extra;        /**< Extra fields (nullable) */
    size_t extra_count;                     /**< Number of extra fields */
} neoc_nep6_wallet_struct_t;

/**
 * @brief Create a new NEP-6 wallet structure
 * 
 * @param name Wallet name
 * @param version Wallet version
 * @param wallet Pointer to store the created wallet (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_create(const char *name,
                                             const char *version,
                                             neoc_nep6_wallet_struct_t **wallet);

/**
 * @brief Free a NEP-6 wallet structure
 * 
 * @param wallet Wallet to free
 */
void neoc_nep6_wallet_struct_free(neoc_nep6_wallet_struct_t *wallet);

/**
 * @brief Add an account to the wallet structure
 * 
 * @param wallet Wallet structure
 * @param account Account to add (wallet takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_add_account(neoc_nep6_wallet_struct_t *wallet,
                                                  neoc_nep6_account_t *account);

/**
 * @brief Remove an account from the wallet structure
 * 
 * @param wallet Wallet structure
 * @param address Account address to remove
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_remove_account(neoc_nep6_wallet_struct_t *wallet,
                                                     const char *address);

/**
 * @brief Convert neoc_nep6_wallet_t to neoc_nep6_wallet_struct_t
 * 
 * @param wallet Main wallet implementation
 * @param wallet_struct Output wallet structure (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_to_struct(const neoc_nep6_wallet_t *wallet,
                                         neoc_nep6_wallet_struct_t **wallet_struct);

/**
 * @brief Convert neoc_nep6_wallet_struct_t to neoc_nep6_wallet_t
 * 
 * @param wallet_struct Wallet structure
 * @param wallet Output main wallet implementation (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_from_struct(const neoc_nep6_wallet_struct_t *wallet_struct,
                                           neoc_nep6_wallet_t **wallet);

/**
 * @brief Parse JSON into NEP-6 wallet structure
 * 
 * @param json_str JSON string to parse
 * @param wallet Pointer to store the parsed wallet (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_from_json(const char *json_str,
                                                neoc_nep6_wallet_struct_t **wallet);

/**
 * @brief Convert NEP-6 wallet structure to JSON string
 * 
 * @param wallet Wallet structure to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_to_json(const neoc_nep6_wallet_struct_t *wallet,
                                              char **json_str);

/**
 * @brief Add extra field to wallet structure
 * 
 * @param wallet Wallet structure
 * @param key Extra field key
 * @param value Extra field value
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_add_extra(neoc_nep6_wallet_struct_t *wallet,
                                                const char *key,
                                                const char *value);

/**
 * @brief Get extra field value by key
 * 
 * @param wallet Wallet structure
 * @param key Extra field key
 * @param value Output extra field value (do not free)
 * @return NEOC_SUCCESS if found, NEOC_ERROR_NOT_FOUND if not found
 */
neoc_error_t neoc_nep6_wallet_struct_get_extra(const neoc_nep6_wallet_struct_t *wallet,
                                                const char *key,
                                                const char **value);

/**
 * @brief Compare two NEP-6 wallet structures for equality
 * 
 * Equivalent to Swift's == operator
 * 
 * @param wallet1 First wallet
 * @param wallet2 Second wallet
 * @return true if wallets are equal, false otherwise
 */
bool neoc_nep6_wallet_struct_equals(const neoc_nep6_wallet_struct_t *wallet1,
                                     const neoc_nep6_wallet_struct_t *wallet2);

/**
 * @brief Create a copy of a NEP-6 wallet structure
 * 
 * @param src Source wallet to copy
 * @param dest Output copied wallet (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_wallet_struct_copy(const neoc_nep6_wallet_struct_t *src,
                                           neoc_nep6_wallet_struct_t **dest);

/*
 * Note: For most wallet operations, use the functions in nep6.h:
 * - neoc_nep6_wallet_create()
 * - neoc_nep6_wallet_from_json()
 * - neoc_nep6_wallet_to_json()
 * - neoc_nep6_wallet_add_account()
 * - etc.
 * 
 * This header provides an alternative interface that more closely
 * matches the Swift NEP6Wallet structure.
 */

#ifdef __cplusplus
}
#endif

#endif /* NEOC_WALLET_NEP6_NEP6_WALLET_H */
