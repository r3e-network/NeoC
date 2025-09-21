/**
 * @file role.c
 * @brief Role implementation
 * 
 * Based on Swift source: protocol/core/Role.swift
 */

#include "neoc/neoc.h"
#include <string.h>

/**
 * @brief Convert role to string
 */
const char *neoc_role_to_string(neoc_role_t value) {
    switch (value) {
        case NEOC_ROLE_STATE_VALIDATOR: return "STATE_VALIDATOR";
        case NEOC_ROLE_ORACLE: return "ORACLE";
        case NEOC_ROLE_NEO_FS_ALPHABET_NODE: return "NEO_FS_ALPHABET_NODE";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to role
 */
neoc_role_t neoc_role_from_string(const char *str) {
    if (!str) return -1;
    
    if (strcmp(str, "STATE_VALIDATOR") == 0) return NEOC_ROLE_STATE_VALIDATOR;
    if (strcmp(str, "ORACLE") == 0) return NEOC_ROLE_ORACLE;
    if (strcmp(str, "NEO_FS_ALPHABET_NODE") == 0) return NEOC_ROLE_NEO_FS_ALPHABET_NODE;
    return -1;
}
