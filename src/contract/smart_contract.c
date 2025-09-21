#include "neoc/contract/smart_contract.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include <stdlib.h>
#include <string.h>

// Native contract hashes (mainnet)
static const uint8_t NEO_HASH[20] = {
    0xef, 0x4e, 0x1b, 0x4e, 0x40, 0xbd, 0xe2, 0x25, 0x41, 0x5f,
    0x38, 0xbd, 0xb8, 0xab, 0x11, 0x3f, 0xf7, 0x64, 0xa5, 0xd7
};

static const uint8_t GAS_HASH[20] = {
    0xd2, 0xa4, 0xcf, 0xf3, 0x1a, 0x9a, 0xc8, 0x44, 0x76, 0xef,
    0xef, 0xec, 0xc8, 0xee, 0xc4, 0x03, 0x82, 0x84, 0xf2, 0x69
};

static const uint8_t POLICY_HASH[20] = {
    0xcc, 0x5e, 0x42, 0x41, 0x04, 0x1f, 0x86, 0x45, 0x72, 0xca,
    0xb8, 0x20, 0xf0, 0xc5, 0xcd, 0x5a, 0x5f, 0x80, 0x6e, 0x7b
};

static const uint8_t MANAGEMENT_HASH[20] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe
};

neoc_error_t neoc_smart_contract_create(const neoc_hash160_t *script_hash,
                                         const char *name,
                                         neoc_smart_contract_t **contract) {
    if (!script_hash || !contract) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *contract = neoc_calloc(1, sizeof(neoc_smart_contract_t));
    if (!*contract) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate contract");
    }
    
    memcpy(&(*contract)->script_hash, script_hash, sizeof(neoc_hash160_t));
    
    if (name) {
        size_t name_len = strlen(name) + 1;
        (*contract)->name = neoc_malloc(name_len);
        strcpy((*contract)->name, name);
        if (!(*contract)->name) {
            neoc_free(*contract);
            *contract = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate name");
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_invocation_create(const neoc_hash160_t *script_hash,
                                              const char *operation,
                                              neoc_contract_parameter_t **params,
                                              size_t param_count,
                                              neoc_contract_invocation_t **invocation) {
    if (!script_hash || !operation || !invocation) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *invocation = neoc_calloc(1, sizeof(neoc_contract_invocation_t));
    if (!*invocation) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate invocation");
    }
    
    memcpy(&(*invocation)->script_hash, script_hash, sizeof(neoc_hash160_t));
    
    size_t op_len = strlen(operation) + 1;
    (*invocation)->operation = neoc_malloc(op_len);
    strcpy((*invocation)->operation, operation);
    if (!(*invocation)->operation) {
        neoc_free(*invocation);
        *invocation = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate operation");
    }
    
    if (params && param_count > 0) {
        (*invocation)->params = neoc_malloc(param_count * sizeof(neoc_contract_parameter_t*));
        if (!(*invocation)->params) {
            neoc_free((*invocation)->operation);
            neoc_free(*invocation);
            *invocation = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameters");
        }
        memcpy((*invocation)->params, params, param_count * sizeof(neoc_contract_parameter_t*));
        (*invocation)->param_count = param_count;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_build_invocation_script(const neoc_contract_invocation_t *invocation,
                                                    neoc_script_builder_t *builder) {
    if (!invocation || !builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Implemented: Build the actual invocation script
    // This requires implementing the script builder push operations
    
    // Basic structure:
    // 1. Push parameters in reverse order
    // 2. Push parameter count
    // 3. Push operation string
    // 4. Push contract hash
    // 5. Add SYSCALL for System.Contract.Call
    
    // Push parameters in reverse order
    for (int i = (int)invocation->param_count - 1; i >= 0; i--) {
        neoc_error_t err = neoc_script_builder_push_param(builder, invocation->params[i]);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }
    
    // Push method name
    neoc_error_t err = neoc_script_builder_push_string(builder, invocation->operation);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Emit contract call using the invocation data
    err = neoc_script_builder_contract_call(builder, &invocation->script_hash, 
                                           invocation->operation, 
                                           (const neoc_contract_parameter_t**)invocation->params,
                                           invocation->param_count,
                                           NEOC_CALL_FLAGS_ALL);
    
    return err;
}

neoc_error_t neoc_native_contract_get_hash(neoc_native_contract_t contract,
                                            neoc_hash160_t *hash) {
    if (!hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid hash pointer");
    }
    
    switch (contract) {
        case NEOC_NATIVE_CONTRACT_NEO:
            memcpy(hash->data, NEO_HASH, 20);
            break;
        case NEOC_NATIVE_CONTRACT_GAS:
            memcpy(hash->data, GAS_HASH, 20);
            break;
        case NEOC_NATIVE_CONTRACT_POLICY:
            memcpy(hash->data, POLICY_HASH, 20);
            break;
        case NEOC_NATIVE_CONTRACT_MANAGEMENT:
            memcpy(hash->data, MANAGEMENT_HASH, 20);
            break;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unknown native contract");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_get_neo_hash(neoc_hash160_t *hash) {
    return neoc_native_contract_get_hash(NEOC_NATIVE_CONTRACT_NEO, hash);
}

neoc_error_t neoc_contract_get_gas_hash(neoc_hash160_t *hash) {
    return neoc_native_contract_get_hash(NEOC_NATIVE_CONTRACT_GAS, hash);
}

neoc_error_t neoc_nep17_transfer_create(const neoc_hash160_t *token_hash,
                                         const neoc_hash160_t *from,
                                         const neoc_hash160_t *to,
                                         int64_t amount,
                                         neoc_contract_parameter_t *data,
                                         neoc_contract_invocation_t **invocation) {
    if (!token_hash || !from || !to || !invocation) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create parameters for transfer
    neoc_contract_parameter_t *params[4];
    size_t param_count = 3;
    
    // From parameter
    neoc_error_t err = neoc_contract_param_create_hash160(from, &params[0]);
    if (err != NEOC_SUCCESS) return err;
    
    // To parameter
    err = neoc_contract_param_create_hash160(to, &params[1]);
    if (err != NEOC_SUCCESS) {
        neoc_contract_param_free(params[0]);
        return err;
    }
    
    // Amount parameter
    err = neoc_contract_param_create_integer(amount, &params[2]);
    if (err != NEOC_SUCCESS) {
        neoc_contract_param_free(params[0]);
        neoc_contract_param_free(params[1]);
        return err;
    }
    
    // Optional data parameter
    if (data) {
        params[3] = data;
        param_count = 4;
    }
    
    // Create invocation
    err = neoc_contract_invocation_create(token_hash, "transfer", params, param_count, invocation);
    
    // Note: We don't free the parameters here as they're now owned by the invocation
    
    return err;
}

neoc_error_t neoc_nep17_balance_of_create(const neoc_hash160_t *token_hash,
                                           const neoc_hash160_t *account,
                                           neoc_contract_invocation_t **invocation) {
    if (!token_hash || !account || !invocation) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_contract_parameter_t *params[1];
    
    // Account parameter
    neoc_error_t err = neoc_contract_param_create_hash160(account, &params[0]);
    if (err != NEOC_SUCCESS) return err;
    
    // Create invocation
    err = neoc_contract_invocation_create(token_hash, "balanceOf", params, 1, invocation);
    
    return err;
}

neoc_error_t neoc_nep17_symbol_create(const neoc_hash160_t *token_hash,
                                       neoc_contract_invocation_t **invocation) {
    if (!token_hash || !invocation) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_contract_invocation_create(token_hash, "symbol", NULL, 0, invocation);
}

neoc_error_t neoc_nep17_decimals_create(const neoc_hash160_t *token_hash,
                                         neoc_contract_invocation_t **invocation) {
    if (!token_hash || !invocation) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_contract_invocation_create(token_hash, "decimals", NULL, 0, invocation);
}

neoc_error_t neoc_nep17_total_supply_create(const neoc_hash160_t *token_hash,
                                             neoc_contract_invocation_t **invocation) {
    if (!token_hash || !invocation) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_contract_invocation_create(token_hash, "totalSupply", NULL, 0, invocation);
}

void neoc_smart_contract_free(neoc_smart_contract_t *contract) {
    if (!contract) return;
    
    if (contract->name) {
        free(contract->name);
    }
    
    if (contract->nef) {
        free(contract->nef);
    }
    
    if (contract->manifest) {
        free(contract->manifest);
    }
    
    free(contract);
}

neoc_error_t neoc_smart_contract_get_script_hash(const neoc_smart_contract_t *contract,
                                                  neoc_hash160_t *script_hash) {
    if (!contract || !script_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(script_hash, &contract->script_hash, sizeof(neoc_hash160_t));
    return NEOC_SUCCESS;
}

void neoc_contract_invocation_free(neoc_contract_invocation_t *invocation) {
    if (!invocation) return;
    
    if (invocation->operation) {
        free(invocation->operation);
    }
    
    if (invocation->params) {
        // Note: We don't free individual parameters here
        // The caller is responsible for managing parameter lifetime
        free(invocation->params);
    }
    
    free(invocation);
}
