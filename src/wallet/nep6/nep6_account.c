#define NEOC_NEP6_ACCOUNT_DISABLE_OVERLOADS

/**
 * @file nep6_account.c
 * @brief NEP-6 account implementation
 * 
 * Based on Swift source: wallet/nep6/NEP6Account.swift
 */

#include "neoc/wallet/nep6/nep6_account.h"
#include "neoc/utils/json.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static bool neoc_nep6_contracts_equal(const neoc_nep6_contract_t *a,
                                      const neoc_nep6_contract_t *b) {
    if (a == b) return true;
    if (!a || !b) return false;

    if (a->is_deployed != b->is_deployed) return false;

    if ((a->script == NULL) != (b->script == NULL)) return false;
    if (a->script && strcmp(a->script, b->script) != 0) return false;

    if (a->parameter_count != b->parameter_count) return false;

    for (size_t i = 0; i < a->parameter_count; ++i) {
        const neoc_nep6_parameter_t *pa = &a->parameters[i];
        const neoc_nep6_parameter_t *pb = &b->parameters[i];
        if (pa->type != pb->type) return false;
        if ((pa->name == NULL) != (pb->name == NULL)) return false;
        if (pa->name && strcmp(pa->name, pb->name) != 0) return false;
    }

    return true;
}

static neoc_error_t neoc_nep6_contract_clone(const neoc_nep6_contract_t *src,
                                             neoc_nep6_contract_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Contract clone: invalid arguments");
    }

    *dest = neoc_calloc(1, sizeof(neoc_nep6_contract_t));
    if (!*dest) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Contract clone: allocation failed");
    }

    if (src->script) {
        (*dest)->script = neoc_strdup(src->script);
        if (!(*dest)->script) {
            neoc_nep6_contract_free(*dest);
            *dest = NULL;
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Contract clone: script copy failed");
        }
    }

    if (src->parameter_count > 0 && src->parameters) {
        (*dest)->parameters = neoc_calloc(src->parameter_count, sizeof(neoc_nep6_parameter_t));
        if (!(*dest)->parameters) {
            neoc_nep6_contract_free(*dest);
            *dest = NULL;
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Contract clone: parameter allocation failed");
        }

        for (size_t i = 0; i < src->parameter_count; ++i) {
            (*dest)->parameters[i].type = src->parameters[i].type;
            if (src->parameters[i].name) {
                (*dest)->parameters[i].name = neoc_strdup(src->parameters[i].name);
                if (!(*dest)->parameters[i].name) {
                    (*dest)->parameter_count = i;
                    neoc_nep6_contract_free(*dest);
                    *dest = NULL;
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Contract clone: parameter name copy failed");
                }
            }
        }

        (*dest)->parameter_count = src->parameter_count;
    }

    (*dest)->is_deployed = src->is_deployed;
    return NEOC_SUCCESS;
}

/**
 * @brief Create a new NEP-6 account
 */
