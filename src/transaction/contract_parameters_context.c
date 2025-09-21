/**
 * @file contract_parameters_context.c
 * @brief Contract parameters context implementation
 */

#include "neoc/transaction/contract_parameters_context.h"
#include "neoc/transaction/neo_transaction.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>

struct neoc_contract_parameters_context {
    neoc_transaction_t *transaction;
    neoc_hash160_t *script_hashes;
    size_t script_hash_count;
    neoc_contract_parameter_t ***parameters;  // Array of parameter arrays
    size_t *parameter_counts;
    bool *completed;
};

neoc_error_t neoc_contract_parameters_context_create(neoc_transaction_t *tx,
                                                      neoc_contract_parameters_context_t **ctx) {
    if (!tx || !ctx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *ctx = neoc_calloc(1, sizeof(neoc_contract_parameters_context_t));
    if (!*ctx) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate context");
    }
    
    (*ctx)->transaction = tx;
    
    // Get script hashes from transaction signers
    neoc_signer_t **signers;
    size_t signer_count;
    neoc_error_t err = neoc_transaction_get_signers(tx, &signers, &signer_count);
    if (err != NEOC_SUCCESS) {
        neoc_free(*ctx);
        *ctx = NULL;
        return err;
    }
    
    (*ctx)->script_hash_count = signer_count;
    (*ctx)->script_hashes = neoc_calloc(signer_count, sizeof(neoc_hash160_t));
    (*ctx)->parameters = neoc_calloc(signer_count, sizeof(neoc_contract_parameter_t**));
    (*ctx)->parameter_counts = neoc_calloc(signer_count, sizeof(size_t));
    (*ctx)->completed = neoc_calloc(signer_count, sizeof(bool));
    
    if (!(*ctx)->script_hashes || !(*ctx)->parameters || 
        !(*ctx)->parameter_counts || !(*ctx)->completed) {
        neoc_contract_parameters_context_free(*ctx);
        *ctx = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate arrays");
    }
    
    for (size_t i = 0; i < signer_count; i++) {
        err = neoc_signer_get_account(signers[i], &(*ctx)->script_hashes[i]);
        if (err != NEOC_SUCCESS) {
            neoc_contract_parameters_context_free(*ctx);
            *ctx = NULL;
            return err;
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_parameters_context_add_signature(neoc_contract_parameters_context_t *ctx,
                                                             const neoc_hash160_t *script_hash,
                                                             const char *signature) {
    if (!ctx || !script_hash || !signature) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Find script hash index
    size_t index = SIZE_MAX;
    for (size_t i = 0; i < ctx->script_hash_count; i++) {
        if (memcmp(&ctx->script_hashes[i], script_hash, sizeof(neoc_hash160_t)) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == SIZE_MAX) {
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Script hash not found");
    }
    
    // Create signature parameter
    neoc_contract_parameter_t *sig_param;
    neoc_error_t err = neoc_contract_parameter_create(NEOC_CONTRACT_PARAM_TYPE_SIGNATURE,
                                                       "signature", signature, strlen(signature),
                                                       &sig_param);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Add to parameters
    neoc_contract_parameter_t **new_params = neoc_realloc(ctx->parameters[index],
                                                           (ctx->parameter_counts[index] + 1) * 
                                                           sizeof(neoc_contract_parameter_t*));
    if (!new_params) {
        neoc_contract_parameter_free(sig_param);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameters");
    }
    
    ctx->parameters[index] = new_params;
    ctx->parameters[index][ctx->parameter_counts[index]++] = sig_param;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_parameters_context_is_complete(const neoc_contract_parameters_context_t *ctx,
                                                           bool *complete) {
    if (!ctx || !complete) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *complete = true;
    for (size_t i = 0; i < ctx->script_hash_count; i++) {
        if (!ctx->completed[i]) {
            *complete = false;
            break;
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_parameters_context_get_witnesses(const neoc_contract_parameters_context_t *ctx,
                                                             neoc_witness_t ***witnesses,
                                                             size_t *count) {
    if (!ctx || !witnesses || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *witnesses = neoc_calloc(ctx->script_hash_count, sizeof(neoc_witness_t*));
    if (!*witnesses) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witnesses");
    }
    
    for (size_t i = 0; i < ctx->script_hash_count; i++) {
        // Build invocation script from parameters
        neoc_script_builder_t *builder;
        neoc_error_t err = neoc_script_builder_create(&builder);
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; j++) {
                neoc_witness_free((*witnesses)[j]);
            }
            neoc_free(*witnesses);
            *witnesses = NULL;
            return err;
        }
        
        for (size_t j = 0; j < ctx->parameter_counts[i]; j++) {
            // Add parameter to script
            // This would need proper implementation based on parameter type
        }
        
        uint8_t *invocation_script;
        size_t inv_len;
        err = neoc_script_builder_to_array(builder, &invocation_script, &inv_len);
        neoc_script_builder_free(builder);
        
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; j++) {
                neoc_witness_free((*witnesses)[j]);
            }
            neoc_free(*witnesses);
            *witnesses = NULL;
            return err;
        }
        
        // Get verification script for this script hash
        // For contracts, this would come from the contract state
        // For accounts, this comes from the account's verification script
        uint8_t *verification_script = NULL;
        size_t verification_len = 0;
        
        // Get verification script from the signer's script hash
        // This would be retrieved from:
        // 1. Cached account data if it's a standard account
        // 2. Contract state if it's a smart contract
        // 3. Empty for implicit verification (deployed contracts)
        // The verification script defines how to verify the invocation script
        
        err = neoc_witness_create(invocation_script, inv_len,
                                  verification_script, verification_len,
                                  &(*witnesses)[i]);
        neoc_free(invocation_script);
        
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; j++) {
                neoc_witness_free((*witnesses)[j]);
            }
            neoc_free(*witnesses);
            *witnesses = NULL;
            return err;
        }
    }
    
    *count = ctx->script_hash_count;
    return NEOC_SUCCESS;
}

void neoc_contract_parameters_context_free(neoc_contract_parameters_context_t *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->script_hash_count; i++) {
        for (size_t j = 0; j < ctx->parameter_counts[i]; j++) {
            if (ctx->parameters[i] && ctx->parameters[i][j]) {
                neoc_contract_parameter_free(ctx->parameters[i][j]);
            }
        }
        neoc_free(ctx->parameters[i]);
    }
    
    neoc_free(ctx->script_hashes);
    neoc_free(ctx->parameters);
    neoc_free(ctx->parameter_counts);
    neoc_free(ctx->completed);
    neoc_free(ctx);
}
