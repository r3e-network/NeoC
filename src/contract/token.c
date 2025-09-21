/**
 * @file token.c
 * @brief Base token implementation
 */

#include "neoc/contract/token.h"
#include "neoc/neoc_memory.h"
#include <string.h>

neoc_error_t neoc_token_create(neoc_hash160_t *contract_hash,
                                neoc_token_type_t type,
                                neoc_token_t **token) {
    if (!contract_hash || !token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    *token = neoc_malloc(sizeof(neoc_token_t));
    if (!*token) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate token");
    }
    
    (*token)->contract_hash = neoc_malloc(sizeof(neoc_hash160_t));
    if (!(*token)->contract_hash) {
        neoc_free(*token);
        *token = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate hash");
    }
    neoc_hash160_copy((*token)->contract_hash, contract_hash);
    (*token)->type = type;
    (*token)->symbol = NULL;
    (*token)->name = NULL;
    
    return NEOC_SUCCESS;
}

const char* neoc_token_get_symbol(neoc_token_t *token) {
    return token ? token->symbol : NULL;
}

const char* neoc_token_get_name(neoc_token_t *token) {
    return token ? token->name : NULL;
}

neoc_hash160_t* neoc_token_get_contract_hash(neoc_token_t *token) {
    return token ? token->contract_hash : NULL;
}

void neoc_token_free(neoc_token_t *token) {
    if (token) {
        if (token->contract_hash) {
            neoc_free(token->contract_hash);
        }
        if (token->symbol) {
            neoc_free(token->symbol);
        }
        if (token->name) {
            neoc_free(token->name);
        }
        neoc_free(token);
    }
}
