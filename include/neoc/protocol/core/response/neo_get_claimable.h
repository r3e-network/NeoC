/**
 * @file neo_get_claimable.h
 * @brief Neo claimable GAS response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetClaimable.swift
 * Provides functionality for retrieving claimable GAS information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_CLAIMABLE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_CLAIMABLE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a single claimable GAS claim
 * 
 * This structure contains information about a specific claimable GAS transaction
 * including transaction details, heights, and calculated gas amounts.
 */
typedef struct {
    char *tx_id;                        /**< Transaction ID */
    int index;                          /**< Transaction output index */
    int neo_value;                      /**< NEO value in the transaction */
    int start_height;                   /**< Block height when claim period started */
    int end_height;                     /**< Block height when claim period ended */
    char *generated_gas;                /**< Generated GAS amount as string */
    char *system_fee;                   /**< System fee as string */
    char *unclaimed_gas;                /**< Unclaimed GAS amount as string */
} neoc_claimable_claim_t;

/**
 * @brief Represents the complete claimable GAS information for an address
 * 
 * This structure contains all claimable GAS information including the address,
 * individual claims, and total unclaimed amount.
 */
typedef struct {
    neoc_claimable_claim_t *claims;     /**< Array of individual claims */
    size_t claims_count;                /**< Number of claims in the array */
    char *address;                      /**< Address for which claims are calculated */
    char *total_unclaimed;              /**< Total unclaimed GAS amount as string */
} neoc_claimables_t;

/**
 * @brief Response structure for getclaimable RPC call
 * 
 * Contains the claimables information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_claimables_t *result;          /**< Claimables result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_claimable_t;

/**
 * @brief Create a new claimable claim structure
 * 
 * @param tx_id Transaction ID
 * @param index Transaction output index
 * @param neo_value NEO value in the transaction
 * @param start_height Block height when claim period started
 * @param end_height Block height when claim period ended
 * @param generated_gas Generated GAS amount as string
 * @param system_fee System fee as string
 * @param unclaimed_gas Unclaimed GAS amount as string
 * @param claim Pointer to store the created claim (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_claimable_claim_create(
    const char *tx_id,
    int index,
    int neo_value,
    int start_height,
    int end_height,
    const char *generated_gas,
    const char *system_fee,
    const char *unclaimed_gas,
    neoc_claimable_claim_t **claim
);

/**
 * @brief Free a claimable claim structure
 * 
 * @param claim Claim to free
 */
void neoc_claimable_claim_free(neoc_claimable_claim_t *claim);

/**
 * @brief Create a new claimables structure
 * 
 * @param claims Array of claims
 * @param claims_count Number of claims
 * @param address Address for which claims are calculated
 * @param total_unclaimed Total unclaimed GAS amount
 * @param claimables Pointer to store the created claimables (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_claimables_create(
    neoc_claimable_claim_t *claims,
    size_t claims_count,
    const char *address,
    const char *total_unclaimed,
    neoc_claimables_t **claimables
);

/**
 * @brief Free a claimables structure
 * 
 * @param claimables Claimables to free
 */
void neoc_claimables_free(neoc_claimables_t *claimables);

/**
 * @brief Create a new Neo getclaimable response
 * 
 * @param id Request ID
 * @param claimables Claimables data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_claimable_create(
    int id,
    neoc_claimables_t *claimables,
    const char *error,
    int error_code,
    neoc_neo_get_claimable_t **response
);

/**
 * @brief Free a Neo getclaimable response
 * 
 * @param response Response to free
 */
void neoc_neo_get_claimable_free(neoc_neo_get_claimable_t *response);

/**
 * @brief Parse JSON response into Neo getclaimable structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_claimable_from_json(
    const char *json_str,
    neoc_neo_get_claimable_t **response
);

/**
 * @brief Convert Neo getclaimable response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_claimable_to_json(
    const neoc_neo_get_claimable_t *response,
    char **json_str
);

/**
 * @brief Get claimables from response (convenience function)
 * 
 * @param response Response to get claimables from
 * @return Claimables pointer, or NULL if error or no result
 */
neoc_claimables_t *neoc_neo_get_claimable_get_claimables(
    const neoc_neo_get_claimable_t *response
);

/**
 * @brief Check if the response contains valid claimables data
 * 
 * @param response Response to check
 * @return true if response has valid claimables, false otherwise
 */
bool neoc_neo_get_claimable_has_claimables(
    const neoc_neo_get_claimable_t *response
);

/**
 * @brief Get total number of claims in the response
 * 
 * @param response Response to check
 * @return Number of claims, or 0 if no claimables or error
 */
size_t neoc_neo_get_claimable_get_claims_count(
    const neoc_neo_get_claimable_t *response
);

/**
 * @brief Get a specific claim by index
 * 
 * @param response Response to get claim from
 * @param index Index of the claim to retrieve
 * @return Claim pointer, or NULL if index is out of bounds or error
 */
neoc_claimable_claim_t *neoc_neo_get_claimable_get_claim(
    const neoc_neo_get_claimable_t *response,
    size_t index
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_CLAIMABLE_H */
