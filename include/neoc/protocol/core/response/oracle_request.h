/**
 * @file oracle_request.h
 * @brief Oracle request structure for Neo Oracle service
 * 
 * Converted from Swift source: protocol/core/response/OracleRequest.swift
 * Represents an Oracle request with callback information and external data source.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_ORACLE_REQUEST_H
#define NEOC_PROTOCOL_CORE_RESPONSE_ORACLE_REQUEST_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include "neoc/types/hash256.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Oracle request structure
 * 
 * Represents a request to the Neo Oracle service with callback information
 * and external data source details.
 */
typedef struct {
    int request_id;                     /**< Request ID */
    neoc_hash256_t *original_tx_hash;   /**< Original transaction hash */
    int gas_for_response;               /**< Gas allocated for response */
    char *url;                          /**< External data source URL */
    char *filter;                       /**< JSONPath filter for response data */
    neoc_hash160_t *callback_contract;  /**< Callback contract hash */
    char *callback_method;              /**< Callback method name */
    char *user_data;                    /**< User-provided data */
} neoc_oracle_request_t;

/**
 * @brief JSON-RPC response for Oracle requests
 * 
 * Complete response structure for Oracle request queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_oracle_request_t *result;      /**< Oracle request result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_oracle_request_response_t;

/**
 * @brief Create a new Oracle request
 * 
 * @param request_id Request ID
 * @param original_tx_hash Original transaction hash
 * @param gas_for_response Gas allocated for response
 * @param url External data source URL
 * @param filter JSONPath filter for response data
 * @param callback_contract Callback contract hash
 * @param callback_method Callback method name
 * @param user_data User-provided data
 * @param oracle_request Output pointer for the created request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_request_create(
    int request_id,
    const neoc_hash256_t *original_tx_hash,
    int gas_for_response,
    const char *url,
    const char *filter,
    const neoc_hash160_t *callback_contract,
    const char *callback_method,
    const char *user_data,
    neoc_oracle_request_t **oracle_request
);

/**
 * @brief Free an Oracle request
 * 
 * @param oracle_request Request to free (can be NULL)
 */
void neoc_oracle_request_free(
    neoc_oracle_request_t *oracle_request
);

/**
 * @brief Create a new Oracle request response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Oracle request result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_request_response_create(
    const char *jsonrpc,
    int id,
    neoc_oracle_request_t *result,
    const char *error,
    int error_code,
    neoc_oracle_request_response_t **response
);

/**
 * @brief Free an Oracle request response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_oracle_request_response_free(
    neoc_oracle_request_response_t *response
);

/**
 * @brief Parse Oracle request from JSON string
 * 
 * @param json_str JSON string containing Oracle request data
 * @param oracle_request Output pointer for the parsed request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_request_from_json(
    const char *json_str,
    neoc_oracle_request_t **oracle_request
);

/**
 * @brief Parse Oracle request response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_request_response_from_json(
    const char *json_str,
    neoc_oracle_request_response_t **response
);

/**
 * @brief Convert Oracle request to JSON string
 * 
 * @param oracle_request Request to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_request_to_json(
    const neoc_oracle_request_t *oracle_request,
    char **json_str
);

/**
 * @brief Create a copy of an Oracle request
 * 
 * @param src Source request to copy
 * @param dest Output pointer for the copied request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_request_copy(
    const neoc_oracle_request_t *src,
    neoc_oracle_request_t **dest
);

/**
 * @brief Compare two Oracle requests for equality
 * 
 * @param request1 First request
 * @param request2 Second request
 * @return true if requests are equal, false otherwise
 */
bool neoc_oracle_request_equals(
    const neoc_oracle_request_t *request1,
    const neoc_oracle_request_t *request2
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_ORACLE_REQUEST_H */
