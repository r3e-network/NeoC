/**
 * @file neo_get_unclaimed_gas.h
 * @brief Neo unclaimed GAS response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetUnclaimedGas.swift
 * Provides functionality for retrieving unclaimed GAS information for accounts.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_UNCLAIMED_GAS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_UNCLAIMED_GAS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents unclaimed GAS information for an account
 * 
 * This structure contains the unclaimed GAS amount and the associated address.
 */
typedef struct {
    char *unclaimed;                    /**< Unclaimed GAS amount as string */
    char *address;                      /**< Account address */
} neoc_get_unclaimed_gas_t;

/**
 * @brief Response structure for getunclaimedgas RPC call
 * 
 * Contains the unclaimed GAS information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_get_unclaimed_gas_t *result;   /**< Unclaimed GAS result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_unclaimed_gas_t;

/**
 * @brief Create a new unclaimed GAS structure
 * 
 * @param unclaimed Unclaimed GAS amount as string
 * @param address Account address
 * @param unclaimed_gas Pointer to store the created structure (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_unclaimed_gas_create(
    const char *unclaimed,
    const char *address,
    neoc_get_unclaimed_gas_t **unclaimed_gas
);

/**
 * @brief Free an unclaimed GAS structure
 * 
 * @param unclaimed_gas Structure to free
 */
void neoc_get_unclaimed_gas_free(neoc_get_unclaimed_gas_t *unclaimed_gas);

/**
 * @brief Create a new Neo get unclaimed GAS response
 * 
 * @param id Request ID
 * @param unclaimed_gas Unclaimed GAS data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unclaimed_gas_create(
    int id,
    neoc_get_unclaimed_gas_t *unclaimed_gas,
    const char *error,
    int error_code,
    neoc_neo_get_unclaimed_gas_t **response
);

/**
 * @brief Free a Neo get unclaimed GAS response
 * 
 * @param response Response to free
 */
void neoc_neo_get_unclaimed_gas_free(neoc_neo_get_unclaimed_gas_t *response);

/**
 * @brief Parse JSON response into Neo get unclaimed GAS structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unclaimed_gas_from_json(
    const char *json_str,
    neoc_neo_get_unclaimed_gas_t **response
);

/**
 * @brief Convert Neo get unclaimed GAS response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unclaimed_gas_to_json(
    const neoc_neo_get_unclaimed_gas_t *response,
    char **json_str
);

/**
 * @brief Get unclaimed GAS data from response (convenience function)
 * 
 * @param response Response to get data from
 * @return Unclaimed GAS data pointer, or NULL if error or no result
 */
neoc_get_unclaimed_gas_t *neoc_neo_get_unclaimed_gas_get_data(
    const neoc_neo_get_unclaimed_gas_t *response
);

/**
 * @brief Check if the response contains valid unclaimed GAS data
 * 
 * @param response Response to check
 * @return true if response has valid data, false otherwise
 */
bool neoc_neo_get_unclaimed_gas_has_data(
    const neoc_neo_get_unclaimed_gas_t *response
);

/**
 * @brief Get unclaimed GAS amount as string from response
 * 
 * @param response Response to get amount from
 * @return Unclaimed GAS amount string, or NULL if error or no result
 */
const char *neoc_neo_get_unclaimed_gas_get_amount_string(
    const neoc_neo_get_unclaimed_gas_t *response
);

/**
 * @brief Get account address from response
 * 
 * @param response Response to get address from
 * @return Account address string, or NULL if error or no result
 */
const char *neoc_neo_get_unclaimed_gas_get_address(
    const neoc_neo_get_unclaimed_gas_t *response
);

/**
 * @brief Get unclaimed GAS amount as double from response
 * 
 * @param response Response to get amount from
 * @param amount Pointer to store the amount value (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unclaimed_gas_get_amount_double(
    const neoc_neo_get_unclaimed_gas_t *response,
    double *amount
);

/**
 * @brief Check if account has any unclaimed GAS
 * 
 * @param response Response to check
 * @return true if account has unclaimed GAS, false otherwise
 */
bool neoc_neo_get_unclaimed_gas_has_unclaimed(
    const neoc_neo_get_unclaimed_gas_t *response
);

/**
 * @brief Copy unclaimed GAS amount string from response
 * 
 * @param response Response to get amount from
 * @param amount_copy Pointer to store the copied amount string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unclaimed_gas_copy_amount_string(
    const neoc_neo_get_unclaimed_gas_t *response,
    char **amount_copy
);

/**
 * @brief Copy account address string from response
 * 
 * @param response Response to get address from
 * @param address_copy Pointer to store the copied address string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_unclaimed_gas_copy_address_string(
    const neoc_neo_get_unclaimed_gas_t *response,
    char **address_copy
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_UNCLAIMED_GAS_H */
