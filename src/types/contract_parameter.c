/**
 * @file contract_parameter.c
 * @brief Contract parameter implementation
 */

#include "neoc/types/contract_parameter.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>

neoc_error_t neoc_contract_parameter_create(neoc_contract_parameter_type_t type,
                                            const char *name,
                                            const void *value,
                                            size_t value_size,
                                            neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = neoc_calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate contract parameter");
    }
    
    (*param)->type = type;
    
    // Copy name if provided
    if (name) {
        size_t name_len = strlen(name) + 1;
        (*param)->name = neoc_malloc(name_len);
        if (!(*param)->name) {
            neoc_free(*param);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter name");
        }
        strcpy((*param)->name, name);
    }
    
    // Copy value if provided
    if (value && value_size > 0) {
        (*param)->value = neoc_malloc(value_size);
        if (!(*param)->value) {
            neoc_free((*param)->name);
            neoc_free(*param);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter value");
        }
        memcpy((*param)->value, value, value_size);
        (*param)->value_size = value_size;
    }
    
    return NEOC_SUCCESS;
}

void neoc_contract_parameter_free(neoc_contract_parameter_t *param) {
    if (!param) return;
    
    neoc_free(param->name);
    neoc_free(param->value);
    neoc_free(param);
}

// Helper function to create boolean parameter
neoc_error_t neoc_contract_parameter_create_bool(bool value, neoc_contract_parameter_t **param) {
    uint8_t bool_val = value ? 1 : 0;
    return neoc_contract_parameter_create(NEOC_CONTRACT_PARAM_TYPE_BOOLEAN, 
                                          NULL, &bool_val, sizeof(bool_val), param);
}

// Helper function to create integer parameter
neoc_error_t neoc_contract_parameter_create_int(int64_t value, neoc_contract_parameter_t **param) {
    return neoc_contract_parameter_create(NEOC_CONTRACT_PARAM_TYPE_INTEGER,
                                          NULL, &value, sizeof(value), param);
}

// Helper function to create string parameter
neoc_error_t neoc_contract_parameter_create_string(const char *value, neoc_contract_parameter_t **param) {
    if (!value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid string value");
    }
    return neoc_contract_parameter_create(NEOC_CONTRACT_PARAM_TYPE_STRING,
                                          NULL, value, strlen(value), param);
}

// Helper function to create byte array parameter
neoc_error_t neoc_contract_parameter_create_bytes(const uint8_t *value, size_t len, 
                                                  neoc_contract_parameter_t **param) {
    if (!value && len > 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid byte array");
    }
    return neoc_contract_parameter_create(NEOC_CONTRACT_PARAM_TYPE_BYTE_ARRAY,
                                          NULL, value, len, param);
}
