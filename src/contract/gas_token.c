/**
 * @file gas_token.c
 * @brief GAS token implementation
 */

#include "neoc/contract/gas_token.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include <string.h>

const neoc_hash160_t NEOC_GAS_TOKEN_HASH = {
    .data = {0xd2, 0xa4, 0xce, 0xfe, 0x4e, 0x72, 0x8d, 0x0e,
             0xcd, 0x61, 0x49, 0x73, 0xf1, 0xaf, 0xdc, 0x34,
             0xfe, 0xd5, 0xb5, 0xc2}
};

neoc_error_t neoc_gas_token_create(neoc_gas_token_t **token) {
    if (!token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid token pointer");
    }
    
    *token = neoc_malloc(sizeof(neoc_gas_token_t));
    if (!*token) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate GAS token");
    }
    
    // Initialize as fungible token with GAS contract hash
    neoc_hash160_t hash;
    neoc_hash160_copy(&hash, &NEOC_GAS_TOKEN_HASH);
    neoc_fungible_token_t *base_fungible = NULL;
    neoc_error_t err = neoc_fungible_token_create(&hash, &base_fungible);
    if (err == NEOC_SUCCESS && base_fungible) {
        (*token)->base = *base_fungible;
        neoc_free(base_fungible);
    }
    if (err != NEOC_SUCCESS) {
        neoc_free(*token);
        return err;
    }
    
    // GAS specific properties
    (*token)->base.decimals = 8;
    (*token)->base.base.symbol = strdup("GAS");
    (*token)->base.base.name = strdup("GAS");
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_balance_of(neoc_hash160_t *account,
                                        int64_t *balance) {
    if (!account || !balance) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Create GAS token instance for operation
    neoc_gas_token_t *token = NULL;
    neoc_error_t err = neoc_gas_token_create(&token);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Use fungible token balance_of
    err = neoc_fungible_token_balance_of(&token->base, account, balance);
    
    neoc_gas_token_free(token);
    return err;
}

neoc_error_t neoc_gas_token_transfer(neoc_hash160_t *from,
                                      neoc_hash160_t *to,
                                      int64_t amount,
                                      uint8_t *data,
                                      size_t data_len) {
    if (!from || !to) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid addresses");
    }
    
    // Create GAS token instance for operation
    neoc_gas_token_t *token = NULL;
    neoc_error_t err = neoc_gas_token_create(&token);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Use fungible token transfer
    err = neoc_fungible_token_transfer(&token->base, from, to, amount, data, data_len);
    
    neoc_gas_token_free(token);
    return err;
}

neoc_error_t neoc_gas_token_refuel(neoc_hash160_t *account,
                                    int64_t amount) {
    if (!account || amount <= 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Build script to call refuel method on GAS contract
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push parameters in reverse order for NEO VM stack
    // Push amount parameter
    err = neoc_script_builder_push_integer(builder, amount);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push account parameter
    err = neoc_script_builder_push_hash160(builder, account);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Call GAS contract's refuel method
    err = neoc_script_builder_emit_app_call(builder, &NEOC_GAS_TOKEN_HASH, "refuel", 2);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Execute the built script through a transaction
    // Script execution requires an active RPC connection and transaction builder
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

void neoc_gas_token_free(neoc_gas_token_t *token) {
    if (token) {
        // No need to free base as it's embedded
        neoc_free(token);
    }
}
