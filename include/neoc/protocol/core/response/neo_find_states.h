/**
 * @file neo_find_states.h
 * @brief Neo find states response for state root queries
 * 
 * Based on Swift source: protocol/core/response/NeoFindStates.swift
 * Response structure for findstates RPC call
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_FIND_STATES_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_FIND_STATES_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Key-value result from state query
 * 
 * Represents a single key-value pair from the state trie
 */
typedef struct {
    char *key;                          /**< State key as hex string */
    char *value;                        /**< State value as hex string */
} neoc_find_states_result_t;

/**
 * @brief States container for find states response
 * 
 * Contains the state query results with pagination proofs
 */
typedef struct {
    char *first_proof;                  /**< First proof for pagination (nullable) */
    char *last_proof;                   /**< Last proof for pagination (nullable) */
    bool truncated;                     /**< Whether results were truncated */
    neoc_find_states_result_t **results; /**< Array of key-value results */
    size_t results_count;               /**< Number of results */
} neoc_find_states_t;

/**
 * @brief FindStates JSON-RPC response
 * 
 * Response for findstates RPC method
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                            /**< Request ID */
    neoc_find_states_t *result;         /**< Find states result */
    
    /* Error fields */
    int error_code;                     /**< Error code if request failed */
    char *error_message;                /**< Error message if request failed */
} neoc_find_states_response_t;

/**
 * @brief Create a new find states result
 * 
 * @param result Pointer to store the created result
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_result_create(
    neoc_find_states_result_t **result
);

/**
 * @brief Free a find states result and its resources
 * 
 * @param result Result to free
 */
void neoc_find_states_result_free(
    neoc_find_states_result_t *result
);

/**
 * @brief Create a new find states container
 * 
 * @param states Pointer to store the created states
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_create(
    neoc_find_states_t **states
);

/**
 * @brief Free a find states container and its resources
 * 
 * @param states States container to free
 */
void neoc_find_states_free(
    neoc_find_states_t *states
);

/**
 * @brief Create a new FindStates response
 * 
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_response_create(
    neoc_find_states_response_t **response
);

/**
 * @brief Free a FindStates response and its resources
 * 
 * @param response Response to free
 */
void neoc_find_states_response_free(
    neoc_find_states_response_t *response
);

/**
 * @brief Parse JSON into FindStates response
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_response_from_json(
    const char *json_str,
    neoc_find_states_response_t **response
);

/**
 * @brief Convert FindStates response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_response_to_json(
    const neoc_find_states_response_t *response,
    char **json_str
);

/**
 * @brief Add a result to the find states container
 * 
 * @param states States container to modify
 * @param result Result to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_add_result(
    neoc_find_states_t *states,
    neoc_find_states_result_t *result
);

/**
 * @brief Set the pagination proofs for find states
 * 
 * @param states States container to modify
 * @param first_proof First proof (can be NULL)
 * @param last_proof Last proof (can be NULL)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_set_proofs(
    neoc_find_states_t *states,
    const char *first_proof,
    const char *last_proof
);

/**
 * @brief Set the truncated flag for find states
 * 
 * @param states States container to modify
 * @param truncated Whether results were truncated
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_find_states_set_truncated(
    neoc_find_states_t *states,
    bool truncated
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_FIND_STATES_H */
