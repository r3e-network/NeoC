/**
 * @file neo_get_nep17_balances.h
 * @brief NEP-17 token balances response
 * 
 * Based on Swift source: protocol/core/response/NeoGetNep17Balances.swift
 * Response structure for getnep17balances RPC call
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP17_BALANCES_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP17_BALANCES_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NEP-17 token balance information
 * 
 * Represents balance information for a single NEP-17 token
 */
typedef struct {
    neoc_hash160_t asset_hash;           /**< Asset contract hash */
    char *name;                          /**< Token name */
    char *symbol;                        /**< Token symbol */
    char *decimals;                      /**< Token decimals as string */
    char *amount;                        /**< Balance amount as string */
    uint64_t last_updated_block;         /**< Last updated block height */
} neoc_nep17_balance_t;

/**
 * @brief NEP-17 balances collection for an address
 * 
 * Contains all NEP-17 token balances for a specific address
 */
typedef struct {
    char *address;                       /**< Account address */
    neoc_nep17_balance_t **balances;     /**< Array of token balances */
    size_t balance_count;                /**< Number of balances */
} neoc_nep17_balances_t;

/**
 * @brief Complete response for getnep17balances RPC call
 * 
 * Standard JSON-RPC response structure containing NEP-17 balances
 */
typedef struct {
    char *jsonrpc;                       /**< JSON-RPC version ("2.0") */
    int id;                              /**< Request ID */
    neoc_nep17_balances_t *result;       /**< Balance result (NULL if error) */
    char *error;                         /**< Error message (NULL if success) */
    int error_code;                      /**< Error code (0 if success) */
} neoc_neo_get_nep17_balances_response_t;

/**
 * @brief Create a new NEP-17 balance structure
 * 
 * @param asset_hash Asset contract hash
 * @param name Token name
 * @param symbol Token symbol
 * @param decimals Token decimals
 * @param amount Balance amount
 * @param last_updated_block Last updated block
 * @param balance Output balance structure (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_balance_create(const neoc_hash160_t *asset_hash,
                                        const char *name,
                                        const char *symbol,
                                        const char *decimals,
                                        const char *amount,
                                        uint64_t last_updated_block,
                                        neoc_nep17_balance_t **balance);

/**
 * @brief Create NEP-17 balances collection
 * 
 * @param address Account address
 * @param balances Output balances collection (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_balances_create(const char *address,
                                         neoc_nep17_balances_t **balances);

/**
 * @brief Add balance to NEP-17 balances collection
 * 
 * @param balances The balances collection
 * @param balance Balance to add (ownership transferred)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_balances_add_balance(neoc_nep17_balances_t *balances,
                                              neoc_nep17_balance_t *balance);

/**
 * @brief Create NEP-17 balances response
 * 
 * @param id Request ID
 * @param result Balances result (can be NULL for error response)
 * @param error Error message (can be NULL for success response)
 * @param error_code Error code (0 for success)
 * @param response Output response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_get_nep17_balances_response_create(int id,
                                                          neoc_nep17_balances_t *result,
                                                          const char *error,
                                                          int error_code,
                                                          neoc_neo_get_nep17_balances_response_t **response);

/**
 * @brief Parse NEP-17 balances response from JSON
 * 
 * @param json_string JSON response string
 * @param response Output parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_get_nep17_balances_response_from_json(const char *json_string,
                                                             neoc_neo_get_nep17_balances_response_t **response);

/**
 * @brief Convert NEP-17 balances response to JSON
 * 
 * @param response The response to convert
 * @param json_string Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_get_nep17_balances_response_to_json(const neoc_neo_get_nep17_balances_response_t *response,
                                                           char **json_string);

/**
 * @brief Get balance for specific asset from balances collection
 * 
 * @param balances The balances collection
 * @param asset_hash Asset hash to find
 * @param balance Output balance (do not free, NULL if not found)
 * @return NEOC_SUCCESS if found, NEOC_ERROR_NOT_FOUND otherwise
 */
neoc_error_t neoc_nep17_balances_get_balance(const neoc_nep17_balances_t *balances,
                                              const neoc_hash160_t *asset_hash,
                                              const neoc_nep17_balance_t **balance);

/**
 * @brief Check if response indicates success
 * 
 * @param response The response to check
 * @return True if response is successful
 */
bool neoc_neo_get_nep17_balances_response_is_success(const neoc_neo_get_nep17_balances_response_t *response);

/**
 * @brief Get total number of different tokens in balances
 * 
 * @param balances The balances collection
 * @return Number of different tokens
 */
size_t neoc_nep17_balances_get_token_count(const neoc_nep17_balances_t *balances);

/**
 * @brief Copy NEP-17 balance
 * 
 * @param source Source balance
 * @param copy Output copied balance (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_balance_copy(const neoc_nep17_balance_t *source,
                                      neoc_nep17_balance_t **copy);

/**
 * @brief Free NEP-17 balance
 * 
 * @param balance The balance to free
 */
void neoc_nep17_balance_free(neoc_nep17_balance_t *balance);

/**
 * @brief Free NEP-17 balances collection
 * 
 * @param balances The balances to free
 */
void neoc_nep17_balances_free(neoc_nep17_balances_t *balances);

/**
 * @brief Free NEP-17 balances response
 * 
 * @param response The response to free
 */
void neoc_neo_get_nep17_balances_response_free(neoc_neo_get_nep17_balances_response_t *response);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP17_BALANCES_H */
