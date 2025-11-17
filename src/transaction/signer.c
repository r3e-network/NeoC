#include "neoc/transaction/signer.h"
#include "neoc/neoc_memory.h"
#include <stdlib.h>
#include <string.h>

neoc_error_t neoc_signer_create(const neoc_hash160_t *account,
                                 uint8_t scopes,
                                 neoc_signer_t **signer) {
    if (!account || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *signer = calloc(1, sizeof(neoc_signer_t));
    if (!*signer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signer");
    }
    
    memcpy(&(*signer)->account, account, sizeof(neoc_hash160_t));
    (*signer)->scopes = scopes;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_signer_create_global(const neoc_hash160_t *account,
                                        neoc_signer_t **signer) {
    return neoc_signer_create(account, NEOC_WITNESS_SCOPE_GLOBAL, signer);
}

neoc_error_t neoc_signer_create_called_by_entry(const neoc_hash160_t *account,
                                                 neoc_signer_t **signer) {
    return neoc_signer_create(account, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, signer);
}
neoc_error_t neoc_signer_add_allowed_contract(neoc_signer_t *signer,
                                               const neoc_hash160_t *contract) {
    if (!signer || !contract) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Global scope cannot have custom contracts
    if (signer->scopes & NEOC_WITNESS_SCOPE_GLOBAL) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Cannot add contracts to global signer");
    }
    
    if (signer->allowed_contracts_count >= NEOC_MAX_SIGNER_SUBITEMS) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Too many allowed contracts");
    }
    
    // Add CUSTOM_CONTRACTS scope
    signer->scopes |= NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS;
    
    // Resize allowed contracts array
    size_t new_count = signer->allowed_contracts_count + 1;
    neoc_hash160_t *new_contracts = realloc(signer->allowed_contracts,
                                             new_count * sizeof(neoc_hash160_t));
    if (!new_contracts) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize allowed contracts");
    }
    
    signer->allowed_contracts = new_contracts;
    memcpy(&signer->allowed_contracts[signer->allowed_contracts_count],
           contract, sizeof(neoc_hash160_t));
    signer->allowed_contracts_count = new_count;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_signer_add_allowed_group(neoc_signer_t *signer,
                                            const uint8_t *group_pubkey,
                                            size_t pubkey_size) {
    if (!signer || !group_pubkey || pubkey_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Global scope cannot have custom groups
    if (signer->scopes & NEOC_WITNESS_SCOPE_GLOBAL) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Cannot add groups to global signer");
    }
    
    if (signer->allowed_groups_count >= NEOC_MAX_SIGNER_SUBITEMS) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Too many allowed groups");
    }
    
    // Add CUSTOM_GROUPS scope
    signer->scopes |= NEOC_WITNESS_SCOPE_CUSTOM_GROUPS;
    
    // Resize arrays
    size_t new_count = signer->allowed_groups_count + 1;
    
    uint8_t **new_groups = realloc(signer->allowed_groups,
                                    new_count * sizeof(uint8_t*));
    if (!new_groups) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize allowed groups");
    }
    
    size_t *new_sizes = realloc(signer->allowed_groups_sizes,
                                 new_count * sizeof(size_t));
    if (!new_sizes) {
        if (new_groups != signer->allowed_groups) {
            free(new_groups);
        }
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize group sizes");
    }
    
    // Allocate and copy public key
    uint8_t *pubkey_copy = malloc(pubkey_size);
    if (!pubkey_copy) {
        if (new_groups != signer->allowed_groups) {
            free(new_groups);
        }
        if (new_sizes != signer->allowed_groups_sizes) {
            free(new_sizes);
        }
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate group public key");
    }
    memcpy(pubkey_copy, group_pubkey, pubkey_size);
    
    signer->allowed_groups = new_groups;
    signer->allowed_groups_sizes = new_sizes;
    signer->allowed_groups[signer->allowed_groups_count] = pubkey_copy;
    signer->allowed_groups_sizes[signer->allowed_groups_count] = pubkey_size;
    signer->allowed_groups_count = new_count;
    
    return NEOC_SUCCESS;
}

