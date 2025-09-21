/**
 * @file service.h
 * @brief Base service protocol interface for Neo JSON-RPC
 * 
 * Based on Swift source: protocol/Service.swift
 * Provides the base interface for Neo blockchain services
 */

#ifndef NEOC_PROTOCOL_SERVICE_H
#define NEOC_PROTOCOL_SERVICE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/array.h"
#include "neoc/protocol/core/request.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations
 */
typedef struct neoc_service_t neoc_service_t;
typedef struct neoc_response_t neoc_response_t;

/**
 * @brief Service type enumeration
 */
typedef enum {
    NEOC_SERVICE_TYPE_HTTP = 1,         /**< HTTP JSON-RPC service */
    NEOC_SERVICE_TYPE_WEBSOCKET = 2,    /**< WebSocket service */
    NEOC_SERVICE_TYPE_IPC = 3           /**< IPC service */
} neoc_service_type_t;

/**
 * @brief Service configuration structure
 */
typedef struct {
    char *endpoint_url;                 /**< Service endpoint URL */
    bool include_raw_responses;         /**< Include raw JSON in responses */
    long timeout_seconds;               /**< Request timeout in seconds */
    bool auto_retry;                    /**< Auto-retry failed requests */
    int max_retries;                    /**< Maximum retry attempts */
} neoc_service_config_t;

/**
 * @brief Service function pointers for polymorphic behavior
 */
typedef struct {
    /**
     * @brief Perform IO operation (send request, receive response)
     */
    neoc_error_t (*perform_io)(neoc_service_t *service,
                               const neoc_byte_array_t *payload,
                               neoc_byte_array_t **result);
    
    /**
     * @brief Free service implementation data
     */
    void (*free_impl)(neoc_service_t *service);
    
    /**
     * @brief Get service configuration
     */
    neoc_error_t (*get_config)(neoc_service_t *service,
                               neoc_service_config_t *config);
    
    /**
     * @brief Set service configuration
     */
    neoc_error_t (*set_config)(neoc_service_t *service,
                               const neoc_service_config_t *config);
} neoc_service_vtable_t;

/**
 * @brief Base service structure
 */
struct neoc_service_t {
    neoc_service_type_t type;           /**< Service type */
    neoc_service_config_t config;       /**< Service configuration */
    neoc_service_vtable_t *vtable;      /**< Virtual function table */
    void *impl_data;                    /**< Implementation-specific data */
};

/**
 * @brief Create a default service configuration
 * 
 * @param url Service endpoint URL
 * @param config Output configuration (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_config_create_default(const char *url,
                                                 neoc_service_config_t **config);

/**
 * @brief Free service configuration
 * 
 * @param config Configuration to free
 */
void neoc_service_config_free(neoc_service_config_t *config);

/**
 * @brief Initialize a service with configuration
 * 
 * @param service Service structure to initialize
 * @param type Service type
 * @param config Service configuration
 * @param vtable Virtual function table
 * @param impl_data Implementation-specific data
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_init(neoc_service_t *service,
                               neoc_service_type_t type,
                               const neoc_service_config_t *config,
                               neoc_service_vtable_t *vtable,
                               void *impl_data);

/**
 * @brief Free a service
 * 
 * @param service Service to free
 */
void neoc_service_free(neoc_service_t *service);

/**
 * @brief Send a JSON-RPC request through the service
 * 
 * This is the main service interface method that handles request serialization,
 * IO operations, and response deserialization.
 * 
 * @param service Service instance
 * @param request JSON-RPC request
 * @param response Output response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_send_request(neoc_service_t *service,
                                       const neoc_request_t *request,
                                       neoc_response_t **response);

/**
 * @brief Perform low-level IO operation
 * 
 * Equivalent to Swift's performIO method. Sends raw payload and
 * receives raw response data.
 * 
 * @param service Service instance
 * @param payload Request payload
 * @param result Output response data (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_perform_io(neoc_service_t *service,
                                      const neoc_byte_array_t *payload,
                                      neoc_byte_array_t **result);

/**
 * @brief Check if service includes raw responses
 * 
 * @param service Service instance
 * @return true if raw responses are included, false otherwise
 */
bool neoc_service_includes_raw_responses(const neoc_service_t *service);

/**
 * @brief Set whether to include raw responses
 * 
 * @param service Service instance
 * @param include_raw Whether to include raw responses
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_set_include_raw_responses(neoc_service_t *service,
                                                    bool include_raw);

/**
 * @brief Get service endpoint URL
 * 
 * @param service Service instance
 * @return Service URL (do not free)
 */
const char* neoc_service_get_url(const neoc_service_t *service);

/**
 * @brief Set service endpoint URL
 * 
 * @param service Service instance
 * @param url New endpoint URL
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_set_url(neoc_service_t *service, const char *url);

/**
 * @brief Get service type as string
 * 
 * @param type Service type
 * @return Type string (static, do not free)
 */
const char* neoc_service_type_to_string(neoc_service_type_t type);

/**
 * @brief Create service from URL (factory method)
 * 
 * Automatically determines service type from URL scheme and creates
 * appropriate service implementation.
 * 
 * @param url Service URL (http://, https://, ws://, wss://)
 * @param service Output service (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_service_create_from_url(const char *url,
                                          neoc_service_t **service);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_SERVICE_H */
