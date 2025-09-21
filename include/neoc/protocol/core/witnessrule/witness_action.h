/**
 * @file witness_action.h
 * @brief Witness action enumeration for witness rules
 * 
 * Based on Swift source: protocol/core/witnessrule/WitnessAction.swift
 * Defines the actions that can be taken when witness conditions are met
 */

#ifndef NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_ACTION_H
#define NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_ACTION_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Witness action enumeration
 * 
 * Defines the action to take when witness rule conditions are met
 */
typedef enum {
    NEOC_WITNESS_ACTION_DENY = 0,       /**< Deny the witness */
    NEOC_WITNESS_ACTION_ALLOW = 1       /**< Allow the witness */
} neoc_witness_action_t;

/**
 * @brief Get the byte value of a witness action
 * 
 * @param action Witness action
 * @return Byte value (0 for deny, 1 for allow)
 */
uint8_t neoc_witness_action_get_byte(
    neoc_witness_action_t action
);

/**
 * @brief Get witness action from byte value
 * 
 * @param byte Byte value
 * @param action Pointer to store the action
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_action_from_byte(
    uint8_t byte,
    neoc_witness_action_t *action
);

/**
 * @brief Get JSON string representation of witness action
 * 
 * @param action Witness action
 * @return JSON string representation ("Deny" or "Allow")
 */
const char* neoc_witness_action_to_json_string(
    neoc_witness_action_t action
);

/**
 * @brief Parse witness action from JSON string
 * 
 * @param json_str JSON string ("Deny" or "Allow")
 * @param action Pointer to store the parsed action
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_action_from_json_string(
    const char *json_str,
    neoc_witness_action_t *action
);

/**
 * @brief Check if witness action is valid
 * 
 * @param action Witness action to validate
 * @return true if valid, false otherwise
 */
bool neoc_witness_action_is_valid(
    neoc_witness_action_t action
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_ACTION_H */
