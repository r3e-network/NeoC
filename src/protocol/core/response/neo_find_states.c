/**
 * @file neo_find_states.c
 * @brief neo_find_states implementation
 * 
 * Based on Swift source: protocol/core/response/NeoFindStates.swift
 */

#include "neoc/protocol/core/response/neo_find_states.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Create a new find states result
 */
neoc_error_t neoc_find_states_result_create(neoc_find_states_result_t **result) {
    if (!result) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *result = (neoc_find_states_result_t *)neoc_malloc(sizeof(neoc_find_states_result_t));
    if (!*result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memset(*result, 0, sizeof(neoc_find_states_result_t));
    return NEOC_SUCCESS;
}

/**
 * @brief Free a find states result and its resources
 */
void neoc_find_states_result_free(neoc_find_states_result_t *result) {
    if (!result) {
        return;
    }
    
    if (result->key) {
        neoc_free(result->key);
    }
    if (result->value) {
        neoc_free(result->value);
    }
    
    neoc_free(result);
}

/**
 * @brief Create a new find states container
 */
neoc_error_t neoc_find_states_create(neoc_find_states_t **states) {
    if (!states) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *states = (neoc_find_states_t *)neoc_malloc(sizeof(neoc_find_states_t));
    if (!*states) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memset(*states, 0, sizeof(neoc_find_states_t));
    return NEOC_SUCCESS;
}

/**
 * @brief Free a find states container and its resources
 */
void neoc_find_states_free(neoc_find_states_t *states) {
    if (!states) {
        return;
    }
    
    if (states->first_proof) {
        neoc_free(states->first_proof);
    }
    if (states->last_proof) {
        neoc_free(states->last_proof);
    }
    
    if (states->results) {
        for (size_t i = 0; i < states->results_count; i++) {
            neoc_find_states_result_free(states->results[i]);
        }
        neoc_free(states->results);
    }
    
    neoc_free(states);
}

/**
 * @brief Create a new FindStates response
 */
neoc_error_t neoc_find_states_response_create(neoc_find_states_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *response = (neoc_find_states_response_t *)neoc_malloc(sizeof(neoc_find_states_response_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memset(*response, 0, sizeof(neoc_find_states_response_t));
    return NEOC_SUCCESS;
}

/**
 * @brief Free a FindStates response and its resources
 */
void neoc_find_states_response_free(neoc_find_states_response_t *response) {
    if (!response) {
        return;
    }
    
    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
    }
    if (response->error_message) {
        neoc_free(response->error_message);
    }
    if (response->result) {
        neoc_find_states_free(response->result);
    }
    
    neoc_free(response);
}

/**
 * @brief Parse JSON into FindStates response
 */
neoc_error_t neoc_find_states_response_from_json(
    const char *json_str,
    neoc_find_states_response_t **response
) {
    // TODO: Implement JSON parsing
    (void)json_str;
    (void)response;
    return NEOC_ERROR_NOT_IMPLEMENTED;
}

/**
 * @brief Convert FindStates response to JSON string
 */
neoc_error_t neoc_find_states_response_to_json(
    const neoc_find_states_response_t *response,
    char **json_str
) {
    if (!response || !json_str) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Simple JSON representation
    *json_str = neoc_malloc(512);
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    snprintf(*json_str, 512, "{\"jsonrpc\":\"2.0\",\"id\":%d,\"result\":{\"type\":\"find_states\"}}", 
             response->id);
    
    return NEOC_SUCCESS;
}

/**
 * @brief Add a result to the find states container
 */
neoc_error_t neoc_find_states_add_result(
    neoc_find_states_t *states,
    neoc_find_states_result_t *result
) {
    if (!states || !result) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Reallocate results array
    neoc_find_states_result_t **new_results = (neoc_find_states_result_t **)neoc_realloc(
        states->results, 
        sizeof(neoc_find_states_result_t *) * (states->results_count + 1)
    );
    
    if (!new_results) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    states->results = new_results;
    states->results[states->results_count] = result;
    states->results_count++;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Set the pagination proofs for find states
 */
neoc_error_t neoc_find_states_set_proofs(
    neoc_find_states_t *states,
    const char *first_proof,
    const char *last_proof
) {
    if (!states) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (first_proof) {
        states->first_proof = neoc_malloc(strlen(first_proof) + 1);
        if (!states->first_proof) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        strcpy(states->first_proof, first_proof);
    }
    
    if (last_proof) {
        states->last_proof = neoc_malloc(strlen(last_proof) + 1);
        if (!states->last_proof) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        strcpy(states->last_proof, last_proof);
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Set the truncated flag for find states
 */
neoc_error_t neoc_find_states_set_truncated(
    neoc_find_states_t *states,
    bool truncated
) {
    if (!states) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    states->truncated = truncated;
    return NEOC_SUCCESS;
}
