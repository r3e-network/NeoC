/**
 * @file neo_c_service.c
 * @brief NeoC service protocol implementation
 * 
 * Based on Swift source: protocol/NeoSwiftService.swift
 * Provides asynchronous JSON-RPC request handling
 */

#include "neoc/neoc.h"
#include "neoc/protocol/neo_c_service.h"
#include "neoc/protocol/service.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief NeoC service implementation structure
 */
typedef struct neoc_neo_c_service_internal {
    neoc_service_t base;                    /**< Base service interface */
    neoc_service_send_func_t send_func;     /**< Send function pointer */
    neoc_service_send_async_func_t send_async_func; /**< Async send function pointer */
    void *user_data;                        /**< User data for callbacks */
} neoc_neo_c_service_internal_t;

/**
 * @brief Create a new NeoC service
 */
neoc_neo_c_service_t *neoc_neo_c_service_create(
    neoc_service_send_func_t send_func,
    neoc_service_send_async_func_t send_async_func,
    void *user_data) {
    
    if (!send_func) {
        return NULL;
    }
    
    neoc_neo_c_service_internal_t *service = 
        (neoc_neo_c_service_internal_t *)neoc_malloc(sizeof(neoc_neo_c_service_internal_t));
    if (!service) {
        return NULL;
    }
    
    // Initialize the service
    service->send_func = send_func;
    service->send_async_func = send_async_func;
    service->user_data = user_data;
    
    // Set up base service interface
    service->base.send = neoc_neo_c_service_send;
    service->base.send_async = neoc_neo_c_service_send_async;
    service->base.free = neoc_neo_c_service_free;
    service->base.impl = service;
    
    return (neoc_neo_c_service_t *)service;
}

/**
 * @brief Send a synchronous request
 */
neoc_error_t neoc_neo_c_service_send(
    neoc_neo_c_service_t *service,
    const neoc_request_t *request,
    neoc_response_t **response_out) {
    
    if (!service || !request || !response_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    neoc_neo_c_service_internal_t *internal = (neoc_neo_c_service_internal_t *)service;
    
    if (!internal->send_func) {
        return NEOC_ERROR_NOT_SUPPORTED;
    }
    
    // Call the underlying send function
    return internal->send_func(request, response_out, internal->user_data);
}

/**
 * @brief Send an asynchronous request
 */
neoc_error_t neoc_neo_c_service_send_async(
    neoc_neo_c_service_t *service,
    const neoc_request_t *request,
    neoc_service_callback_t callback,
    void *callback_data) {
    
    if (!service || !request || !callback) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    neoc_neo_c_service_internal_t *internal = (neoc_neo_c_service_internal_t *)service;
    
    if (!internal->send_async_func) {
        // Fall back to synchronous if async not available
        neoc_response_t *response = NULL;
        neoc_error_t error = neoc_neo_c_service_send(service, request, &response);
        
        // Call callback with result
        callback(response, error, callback_data);
        
        return error;
    }
    
    // Call the underlying async send function
    return internal->send_async_func(request, callback, callback_data, internal->user_data);
}

/**
 * @brief Get the service implementation
 */
void *neoc_neo_c_service_get_impl(neoc_neo_c_service_t *service) {
    if (!service) {
        return NULL;
    }
    
    neoc_neo_c_service_internal_t *internal = (neoc_neo_c_service_internal_t *)service;
    return internal->user_data;
}

/**
 * @brief Free the NeoC service
 */
void neoc_neo_c_service_free(neoc_neo_c_service_t *service) {
    if (!service) {
        return;
    }
    
    neoc_free(service);
}