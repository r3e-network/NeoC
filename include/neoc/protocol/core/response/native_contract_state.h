/**
 * @file native_contract_state.h
 * @brief Native contract state structure
 * 
 * Converted from Swift source: protocol/core/response/NativeContractState.swift
 * This header provides access to native contract state types defined in contract_response_types.h
 * and adds Swift-compatible functions.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NATIVE_CONTRACT_STATE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NATIVE_CONTRACT_STATE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/protocol/contract_response_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Note: neoc_native_contract_state_t and neoc_contract_state_t are defined in contract_response_types.h */

/**
 * @brief Create a new native contract state (Swift-compatible)
 * 
 * Extended version with Swift parameter mapping.
 * 
 * @param id Native contract ID
 * @param hash Contract hash
 * @param manifest Contract manifest
 * @param nef Contract NEF
 * @param update_history Array of update block heights (can be NULL)
 * @param update_history_count Number of updates in history
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_native_contract_state_create_swift(
    int id,
    const neoc_hash160_t *hash,
    const neoc_contract_manifest_t *manifest,
    const neoc_contract_nef_t *nef,
    const int *update_history,
    size_t update_history_count,
    neoc_native_contract_state_t **state
);

/**
 * @brief Parse native contract state from JSON string (Swift-compatible)
 * 
 * @param json_str JSON string containing contract state data
 * @param state Output pointer for the parsed state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_native_contract_state_from_json_swift(
    const char *json_str,
    neoc_native_contract_state_t **state
);

/**
 * @brief Convert native contract state to JSON string (Swift-compatible)
 * 
 * @param state Contract state to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_native_contract_state_to_json_swift(
    const neoc_native_contract_state_t *state,
    char **json_str
);

/**
 * @brief Create a copy of a native contract state (Swift-compatible)
 * 
 * @param src Source state to copy
 * @param dest Output pointer for the copied state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_native_contract_state_copy_swift(
    const neoc_native_contract_state_t *src,
    neoc_native_contract_state_t **dest
);

/**
 * @brief Compare two native contract states for equality (Swift-compatible)
 * 
 * @param state1 First state
 * @param state2 Second state
 * @return true if states are equal, false otherwise
 */
bool neoc_native_contract_state_equals_swift(
    const neoc_native_contract_state_t *state1,
    const neoc_native_contract_state_t *state2
);

/**
 * @brief Get the latest update block height from history
 * 
 * @param state Native contract state
 * @param latest_update Output pointer for latest update height
 * @return NEOC_SUCCESS if update history exists, error code otherwise
 */
neoc_error_t neoc_native_contract_state_get_latest_update(
    const neoc_native_contract_state_t *state,
    int *latest_update
);

/**
 * @brief Check if native contract is active in a specific network version
 * 
 * @param state Native contract state
 * @param version Network version to check
 * @return true if active, false otherwise
 */
bool neoc_native_contract_state_is_active_in_version(
    const neoc_native_contract_state_t *state,
    const char *version
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NATIVE_CONTRACT_STATE_H */
