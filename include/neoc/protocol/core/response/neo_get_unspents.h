/**
 * @file neo_get_unspents.h
 * @brief Neo unspent transaction outputs (UTXOs) structure
 * 
 * Converted from Swift source: protocol/core/response/NeoGetUnspents.swift
 * Represents unspent transaction outputs for an address with asset details.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_UNSPENTS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_UNSPENTS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unspent transaction structure
 * 
 * Represents a single unspent transaction output (UTXO).
 */
typedef struct {
    char *tx_id;                        /**< Transaction ID */
    int index;                          /**< Output index */
    double value;                       /**< Output value */
} neoc_unspent_transaction_t;

/**
 * @brief Balance structure for unspents
 * 
 * Represents asset balance information including unspent transactions.
 */
typedef struct {
    neoc_unspent_transaction_t *unspent_transactions; /**< Array of unspent transactions */
    size_t unspent_transactions_count;  /**< Number of unspent transactions */
    char *asset_hash;                   /**< Asset hash */
    char *asset_name;                   /**< Asset name */
    char *asset_symbol;                 /**< Asset symbol */
    double amount;                      /**< Total amount */
} neoc_unspents_balance_t;

/**
 * @brief Unspents structure
 * 
 * Contains all unspent transaction outputs for an address.
 */
typedef struct {
    neoc_unspents_balance_t *balances;  /**< Array of asset balances */
    size_t balances_count;              /**< Number of asset balances */
    char *address;                      /**< Address */
} neoc_unspents_t;

/**
 * @brief JSON-RPC response for Neo unspents
 * 
 * Complete response structure for unspent transaction queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_unspents_t *result;            /**< Unspents result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_neo_get_unspents_response_t;

/**
 * @brief Create a new unspent transaction
 * 
 * @param tx_id Transaction ID
 * @param index Output index
 * @param value Output value
 * @param unspent_tx Output pointer for the created transaction (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_unspent_transaction_create(
    const char *tx_id,
    int index,
    double value,
    neoc_unspent_transaction_t **unspent_tx
);

/**
 * @brief Free an unspent transaction
 * 
 * @param unspent_tx Transaction to free (can be NULL)
 */
void neoc_unspent_transaction_free(
    neoc_unspent_transaction_t *unspent_tx
);

/**
 * @brief Create a new unspents balance
 * 
 * @param unspent_transactions Array of unspent transactions
 * @param unspent_transactions_count Number of unspent transactions
 * @param asset_hash Asset hash
 * @param asset_name Asset name
 * @param asset_symbol Asset symbol
 * @param amount Total amount
 * @param balance Output pointer for the created balance (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_unspents_balance_create(
    const neoc_unspent_transaction_t *unspent_transactions,
    size_t unspent_transactions_count,
    const char *asset_hash,
    const char *asset_name,
    const char *asset_symbol,
    double amount,
    neoc_unspents_balance_t **balance
);

/**
 * @brief Free an unspents balance
 * 
 * @param balance Balance to free (can be NULL)
 */
void neoc_unspents_balance_free(
    neoc_unspents_balance_t *balance
);

/**
 * @brief Create a new unspents structure
 * 
 * @param balances Array of asset balances
 * @param balances_count Number of asset balances
 * @param address Address
 * @param unspents Output pointer for the created unspents (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_unspents_create(
    const neoc_unspents_balance_t *balances,
    size_t balances_count,
    const char *address,
    neoc_unspents_t **unspents
);

/**
 * @brief Free an unspents structure
 * 
 * @param unspents Unspents to free (can be NULL)
 */
void neoc_unspents_free(
    neoc_unspents_t *unspents
);

/**
 * @brief Create a new unspents response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Unspents result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unspents_response_create(
    const char *jsonrpc,
    int id,
    neoc_unspents_t *result,
    const char *error,
    int error_code,
    neoc_neo_get_unspents_response_t **response
);

/**
 * @brief Free an unspents response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_neo_get_unspents_response_free(
    neoc_neo_get_unspents_response_t *response
);

/**
 * @brief Parse unspents from JSON string
 * 
 * @param json_str JSON string containing unspents data
 * @param unspents Output pointer for the parsed unspents (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_unspents_from_json(
    const char *json_str,
    neoc_unspents_t **unspents
);

/**
 * @brief Parse unspents response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unspents_response_from_json(
    const char *json_str,
    neoc_neo_get_unspents_response_t **response
);

/**
 * @brief Get total balance for a specific asset
 * 
 * @param unspents Unspents structure
 * @param asset_hash Asset hash to search for
 * @param total_balance Output pointer for total balance
 * @return NEOC_SUCCESS if asset found, error code otherwise
 */
neoc_error_t neoc_unspents_get_asset_balance(
    const neoc_unspents_t *unspents,
    const char *asset_hash,
    double *total_balance
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_UNSPENTS_H */
