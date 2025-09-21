/**
 * @file contract_error.c
 * @brief Implementation of contract error handling
 */

#include "neoc/contract/contract_error.h"
#include <string.h>
#include <stdio.h>
#include <regex.h>

neoc_error_t neoc_contract_error_get_description(
    neoc_contract_error_t error_code,
    const neoc_contract_error_context_t* context,
    char* buffer,
    size_t buffer_size
) {
    if (!buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    switch (error_code) {
        case NEOC_CONTRACT_ERROR_INVALID_NEO_NAME:
            if (context && context->name[0] != '\0') {
                snprintf(buffer, buffer_size, "'%s' is not a valid NNS name.", context->name);
            } else {
                snprintf(buffer, buffer_size, "Invalid NNS name format.");
            }
            break;
            
        case NEOC_CONTRACT_ERROR_INVALID_NNS_ROOT:
            if (context && context->name[0] != '\0') {
                snprintf(buffer, buffer_size, "'%s' is not a valid NNS root.", context->name);
            } else {
                snprintf(buffer, buffer_size, "Invalid NNS root format.");
            }
            break;
            
        case NEOC_CONTRACT_ERROR_UNEXPECTED_RETURN_TYPE:
            if (context) {
                if (context->expected_types[0] != '\0') {
                    snprintf(buffer, buffer_size, 
                        "Got stack item of type %s but expected %s.", 
                        context->actual_type, context->expected_types);
                } else {
                    snprintf(buffer, buffer_size, "Unexpected return type: %s", context->actual_type);
                }
            } else {
                snprintf(buffer, buffer_size, "Unexpected stack item return type.");
            }
            break;
            
        case NEOC_CONTRACT_ERROR_UNRESOLVABLE_DOMAIN:
            if (context && context->name[0] != '\0') {
                snprintf(buffer, buffer_size, 
                    "The provided domain name '%s' could not be resolved.", context->name);
            } else {
                snprintf(buffer, buffer_size, "Domain name could not be resolved.");
            }
            break;
            
        case NEOC_CONTRACT_ERROR_INVALID_MANIFEST:
            snprintf(buffer, buffer_size, "Invalid contract manifest format.");
            break;
            
        case NEOC_CONTRACT_ERROR_INVALID_NEF:
            snprintf(buffer, buffer_size, "Invalid NEF file format.");
            break;
            
        case NEOC_CONTRACT_ERROR_INSUFFICIENT_FUNDS:
            snprintf(buffer, buffer_size, "Insufficient funds for contract operation.");
            break;
            
        case NEOC_CONTRACT_ERROR_INVALID_TOKEN_AMOUNT:
            snprintf(buffer, buffer_size, "Invalid token amount or decimal places.");
            break;
            
        default:
            snprintf(buffer, buffer_size, "Unknown contract error: %d", error_code);
            return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return NEOC_SUCCESS;
}

bool neoc_contract_is_valid_nns_name(const char* name) {
    if (!name || strlen(name) == 0) {
        return false;
    }
    
    // Basic NNS name validation - must contain only alphanumeric, dots, and hyphens
    // Cannot start or end with dot or hyphen
    size_t len = strlen(name);
    if (len > 253) { // Domain name too long
        return false;
    }
    
    if (name[0] == '.' || name[0] == '-' || name[len-1] == '.' || name[len-1] == '-') {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || 
              (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || 
              c == '.' || c == '-')) {
            return false;
        }
        
        // No consecutive dots or hyphens
        if (i > 0 && ((c == '.' && name[i-1] == '.') || 
                      (c == '-' && name[i-1] == '-'))) {
            return false;
        }
    }
    
    return true;
}

bool neoc_contract_is_valid_nns_root(const char* root) {
    if (!root || strlen(root) == 0) {
        return false;
    }
    
    // NNS root domains are typically single labels without dots
    // Common roots: neo, eth, com, org, etc.
    size_t len = strlen(root);
    if (len > 63) { // Single label too long
        return false;
    }
    
    // Cannot contain dots (root domains are single labels)
    if (strchr(root, '.') != NULL) {
        return false;
    }
    
    // Must start and end with alphanumeric
    if (!((root[0] >= 'a' && root[0] <= 'z') || 
          (root[0] >= 'A' && root[0] <= 'Z') || 
          (root[0] >= '0' && root[0] <= '9')) ||
        !((root[len-1] >= 'a' && root[len-1] <= 'z') || 
          (root[len-1] >= 'A' && root[len-1] <= 'Z') || 
          (root[len-1] >= '0' && root[len-1] <= '9'))) {
        return false;
    }
    
    // Check all characters are valid
    for (size_t i = 0; i < len; i++) {
        char c = root[i];
        if (!((c >= 'a' && c <= 'z') || 
              (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || 
              c == '-')) {
            return false;
        }
    }
    
    return true;
}
