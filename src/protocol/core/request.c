/**
 * @file request.c
 * @brief Core RPC request implementation
 * 
 * Implements the core Neo RPC request structure and functionality
 * based on the Swift Request implementation.
 */

#include "neoc/neoc.h"
#include "neoc/protocol/core/request.h"
#include "neoc/protocol/service/neo_c_service.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Global request counter for generating unique request IDs
static int request_counter = 1;

/**
 * @brief Create a new Neo RPC request
 * @param method The RPC method name
 * @param params JSON string containing parameters
 * @param service The NeoSwift service instance
 * @return Newly created request or NULL on error
 */
neoc_request_t *neoc_request_create(const char *method, const char *params, void *service) {
    if (!method) {
        return NULL;
    }
    
    neoc_request_t *request = (neoc_request_t *)neoc_malloc(sizeof(neoc_request_t));
    if (!request) {
        return NULL;
    }
    
    // Initialize with default values
    request->jsonrpc = neoc_malloc(strlen("2.0") + 1);
    if (!request->jsonrpc) {
        neoc_free(request);
        return NULL;
    }
    strcpy(request->jsonrpc, "2.0");
    
    request->method = neoc_malloc(strlen(method) + 1);
    if (!request->method) {
        neoc_free(request->jsonrpc);
        neoc_free(request);
        return NULL;
    }
    strcpy(request->method, method);
    
    if (params) {
        request->params = neoc_malloc(strlen(params) + 1);
        if (!request->params) {
            neoc_free(request->method);
            neoc_free(request->jsonrpc);
            neoc_free(request);
            return NULL;
        }
        strcpy(request->params, params);
    } else {
        request->params = NULL;
    }
    
    request->id = request_counter++;
    request->service = service;
    
    return request;
}

/**
 * @brief Free a Neo RPC request
 * @param request The request to free
 */
void neoc_request_free(neoc_request_t *request) {
    if (!request) {
        return;
    }
    
    if (request->jsonrpc) {
        neoc_free(request->jsonrpc);
    }
    if (request->method) {
        neoc_free(request->method);
    }
    if (request->params) {
        neoc_free(request->params);
    }
    
    neoc_free(request);
}

/**
 * @brief Convert request to JSON string
 * @param request The request to serialize
 * @return JSON string (caller must free) or NULL on error
 */
char *neoc_request_to_json(const neoc_request_t *request) {
    if (!request || !request->method) {
        return NULL;
    }

    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NULL;
    }

    neoc_error_t err = neoc_json_add_string(json, "jsonrpc", request->jsonrpc ? request->jsonrpc : "2.0");
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NULL;
    }

    err = neoc_json_add_string(json, "method", request->method);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NULL;
    }

    err = neoc_json_add_number(json, "id", (double)request->id);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NULL;
    }

    if (request->params) {
        neoc_json_t *params_json = neoc_json_parse(request->params);
        if (params_json) {
            err = neoc_json_add_object(json, "params", params_json);
            if (err != NEOC_SUCCESS) {
                neoc_json_free(params_json);
                neoc_json_free(json);
                return NULL;
            }
        } else {
            neoc_json_t *params_array = neoc_json_create_array();
            if (!params_array) {
                neoc_json_free(json);
                return NULL;
            }
            err = neoc_json_add_object(json, "params", params_array);
            if (err != NEOC_SUCCESS) {
                neoc_json_free(params_array);
                neoc_json_free(json);
                return NULL;
            }
        }
    } else {
        neoc_json_t *params_array = neoc_json_create_array();
        if (!params_array) {
            neoc_json_free(json);
            return NULL;
        }
        err = neoc_json_add_object(json, "params", params_array);
        if (err != NEOC_SUCCESS) {
            neoc_json_free(params_array);
            neoc_json_free(json);
            return NULL;
        }
    }

    char *json_string = neoc_json_to_string(json);
    neoc_json_free(json);
    return json_string;
}

/**
 * @brief Create request from JSON string
 * @param json_string The JSON string to parse
 * @param service The service instance to associate
 * @return Parsed request or NULL on error
 */
neoc_request_t *neoc_request_from_json(const char *json_string, void *service) {
    if (!json_string) {
        return NULL;
    }

    neoc_json_t *json = neoc_json_parse(json_string);
    if (!json) {
        return NULL;
    }

    const char *method = neoc_json_get_string(json, "method");
    if (!method) {
        neoc_json_free(json);
        return NULL;
    }

    neoc_json_t *params_json = neoc_json_get_array(json, "params");
    if (!params_json) {
        params_json = neoc_json_get_object(json, "params");
    }

    char *params_string = NULL;
    if (params_json) {
        params_string = neoc_json_to_string(params_json);
        if (!params_string) {
            neoc_json_free(json);
            return NULL;
        }
    }

    neoc_request_t *request = neoc_request_create(method, params_string, service);
    if (params_string) {
        neoc_free(params_string);
    }

    if (!request) {
        neoc_json_free(json);
        return NULL;
    }

    int64_t id_value = 0;
    if (neoc_json_get_int(json, "id", &id_value) == NEOC_SUCCESS) {
        request->id = (int)id_value;
    }

    neoc_json_free(json);
    return request;
}

/**
 * @brief Get the current request counter value
 * @return Current counter value
 */
int neoc_request_get_counter(void) {
    return request_counter;
}

/**
 * @brief Reset the request counter
 * @param value New counter value
 */
void neoc_request_set_counter(int value) {
    request_counter = value;
}
