/**
 * @file neo_get_wallet_balance.h
 * @brief Neo wallet balance response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetWalletBalance.swift
 * Provides functionality for retrieving wallet token balance information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_WALLET_BALANCE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_WALLET_BALANCE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents wallet balance information for a specific token
 * 
 * This structure contains the balance amount for a token in the wallet.
 * The balance is represented as a string to preserve precision.
 */
typedef struct {
    char *balance;                      /**< Token balance as string */
} neoc_wallet_balance_t;

/**
 * @brief Response structure for getwalletbalance RPC call
 * 
 * Contains the wallet balance information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_wallet_balance_t *result;      /**< Wallet balance result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_wallet_balance_t;

/**
 * @brief Create a new wallet balance structure
 * 
 * @param balance Token balance as string
 * @param wallet_balance Pointer to store the created balance (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_wallet_balance_create(
    const char *balance,
    neoc_wallet_balance_t **wallet_balance
);

/**
 * @brief Free a wallet balance structure
 * 
 * @param wallet_balance Balance to free
 */
void neoc_wallet_balance_free(neoc_wallet_balance_t *wallet_balance);

/**
 * @brief Create a new Neo get wallet balance response
 * 
 * @param id Request ID
 * @param wallet_balance Wallet balance data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_wallet_balance_create(
    int id,
    neoc_wallet_balance_t *wallet_balance,
    const char *error,
    int error_code,
    neoc_neo_get_wallet_balance_t **response
);

/**
 * @brief Free a Neo get wallet balance response
 * 
 * @param response Response to free
 */
void neoc_neo_get_wallet_balance_free(neoc_neo_get_wallet_balance_t *response);

/**
 * @brief Parse JSON response into Neo get wallet balance structure
 * 
 * This function handles both "balance" and "Balance" field names to match
 * the Swift implementation's flexible JSON decoding.
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_wallet_balance_from_json(
    const char *json_str,
    neoc_neo_get_wallet_balance_t **response
);

/**
 * @brief Convert Neo get wallet balance response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_wallet_balance_to_json(
    const neoc_neo_get_wallet_balance_t *response,
    char **json_str
);

/**
 * @brief Get wallet balance from response (convenience function)
 * 
 * @param response Response to get balance from
 * @return Wallet balance pointer, or NULL if error or no result
 */
neoc_wallet_balance_t *neoc_neo_get_wallet_balance_get_balance(
    const neoc_neo_get_wallet_balance_t *response
);

/**
 * @brief Check if the response contains valid wallet balance data
 * 
 * @param response Response to check
 * @return true if response has valid balance, false otherwise
 */
bool neoc_neo_get_wallet_balance_has_balance(
    const neoc_neo_get_wallet_balance_t *response
);

/**
 * @brief Get balance amount as string from response
 * 
 * @param response Response to get balance from
 * @return Balance string, or NULL if error or no result
 */
const char *neoc_neo_get_wallet_balance_get_balance_string(
    const neoc_neo_get_wallet_balance_t *response
);

/**
 * @brief Get balance amount as double from response
 * 
 * @param response Response to get balance from
 * @param balance Pointer to store the balance value (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_wallet_balance_get_balance_double(
    const neoc_neo_get_wallet_balance_t *response,
    double *balance
);

/**
 * @brief Check if wallet has any balance for the token
 * 
 * @param response Response to check
 * @return true if wallet has non-zero balance, false otherwise
 */
bool neoc_neo_get_wallet_balance_has_funds(
    const neoc_neo_get_wallet_balance_t *response
);

/**
 * @brief Copy balance string from response
 * 
 * @param response Response to get balance from
 * @param balance_copy Pointer to store the copied balance string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_wallet_balance_copy_balance_string(
    const neoc_neo_get_wallet_balance_t *response,
    char **balance_copy
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_WALLET_BALANCE_H */
