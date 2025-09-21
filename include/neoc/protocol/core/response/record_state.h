/**
 * @file record_state.h
 * @brief Record state structure for NNS records
 * 
 * Converted from Swift source: protocol/core/response/RecordState.swift
 * Represents a DNS-like record state in the Neo Name Service (NNS).
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_RECORD_STATE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_RECORD_STATE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/core/record_type.h"
#include "neoc/protocol/stack_item.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Record state structure
 * 
 * Represents the state of a DNS-like record in the Neo Name Service (NNS).
 */
typedef struct {
    char *name;                         /**< Record name */
    neoc_record_type_t record_type;     /**< Record type (A, CNAME, TXT, AAAA) */
    char *data;                         /**< Record data */
} neoc_record_state_t;

/**
 * @brief JSON-RPC response for record state
 * 
 * Complete response structure for record state queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_record_state_t *result;        /**< Record state result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_record_state_response_t;

/**
 * @brief Create a new record state
 * 
 * @param name Record name
 * @param record_type Record type
 * @param data Record data
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_create(
    const char *name,
    neoc_record_type_t record_type,
    const char *data,
    neoc_record_state_t **state
);

/**
 * @brief Free a record state
 * 
 * @param state State to free (can be NULL)
 */
void neoc_record_state_free(
    neoc_record_state_t *state
);

/**
 * @brief Create a new record state response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Record state result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_response_create(
    const char *jsonrpc,
    int id,
    neoc_record_state_t *result,
    const char *error,
    int error_code,
    neoc_record_state_response_t **response
);

/**
 * @brief Free a record state response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_record_state_response_free(
    neoc_record_state_response_t *response
);

/**
 * @brief Parse record state from JSON string
 * 
 * @param json_str JSON string containing record state data
 * @param state Output pointer for the parsed state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_from_json(
    const char *json_str,
    neoc_record_state_t **state
);

/**
 * @brief Parse record state response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_response_from_json(
    const char *json_str,
    neoc_record_state_response_t **response
);

/**
 * @brief Convert record state to JSON string
 * 
 * @param state Record state to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_to_json(
    const neoc_record_state_t *state,
    char **json_str
);

/**
 * @brief Create record state from stack item
 * 
 * Converts a Neo stack item array to a record state structure.
 * 
 * @param stack_item Stack item containing record state data
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_from_stack_item(
    const neoc_stack_item_t *stack_item,
    neoc_record_state_t **state
);

/**
 * @brief Create a copy of a record state
 * 
 * @param src Source state to copy
 * @param dest Output pointer for the copied state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_state_copy(
    const neoc_record_state_t *src,
    neoc_record_state_t **dest
);

/**
 * @brief Compare two record states for equality
 * 
 * @param state1 First state
 * @param state2 Second state
 * @return true if states are equal, false otherwise
 */
bool neoc_record_state_equals(
    const neoc_record_state_t *state1,
    const neoc_record_state_t *state2
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_RECORD_STATE_H */
