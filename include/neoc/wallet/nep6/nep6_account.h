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

#ifndef NEOC_PP_OVERLOAD
#define NEOC_PP_CONCAT(a,b) NEOC_PP_CONCAT_IMPL(a,b)
#define NEOC_PP_CONCAT_IMPL(a,b) a##b
#define NEOC_PP_NARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define NEOC_PP_NARGS(...) NEOC_PP_NARGS_IMPL(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
#define NEOC_PP_OVERLOAD(prefix, ...) NEOC_PP_CONCAT(prefix, NEOC_PP_NARGS(__VA_ARGS__))(__VA_ARGS__)
#endif

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

neoc_error_t neoc_nep6_account_set_address(neoc_nep6_account_t *account, const char *address);
const char* neoc_nep6_account_get_address(const neoc_nep6_account_t *account);

neoc_error_t neoc_nep6_account_set_label(neoc_nep6_account_t *account, const char *label);
const char* neoc_nep6_account_get_label(const neoc_nep6_account_t *account);

neoc_error_t neoc_nep6_account_set_default(neoc_nep6_account_t *account, bool is_default);
bool neoc_nep6_account_is_default(const neoc_nep6_account_t *account);

neoc_error_t neoc_nep6_account_set_locked(neoc_nep6_account_t *account, bool lock);
bool neoc_nep6_account_is_locked(const neoc_nep6_account_t *account);

neoc_error_t neoc_nep6_account_set_key(neoc_nep6_account_t *account, const char *key);
const char* neoc_nep6_account_get_key(const neoc_nep6_account_t *account);

neoc_error_t neoc_nep6_account_set_contract(neoc_nep6_account_t *account,
                                            neoc_nep6_contract_t *contract);
neoc_nep6_contract_t* neoc_nep6_account_get_contract(const neoc_nep6_account_t *account);

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

#ifndef NEOC_NEP6_ACCOUNT_DISABLE_OVERLOADS
#define NEOC_NEP6_ACCOUNT_CREATE_1(account_out) \
    neoc_nep6_account_create(NULL, NULL, false, false, NULL, NULL, account_out)
#define NEOC_NEP6_ACCOUNT_CREATE_3(address, label, account_out) \
    neoc_nep6_account_create(address, label, false, false, NULL, NULL, account_out)
#define NEOC_NEP6_ACCOUNT_CREATE_6(address, label, is_default, lock, key, account_out) \
    neoc_nep6_account_create(address, label, is_default, lock, key, NULL, account_out)
#define NEOC_NEP6_ACCOUNT_CREATE_7(address, label, is_default, lock, key, contract, account_out) \
    neoc_nep6_account_create(address, label, is_default, lock, key, contract, account_out)
#define neoc_nep6_account_create(...) \
    NEOC_PP_OVERLOAD(NEOC_NEP6_ACCOUNT_CREATE_, __VA_ARGS__)
#endif /* NEOC_NEP6_ACCOUNT_DISABLE_OVERLOADS */

#ifdef __cplusplus
}
#endif

#endif /* NEOC_WALLET_NEP6_NEP6_ACCOUNT_H */