bool neoc_signer_has_global_scope(const neoc_signer_t *signer) {
    return signer && (signer->scopes & NEOC_WITNESS_SCOPE_GLOBAL) != 0;
}

bool neoc_signer_has_called_by_entry_scope(const neoc_signer_t *signer) {
    return signer && (signer->scopes & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0;
}

bool neoc_signer_has_custom_contracts_scope(const neoc_signer_t *signer) {
    return signer && (signer->scopes & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) != 0;
}

bool neoc_signer_has_custom_groups_scope(const neoc_signer_t *signer) {
    return signer && (signer->scopes & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) != 0;
}

size_t neoc_signer_get_size(const neoc_signer_t *signer) {
    if (!signer) return 0;
    
    size_t size = 0;
    
    // Account hash (20 bytes)
    size += sizeof(neoc_hash160_t);
    
    // Scopes (1 byte)
    size += 1;
    
    // Custom contracts if present
    if (signer->scopes & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) {
        size += 1;  // Count varint (assuming fits in 1 byte)
        size += signer->allowed_contracts_count * sizeof(neoc_hash160_t);
    }
    
    // Custom groups if present
    if (signer->scopes & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) {
        size += 1;  // Count varint (assuming fits in 1 byte)
        for (size_t i = 0; i < signer->allowed_groups_count; i++) {
            size += 1;  // Length byte
            size += signer->allowed_groups_sizes[i];
        }
    }
    
    // Witness rules if present
    if (signer->scopes & NEOC_WITNESS_SCOPE_WITNESS_RULES) {
        // Note: Implement witness rules size calculation
        size += 1;  // Count = 0 for now
    }
    
    return size;
}

neoc_error_t neoc_signer_copy(const neoc_signer_t *source, neoc_signer_t **dest) {
    if (!source || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *dest = calloc(1, sizeof(neoc_signer_t));
    if (!*dest) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signer copy");
    }
    
    // Copy basic fields
    memcpy(&(*dest)->account, &source->account, sizeof(neoc_hash160_t));
    (*dest)->scopes = source->scopes;
    
    // Copy allowed contracts
    if (source->allowed_contracts_count > 0 && source->allowed_contracts) {
        (*dest)->allowed_contracts = malloc(source->allowed_contracts_count * sizeof(neoc_hash160_t));
        if (!(*dest)->allowed_contracts) {
            free(*dest);
            *dest = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate allowed contracts");
        }
        memcpy((*dest)->allowed_contracts, source->allowed_contracts, 
               source->allowed_contracts_count * sizeof(neoc_hash160_t));
        (*dest)->allowed_contracts_count = source->allowed_contracts_count;
    }
    
    // Copy allowed groups
    if (source->allowed_groups_count > 0 && source->allowed_groups) {
        (*dest)->allowed_groups = calloc(source->allowed_groups_count, sizeof(uint8_t*));
        (*dest)->allowed_groups_sizes = calloc(source->allowed_groups_count, sizeof(size_t));
        
        if (!(*dest)->allowed_groups || !(*dest)->allowed_groups_sizes) {
            if ((*dest)->allowed_contracts) free((*dest)->allowed_contracts);
            if ((*dest)->allowed_groups) free((*dest)->allowed_groups);
            if ((*dest)->allowed_groups_sizes) free((*dest)->allowed_groups_sizes);
            free(*dest);
            *dest = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate allowed groups");
        }
        
        for (size_t i = 0; i < source->allowed_groups_count; i++) {
            (*dest)->allowed_groups[i] = malloc(source->allowed_groups_sizes[i]);
            if (!(*dest)->allowed_groups[i]) {
                // Clean up on failure
                for (size_t j = 0; j < i; j++) {
                    free((*dest)->allowed_groups[j]);
                }
                if ((*dest)->allowed_contracts) free((*dest)->allowed_contracts);
                free((*dest)->allowed_groups);
                free((*dest)->allowed_groups_sizes);
                free(*dest);
                *dest = NULL;
                return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate group public key");
            }
            memcpy((*dest)->allowed_groups[i], source->allowed_groups[i], 
                   source->allowed_groups_sizes[i]);
            (*dest)->allowed_groups_sizes[i] = source->allowed_groups_sizes[i];
        }
        (*dest)->allowed_groups_count = source->allowed_groups_count;
    }
    
    if (source->rules_count > 0 && source->rules) {
        (*dest)->rules = calloc(source->rules_count, sizeof(neoc_witness_rule_t*));
        if (!(*dest)->rules) {
            neoc_signer_free(*dest);
            *dest = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness rules");
        }

        for (size_t i = 0; i < source->rules_count; i++) {
            if (!source->rules[i]) {
                continue;
            }
            neoc_error_t rule_err = neoc_witness_rule_clone(source->rules[i], &(*dest)->rules[i]);
            if (rule_err != NEOC_SUCCESS) {
                neoc_signer_free(*dest);
                *dest = NULL;
                return rule_err;
            }
        }
        (*dest)->rules_count = source->rules_count;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_signer_get_account(const neoc_signer_t *signer, neoc_hash160_t *account) {
    if (!signer || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(account, &signer->account, sizeof(neoc_hash160_t));
    return NEOC_SUCCESS;
}

void neoc_signer_free(neoc_signer_t *signer) {
    if (!signer) return;
    
    if (signer->allowed_contracts) {
        free(signer->allowed_contracts);
    }
    
    if (signer->allowed_groups) {
        for (size_t i = 0; i < signer->allowed_groups_count; i++) {
            if (signer->allowed_groups[i]) {
                free(signer->allowed_groups[i]);
            }
        }
        free(signer->allowed_groups);
    }
    
    if (signer->allowed_groups_sizes) {
        free(signer->allowed_groups_sizes);
    }
    
    if (signer->rules) {
        for (size_t i = 0; i < signer->rules_count; i++) {
            if (signer->rules[i]) {
                neoc_witness_rule_free(signer->rules[i]);
            }
        }
        neoc_free(signer->rules);
    }
    
    free(signer);
}

neoc_error_t neoc_signer_serialize(const neoc_signer_t *signer, neoc_binary_writer_t *writer) {
    if (!signer || !writer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Write account hash (20 bytes)
    neoc_error_t err = neoc_binary_writer_write_bytes(writer, signer->account.data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) return err;
    
    // Write scopes (1 byte)
    err = neoc_binary_writer_write_byte(writer, signer->scopes);
    if (err != NEOC_SUCCESS) return err;
    
    // Write custom contracts if present
    if (signer->scopes & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) {
        err = neoc_binary_writer_write_var_int(writer, signer->allowed_contracts_count);
        if (err != NEOC_SUCCESS) return err;
        
        for (size_t i = 0; i < signer->allowed_contracts_count; i++) {
            err = neoc_binary_writer_write_bytes(writer, signer->allowed_contracts[i].data, sizeof(neoc_hash160_t));
            if (err != NEOC_SUCCESS) return err;
        }
    }
    
    // Write custom groups if present
    if (signer->scopes & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) {
        err = neoc_binary_writer_write_var_int(writer, signer->allowed_groups_count);
        if (err != NEOC_SUCCESS) return err;
        
        for (size_t i = 0; i < signer->allowed_groups_count; i++) {
            err = neoc_binary_writer_write_var_bytes(writer, signer->allowed_groups[i], signer->allowed_groups_sizes[i]);
            if (err != NEOC_SUCCESS) return err;
        }
    }
    
    // Write witness rules if present
    if (signer->scopes & NEOC_WITNESS_SCOPE_WITNESS_RULES) {
        err = neoc_binary_writer_write_var_int(writer, signer->rules_count);
        if (err != NEOC_SUCCESS) return err;
        
        // Write each witness rule
        for (size_t i = 0; i < signer->rules_count && signer->rules; i++) {
            neoc_witness_rule_t *rule = signer->rules[i];
            if (!rule) {
                err = neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Witness rule is NULL");
                return err;
            }

            err = neoc_binary_writer_write_byte(writer, (uint8_t)rule->action);
            if (err != NEOC_SUCCESS) return err;
            
            // Write rule condition
            err = neoc_witness_condition_serialize(rule->condition, writer);
            if (err != NEOC_SUCCESS) return err;
        }
    }
    
    return NEOC_SUCCESS;
}
