/**
 * @file neo_get_nep11_balances.h
 * @brief NEP-11 NFT balances response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetNep11Balances.swift
 * Provides functionality for retrieving NEP-11 (Non-Fungible Token) balance information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP11_BALANCES_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP11_BALANCES_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a single NEP-11 token instance
 * 
 * Contains information about an individual NFT token including its ID,
 * amount, and last update information.
 */
typedef struct {
    char *token_id;                     /**< Unique token identifier */
    char *amount;                       /**< Token amount as string */
    int last_updated_block;             /**< Block when last updated */
} neoc_nep11_token_t;

/**
 * @brief Represents NEP-11 balance for a specific contract
 * 
 * Contains information about all tokens held from a specific NEP-11 contract
 * including contract metadata and individual token instances.
 */
typedef struct {
    char *name;                         /**< Contract name */
    char *symbol;                       /**< Token symbol */
    char *decimals;                     /**< Token decimals as string */
    neoc_hash160_t asset_hash;          /**< Contract hash */
    neoc_nep11_token_t *tokens;         /**< Array of token instances */
    size_t tokens_count;                /**< Number of tokens */
} neoc_nep11_balance_t;

/**
 * @brief NEP-11 balances collection for an address
 * 
 * Contains all NEP-11 token balances for a specific address.
 */
typedef struct {
    char *address;                      /**< Account address */
    neoc_nep11_balance_t *balances;     /**< Array of contract balances */
    size_t balances_count;              /**< Number of contract balances */
} neoc_nep11_balances_t;

/**
 * @brief Response structure for getnep11balances RPC call
 * 
 * Contains the NEP-11 balance information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_nep11_balances_t *result;      /**< NEP-11 balances result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_nep11_balances_t;

/**
 * @brief Create a new NEP-11 token structure
 * 
 * @param token_id Token identifier
 * @param amount Token amount as string
 * @param last_updated_block Last updated block
 * @param token Pointer to store the created token (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep11_token_create(
    const char *token_id,
    const char *amount,
    int last_updated_block,
    neoc_nep11_token_t **token
);

/**
 * @brief Free a NEP-11 token structure
 * 
 * @param token Token to free
 */
void neoc_nep11_token_free(neoc_nep11_token_t *token);

/**
 * @brief Create a new NEP-11 balance structure
 * 
 * @param name Contract name
 * @param symbol Token symbol
 * @param decimals Token decimals
 * @param asset_hash Contract hash
 * @param tokens Array of tokens
 * @param tokens_count Number of tokens
 * @param balance Pointer to store the created balance (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep11_balance_create(
    const char *name,
    const char *symbol,
    const char *decimals,
    const neoc_hash160_t *asset_hash,
    neoc_nep11_token_t *tokens,
    size_t tokens_count,
    neoc_nep11_balance_t **balance
);

/**
 * @brief Free a NEP-11 balance structure
 * 
 * @param balance Balance to free
 */
void neoc_nep11_balance_free(neoc_nep11_balance_t *balance);

/**
 * @brief Create a new NEP-11 balances collection
 * 
 * @param address Account address
 * @param balances Array of contract balances
 * @param balances_count Number of contract balances
 * @param nep11_balances Pointer to store the created collection (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep11_balances_create(
    const char *address,
    neoc_nep11_balance_t *balances,
    size_t balances_count,
    neoc_nep11_balances_t **nep11_balances
);

/**
 * @brief Free a NEP-11 balances collection
 * 
 * @param nep11_balances Collection to free
 */
void neoc_nep11_balances_free(neoc_nep11_balances_t *nep11_balances);

/**
 * @brief Create a new Neo get NEP-11 balances response
 * 
 * @param id Request ID
 * @param nep11_balances NEP-11 balances data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_balances_create(
    int id,
    neoc_nep11_balances_t *nep11_balances,
    const char *error,
    int error_code,
    neoc_neo_get_nep11_balances_t **response
);

/**
 * @brief Free a Neo get NEP-11 balances response
 * 
 * @param response Response to free
 */
void neoc_neo_get_nep11_balances_free(neoc_neo_get_nep11_balances_t *response);

/**
 * @brief Parse JSON response into Neo get NEP-11 balances structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_balances_from_json(
    const char *json_str,
    neoc_neo_get_nep11_balances_t **response
);

/**
 * @brief Convert Neo get NEP-11 balances response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_balances_to_json(
    const neoc_neo_get_nep11_balances_t *response,
    char **json_str
);

/**
 * @brief Get NEP-11 balances from response (convenience function)
 * 
 * @param response Response to get balances from
 * @return NEP-11 balances pointer, or NULL if error or no result
 */
neoc_nep11_balances_t *neoc_neo_get_nep11_balances_get_balances(
    const neoc_neo_get_nep11_balances_t *response
);

/**
 * @brief Check if the response contains valid NEP-11 balances data
 * 
 * @param response Response to check
 * @return true if response has valid balances, false otherwise
 */
bool neoc_neo_get_nep11_balances_has_balances(
    const neoc_neo_get_nep11_balances_t *response
);

/**
 * @brief Get total number of NFT contracts with balances
 * 
 * @param response Response to count contracts from
 * @return Number of contracts with balances, or 0 on error
 */
size_t neoc_neo_get_nep11_balances_get_contracts_count(
    const neoc_neo_get_nep11_balances_t *response
);

/**
 * @brief Get total number of individual NFT tokens
 * 
 * @param response Response to count tokens from
 * @return Total number of NFT tokens across all contracts, or 0 on error
 */
size_t neoc_neo_get_nep11_balances_get_total_tokens_count(
    const neoc_neo_get_nep11_balances_t *response
);

/**
 * @brief Find balance for specific contract
 * 
 * @param response Response to search in
 * @param asset_hash Contract hash to find
 * @return Balance pointer, or NULL if not found or error
 */
neoc_nep11_balance_t *neoc_neo_get_nep11_balances_find_contract(
    const neoc_neo_get_nep11_balances_t *response,
    const neoc_hash160_t *asset_hash
);

/**
 * @brief Get balance by contract index
 * 
 * @param response Response to get balance from
 * @param index Index of the contract balance
 * @return Balance pointer, or NULL if index out of bounds or error
 */
neoc_nep11_balance_t *neoc_neo_get_nep11_balances_get_balance(
    const neoc_neo_get_nep11_balances_t *response,
    size_t index
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP11_BALANCES_H */