neoc_error_t neoc_nep6_account_create(
    const char *address,
    const char *label,
    bool is_default,
    bool lock,
    const char *key,
    neoc_nep6_contract_t *contract,
    neoc_nep6_account_t **account) {
    
    if (!address || !account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *account = neoc_calloc(1, sizeof(neoc_nep6_account_t));
    if (!*account) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Set required fields
    (*account)->address = neoc_strdup(address);
    if (!(*account)->address) {
        neoc_free(*account);
        *account = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Set optional fields
    if (label) {
        (*account)->label = neoc_strdup(label);
        if (!(*account)->label) {
            neoc_free((*account)->address);
            neoc_free(*account);
            *account = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    
    if (key) {
        (*account)->key = neoc_strdup(key);
        if (!(*account)->key) {
            if ((*account)->label) neoc_free((*account)->label);
            neoc_free((*account)->address);
            neoc_free(*account);
            *account = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    
    (*account)->is_default = is_default;
    (*account)->lock = lock;
    (*account)->contract = contract;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Free a NEP-6 account
 */
void neoc_nep6_account_free(neoc_nep6_account_t *account) {
    if (!account) {
        return;
    }
    
    // Free string fields
    if (account->address) neoc_free(account->address);
    if (account->label) neoc_free(account->label);
    if (account->key) neoc_free(account->key);
    
    // Free contract
    if (account->contract) {
        neoc_nep6_contract_free(account->contract);
    }
    
    // Free extra fields
    if (account->extra) {
        for (size_t i = 0; i < account->extra_count; i++) {
            if (account->extra[i].key) neoc_free(account->extra[i].key);
            if (account->extra[i].value) neoc_free(account->extra[i].value);
        }
        neoc_free(account->extra);
    }
    
    neoc_free(account);
}

neoc_error_t neoc_nep6_account_set_address(neoc_nep6_account_t *account, const char *address) {
    if (!account || !address) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    char *copy = neoc_strdup(address);
    if (!copy) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    if (account->address) {
        neoc_free(account->address);
    }
    account->address = copy;
    return NEOC_SUCCESS;
}

const char* neoc_nep6_account_get_address(const neoc_nep6_account_t *account) {
    return account ? account->address : NULL;
}

neoc_error_t neoc_nep6_account_set_label(neoc_nep6_account_t *account, const char *label) {
    if (!account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    char *copy = NULL;
    if (label) {
        copy = neoc_strdup(label);
        if (!copy) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    if (account->label) {
        neoc_free(account->label);
    }
    account->label = copy;
    return NEOC_SUCCESS;
}

const char* neoc_nep6_account_get_label(const neoc_nep6_account_t *account) {
    return account ? account->label : NULL;
}

neoc_error_t neoc_nep6_account_set_default(neoc_nep6_account_t *account, bool is_default) {
    if (!account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    account->is_default = is_default;
    return NEOC_SUCCESS;
}

bool neoc_nep6_account_is_default(const neoc_nep6_account_t *account) {
    return account ? account->is_default : false;
}

neoc_error_t neoc_nep6_account_set_locked(neoc_nep6_account_t *account, bool lock) {
    if (!account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    account->lock = lock;
    return NEOC_SUCCESS;
}

bool neoc_nep6_account_is_locked(const neoc_nep6_account_t *account) {
    return account ? account->lock : false;
}

neoc_error_t neoc_nep6_account_set_key(neoc_nep6_account_t *account, const char *key) {
    if (!account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    char *copy = NULL;
    if (key) {
        copy = neoc_strdup(key);
        if (!copy) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    if (account->key) {
        neoc_free(account->key);
    }
    account->key = copy;
    return NEOC_SUCCESS;
}

const char* neoc_nep6_account_get_key(const neoc_nep6_account_t *account) {
    return account ? account->key : NULL;
}

neoc_error_t neoc_nep6_account_set_contract(neoc_nep6_account_t *account,
                                            neoc_nep6_contract_t *contract) {
    if (!account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    if (account->contract) {
        neoc_nep6_contract_free(account->contract);
    }
    account->contract = contract;
    return NEOC_SUCCESS;
}

neoc_nep6_contract_t* neoc_nep6_account_get_contract(const neoc_nep6_account_t *account) {
    return account ? account->contract : NULL;
}

/**
 * @brief Convert NEP-6 account to JSON string
 */
neoc_error_t neoc_nep6_account_to_json(
    const neoc_nep6_account_t *account,
    char **json_str) {
    
    if (!account || !json_str) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON object
    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add required fields
    neoc_json_add_string(json, "address", account->address);
    
    // Add optional fields
    if (account->label) {
        neoc_json_add_string(json, "label", account->label);
    }
    
    neoc_json_add_bool(json, "isDefault", account->is_default);
    neoc_json_add_bool(json, "lock", account->lock);
    
    if (account->key) {
        neoc_json_add_string(json, "key", account->key);
    }
    
    // Add contract if present
    if (account->contract) {
        char *contract_json = NULL;
        if (neoc_nep6_contract_to_json(account->contract, &contract_json) == NEOC_SUCCESS && contract_json) {
            neoc_json_t *contract_obj = neoc_json_parse(contract_json);
            if (contract_obj) {
                neoc_json_add_object(json, "contract", contract_obj);
            }
            neoc_free(contract_json);
        }
    }
    
    // Add extra fields if present
    if (account->extra && account->extra_count > 0) {
        neoc_json_t *extra = neoc_json_create_object();
        for (size_t i = 0; i < account->extra_count; i++) {
            neoc_json_add_string(extra, account->extra[i].key, account->extra[i].value);
        }
        neoc_json_add_object(json, "extra", extra);
    }
    
    // Convert to string
    *json_str = neoc_json_to_string(json);
    neoc_json_free(json);
    
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Parse JSON into NEP-6 account
 */
neoc_error_t neoc_nep6_account_from_json(
    const char *json_str,
    neoc_nep6_account_t **account) {
    
    if (!json_str || !account) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Parse JSON
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Get required address field
    const char *address = neoc_json_get_string(json, "address");
    if (!address) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Get optional fields
    const char *label = neoc_json_get_string(json, "label");
    bool is_default = false;
    neoc_json_get_bool(json, "isDefault", &is_default);
    bool lock = false;
    neoc_json_get_bool(json, "lock", &lock);
    const char *key = neoc_json_get_string(json, "key");
    
    // Parse contract if present
    neoc_nep6_contract_t *contract = NULL;
    neoc_json_t *contract_json = neoc_json_get_object(json, "contract");
    if (contract_json) {
        char *contract_str = neoc_json_to_string(contract_json);
        if (contract_str) {
            neoc_nep6_contract_from_json(contract_str, &contract);
            neoc_free(contract_str);
        }
    }
    
    // Create account
    neoc_error_t err = neoc_nep6_account_create(
        address, label, is_default, lock, key, contract, account
    );
    
    if (err != NEOC_SUCCESS) {
        if (contract) neoc_nep6_contract_free(contract);
        neoc_json_free(json);
        return err;
    }
    
    // Parse extra fields if present
    neoc_json_t *extra_json = neoc_json_get_object(json, "extra");
    if (extra_json) {
        // Extra field parsing would require JSON object iteration
        // which depends on the JSON library implementation
    }
    
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

/**
 * @brief Compare two NEP-6 accounts for equality
 */
bool neoc_nep6_account_equals(
    const neoc_nep6_account_t *account1,
    const neoc_nep6_account_t *account2) {
    
    if (account1 == account2) return true;
    if (!account1 || !account2) return false;
    
    // Compare addresses (required field)
    if (strcmp(account1->address, account2->address) != 0) {
        return false;
    }
    
    // Compare labels (optional field)
    if ((account1->label == NULL) != (account2->label == NULL)) {
        return false;
    }
    if (account1->label && strcmp(account1->label, account2->label) != 0) {
        return false;
    }
    
    // Compare boolean fields
    if (account1->is_default != account2->is_default) {
        return false;
    }
    if (account1->lock != account2->lock) {
        return false;
    }
    
    // Compare keys (optional field)
    if ((account1->key == NULL) != (account2->key == NULL)) {
        return false;
    }
    if (account1->key && strcmp(account1->key, account2->key) != 0) {
        return false;
    }
    
    // Compare contracts (optional field)
    if ((account1->contract == NULL) != (account2->contract == NULL)) {
        return false;
    }
    if (account1->contract && account2->contract) {
        // Deep comparison of contracts
        if (!neoc_nep6_contracts_equal(account1->contract, account2->contract)) {
            return false;
        }
    }
    
    // Compare extra fields count
    if (account1->extra_count != account2->extra_count) {
        return false;
    }
    
    // Compare extra fields
    if (account1->extra_count > 0) {
        // Would need to compare each extra field
        // This requires sorting or hash table for proper comparison
        for (size_t i = 0; i < account1->extra_count; i++) {
            bool found = false;
            for (size_t j = 0; j < account2->extra_count; j++) {
                if (strcmp(account1->extra[i].key, account2->extra[j].key) == 0) {
                    if (strcmp(account1->extra[i].value, account2->extra[j].value) != 0) {
                        return false;
                    }
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
    }
    
    return true;
}

/**
 * @brief Create a copy of a NEP-6 account
 */
neoc_error_t neoc_nep6_account_copy(
    const neoc_nep6_account_t *src,
    neoc_nep6_account_t **dest) {
    
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Copy contract if present
    neoc_nep6_contract_t *contract_copy = NULL;
    if (src->contract) {
        // Deep copy of contract
        neoc_error_t err = neoc_nep6_contract_clone(src->contract, &contract_copy);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }
    
    // Create new account with copied fields
    neoc_error_t err = neoc_nep6_account_create(
        src->address,
        src->label,
        src->is_default,
        src->lock,
        src->key,
        contract_copy,
        dest
    );
    
    if (err != NEOC_SUCCESS) {
        if (contract_copy) neoc_nep6_contract_free(contract_copy);
        return err;
    }
    
    // Copy extra fields
    if (src->extra_count > 0 && src->extra) {
        (*dest)->extra = neoc_calloc(src->extra_count, sizeof(neoc_nep6_account_extra_t));
        if (!(*dest)->extra) {
            neoc_nep6_account_free(*dest);
            *dest = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        
        (*dest)->extra_count = src->extra_count;
        
        for (size_t i = 0; i < src->extra_count; i++) {
            (*dest)->extra[i].key = neoc_strdup(src->extra[i].key);
            (*dest)->extra[i].value = neoc_strdup(src->extra[i].value);
            
            if (!(*dest)->extra[i].key || !(*dest)->extra[i].value) {
                neoc_nep6_account_free(*dest);
                *dest = NULL;
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
        }
    }
    
    return NEOC_SUCCESS;
}
