/**
 * @file gas_token.h
 * @brief GAS token contract interface
 */

#ifndef NEOC_GAS_TOKEN_H
#define NEOC_GAS_TOKEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/contract/fungible_token.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

// GAS token contract hash
extern const neoc_hash160_t NEOC_GAS_TOKEN_HASH;

#define NEOC_GAS_TOKEN_NAME     "GasToken"
#define NEOC_GAS_TOKEN_SYMBOL   "GAS"
#define NEOC_GAS_TOKEN_DECIMALS 8

/**
 * GAS token structure
 */
typedef struct neoc_gas_token {
    neoc_fungible_token_t *fungible;    /**< Underlying fungible token wrapper */
} neoc_gas_token_t;

/**
 * @brief Transfer parameters used for multi-transfer script generation.
 */
typedef struct neoc_gas_token_transfer_request {
    neoc_hash160_t to;                  /**< Recipient script hash */
    uint64_t amount;                    /**< Transfer amount (token fractions) */
    const uint8_t *data;                /**< Optional onPayment data payload */
    size_t data_len;                    /**< Length of data payload */
} neoc_gas_token_transfer_request_t;

/**
 * Create GAS token instance
 * @param token Output GAS token
 * @return Error code
 */
neoc_error_t neoc_gas_token_create(neoc_gas_token_t **token);

/**
 * Get GAS balance for account
 * @param account Account hash
 * @param balance Output balance
 * @return Error code
 */
neoc_error_t neoc_gas_token_balance_of(neoc_gas_token_t *token,
                                       const neoc_hash160_t *account,
                                       uint64_t *balance);

/**
 * Transfer GAS
 * @param from Source account
 * @param to Destination account
 * @param amount Amount to transfer (in fractions)
 * @param data Additional data
 * @param data_len Data length
 * @return Error code
 */
neoc_error_t neoc_gas_token_transfer(neoc_gas_token_t *token,
                                     const neoc_hash160_t *from,
                                     const neoc_hash160_t *to,
                                     uint64_t amount,
                                     const uint8_t *data,
                                     size_t data_len);

/**
 * Refuel GAS for transaction
 * @param account Account to refuel
 * @param amount Amount of GAS
 * @return Error code
 */
neoc_error_t neoc_gas_token_refuel(neoc_hash160_t *account,
                                    int64_t amount);

/**
 * Get the GAS token name.
 * The returned string must be freed with neoc_free().
 */
neoc_error_t neoc_gas_token_get_name(neoc_gas_token_t *token, char **name);

/**
 * Get the GAS token symbol.
 * The returned string must be freed with neoc_free().
 */
neoc_error_t neoc_gas_token_get_symbol(neoc_gas_token_t *token, char **symbol);

/**
 * Get the number of decimals (fractions) supported by GAS.
 */
neoc_error_t neoc_gas_token_get_decimals(neoc_gas_token_t *token, int *decimals);

/**
 * Get the GAS contract script hash.
 */
neoc_error_t neoc_gas_token_get_script_hash(neoc_gas_token_t *token,
                                            neoc_hash160_t *script_hash);

/**
 * Get the cached total supply for GAS in token fractions.
 */
neoc_error_t neoc_gas_token_get_total_supply(neoc_gas_token_t *token,
                                             uint64_t *total_supply);

/**
 * Build a GAS transfer script invoking transfer(from, to, amount, data).
 * The returned script buffer must be freed with neoc_free().
 */
neoc_error_t neoc_gas_token_build_transfer_script(
    neoc_gas_token_t *token,
    const neoc_hash160_t *from,
    const neoc_hash160_t *to,
    uint64_t amount,
    const uint8_t *data,
    size_t data_len,
    uint8_t **script,
    size_t *script_len);

/**
 * Build a GAS multi-transfer script by concatenating transfer calls.
 * The returned script buffer must be freed with neoc_free().
 */
neoc_error_t neoc_gas_token_build_multi_transfer_script(
    neoc_gas_token_t *token,
    const neoc_hash160_t *from,
    const neoc_gas_token_transfer_request_t *transfers,
    size_t transfer_count,
    uint8_t **script,
    size_t *script_len);

/**
 * Determine if the GAS token is a native contract (always true).
 */
neoc_error_t neoc_gas_token_is_native_contract(neoc_gas_token_t *token,
                                               bool *is_native);

/**
 * Get the GAS contract version. Returns 0 for current Neo N3 deployment.
 */
neoc_error_t neoc_gas_token_get_contract_version(neoc_gas_token_t *token,
                                                 int *version);

/**
 * Get list of supported method names for the GAS native contract.
 * The caller is responsible for freeing the array and each string.
 */
neoc_error_t neoc_gas_token_get_supported_methods(neoc_gas_token_t *token,
                                                  char ***methods,
                                                  size_t *method_count);

/**
 * Free GAS token
 * @param token Token to free
 */
void neoc_gas_token_free(neoc_gas_token_t *token);

#ifdef __cplusplus
}
#endif

#endif // NEOC_GAS_TOKEN_H
