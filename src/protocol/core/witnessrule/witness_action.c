/**
 * @file witness_action.c
 * @brief Implementation of witness action helpers.
 *
 * Mirrors NeoSwift's WitnessAction enum semantics.
 */

#include "neoc/protocol/core/witnessrule/witness_action.h"
#include "neoc/neoc_error.h"
#include <string.h>

static const char *NEOC_WITNESS_ACTION_JSON_DENY = "Deny";
static const char *NEOC_WITNESS_ACTION_JSON_ALLOW = "Allow";

uint8_t neoc_witness_action_get_byte(neoc_witness_action_t action) {
    return (action == NEOC_WITNESS_ACTION_ALLOW) ? 1u : 0u;
}

neoc_error_t neoc_witness_action_from_byte(uint8_t byte,
                                           neoc_witness_action_t *action) {
    if (!action) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "action pointer is NULL");
    }

    switch (byte) {
        case 0:
            *action = NEOC_WITNESS_ACTION_DENY;
            return NEOC_SUCCESS;
        case 1:
            *action = NEOC_WITNESS_ACTION_ALLOW;
            return NEOC_SUCCESS;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                                  "Invalid witness action byte");
    }
}

const char *neoc_witness_action_to_json_string(neoc_witness_action_t action) {
    switch (action) {
        case NEOC_WITNESS_ACTION_DENY:
            return NEOC_WITNESS_ACTION_JSON_DENY;
        case NEOC_WITNESS_ACTION_ALLOW:
            return NEOC_WITNESS_ACTION_JSON_ALLOW;
        default:
            return "Unknown";
    }
}

neoc_error_t neoc_witness_action_from_json_string(
    const char *json_str,
    neoc_witness_action_t *action) {
    if (!json_str || !action) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid arguments");
    }

    if (strcmp(json_str, NEOC_WITNESS_ACTION_JSON_DENY) == 0) {
        *action = NEOC_WITNESS_ACTION_DENY;
        return NEOC_SUCCESS;
    }

    if (strcmp(json_str, NEOC_WITNESS_ACTION_JSON_ALLOW) == 0) {
        *action = NEOC_WITNESS_ACTION_ALLOW;
        return NEOC_SUCCESS;
    }

    return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                          "Invalid witness action JSON string");
}

bool neoc_witness_action_is_valid(neoc_witness_action_t action) {
    return action == NEOC_WITNESS_ACTION_DENY ||
           action == NEOC_WITNESS_ACTION_ALLOW;
}
