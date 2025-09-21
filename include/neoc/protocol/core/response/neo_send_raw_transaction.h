/**
 * @file neo_send_raw_transaction.h
 * @brief Neo send raw transaction response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoSendRawTransaction.swift
 * Provides functionality for handling raw transaction broadcast responses.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_SEND_RAW_TRANSACTION_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_SEND_RAW_TRANSACTION_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash256.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the result of a raw transaction broadcast
 * 
 * This structure contains the transaction hash returned when a transaction
 * is successfully broadcast to the Neo network.
 */
typedef struct {
    neoc_hash256_t hash;                /**< Transaction hash */
} neoc_raw_transaction_result_t;

/**
 * @brief Response structure for sendrawtransaction RPC call
 * 
 * Contains the transaction hash returned by the Neo node after successfully
 * broadcasting a raw transaction.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_raw_transaction_result_t *result; /**< Transaction result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_send_raw_transaction_t;

/**
 * @brief Create a new raw transaction result structure
 * 
 * @param hash Transaction hash
 * @param result Pointer to store the created result (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_raw_transaction_result_create(
    const neoc_hash256_t *hash,
    neoc_raw_transaction_result_t **result
);

/**
 * @brief Free a raw transaction result structure
 * 
 * @param result Result to free
 */
void neoc_raw_transaction_result_free(neoc_raw_transaction_result_t *result);

/**
 * @brief Create a new Neo send raw transaction response
 * 
 * @param id Request ID
 * @param transaction_result Transaction result (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_send_raw_transaction_create(
    int id,
    neoc_raw_transaction_result_t *transaction_result,
    const char *error,
    int error_code,
    neoc_neo_send_raw_transaction_t **response
);

/**
 * @brief Free a Neo send raw transaction response
 * 
 * @param response Response to free
 */
void neoc_neo_send_raw_transaction_free(neoc_neo_send_raw_transaction_t *response);

/**
 * @brief Parse JSON response into Neo send raw transaction structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_send_raw_transaction_from_json(
    const char *json_str,
    neoc_neo_send_raw_transaction_t **response
);

/**
 * @brief Convert Neo send raw transaction response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_send_raw_transaction_to_json(
    const neoc_neo_send_raw_transaction_t *response,
    char **json_str
);

/**
 * @brief Get transaction result from response (convenience function)
 * 
 * @param response Response to get result from
 * @return Transaction result pointer, or NULL if error or no result
 */
neoc_raw_transaction_result_t *neoc_neo_send_raw_transaction_get_result(
    const neoc_neo_send_raw_transaction_t *response
);

/**
 * @brief Check if the response contains a valid transaction result
 * 
 * @param response Response to check
 * @return true if response has valid result, false otherwise
 */
bool neoc_neo_send_raw_transaction_has_result(
    const neoc_neo_send_raw_transaction_t *response
);

/**
 * @brief Get transaction hash from response (convenience function)
 * 
 * @param response Response to get hash from
 * @param hash Pointer to store the hash (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_send_raw_transaction_get_hash(
    const neoc_neo_send_raw_transaction_t *response,
    neoc_hash256_t *hash
);

/**
 * @brief Get transaction hash as string from response
 * 
 * @param response Response to get hash from
 * @param hash_str Pointer to store the hash string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_send_raw_transaction_get_hash_string(
    const neoc_neo_send_raw_transaction_t *response,
    char **hash_str
);

/**
 * @brief Check if transaction was successfully broadcast
 * 
 * @param response Response to check
 * @return true if transaction was successfully broadcast, false otherwise
 */
bool neoc_neo_send_raw_transaction_is_successful(
    const neoc_neo_send_raw_transaction_t *response
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_SEND_RAW_TRANSACTION_H */
