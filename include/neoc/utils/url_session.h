/**
 * @file url_session.h
 * @brief HTTP request utilities for networking
 * 
 * Based on Swift source: utils/URLSession.swift
 * Provides HTTP request functionality using libcurl
 */

#ifndef NEOC_UTILS_URL_SESSION_H
#define NEOC_UTILS_URL_SESSION_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/array.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HTTP request method enumeration
 */
typedef enum {
    NEOC_HTTP_GET = 0,
    NEOC_HTTP_POST,
    NEOC_HTTP_PUT,
    NEOC_HTTP_DELETE,
    NEOC_HTTP_HEAD,
    NEOC_HTTP_OPTIONS,
    NEOC_HTTP_PATCH
} neoc_http_method_t;

/**
 * @brief HTTP request header structure
 */
typedef struct {
    char *name;             /**< Header name */
    char *value;            /**< Header value */
} neoc_http_header_t;

/**
 * @brief HTTP request structure
 */
typedef struct {
    char *url;                              /**< Request URL */
    neoc_http_method_t method;              /**< HTTP method */
    neoc_http_header_t *headers;            /**< Request headers */
    size_t header_count;                    /**< Number of headers */
    neoc_byte_array_t *body;                /**< Request body (nullable) */
    long timeout_seconds;                   /**< Timeout in seconds */
    bool follow_redirects;                  /**< Follow HTTP redirects */
    bool verify_ssl;                        /**< Verify SSL certificates */
} neoc_http_request_t;

/**
 * @brief HTTP response structure
 */
typedef struct {
    long status_code;                       /**< HTTP status code */
    neoc_http_header_t *headers;            /**< Response headers */
    size_t header_count;                    /**< Number of headers */
    neoc_byte_array_t *body;                /**< Response body */
    char *error_message;                    /**< Error message (nullable) */
} neoc_http_response_t;

/**
 * @brief URL session configuration
 */
typedef struct {
    long timeout_seconds;                   /**< Default timeout in seconds */
    bool follow_redirects;                  /**< Follow redirects by default */
    bool verify_ssl;                        /**< Verify SSL by default */
    char *user_agent;                       /**< User agent string */
    neoc_http_header_t *default_headers;    /**< Default headers for all requests */
    size_t default_header_count;            /**< Number of default headers */
} neoc_url_session_config_t;

/**
 * @brief URL session for making HTTP requests
 */
typedef struct neoc_url_session_t neoc_url_session_t;

/**
 * @brief Create a new URL session with default configuration
 * 
 * @param session Output session handle (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_url_session_create(neoc_url_session_t **session);

/**
 * @brief Create a new URL session with custom configuration
 * 
 * @param config Session configuration
 * @param session Output session handle (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_url_session_create_with_config(const neoc_url_session_config_t *config,
                                                   neoc_url_session_t **session);

/**
 * @brief Free a URL session
 * 
 * @param session Session to free
 */
void neoc_url_session_free(neoc_url_session_t *session);

/**
 * @brief Create an HTTP request
 * 
 * @param url Request URL
 * @param method HTTP method
 * @param request Output request handle (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_request_create(const char *url,
                                       neoc_http_method_t method,
                                       neoc_http_request_t **request);

/**
 * @brief Add header to HTTP request
 * 
 * @param request HTTP request
 * @param name Header name
 * @param value Header value
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_request_add_header(neoc_http_request_t *request,
                                           const char *name,
                                           const char *value);

/**
 * @brief Set request body
 * 
 * @param request HTTP request
 * @param body Request body data
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_request_set_body(neoc_http_request_t *request,
                                         const neoc_byte_array_t *body);

/**
 * @brief Set request body from string
 * 
 * @param request HTTP request
 * @param body_string String body content
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_http_request_set_body_string(neoc_http_request_t *request,
                                                const char *body_string);

/**
 * @brief Free an HTTP request
 * 
 * @param request Request to free
 */
void neoc_http_request_free(neoc_http_request_t *request);

/**
 * @brief Free an HTTP response
 * 
 * @param response Response to free
 */
void neoc_http_response_free(neoc_http_response_t *response);

/**
 * @brief Perform HTTP request (blocking)
 * 
 * @param session URL session
 * @param request HTTP request
 * @param response Output response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_url_session_perform_request(neoc_url_session_t *session,
                                               const neoc_http_request_t *request,
                                               neoc_http_response_t **response);

/**
 * @brief Convenience function for GET request
 * 
 * @param session URL session
 * @param url URL to request
 * @param response Output response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_url_session_get(neoc_url_session_t *session,
                                   const char *url,
                                   neoc_http_response_t **response);

/**
 * @brief Convenience function for POST request with JSON body
 * 
 * @param session URL session
 * @param url URL to request
 * @param json_body JSON body as string
 * @param response Output response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_url_session_post_json(neoc_url_session_t *session,
                                         const char *url,
                                         const char *json_body,
                                         neoc_http_response_t **response);

/**
 * @brief Get HTTP method as string
 * 
 * @param method HTTP method
 * @return Method string (static, do not free)
 */
const char* neoc_http_method_to_string(neoc_http_method_t method);

/**
 * @brief Get default URL session configuration
 * 
 * @param config Output configuration (caller must free values)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_url_session_get_default_config(neoc_url_session_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_URL_SESSION_H */
