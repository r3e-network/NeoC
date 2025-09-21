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
#include <string.h>
#include <stdio.h>
#include <cjson/cJSON.h>

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
    
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    cJSON_AddStringToObject(json, "jsonrpc", request->jsonrpc ? request->jsonrpc : "2.0");
    cJSON_AddStringToObject(json, "method", request->method);
    cJSON_AddNumberToObject(json, "id", request->id);
    
    // Parse params if they exist
    if (request->params) {
        cJSON *params_json = cJSON_Parse(request->params);
        if (params_json) {
            cJSON_AddItemToObject(json, "params", params_json);
        } else {
            // If parsing fails, treat as empty array
            cJSON_AddArrayToObject(json, "params");
        }
    } else {
        cJSON_AddArrayToObject(json, "params");
    }
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
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
    
    cJSON *json = cJSON_Parse(json_string);
    if (!json) {
        return NULL;
    }
    
    cJSON *method_json = cJSON_GetObjectItem(json, "method");
    if (!method_json || !cJSON_IsString(method_json)) {
        cJSON_Delete(json);
        return NULL;
    }
    
    cJSON *params_json = cJSON_GetObjectItem(json, "params");
    char *params_string = NULL;
    if (params_json) {
        params_string = cJSON_Print(params_json);
    }
    
    neoc_request_t *request = neoc_request_create(method_json->valuestring, params_string, service);
    
    if (params_string) {
        neoc_free(params_string);
    }
    
    // Override ID if present
    cJSON *id_json = cJSON_GetObjectItem(json, "id");
    if (id_json && cJSON_IsNumber(id_json)) {
        request->id = (int)id_json->valuedouble;
    }
    
    cJSON_Delete(json);
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
