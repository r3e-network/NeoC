/**
 * @file witness_action.c
 * @brief WitnessAction implementation
 * 
 * Based on Swift source: protocol/core/witnessrule/WitnessAction.swift
 */

#include "neoc/neoc.h"
#include <string.h>

/**
 * @brief Convert witnessaction to string
 */
const char *neoc_witnessaction_to_string(neoc_witnessaction_t value) {
    switch (value) {
        case NEOC_WITNESSACTION_DENY: return "DENY";
        case NEOC_WITNESSACTION_ALLOW: return "ALLOW";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to witnessaction
 */
neoc_witnessaction_t neoc_witnessaction_from_string(const char *str) {
    if (!str) return -1;
    
    if (strcmp(str, "DENY") == 0) return NEOC_WITNESSACTION_DENY;
    if (strcmp(str, "ALLOW") == 0) return NEOC_WITNESSACTION_ALLOW;
    return -1;
}
