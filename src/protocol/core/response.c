/**
 * @file response.c
 * @brief JSON-RPC 2.0 response implementation
 * 
 * Based on Swift source: protocol/core/Response.swift
 * Implements JSON-RPC response handling and parsing
 */

#include "../../../include/neoc/neoc_error.h"
#include "../../../include/neoc/neoc_memory.h"
#include "../../../include/neoc/protocol/core/response.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Create a new response structure
 */
neoc_response_t *neoc_response_create(int id) {
    neoc_response_t *response = neoc_malloc(sizeof(neoc_response_t));
    if (!response) {
        return NULL;
    }
    
    // Initialize response structure
    response->id = id;
    response->result = NULL;
    response->error = NULL;
    response->has_error = false;
    response->raw_response = NULL;
    
    // Set default JSON-RPC version
    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        neoc_free(response);
        return NULL;
    }
    
    return response;
}

/**
 * @brief Set the result for a response
 */
neoc_error_t neoc_response_set_result(neoc_response_t *response, void *result) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    response->result = result;
    response->has_error = false;  // Clear error flag when setting result
    return NEOC_SUCCESS;
}

/**
 * @brief Set an error for a response
 */
neoc_error_t neoc_response_set_error(neoc_response_t *response, int code, 
                                    const char *message, const char *data) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free existing error
    if (response->error) {
        neoc_rpc_error_free(response->error);
    }
    
    // Create new error
    response->error = neoc_rpc_error_create(code, message, data);
    if (!response->error && (message || data)) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    response->has_error = true;
    response->result = NULL;  // Clear result when setting error
    
    return NEOC_SUCCESS;
}

/**
 * @brief Check if response has an error
 */
bool neoc_response_has_error(const neoc_response_t *response) {
    if (!response) {
        return true;  // NULL response is considered an error
    }
    
    return response->has_error;
}

/**
 * @brief Get the result from response, checking for errors
 */
neoc_error_t neoc_response_get_result(const neoc_response_t *response, void **result_out) {
    if (!response || !result_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result_out = NULL;
    
    if (response->has_error) {
        return NEOC_ERROR_PROTOCOL;  // Response contains an error
    }
    
    *result_out = response->result;
    return NEOC_SUCCESS;
}

/**
 * @brief Set raw response string
 */
neoc_error_t neoc_response_set_raw(neoc_response_t *response, const char *raw_response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free existing raw response
    if (response->raw_response) {
        neoc_free(response->raw_response);
        response->raw_response = NULL;
    }
    
    // Set new raw response if provided
    if (raw_response) {
        response->raw_response = neoc_strdup(raw_response);
        if (!response->raw_response) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Create RPC error structure
 */
neoc_rpc_error_t *neoc_rpc_error_create(int code, const char *message, const char *data) {
    neoc_rpc_error_t *error = neoc_malloc(sizeof(neoc_rpc_error_t));
    if (!error) {
        return NULL;
    }
    
    error->code = code;
    
    // Copy message
    if (message) {
        error->message = neoc_strdup(message);
        if (!error->message) {
            neoc_free(error);
            return NULL;
        }
    } else {
        error->message = NULL;
    }
    
    // Copy data
    if (data) {
        error->data = neoc_strdup(data);
        if (!error->data) {
            neoc_free(error->message);
            neoc_free(error);
            return NULL;
        }
    } else {
        error->data = NULL;
    }
    
    return error;
}

/**
 * @brief Free RPC error structure
 */
void neoc_rpc_error_free(neoc_rpc_error_t *error) {
    if (!error) {
        return;
    }
    
    if (error->message) {
        neoc_free(error->message);
    }
    
    if (error->data) {
        neoc_free(error->data);
    }
    
    neoc_free(error);
}

/**
 * @brief Get error string representation
 */
char *neoc_rpc_error_to_string(const neoc_rpc_error_t *error) {
    if (!error) {
        return NULL;
    }
    
    const char *message = error->message ? error->message : "(no message)";
    const char *data = error->data ? error->data : "";
    
    // Calculate required buffer size
    size_t str_len;
    if (error->data) {
        str_len = snprintf(NULL, 0, "[%d]: %s (%s)", error->code, message, data) + 1;
    } else {
        str_len = snprintf(NULL, 0, "[%d]: %s", error->code, message) + 1;
    }
    
    char *result = neoc_malloc(str_len);
    if (!result) {
        return NULL;
    }
    
    if (error->data) {
        snprintf(result, str_len, "[%d]: %s (%s)", error->code, message, data);
    } else {
        snprintf(result, str_len, "[%d]: %s", error->code, message);
    }
    
    return result;
}

/**
 * @brief Free response structure
 */
void neoc_response_free(neoc_response_t *response) {
    if (!response) {
        return;
    }
    
    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
    }
    
    if (response->error) {
        neoc_rpc_error_free(response->error);
    }
    
    if (response->raw_response) {
        neoc_free(response->raw_response);
    }
    
    if (response->result) {
        neoc_free(response->result);
    }

    neoc_free(response);
}
