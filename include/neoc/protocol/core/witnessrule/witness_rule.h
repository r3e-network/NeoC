/**
 * @file witness_rule.h
 * @brief Witness rule combining action and condition
 * 
 * Based on Swift source: protocol/core/witnessrule/WitnessRule.swift
 * Combines witness actions and conditions for transaction validation
 */

#ifndef NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_RULE_H
#define NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_RULE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/core/witnessrule/witness_action.h"
#include "neoc/protocol/core/witnessrule/witness_condition.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Witness rule structure
 * 
 * Combines an action and condition for witness validation
 */
typedef struct {
    neoc_witness_action_t action;       /**< Action to take when condition is met */
    neoc_witness_condition_t *condition; /**< Condition to evaluate */
} neoc_witness_rule_t;

/**
 * @brief Create a new witness rule
 * 
 * @param action Action to take
 * @param condition Condition to evaluate (takes ownership)
 * @param rule Pointer to store the created rule
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_rule_create(
    neoc_witness_action_t action,
    neoc_witness_condition_t *condition,
    neoc_witness_rule_t **rule
);

/**
 * @brief Free a witness rule and its resources
 * 
 * @param rule Rule to free
 */
void neoc_witness_rule_free(
    neoc_witness_rule_t *rule
);

/**
 * @brief Clone a witness rule
 * 
 * @param src Source rule
 * @param dest Pointer to store the cloned rule
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_rule_clone(
    const neoc_witness_rule_t *src,
    neoc_witness_rule_t **dest
);

/**
 * @brief Get the action from a witness rule
 * 
 * @param rule Witness rule
 * @return Witness action
 */
neoc_witness_action_t neoc_witness_rule_get_action(
    const neoc_witness_rule_t *rule
);

/**
 * @brief Get the condition from a witness rule
 * 
 * @param rule Witness rule
 * @return Witness condition (do not free)
 */
const neoc_witness_condition_t* neoc_witness_rule_get_condition(
    const neoc_witness_rule_t *rule
);

/**
 * @brief Calculate the serialized size of a witness rule
 * 
 * @param rule Witness rule
 * @return Size in bytes (1 + condition size)
 */
size_t neoc_witness_rule_get_size(
    const neoc_witness_rule_t *rule
);

/**
 * @brief Serialize a witness rule to bytes
 * 
 * @param rule Witness rule to serialize
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @param bytes_written Pointer to store bytes written
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_rule_serialize(
    const neoc_witness_rule_t *rule,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *bytes_written
);

/**
 * @brief Deserialize a witness rule from bytes
 * 
 * @param buffer Input buffer
 * @param buffer_size Buffer size
 * @param rule Pointer to store the deserialized rule
 * @param bytes_read Pointer to store bytes read
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_rule_deserialize(
    const uint8_t *buffer,
    size_t buffer_size,
    neoc_witness_rule_t **rule,
    size_t *bytes_read
);

/**
 * @brief Parse JSON into witness rule
 * 
 * @param json_str JSON string to parse
 * @param rule Pointer to store the parsed rule
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_rule_from_json(
    const char *json_str,
    neoc_witness_rule_t **rule
);

/**
 * @brief Convert witness rule to JSON string
 * 
 * @param rule Rule to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_rule_to_json(
    const neoc_witness_rule_t *rule,
    char **json_str
);

/**
 * @brief Compare two witness rules for equality
 * 
 * @param rule1 First rule
 * @param rule2 Second rule
 * @return true if rules are equal, false otherwise
 */
bool neoc_witness_rule_equals(
    const neoc_witness_rule_t *rule1,
    const neoc_witness_rule_t *rule2
);

/**
 * @brief Validate a witness rule
 * 
 * @param rule Rule to validate
 * @return NEOC_SUCCESS if valid, error code if invalid
 */
neoc_error_t neoc_witness_rule_validate(
    const neoc_witness_rule_t *rule
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_RULE_H */
