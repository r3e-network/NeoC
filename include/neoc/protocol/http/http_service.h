/**
 * @file http_service.h
 * @brief HTTP service implementation for Neo JSON-RPC
 * 
 * Based on Swift source: protocol/http/HttpService.swift
 * Provides HTTP-based service implementation using libcurl
 */

#ifndef NEOC_PROTOCOL_HTTP_HTTP_SERVICE_H
#define NEOC_PROTOCOL_HTTP_HTTP_SERVICE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/service.h"
#include "neoc/utils/url_session.h"
#include "neoc/utils/array.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HTTP service constants
 */
#define NEOC_HTTP_SERVICE_JSON_MEDIA_TYPE "application/json; charset=utf-8"
#define NEOC_HTTP_SERVICE_DEFAULT_URL "http://localhost:10333/"
#define NEOC_HTTP_SERVICE_DEFAULT_TIMEOUT 30
#define NEOC_HTTP_SERVICE_MAX_HEADERS 32

/**
 * @brief HTTP service implementation structure
 */
typedef struct {
    neoc_service_t *base;                   /**< Underlying base service */
    neoc_http_header_t *headers;            /**< Custom headers */
    size_t header_count;                    /**< Number of custom headers */
} neoc_http_service_t;

/**
 * @brief Create a new HTTP service with default configuration
 * 
 * @param service Output service handle (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_create(neoc_http_service_t **service);

/**
 * @brief Create a new HTTP service with custom URL
 * 
 * @param url Service endpoint URL
 * @param service Output service handle (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_create_with_url(const char *url,
                                                neoc_http_service_t **service);

/**
 * @brief Create a new HTTP service with full configuration
 * 
 * @param url Service endpoint URL
 * @param include_raw_responses Include raw JSON in responses
 * @param timeout_seconds Request timeout in seconds
 * @param service Output service handle (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_create_with_config(const char *url,
                                                   bool include_raw_responses,
                                                   long timeout_seconds,
                                                   neoc_http_service_t **service);

/**
 * @brief Free an HTTP service
 * 
 * @param service Service to free
 */
void neoc_http_service_free(neoc_http_service_t *service);

/**
 * @brief Add a custom header to all requests
 * 
 * Equivalent to Swift's addHeader method
 * 
 * @param service HTTP service
 * @param name Header name (e.g., "Authorization")
 * @param value Header value (e.g., "Bearer token")
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_add_header(neoc_http_service_t *service,
                                           const char *name,
                                           const char *value);

/**
 * @brief Add multiple headers to all requests
 * 
 * Equivalent to Swift's addHeaders method
 * 
 * @param service HTTP service
 * @param headers Array of headers to add
 * @param header_count Number of headers in the array
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_add_headers(neoc_http_service_t *service,
                                            const neoc_http_header_t *headers,
                                            size_t header_count);

/**
 * @brief Remove a header from all requests
 * 
 * @param service HTTP service
 * @param name Header name to remove
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_remove_header(neoc_http_service_t *service,
                                              const char *name);

/**
 * @brief Clear all custom headers
 * 
 * @param service HTTP service
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_clear_headers(neoc_http_service_t *service);

/**
 * @brief Perform IO operation (send JSON-RPC request)
 * 
 * Equivalent to Swift's performIO method. This is the core method
 * that sends the JSON payload and receives the response.
 * 
 * @param service HTTP service
 * @param payload JSON request payload
 * @param result Output response data (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_perform_io(neoc_http_service_t *service,
                                           const neoc_byte_array_t *payload,
                                           neoc_byte_array_t **result);

/**
 * @brief Get the service URL
 * 
 * @param service HTTP service
 * @return Service URL (do not free)
 */
const char* neoc_http_service_get_url(const neoc_http_service_t *service);

/**
 * @brief Set the service URL
 * 
 * @param service HTTP service
 * @param url New service URL
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_set_url(neoc_http_service_t *service,
                                        const char *url);

/**
 * @brief Check if service includes raw responses
 * 
 * @param service HTTP service
 * @return true if raw responses are included, false otherwise
 */
bool neoc_http_service_includes_raw_responses(const neoc_http_service_t *service);

/**
 * @brief Set whether to include raw responses
 * 
 * @param service HTTP service
 * @param include_raw Whether to include raw JSON responses
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_set_include_raw_responses(neoc_http_service_t *service,
                                                          bool include_raw);

/**
 * @brief Get the base service interface
 * 
 * Allows the HTTP service to be used as a generic service
 * 
 * @param http_service HTTP service instance
 * @return Base service interface
 */
neoc_service_t* neoc_http_service_get_base(neoc_http_service_t *http_service);

/**
 * @brief Cast base service to HTTP service
 * 
 * @param service Base service (must be HTTP type)
 * @return HTTP service or NULL if not HTTP type
 */
neoc_http_service_t* neoc_http_service_from_base(neoc_service_t *service);

/**
 * @brief Get header count
 * 
 * @param service HTTP service
 * @return Number of custom headers
 */
size_t neoc_http_service_get_header_count(const neoc_http_service_t *service);

/**
 * @brief Get header by index
 * 
 * @param service HTTP service
 * @param index Header index
 * @param header Output header (do not free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_service_get_header(const neoc_http_service_t *service,
                                           size_t index,
                                           const neoc_http_header_t **header);

/**
 * @brief Find header by name
 * 
 * @param service HTTP service
 * @param name Header name
 * @param value Output header value (do not free)
 * @return NEOC_SUCCESS if found, NEOC_ERROR_NOT_FOUND if not found
 */
neoc_error_t neoc_http_service_find_header(const neoc_http_service_t *service,
                                            const char *name,
                                            const char **value);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_HTTP_HTTP_SERVICE_H */
