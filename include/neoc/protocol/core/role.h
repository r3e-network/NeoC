/**
 * @file role.h
 * @brief Neo node role definitions
 * 
 * Based on Swift source: protocol/core/Role.swift
 * Defines the different roles a node can have in the Neo network
 */

#ifndef NEOC_PROTOCOL_CORE_ROLE_H
#define NEOC_PROTOCOL_CORE_ROLE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neo node role enumeration
 * 
 * Defines the different roles a node can have in the Neo network
 */
typedef enum {
    NEOC_ROLE_STATE_VALIDATOR = 0x04,       /**< State validator node */
    NEOC_ROLE_ORACLE = 0x08,                /**< Oracle node */
    NEOC_ROLE_NEOFS_ALPHABET_NODE = 0x10    /**< NeoFS alphabet node */
} neoc_role_t;

/**
 * @brief Get the byte value of a role
 * 
 * @param role Node role
 * @return Byte value
 */
uint8_t neoc_role_get_byte(
    neoc_role_t role
);

/**
 * @brief Get role from byte value
 * 
 * @param byte Byte value
 * @param role Pointer to store the role
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_role_from_byte(
    uint8_t byte,
    neoc_role_t *role
);

/**
 * @brief Get JSON string representation of role
 * 
 * @param role Node role
 * @return JSON string representation
 */
const char* neoc_role_to_json_string(
    neoc_role_t role
);

/**
 * @brief Parse role from JSON string
 * 
 * @param json_str JSON string
 * @param role Pointer to store the parsed role
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_role_from_json_string(
    const char *json_str,
    neoc_role_t *role
);

/**
 * @brief Check if role is valid
 * 
 * @param role Role to validate
 * @return true if valid, false otherwise
 */
bool neoc_role_is_valid(
    neoc_role_t role
);

/**
 * @brief Check if role has specific capabilities
 * 
 * @param role Role to check
 * @param capability Capability to check for
 * @return true if role has capability, false otherwise
 */
bool neoc_role_has_capability(
    neoc_role_t role,
    uint8_t capability
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_ROLE_H */
