/**
 * @file contract_signer.c
 * @brief Contract signer implementation
 */

#include "neoc/transaction/contract_signer.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>

// Structure is already defined in the header file

neoc_error_t neoc_contract_signer_create(const neoc_hash160_t *contract_hash,
                                          neoc_contract_signer_t **signer) {
    if (!contract_hash || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *signer = neoc_calloc(1, sizeof(neoc_contract_signer_t));
    if (!*signer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signer");
    }
    
    memcpy(&(*signer)->base.account, contract_hash, sizeof(neoc_hash160_t));
    (*signer)->base.scopes = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_signer_add_parameter(neoc_contract_signer_t *signer,
                                                 neoc_contract_parameter_t *param) {
    if (!signer || !param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_contract_parameter_t **new_params = neoc_realloc(signer->verify_params,
                                                           (signer->verify_params_count + 1) * sizeof(neoc_contract_parameter_t*));
    if (!new_params) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameters");
    }
    
    signer->verify_params = new_params;
    signer->verify_params[signer->verify_params_count++] = param;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_signer_get_contract_hash(const neoc_contract_signer_t *signer,
                                                     neoc_hash160_t *hash) {
    if (!signer || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(hash, &signer->base.account, sizeof(neoc_hash160_t));
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_signer_get_parameters(const neoc_contract_signer_t *signer,
                                                  neoc_contract_parameter_t ***params,
                                                  size_t *count) {
    if (!signer || !params || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *params = signer->verify_params;
    *count = signer->verify_params_count;
    
    return NEOC_SUCCESS;
}

void neoc_contract_signer_free(neoc_contract_signer_t *signer) {
    if (!signer) return;
    
    for (size_t i = 0; i < signer->verify_params_count; i++) {
        if (signer->verify_params[i]) {
            neoc_contract_parameter_free(signer->verify_params[i]);
        }
    }
    
    neoc_free(signer->verify_params);
    neoc_free(signer);
}
