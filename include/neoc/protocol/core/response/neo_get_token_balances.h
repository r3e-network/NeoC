/**
 * @file neo_get_token_balances.h
 * @brief Token balances structure for generic token queries
 * 
 * Converted from Swift source: protocol/core/response/NeoGetTokenBalances.swift
 * Provides generic protocol for token balance queries (NEP-11, NEP-17, etc.).
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_TOKEN_BALANCES_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_TOKEN_BALANCES_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Token balance structure (generic)
 * 
 * Base structure for representing any token balance.
 */
typedef struct {
    neoc_hash160_t *asset_hash;         /**< Asset contract hash */
} neoc_token_balance_t;

/**
 * @brief Token balances structure (generic)
 * 
 * Container for multiple token balances for a single address.
 */
typedef struct {
    char *address;                      /**< Account address */
    neoc_token_balance_t *balances;     /**< Array of token balances */
    size_t balances_count;              /**< Number of token balances */
} neoc_token_balances_t;

/**
 * @brief JSON-RPC response for token balances
 * 
 * Complete response structure for generic token balance queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_token_balances_t *result;      /**< Token balances result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_neo_get_token_balances_response_t;

/**
 * @brief Create a new token balance
 * 
 * @param asset_hash Asset contract hash
 * @param balance Output pointer for the created balance (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_balance_create(
    const neoc_hash160_t *asset_hash,
    neoc_token_balance_t **balance
);

/**
 * @brief Free a token balance
 * 
 * @param balance Balance to free (can be NULL)
 */
void neoc_token_balance_free(
    neoc_token_balance_t *balance
);

/**
 * @brief Create a new token balances structure
 * 
 * @param address Account address
 * @param balances Array of token balances
 * @param balances_count Number of token balances
 * @param token_balances Output pointer for the created structure (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_balances_create(
    const char *address,
    const neoc_token_balance_t *balances,
    size_t balances_count,
    neoc_token_balances_t **token_balances
);

/**
 * @brief Free a token balances structure
 * 
 * @param token_balances Structure to free (can be NULL)
 */
void neoc_token_balances_free(
    neoc_token_balances_t *token_balances
);

/**
 * @brief Create a new token balances response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Token balances result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_token_balances_response_create(
    const char *jsonrpc,
    int id,
    neoc_token_balances_t *result,
    const char *error,
    int error_code,
    neoc_neo_get_token_balances_response_t **response
);

/**
 * @brief Free a token balances response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_neo_get_token_balances_response_free(
    neoc_neo_get_token_balances_response_t *response
);

/**
 * @brief Parse token balances from JSON string
 * 
 * @param json_str JSON string containing token balances data
 * @param token_balances Output pointer for the parsed structure (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_balances_from_json(
    const char *json_str,
    neoc_token_balances_t **token_balances
);

/**
 * @brief Serialize token balances to JSON string
 *
 * @param token_balances Token balances structure to serialize
 * @param json_string Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_balances_to_json(
    const neoc_token_balances_t *token_balances,
    char **json_string
);

/**
 * @brief Parse token balances response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_token_balances_response_from_json(
    const char *json_str,
    neoc_neo_get_token_balances_response_t **response
);

/**
 * @brief Serialize token balances response to JSON string
 *
 * @param response Response to serialize
 * @param json_string Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_token_balances_response_to_json(
    const neoc_neo_get_token_balances_response_t *response,
    char **json_string
);

/**
 * @brief Find balance for a specific asset
 * 
 * @param token_balances Token balances structure
 * @param asset_hash Asset hash to search for
 * @param balance Output pointer to balance (not owned by caller)
 * @return NEOC_SUCCESS if found, error code otherwise
 */
neoc_error_t neoc_token_balances_find_asset(
    const neoc_token_balances_t *token_balances,
    const neoc_hash160_t *asset_hash,
    const neoc_token_balance_t **balance
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_TOKEN_BALANCES_H */
