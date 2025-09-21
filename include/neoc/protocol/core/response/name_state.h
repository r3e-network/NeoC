/**
 * @file name_state.h
 * @brief Name state structure for NNS (Neo Name Service)
 * 
 * Converted from Swift source: protocol/core/response/NameState.swift
 * Represents the state of a domain name in the Neo Name Service.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NAME_STATE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NAME_STATE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Name state structure for NNS domains
 * 
 * Represents the state of a domain name in the Neo Name Service (NNS),
 * including name, expiration time, and admin hash.
 */
typedef struct {
    char *name;                         /**< Domain name */
    int *expiration;                    /**< Expiration timestamp (NULL if not set) */
    neoc_hash160_t *admin;              /**< Admin hash (NULL if not set) */
} neoc_name_state_t;

/**
 * @brief Create a new name state
 * 
 * @param name Domain name
 * @param expiration Expiration timestamp (can be NULL)
 * @param admin Admin hash (can be NULL)
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_name_state_create(
    const char *name,
    const int *expiration,
    const neoc_hash160_t *admin,
    neoc_name_state_t **state
);

/**
 * @brief Free a name state
 * 
 * @param state State to free (can be NULL)
 */
void neoc_name_state_free(
    neoc_name_state_t *state
);

/**
 * @brief Parse name state from JSON string
 * 
 * @param json_str JSON string containing name state data
 * @param state Output pointer for the parsed state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_name_state_from_json(
    const char *json_str,
    neoc_name_state_t **state
);

/**
 * @brief Convert name state to JSON string
 * 
 * @param state Name state to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_name_state_to_json(
    const neoc_name_state_t *state,
    char **json_str
);

/**
 * @brief Create a copy of a name state
 * 
 * @param src Source state to copy
 * @param dest Output pointer for the copied state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_name_state_copy(
    const neoc_name_state_t *src,
    neoc_name_state_t **dest
);

/**
 * @brief Compare two name states for equality
 * 
 * @param state1 First state
 * @param state2 Second state
 * @return true if states are equal, false otherwise
 */
bool neoc_name_state_equals(
    const neoc_name_state_t *state1,
    const neoc_name_state_t *state2
);

/**
 * @brief Check if a name state has expired
 * 
 * @param state Name state to check
 * @param current_time Current timestamp to compare against
 * @return true if expired, false if not expired or no expiration set
 */
bool neoc_name_state_is_expired(
    const neoc_name_state_t *state,
    int current_time
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NAME_STATE_H */
