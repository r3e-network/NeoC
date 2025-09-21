/**
 * @file response.h
 * @brief Core JSON-RPC 2.0 response structure for Neo
 * 
 * Based on Swift source: protocol/core/Response.swift
 * Provides foundation for all Neo JSON-RPC responses
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JSON-RPC error structure
 * 
 * Based on Swift Response.Error
 */
typedef struct {
    int code;                           /**< Error code */
    char *message;                      /**< Error message */
    char *data;                         /**< Additional error data (nullable) */
} neoc_rpc_error_t;

/**
 * @brief Generic JSON-RPC 2.0 response structure
 * 
 * Based on Swift Response<T> class
 * This is the base structure for all Neo JSON-RPC responses
 */
struct neoc_response_t {
    int id;                             /**< Request ID */
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    void *result;                       /**< Response result (typed per request) */
    neoc_rpc_error_t *error;            /**< Error information (nullable) */
    char *raw_response;                 /**< Raw JSON response (nullable) */
    bool has_error;                     /**< Quick error check flag */
};

/**
 * @brief Complete typedef for neoc_response_t (already forward declared in service.h)
 */
#ifndef NEOC_RESPONSE_TYPEDEF_COMPLETE
#define NEOC_RESPONSE_TYPEDEF_COMPLETE
typedef struct neoc_response_t neoc_response_t;
#endif

/**
 * @brief Create a new response structure
 * @param id Request ID
 * @return Newly allocated response or NULL on failure
 */
neoc_response_t *neoc_response_create(int id);

/**
 * @brief Set the result for a response
 * @param response Response structure
 * @param result Result data pointer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_response_set_result(neoc_response_t *response, void *result);

/**
 * @brief Set an error for a response
 * @param response Response structure
 * @param code Error code
 * @param message Error message
 * @param data Additional error data (nullable)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_response_set_error(neoc_response_t *response, int code, 
                                    const char *message, const char *data);

/**
 * @brief Check if response has an error
 * @param response Response structure
 * @return true if has error, false otherwise
 */
bool neoc_response_has_error(const neoc_response_t *response);

/**
 * @brief Get the result from response, checking for errors
 * @param response Response structure
 * @param result_out Output pointer for result
 * @return NEOC_SUCCESS on success, error code if response has error
 */
neoc_error_t neoc_response_get_result(const neoc_response_t *response, void **result_out);

/**
 * @brief Set raw response string
 * @param response Response structure
 * @param raw_response Raw JSON string
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_response_set_raw(neoc_response_t *response, const char *raw_response);

/**
 * @brief Create RPC error structure
 * @param code Error code
 * @param message Error message
 * @param data Additional error data (nullable)
 * @return Newly allocated error or NULL on failure
 */
neoc_rpc_error_t *neoc_rpc_error_create(int code, const char *message, const char *data);

/**
 * @brief Free RPC error structure
 * @param error Error to free
 */
void neoc_rpc_error_free(neoc_rpc_error_t *error);

/**
 * @brief Get error string representation
 * @param error Error structure
 * @return String representation (caller must free)
 */
char *neoc_rpc_error_to_string(const neoc_rpc_error_t *error);

/**
 * @brief Free response structure
 * @param response Response to free
 */
void neoc_response_free(neoc_response_t *response);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_H */
