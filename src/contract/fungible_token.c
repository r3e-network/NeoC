/**
 * @file fungible_token.c
 * @brief NEP-17 fungible token implementation
 */

#include "neoc/contract/fungible_token.h"
#include "neoc/neoc_memory.h"
#include "neoc/script/script_builder_full.h"
#include <string.h>

neoc_error_t neoc_fungible_token_create(neoc_hash160_t *contract_hash,
                                         neoc_fungible_token_t **token) {
    if (!contract_hash || !token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    *token = neoc_malloc(sizeof(neoc_fungible_token_t));
    if (!*token) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate fungible token");
    }
    
    // Initialize base token
    neoc_token_t *base_token = NULL;
    neoc_error_t err = neoc_token_create(contract_hash, NEOC_TOKEN_TYPE_FUNGIBLE, &base_token);
    if (err == NEOC_SUCCESS && base_token) {
        (*token)->base = *base_token;
        neoc_free(base_token);
    }
    if (err != NEOC_SUCCESS) {
        neoc_free(*token);
        return err;
    }
    
    (*token)->decimals = 8;  // Default decimals
    (*token)->total_supply = 0;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_fungible_token_balance_of(neoc_fungible_token_t *token,
                                             neoc_hash160_t *account,
                                             int64_t *balance) {
    if (!token || !account || !balance) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Build script to call balanceOf
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push account parameter
    err = neoc_script_builder_push_hash160(builder, account);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Call balanceOf method
    uint8_t contract_hash[20];
    neoc_hash160_to_bytes(token->base.contract_hash, contract_hash, sizeof(contract_hash));
    err = neoc_script_builder_emit_app_call(builder, token->base.contract_hash, "balanceOf", 1);
    
    neoc_script_builder_free(builder);
    
    // Execute the script through RPC to get the balance
    // In production, this would use the RPC client to invoke the script
    // and parse the BigInteger result from the VM stack
    *balance = 0;
    
    return err;
}

neoc_error_t neoc_fungible_token_transfer(neoc_fungible_token_t *token,
                                           neoc_hash160_t *from,
                                           neoc_hash160_t *to,
                                           int64_t amount,
                                           uint8_t *data,
                                           size_t data_len) {
    if (!token || !from || !to) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    if (amount <= 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Amount must be positive");
    }
    
    // Build transfer script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push data parameter
    if (data && data_len > 0) {
        err = neoc_script_builder_push_data(builder, data, data_len);
    } else {
        err = neoc_script_builder_push_null(builder);
    }
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push amount
    err = neoc_script_builder_push_integer(builder, amount);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push to address
    err = neoc_script_builder_push_hash160(builder, to);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push from address
    err = neoc_script_builder_push_hash160(builder, from);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Call transfer method
    uint8_t contract_hash[20];
    neoc_hash160_to_bytes(token->base.contract_hash, contract_hash, sizeof(contract_hash));
    err = neoc_script_builder_emit_app_call(builder, token->base.contract_hash, "transfer", 4);
    
    neoc_script_builder_free(builder);
    return err;
}

uint8_t neoc_fungible_token_get_decimals(neoc_fungible_token_t *token) {
    return token ? token->decimals : 0;
}

int64_t neoc_fungible_token_get_total_supply(neoc_fungible_token_t *token) {
    return token ? token->total_supply : 0;
}

void neoc_fungible_token_free(neoc_fungible_token_t *token) {
    if (token) {
        // Base is embedded, no need to free separately
        neoc_free(token);
    }
}
