/**
 * @file witness_scope.c
 * @brief Witness scope implementation
 */

#include "neoc/transaction/witness_scope.h"
#include <string.h>

const char* neoc_witness_scope_to_json(neoc_witness_scope_t scope) {
    switch (scope) {
        case NEOC_WITNESS_SCOPE_NONE:
            return "None";
        case NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY:
            return "CalledByEntry";
        case NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS:
            return "CustomContracts";
        case NEOC_WITNESS_SCOPE_CUSTOM_GROUPS:
            return "CustomGroups";
        case NEOC_WITNESS_SCOPE_WITNESS_RULES:
            return "WitnessRules";
        case NEOC_WITNESS_SCOPE_GLOBAL:
            return "Global";
        default:
            return "Unknown";
    }
}

neoc_error_t neoc_witness_scope_from_json(const char *json, 
                                           neoc_witness_scope_t *scope) {
    if (!json || !scope) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    if (strcmp(json, "None") == 0) {
        *scope = NEOC_WITNESS_SCOPE_NONE;
    } else if (strcmp(json, "CalledByEntry") == 0) {
        *scope = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY;
    } else if (strcmp(json, "CustomContracts") == 0) {
        *scope = NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS;
    } else if (strcmp(json, "CustomGroups") == 0) {
        *scope = NEOC_WITNESS_SCOPE_CUSTOM_GROUPS;
    } else if (strcmp(json, "WitnessRules") == 0) {
        *scope = NEOC_WITNESS_SCOPE_WITNESS_RULES;
    } else if (strcmp(json, "Global") == 0) {
        *scope = NEOC_WITNESS_SCOPE_GLOBAL;
    } else {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unknown witness scope");
    }
    
    return NEOC_SUCCESS;
}

uint8_t neoc_witness_scope_combine(const neoc_witness_scope_t *scopes, 
                                    size_t count) {
    if (!scopes || count == 0) {
        return 0;
    }
    
    uint8_t combined = 0;
    for (size_t i = 0; i < count; i++) {
        combined |= (uint8_t)scopes[i];
    }
    return combined;
}

neoc_error_t neoc_witness_scope_extract(uint8_t combined,
                                         neoc_witness_scope_t *scopes,
                                         size_t max_count,
                                         size_t *count) {
    if (!scopes || !count || max_count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    *count = 0;
    
    if (combined == NEOC_WITNESS_SCOPE_NONE) {
        if (max_count > 0) {
            scopes[0] = NEOC_WITNESS_SCOPE_NONE;
            *count = 1;
        }
        return NEOC_SUCCESS;
    }
    
    neoc_witness_scope_t all_scopes[] = {
        NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY,
        NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS,
        NEOC_WITNESS_SCOPE_CUSTOM_GROUPS,
        NEOC_WITNESS_SCOPE_WITNESS_RULES,
        NEOC_WITNESS_SCOPE_GLOBAL
    };
    
    for (size_t i = 0; i < sizeof(all_scopes)/sizeof(all_scopes[0]); i++) {
        if ((combined & all_scopes[i]) != 0) {
            if (*count >= max_count) {
                return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
            }
            scopes[(*count)++] = all_scopes[i];
        }
    }
    
    return NEOC_SUCCESS;
}

bool neoc_witness_scope_has(uint8_t combined, neoc_witness_scope_t scope) {
    return (combined & scope) != 0;
}

bool neoc_witness_scope_is_valid(neoc_witness_scope_t scope) {
    switch (scope) {
        case NEOC_WITNESS_SCOPE_NONE:
        case NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY:
        case NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS:
        case NEOC_WITNESS_SCOPE_CUSTOM_GROUPS:
        case NEOC_WITNESS_SCOPE_WITNESS_RULES:
        case NEOC_WITNESS_SCOPE_GLOBAL:
            return true;
        default:
            return false;
    }
}
