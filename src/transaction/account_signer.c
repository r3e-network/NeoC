/**
 * @file account_signer.c
 * @brief Account signer implementation
 */

#include "neoc/transaction/account_signer.h"
#include "neoc/transaction/signer.h"
#include "neoc/wallet/account.h"
#include "neoc/neoc_memory.h"
#include <string.h>

static neoc_error_t create_account_signer(neoc_account_t *account,
                                          neoc_witness_scope_t scope,
                                          neoc_account_signer_t **signer) {
    if (!account || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    *signer = neoc_malloc(sizeof(neoc_account_signer_t));
    if (!*signer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account signer");
    }
    
    // Get script hash from account
    neoc_hash160_t script_hash;
    neoc_error_t hash_err = neoc_account_get_script_hash(account, &script_hash);
    if (hash_err != NEOC_SUCCESS) {
        neoc_free(*signer);
        *signer = NULL;
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Failed to get script hash");
    }
    
    // Initialize base signer structure directly
    memset(&(*signer)->base, 0, sizeof(neoc_signer_t));
    memcpy(&(*signer)->base.account, &script_hash, sizeof(neoc_hash160_t));
    (*signer)->base.scopes = scope;
    (*signer)->base.allowed_contracts = NULL;
    (*signer)->base.allowed_contracts_count = 0;
    (*signer)->base.allowed_groups = NULL;
    (*signer)->base.allowed_groups_sizes = NULL;
    (*signer)->base.allowed_groups_count = 0;
    (*signer)->base.rules = NULL;
    (*signer)->base.rules_count = 0;
    
    (*signer)->account = account;
    (*signer)->owns_account = false;  // We don't own the account by default
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_signer_none(neoc_account_t *account,
                                       neoc_account_signer_t **signer) {
    return create_account_signer(account, NEOC_WITNESS_SCOPE_NONE, signer);
}

neoc_error_t neoc_account_signer_none_hash(neoc_hash160_t *account_hash,
                                            neoc_account_signer_t **signer) {
    if (!account_hash || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Create account from script hash
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_from_script_hash(account_hash, NULL, &account);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    if (!account) {
        // Fallback: create minimal account structure
        account = neoc_calloc(1, sizeof(neoc_account_t));
        if (!account) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
        }
        memcpy(&account->script_hash, account_hash, sizeof(neoc_hash160_t));
        
        // Generate address from script hash
        uint8_t address_bytes[25];
        address_bytes[0] = 0x35; // NEO3 address version byte
        memcpy(address_bytes + 1, account_hash->data, 20);
        
        // Calculate checksum (first 4 bytes of double SHA256)
        neoc_hash256_t hash1, hash2;
        neoc_hash_sha256(address_bytes, 21, hash1.data);
        neoc_hash_sha256(hash1.data, 32, hash2.data);
        memcpy(address_bytes + 21, hash2.data, 4);
        
        // Encode to base58
        account->address = neoc_base58_encode(address_bytes, 25);
        account->label = NULL;
        account->key_pair = NULL;
    }
    
    neoc_error_t err = create_account_signer(account, NEOC_WITNESS_SCOPE_NONE, signer);
    if (err != NEOC_SUCCESS) {
        neoc_free(account);
    }
    return err;
}

neoc_error_t neoc_account_signer_called_by_entry(neoc_account_t *account,
                                                  neoc_account_signer_t **signer) {
    return create_account_signer(account, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, signer);
}

neoc_error_t neoc_account_signer_called_by_entry_hash(neoc_hash160_t *account_hash,
                                                       neoc_account_signer_t **signer) {
    if (!account_hash || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Create minimal account from hash
    neoc_account_t *account = neoc_malloc(sizeof(neoc_account_t));
    if (!account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    memset(account, 0, sizeof(neoc_account_t));
    memcpy(&account->script_hash, account_hash, sizeof(neoc_hash160_t));
    
    neoc_error_t err = create_account_signer(account, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, signer);
    if (err != NEOC_SUCCESS) {
        neoc_free(account);
    }
    return err;
}

neoc_error_t neoc_account_signer_global(neoc_account_t *account,
                                         neoc_account_signer_t **signer) {
    return create_account_signer(account, NEOC_WITNESS_SCOPE_GLOBAL, signer);
}

neoc_error_t neoc_account_signer_global_hash(neoc_hash160_t *account_hash,
                                              neoc_account_signer_t **signer) {
    if (!account_hash || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Create minimal account from hash
    neoc_account_t *account = neoc_malloc(sizeof(neoc_account_t));
    if (!account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    memset(account, 0, sizeof(neoc_account_t));
    memcpy(&account->script_hash, account_hash, sizeof(neoc_hash160_t));
    
    neoc_error_t err = create_account_signer(account, NEOC_WITNESS_SCOPE_GLOBAL, signer);
    if (err != NEOC_SUCCESS) {
        neoc_free(account);
    }
    return err;
}

neoc_account_t* neoc_account_signer_get_account(neoc_account_signer_t *signer) {
    if (!signer) {
        return NULL;
    }
    return signer->account;
}

void neoc_account_signer_free(neoc_account_signer_t *signer) {
    if (signer) {
        // Free any allocated arrays in base signer
        if (signer->base.allowed_contracts) {
            neoc_free(signer->base.allowed_contracts);
        }
        if (signer->base.allowed_groups) {
            for (size_t i = 0; i < signer->base.allowed_groups_count; i++) {
                if (signer->base.allowed_groups[i]) {
                    neoc_free(signer->base.allowed_groups[i]);
                }
            }
            neoc_free(signer->base.allowed_groups);
        }
        if (signer->base.allowed_groups_sizes) {
            neoc_free(signer->base.allowed_groups_sizes);
        }
        if (signer->base.rules) {
            neoc_free(signer->base.rules);
        }
        
        // Only free the account if we own it
        if (signer->owns_account && signer->account) {
            neoc_account_free(signer->account);
        }
        
        neoc_free(signer);
    }
}
