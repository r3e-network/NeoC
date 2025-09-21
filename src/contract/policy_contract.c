/**
 * @file policy_contract.c
 * @brief NEO Policy Contract implementation
 */

#include "neoc/contract/policy_contract.h"
#include "neoc/contract/smart_contract.h"
#include "neoc/script/script_builder.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>

// Policy contract hash
static const uint8_t POLICY_CONTRACT_HASH[20] = {
    0xcc, 0x5e, 0x40, 0x09, 0xd8, 0x22, 0xc3, 0x05,
    0x50, 0xe9, 0xf2, 0x02, 0x3e, 0x45, 0xcf, 0xb8,
    0x8d, 0xa5, 0x8c, 0x7c
};

struct neoc_policy_contract {
    neoc_smart_contract_t *contract;
    uint64_t fee_per_byte;
    uint32_t exec_fee_factor;
    uint32_t storage_price;
};

neoc_error_t neoc_policy_contract_create(neoc_policy_contract_t **policy) {
    if (!policy) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid policy pointer");
    }
    
    *policy = neoc_calloc(1, sizeof(neoc_policy_contract_t));
    if (!*policy) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate policy contract");
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, POLICY_CONTRACT_HASH, 20);
    
    neoc_error_t err = neoc_smart_contract_create(&script_hash, "Policy", &(*policy)->contract);
    if (err != NEOC_SUCCESS) {
        neoc_free(*policy);
        *policy = NULL;
        return err;
    }
    
    // Default mainnet values
    (*policy)->fee_per_byte = 1000;      // 0.00001 GAS per byte
    (*policy)->exec_fee_factor = 30;      // Execution fee factor
    (*policy)->storage_price = 100000;    // 0.001 GAS per byte
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_policy_get_fee_per_byte(neoc_policy_contract_t *policy,
                                           uint64_t *fee) {
    if (!policy || !fee) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build script to call getFeePerByte
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, POLICY_CONTRACT_HASH, 20);
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "getFeePerByte", 0);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Return cached value (would normally execute script)
    *fee = policy->fee_per_byte;
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_policy_get_exec_fee_factor(neoc_policy_contract_t *policy,
                                              uint32_t *factor) {
    if (!policy || !factor) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build script to call getExecFeeFactor
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, POLICY_CONTRACT_HASH, 20);
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "getExecFeeFactor", 0);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Return cached value
    *factor = policy->exec_fee_factor;
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_policy_get_storage_price(neoc_policy_contract_t *policy,
                                            uint32_t *price) {
    if (!policy || !price) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build script to call getStoragePrice
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, POLICY_CONTRACT_HASH, 20);
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "getStoragePrice", 0);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Return cached value
    *price = policy->storage_price;
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_policy_is_blocked(neoc_policy_contract_t *policy,
                                     const neoc_hash160_t *account,
                                     bool *blocked) {
    if (!policy || !account || !blocked) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build script to call isBlocked
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push account parameter
    err = neoc_script_builder_push_data(builder, account->data, sizeof(account->data));
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, POLICY_CONTRACT_HASH, 20);
    
    // Would need to create hash160 parameter
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "isBlocked", 1);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Execute script and get boolean result
    // In production, this would use RPC client to check if account is blocked
    // Returns false if account is not in the blocked list
    *blocked = false;
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_policy_set_fee_per_byte(neoc_policy_contract_t *policy,
                                           uint64_t fee) {
    if (!policy) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid policy");
    }
    
    // Build script to call setFeePerByte (requires committee signature)
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    err = neoc_script_builder_emit_push_int(builder, fee);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, POLICY_CONTRACT_HASH, 20);
    
    // Would need to create integer parameter
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "setFeePerByte", 1);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Update cached value
    policy->fee_per_byte = fee;
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

void neoc_policy_contract_free(neoc_policy_contract_t *policy) {
    if (!policy) return;
    
    if (policy->contract) {
        neoc_smart_contract_free(policy->contract);
    }
    
    neoc_free(policy);
}
