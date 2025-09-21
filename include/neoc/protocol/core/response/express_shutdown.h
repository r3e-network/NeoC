/**
 * @file express_shutdown.h
 * @brief Express shutdown response structure
 * 
 * Converted from Swift source: protocol/core/response/ExpressShutdown.swift
 * Represents a Neo Express shutdown response containing process ID.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_EXPRESS_SHUTDOWN_H
#define NEOC_PROTOCOL_CORE_RESPONSE_EXPRESS_SHUTDOWN_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Express shutdown response structure
 * 
 * Contains the process ID of the shutdown Neo Express instance.
 * The process ID is decoded from string format using StringDecode pattern.
 */
typedef struct {
    int process_id;                     /**< Process ID of the shutdown instance */
} neoc_express_shutdown_t;

/**
 * @brief JSON-RPC response for Express shutdown
 * 
 * Complete response structure for Neo Express shutdown operations.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_express_shutdown_t *result;    /**< Shutdown result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_express_shutdown_response_t;

/**
 * @brief Create a new express shutdown structure
 * 
 * @param process_id Process ID of the shutdown instance
 * @param shutdown Output pointer for the created shutdown info (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_express_shutdown_create(
    int process_id,
    neoc_express_shutdown_t **shutdown
);

/**
 * @brief Free an express shutdown structure
 * 
 * @param shutdown Shutdown info to free (can be NULL)
 */
void neoc_express_shutdown_free(
    neoc_express_shutdown_t *shutdown
);

/**
 * @brief Create a new express shutdown response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Shutdown result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_express_shutdown_response_create(
    const char *jsonrpc,
    int id,
    neoc_express_shutdown_t *result,
    const char *error,
    int error_code,
    neoc_express_shutdown_response_t **response
);

/**
 * @brief Free an express shutdown response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_express_shutdown_response_free(
    neoc_express_shutdown_response_t *response
);

/**
 * @brief Parse express shutdown from JSON string
 * 
 * @param json_str JSON string containing shutdown data
 * @param shutdown Output pointer for the parsed shutdown (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_express_shutdown_from_json(
    const char *json_str,
    neoc_express_shutdown_t **shutdown
);

/**
 * @brief Parse express shutdown response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_express_shutdown_response_from_json(
    const char *json_str,
    neoc_express_shutdown_response_t **response
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_EXPRESS_SHUTDOWN_H */
