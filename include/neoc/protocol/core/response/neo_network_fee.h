/**
 * @file neo_network_fee.h
 * @brief Neo network fee response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoNetworkFee.swift
 * Provides functionality for handling network fee calculations.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_NETWORK_FEE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_NETWORK_FEE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents network fee information
 * 
 * This structure contains the calculated network fee for a transaction.
 * The fee is stored as an integer representing the fee in the smallest unit.
 */
typedef struct {
    int64_t network_fee;                /**< Network fee as integer */
} neoc_neo_network_fee_t;

/**
 * @brief Response structure for calculatenetworkfee RPC call
 * 
 * Contains the network fee information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_neo_network_fee_t *result;     /**< Network fee result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_calculate_network_fee_t;

/**
 * @brief Create a new network fee structure
 * 
 * @param network_fee Network fee value
 * @param fee_struct Pointer to store the created fee structure (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_network_fee_create(
    int64_t network_fee,
    neoc_neo_network_fee_t **fee_struct
);

/**
 * @brief Free a network fee structure
 * 
 * @param fee_struct Fee structure to free
 */
void neoc_neo_network_fee_free(neoc_neo_network_fee_t *fee_struct);

/**
 * @brief Create a new Neo calculate network fee response
 * 
 * @param id Request ID
 * @param network_fee Network fee data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_calculate_network_fee_create(
    int id,
    neoc_neo_network_fee_t *network_fee,
    const char *error,
    int error_code,
    neoc_neo_calculate_network_fee_t **response
);

/**
 * @brief Free a Neo calculate network fee response
 * 
 * @param response Response to free
 */
void neoc_neo_calculate_network_fee_free(neoc_neo_calculate_network_fee_t *response);

/**
 * @brief Parse JSON response into Neo calculate network fee structure
 * 
 * This function handles the "networkfee" field with StringDecode attribute
 * to parse string values into integer fee amounts.
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_calculate_network_fee_from_json(
    const char *json_str,
    neoc_neo_calculate_network_fee_t **response
);

/**
 * @brief Convert Neo calculate network fee response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_calculate_network_fee_to_json(
    const neoc_neo_calculate_network_fee_t *response,
    char **json_str
);

/**
 * @brief Get network fee from response (convenience function)
 * 
 * @param response Response to get fee from
 * @return Network fee pointer, or NULL if error or no result
 */
neoc_neo_network_fee_t *neoc_neo_calculate_network_fee_get_fee(
    const neoc_neo_calculate_network_fee_t *response
);

/**
 * @brief Check if the response contains valid network fee data
 * 
 * @param response Response to check
 * @return true if response has valid fee, false otherwise
 */
bool neoc_neo_calculate_network_fee_has_fee(
    const neoc_neo_calculate_network_fee_t *response
);

/**
 * @brief Get network fee value from response
 * 
 * @param response Response to get fee from
 * @param fee Pointer to store the fee value (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_calculate_network_fee_get_fee_value(
    const neoc_neo_calculate_network_fee_t *response,
    int64_t *fee
);

/**
 * @brief Get network fee as string from response
 * 
 * @param response Response to get fee from
 * @param fee_str Pointer to store the fee string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_calculate_network_fee_get_fee_string(
    const neoc_neo_calculate_network_fee_t *response,
    char **fee_str
);

/**
 * @brief Check if network fee is positive (valid transaction)
 * 
 * @param response Response to check
 * @return true if network fee is positive, false otherwise
 */
bool neoc_neo_calculate_network_fee_is_valid(
    const neoc_neo_calculate_network_fee_t *response
);

/**
 * @brief Compare network fees between two responses
 * 
 * @param response1 First response to compare
 * @param response2 Second response to compare
 * @param result Pointer to store comparison result (output)
 *               -1 if fee1 < fee2, 0 if equal, 1 if fee1 > fee2
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_calculate_network_fee_compare(
    const neoc_neo_calculate_network_fee_t *response1,
    const neoc_neo_calculate_network_fee_t *response2,
    int *result
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_NETWORK_FEE_H */
