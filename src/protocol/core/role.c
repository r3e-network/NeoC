/**
 * @file role.c
 * @brief Role implementation
 *
 * Converted from protocol/core/Role.swift
 */

#include "neoc/protocol/core/role.h"

#include <string.h>

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

static const char *neoc_role_name(neoc_role_t role) {
    switch (role) {
        case NEOC_ROLE_STATE_VALIDATOR:
            return "STATE_VALIDATOR";
        case NEOC_ROLE_ORACLE:
            return "ORACLE";
        case NEOC_ROLE_NEOFS_ALPHABET_NODE:
            return "NEO_FS_ALPHABET_NODE";
        default:
            return "UNKNOWN";
    }
}

static bool neoc_role_decode(const char *str, neoc_role_t *role) {
    if (!str || !role) {
        return false;
    }

    if (strcmp(str, "STATE_VALIDATOR") == 0) {
        *role = NEOC_ROLE_STATE_VALIDATOR;
        return true;
    }
    if (strcmp(str, "ORACLE") == 0) {
        *role = NEOC_ROLE_ORACLE;
        return true;
    }
    if (strcmp(str, "NEO_FS_ALPHABET_NODE") == 0) {
        *role = NEOC_ROLE_NEOFS_ALPHABET_NODE;
        return true;
    }
    return false;
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

uint8_t neoc_role_get_byte(neoc_role_t role) {
    return (uint8_t)role;
}

neoc_error_t neoc_role_from_byte(uint8_t byte, neoc_role_t *role) {
    if (!role) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Role pointer is NULL");
    }

    switch (byte) {
        case NEOC_ROLE_STATE_VALIDATOR:
        case NEOC_ROLE_ORACLE:
        case NEOC_ROLE_NEOFS_ALPHABET_NODE:
            *role = (neoc_role_t)byte;
            return NEOC_SUCCESS;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unknown role byte");
    }
}

const char* neoc_role_to_json_string(neoc_role_t role) {
    return neoc_role_name(role);
}

neoc_error_t neoc_role_from_json_string(const char *json_str, neoc_role_t *role) {
    if (!role) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Role output pointer is NULL");
    }
    if (!json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Role JSON string is NULL");
    }

    if (neoc_role_decode(json_str, role)) {
        return NEOC_SUCCESS;
    }
    return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unknown role string");
}

bool neoc_role_is_valid(neoc_role_t role) {
    return role == NEOC_ROLE_STATE_VALIDATOR ||
           role == NEOC_ROLE_ORACLE ||
           role == NEOC_ROLE_NEOFS_ALPHABET_NODE;
}

bool neoc_role_has_capability(neoc_role_t role, uint8_t capability) {
    return (role & capability) == capability;
}
