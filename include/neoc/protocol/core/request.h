/**
 * @file request.h
 * @brief Core RPC request interface
 * 
 * Defines the core Neo RPC request structure and API functions
 * based on the Swift Request implementation.
 */

#ifndef NEOC_PROTOCOL_CORE_REQUEST_H
#define NEOC_PROTOCOL_CORE_REQUEST_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neo RPC request structure
 * 
 * Represents a JSON-RPC 2.0 request for the Neo blockchain.
 */
typedef struct neoc_request {
    char *jsonrpc;     /**< JSON-RPC version (always "2.0") */
    char *method;      /**< RPC method name */
    char *params;      /**< JSON string containing parameters */
    int id;            /**< Unique request identifier */
    void *service;     /**< Associated service instance */
} neoc_request_t;

/**
 * @brief Create a new Neo RPC request
 * @param method The RPC method name
 * @param params JSON string containing parameters (can be NULL)
 * @param service The NeoSwift service instance
 * @return Newly created request or NULL on error
 */
neoc_request_t *neoc_request_create(const char *method, const char *params, void *service);

/**
 * @brief Free a Neo RPC request
 * @param request The request to free
 */
void neoc_request_free(neoc_request_t *request);

/**
 * @brief Convert request to JSON string
 * @param request The request to serialize
 * @return JSON string (caller must free) or NULL on error
 */
char *neoc_request_to_json(const neoc_request_t *request);

/**
 * @brief Create request from JSON string
 * @param json_string The JSON string to parse
 * @param service The service instance to associate
 * @return Parsed request or NULL on error
 */
neoc_request_t *neoc_request_from_json(const char *json_string, void *service);

/**
 * @brief Get the current request counter value
 * @return Current counter value
 */
int neoc_request_get_counter(void);

/**
 * @brief Reset the request counter
 * @param value New counter value
 */
void neoc_request_set_counter(int value);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_REQUEST_H */
