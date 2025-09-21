/**
 * @file neo_get_state_root.h
 * @brief Neo state root response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetStateRoot.swift
 * Provides functionality for retrieving blockchain state root information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_STATE_ROOT_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_STATE_ROOT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash256.h"
#include "neoc/transaction/witness.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents blockchain state root information
 * 
 * Contains comprehensive state root data including version, index,
 * root hash, and associated witnesses for state verification.
 */
typedef struct {
    int version;                        /**< State root version */
    int index;                          /**< Block index */
    neoc_hash256_t root_hash;           /**< State root hash */
    neoc_witness_t *witnesses;          /**< Array of witnesses */
    size_t witnesses_count;             /**< Number of witnesses */
} neoc_state_root_t;

/**
 * @brief Response structure for getstateroot RPC call
 * 
 * Contains the state root information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_state_root_t *result;          /**< State root result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_state_root_t;

/**
 * @brief Create a new state root structure
 * 
 * @param version State root version
 * @param index Block index
 * @param root_hash State root hash
 * @param witnesses Array of witnesses
 * @param witnesses_count Number of witnesses
 * @param state_root Pointer to store the created state root (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_state_root_create(
    int version,
    int index,
    const neoc_hash256_t *root_hash,
    neoc_witness_t *witnesses,
    size_t witnesses_count,
    neoc_state_root_t **state_root
);

/**
 * @brief Free a state root structure
 * 
 * @param state_root State root to free
 */
void neoc_state_root_free(neoc_state_root_t *state_root);

/**
 * @brief Create a new Neo get state root response
 * 
 * @param id Request ID
 * @param state_root State root data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_create(
    int id,
    neoc_state_root_t *state_root,
    const char *error,
    int error_code,
    neoc_neo_get_state_root_t **response
);

/**
 * @brief Free a Neo get state root response
 * 
 * @param response Response to free
 */
void neoc_neo_get_state_root_free(neoc_neo_get_state_root_t *response);

/**
 * @brief Parse JSON response into Neo get state root structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_from_json(
    const char *json_str,
    neoc_neo_get_state_root_t **response
);

/**
 * @brief Convert Neo get state root response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_to_json(
    const neoc_neo_get_state_root_t *response,
    char **json_str
);

/**
 * @brief Get state root from response (convenience function)
 * 
 * @param response Response to get state root from
 * @return State root pointer, or NULL if error or no result
 */
neoc_state_root_t *neoc_neo_get_state_root_get_state_root(
    const neoc_neo_get_state_root_t *response
);

/**
 * @brief Check if the response contains valid state root data
 * 
 * @param response Response to check
 * @return true if response has valid state root, false otherwise
 */
bool neoc_neo_get_state_root_has_state_root(
    const neoc_neo_get_state_root_t *response
);

/**
 * @brief Get state root version from response
 * 
 * @param response Response to get version from
 * @param version Pointer to store the version (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_get_version(
    const neoc_neo_get_state_root_t *response,
    int *version
);

/**
 * @brief Get block index from response
 * 
 * @param response Response to get index from
 * @param index Pointer to store the index (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_get_index(
    const neoc_neo_get_state_root_t *response,
    int *index
);

/**
 * @brief Get root hash from response
 * 
 * @param response Response to get hash from
 * @param root_hash Pointer to store the hash (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_get_root_hash(
    const neoc_neo_get_state_root_t *response,
    neoc_hash256_t *root_hash
);

/**
 * @brief Get number of witnesses from response
 * 
 * @param response Response to count witnesses from
 * @return Number of witnesses, or 0 on error
 */
size_t neoc_neo_get_state_root_get_witnesses_count(
    const neoc_neo_get_state_root_t *response
);

/**
 * @brief Get witness by index from response
 * 
 * @param response Response to get witness from
 * @param index Index of the witness
 * @return Witness pointer, or NULL if index out of bounds or error
 */
neoc_witness_t *neoc_neo_get_state_root_get_witness(
    const neoc_neo_get_state_root_t *response,
    size_t index
);

/**
 * @brief Copy root hash as string from response
 * 
 * @param response Response to get hash from
 * @param hash_str Pointer to store the hash string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_root_get_root_hash_string(
    const neoc_neo_get_state_root_t *response,
    char **hash_str
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_STATE_ROOT_H */
