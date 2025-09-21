/**
 * @file neo_c_service.h
 * @brief NeoC service protocol interface for C
 * 
 * Based on Swift source: protocol/NeoSwiftService.swift
 * Provides the base service protocol for Neo JSON-RPC operations
 */

#ifndef NEOC_PROTOCOL_NEO_C_SERVICE_H
#define NEOC_PROTOCOL_NEO_C_SERVICE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/core/response.h"
#include "neoc/protocol/core/request.h"
#include "neoc/utils/array.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations
 */
typedef struct neoc_neo_c_service_t neoc_neo_c_service_t;

/**
 * @brief Callback function for service requests
 * @param service Service instance
 * @param request Request to process
 * @param response_out Output for response
 * @return NEOC_SUCCESS on success, error code on failure
 */
typedef neoc_error_t (*neoc_neo_c_service_send_fn)(
    neoc_neo_c_service_t *service,
    const neoc_request_t *request,
    neoc_response_t **response_out
);

/**
 * @brief Callback function for asynchronous service requests
 * @param service Service instance
 * @param request Request to process
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
typedef neoc_error_t (*neoc_neo_c_service_send_async_fn)(
    neoc_neo_c_service_t *service,
    const neoc_request_t *request,
    void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
    void *user_data
);

/**
 * @brief Service cleanup callback
 * @param service Service instance to clean up
 */
typedef void (*neoc_neo_c_service_free_fn)(neoc_neo_c_service_t *service);

/**
 * @brief NeoSwiftService virtual table
 * 
 * Function pointers for polymorphic service implementation
 */
typedef struct {
    neoc_neo_c_service_send_fn send;        /**< Synchronous send function */
    neoc_neo_c_service_send_async_fn send_async; /**< Asynchronous send function */
    neoc_neo_c_service_free_fn free_impl;   /**< Cleanup function */
} neoc_neo_c_service_vtable_t;

/**
 * @brief NeoC service base structure
 * 
 * Based on Swift NeoSwiftService protocol
 * Provides foundation for all Neo service implementations
 */
typedef struct neoc_neo_c_service_t {
    const neoc_neo_c_service_vtable_t *vtable; /**< Virtual function table */
    void *impl_data;                    /**< Implementation-specific data */
} neoc_neo_c_service_t;

/**
 * @brief Get request ID
 * @param request Request instance
 * @return Request ID
 */
int neoc_neo_c_service_request_get_id(const neoc_request_t *request);

/**
 * @brief Get request method
 * @param request Request instance
 * @return Method name (do not free)
 */
const char *neoc_neo_c_service_request_get_method(const neoc_request_t *request);

/**
 * @brief Get request parameters
 * @param request Request instance
 * @return Parameters string (do not free)
 */
const char *neoc_neo_c_service_request_get_params(const neoc_request_t *request);

/**
 * @brief Send a request using the associated service
 * @param request Request to send
 * @param response_out Output for response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_service_request_send(const neoc_request_t *request, neoc_response_t **response_out);

/**
 * @brief Send a request asynchronously using the associated service
 * @param request Request to send
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_service_request_send_async(const neoc_request_t *request,
                                                      void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                      void *user_data);

/**
 * @brief Initialize a NeoSwiftService base structure
 * @param service Service to initialize
 * @param vtable Virtual function table
 * @param impl_data Implementation-specific data
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_service_init(neoc_neo_c_service_t *service,
                                         const neoc_neo_c_service_vtable_t *vtable,
                                         void *impl_data);

/**
 * @brief Send a request through a service
 * @param service Service instance
 * @param request Request to send
 * @param response_out Output for response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_service_send(neoc_neo_c_service_t *service,
                                        const neoc_request_t *request,
                                        neoc_response_t **response_out);

/**
 * @brief Send a request through a service asynchronously
 * @param service Service instance
 * @param request Request to send
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_service_send_async(neoc_neo_c_service_t *service,
                                              const neoc_request_t *request,
                                              void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                              void *user_data);

/**
 * @brief Free a NeoSwiftService instance
 * @param service Service to free
 */
void neoc_neo_c_service_free(neoc_neo_c_service_t *service);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_NEO_C_SERVICE_H */
