/**
 * @file neo_get_state_height.h
 * @brief Neo state height response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetStateHeight.swift
 * Provides functionality for retrieving blockchain state height information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_STATE_HEIGHT_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_STATE_HEIGHT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents blockchain state height information
 * 
 * Contains information about local and validated state root indices
 * for blockchain state tracking.
 */
typedef struct {
    int local_root_index;               /**< Local root index */
    int validated_root_index;           /**< Validated root index */
} neoc_state_height_t;

/**
 * @brief Response structure for getstateheight RPC call
 * 
 * Contains the state height information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_state_height_t *result;        /**< State height result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_state_height_t;

/**
 * @brief Create a new state height structure
 * 
 * @param local_root_index Local root index
 * @param validated_root_index Validated root index
 * @param state_height Pointer to store the created state height (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_state_height_create(
    int local_root_index,
    int validated_root_index,
    neoc_state_height_t **state_height
);

/**
 * @brief Free a state height structure
 * 
 * @param state_height State height to free
 */
void neoc_state_height_free(neoc_state_height_t *state_height);

/**
 * @brief Create a new Neo get state height response
 * 
 * @param id Request ID
 * @param state_height State height data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_height_create(
    int id,
    neoc_state_height_t *state_height,
    const char *error,
    int error_code,
    neoc_neo_get_state_height_t **response
);

/**
 * @brief Free a Neo get state height response
 * 
 * @param response Response to free
 */
void neoc_neo_get_state_height_free(neoc_neo_get_state_height_t *response);

/**
 * @brief Parse JSON response into Neo get state height structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_height_from_json(
    const char *json_str,
    neoc_neo_get_state_height_t **response
);

/**
 * @brief Convert Neo get state height response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_height_to_json(
    const neoc_neo_get_state_height_t *response,
    char **json_str
);

/**
 * @brief Get state height from response (convenience function)
 * 
 * @param response Response to get state height from
 * @return State height pointer, or NULL if error or no result
 */
neoc_state_height_t *neoc_neo_get_state_height_get_state_height(
    const neoc_neo_get_state_height_t *response
);

/**
 * @brief Check if the response contains valid state height data
 * 
 * @param response Response to check
 * @return true if response has valid state height, false otherwise
 */
bool neoc_neo_get_state_height_has_state_height(
    const neoc_neo_get_state_height_t *response
);

/**
 * @brief Get local root index from response
 * 
 * @param response Response to get index from
 * @param local_root_index Pointer to store the index (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_height_get_local_root_index(
    const neoc_neo_get_state_height_t *response,
    int *local_root_index
);

/**
 * @brief Get validated root index from response
 * 
 * @param response Response to get index from
 * @param validated_root_index Pointer to store the index (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_height_get_validated_root_index(
    const neoc_neo_get_state_height_t *response,
    int *validated_root_index
);

/**
 * @brief Check if local and validated indices are synchronized
 * 
 * @param response Response to check
 * @return true if indices are equal (synchronized), false otherwise
 */
bool neoc_neo_get_state_height_is_synchronized(
    const neoc_neo_get_state_height_t *response
);

/**
 * @brief Get synchronization lag
 * 
 * @param response Response to check
 * @param lag Pointer to store the lag value (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_state_height_get_lag(
    const neoc_neo_get_state_height_t *response,
    int *lag
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_STATE_HEIGHT_H */
