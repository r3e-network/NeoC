/**
 * @file neo_get_state_height.c
 * @brief Neo state height response implementation
 * 
 * Based on Swift source: protocol/core/response/NeoGetStateHeight.swift
 * Implements Neo blockchain state height information handling
 */

#include "../../../../include/neoc/neoc_error.h"
#include "../../../../include/neoc/neoc_memory.h"
#include "../../../../include/neoc/protocol/core/response/neo_get_state_height.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Create a new state height structure
 */
neoc_error_t neoc_state_height_create(int local_root_index,
                                       int validated_root_index,
                                       neoc_state_height_t **state_height) {
    if (!state_height) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *state_height = NULL;
    
    neoc_state_height_t *new_state_height = neoc_malloc(sizeof(neoc_state_height_t));
    if (!new_state_height) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_state_height->local_root_index = local_root_index;
    new_state_height->validated_root_index = validated_root_index;
    
    *state_height = new_state_height;
    return NEOC_SUCCESS;
}

/**
 * @brief Free a state height structure
 */
void neoc_state_height_free(neoc_state_height_t *state_height) {
    if (!state_height) {
        return;
    }
    
    neoc_free(state_height);
}

/**
 * @brief Create a new Neo get state height response
 */
neoc_error_t neoc_neo_get_state_height_create(int id,
                                               neoc_state_height_t *state_height,
                                               const char *error,
                                               int error_code,
                                               neoc_neo_get_state_height_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *response = NULL;
    
    neoc_neo_get_state_height_t *new_response = neoc_malloc(sizeof(neoc_neo_get_state_height_t));
    if (!new_response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize response
    new_response->jsonrpc = neoc_strdup("2.0");
    if (!new_response->jsonrpc) {
        neoc_free(new_response);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_response->id = id;
    new_response->result = state_height;  // Take ownership
    new_response->error_code = error_code;
    
    if (error) {
        new_response->error = neoc_strdup(error);
        if (!new_response->error) {
            neoc_free(new_response->jsonrpc);
            neoc_free(new_response);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    } else {
        new_response->error = NULL;
    }
    
    *response = new_response;
    return NEOC_SUCCESS;
}

/**
 * @brief Free a Neo get state height response
 */
void neoc_neo_get_state_height_free(neoc_neo_get_state_height_t *response) {
    if (!response) {
        return;
    }
    
    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
    }
    
    if (response->error) {
        neoc_free(response->error);
    }
    
    if (response->result) {
        neoc_state_height_free(response->result);
    }
    
    neoc_free(response);
}

/**
 * @brief Parse JSON response into Neo get state height structure
 */
neoc_error_t neoc_neo_get_state_height_from_json(const char *json_str,
                                                  neoc_neo_get_state_height_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *response = NULL;
    
    // Parse JSON using the neoc_json library
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Create response structure
    *response = neoc_calloc(1, sizeof(neoc_neo_get_state_height_t));
    if (!*response) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Parse the result object
    neoc_json_t *result = neoc_json_get_object(json, "result");
    if (result) {
        // Get local root index
        (*response)->local_root_index = (uint32_t)neoc_json_get_int(result, "localrootindex");
        
        // Get validated root index
        (*response)->validated_root_index = (uint32_t)neoc_json_get_int(result, "validatedrootindex");
    } else {
        // Try parsing direct fields if not wrapped in result
        (*response)->local_root_index = (uint32_t)neoc_json_get_int(json, "localrootindex");
        (*response)->validated_root_index = (uint32_t)neoc_json_get_int(json, "validatedrootindex");
    }
    
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

/**
 * @brief Convert Neo get state height response to JSON string
 */
neoc_error_t neoc_neo_get_state_height_to_json(const neoc_neo_get_state_height_t *response,
                                                char **json_str) {
    if (!response || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *json_str = NULL;
    
    // Create JSON object
    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add local root index
    neoc_json_add_int(json, "localrootindex", response->local_root_index);
    
    // Add validated root index
    neoc_json_add_int(json, "validatedrootindex", response->validated_root_index);
    
    // Convert to string
    *json_str = neoc_json_to_string(json);
    neoc_json_free(json);
    
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get state height from response (convenience function)
 */
neoc_state_height_t *neoc_neo_get_state_height_get_state_height(const neoc_neo_get_state_height_t *response) {
    if (!response) {
        return NULL;
    }
    
    return response->result;
}

/**
 * @brief Check if the response contains valid state height data
 */
bool neoc_neo_get_state_height_has_state_height(const neoc_neo_get_state_height_t *response) {
    if (!response) {
        return false;
    }
    
    return (response->result != NULL && response->error_code == 0);
}

/**
 * @brief Get local root index from response
 */
neoc_error_t neoc_neo_get_state_height_get_local_root_index(const neoc_neo_get_state_height_t *response,
                                                             int *local_root_index) {
    if (!response || !local_root_index) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    
    *local_root_index = response->result->local_root_index;
    return NEOC_SUCCESS;
}

/**
 * @brief Get validated root index from response
 */
neoc_error_t neoc_neo_get_state_height_get_validated_root_index(const neoc_neo_get_state_height_t *response,
                                                                 int *validated_root_index) {
    if (!response || !validated_root_index) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    
    *validated_root_index = response->result->validated_root_index;
    return NEOC_SUCCESS;
}

/**
 * @brief Check if local and validated indices are synchronized
 */
bool neoc_neo_get_state_height_is_synchronized(const neoc_neo_get_state_height_t *response) {
    if (!response || !response->result) {
        return false;
    }
    
    return (response->result->local_root_index == response->result->validated_root_index);
}

/**
 * @brief Get synchronization lag
 */
neoc_error_t neoc_neo_get_state_height_get_lag(const neoc_neo_get_state_height_t *response,
                                                int *lag) {
    if (!response || !lag) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    
    // Calculate lag as absolute difference between local and validated indices
    *lag = response->result->local_root_index - response->result->validated_root_index;
    if (*lag < 0) {
        *lag = -*lag;  // Make positive
    }
    
    return NEOC_SUCCESS;
}

