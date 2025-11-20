/**
 * @file transaction_signer.c
 * @brief Transaction signer RPC model implementation
 */

#include "neoc/protocol/core/response/transaction_signer.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/transaction/witness_scope.h"
#include "neoc/protocol/core/witnessrule/witness_rule.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

static neoc_error_t copy_string_array(const char **src,
                                      size_t count,
                                      char ***dest_out) {
    if (!dest_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *dest_out = NULL;
    if (!src || count == 0) {
        return NEOC_SUCCESS;
    }

    char **arr = neoc_calloc(count, sizeof(char *));
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        arr[i] = dup_string(src[i]);
        if (!arr[i]) {
            for (size_t j = 0; j <= i; j++) {
                neoc_free(arr[j]);
            }
            neoc_free(arr);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    *dest_out = arr;
    return NEOC_SUCCESS;
}

static neoc_error_t copy_rules(const neoc_witness_rule_t **src,
                               size_t count,
                               neoc_witness_rule_t ***dest_out) {
    if (!dest_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *dest_out = NULL;
    if (!src || count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_witness_rule_t **arr = neoc_calloc(count, sizeof(neoc_witness_rule_t *));
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        neoc_error_t err = neoc_witness_rule_clone(src[i], &arr[i]);
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j <= i; j++) {
                neoc_witness_rule_free(arr[j]);
            }
            neoc_free(arr);
            return err;
        }
    }
    *dest_out = arr;
    return NEOC_SUCCESS;
}

static void free_string_array(char **arr, size_t count) {
    if (!arr) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        neoc_free(arr[i]);
    }
    neoc_free(arr);
}

static void free_rules_array(neoc_witness_rule_t **arr, size_t count) {
    if (!arr) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        neoc_witness_rule_free(arr[i]);
    }
    neoc_free(arr);
}

neoc_error_t neoc_transaction_signer_create(neoc_transaction_signer_t **signer) {
    if (!signer) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *signer = neoc_calloc(1, sizeof(neoc_transaction_signer_t));
    return *signer ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

void neoc_transaction_signer_free(neoc_transaction_signer_t *signer) {
    if (!signer) {
        return;
    }
    if (signer->account) {
        neoc_free(signer->account);
    }
    if (signer->scopes) {
        neoc_free(signer->scopes);
    }
    free_string_array(signer->allowed_contracts, signer->allowed_contracts_count);
    free_string_array(signer->allowed_groups, signer->allowed_groups_count);
    free_rules_array(signer->rules, signer->rules_count);
    neoc_free(signer);
}

neoc_error_t neoc_transaction_signer_create_with_scopes(const neoc_hash160_t *account,
                                                        const neoc_witness_scope_t *scopes,
                                                        size_t scopes_count,
                                                        neoc_transaction_signer_t **signer) {
    return neoc_transaction_signer_create_full(account,
                                               scopes,
                                               scopes_count,
                                               NULL,
                                               0,
                                               NULL,
                                               0,
                                               NULL,
                                               0,
                                               signer);
}

neoc_error_t neoc_transaction_signer_create_full(const neoc_hash160_t *account,
                                                 const neoc_witness_scope_t *scopes,
                                                 size_t scopes_count,
                                                 const char **allowed_contracts,
                                                 size_t allowed_contracts_count,
                                                 const char **allowed_groups,
                                                 size_t allowed_groups_count,
                                                 const neoc_witness_rule_t **rules,
                                                 size_t rules_count,
                                                 neoc_transaction_signer_t **signer) {
    if (!account || !signer) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    neoc_error_t err = neoc_transaction_signer_create(signer);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    (*signer)->account = neoc_malloc(sizeof(neoc_hash160_t));
    if (!(*signer)->account) {
        neoc_transaction_signer_free(*signer);
        *signer = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    *(*signer)->account = *account;

    if (scopes_count > 0 && scopes) {
        (*signer)->scopes = neoc_calloc(scopes_count, sizeof(neoc_witness_scope_t));
        if (!(*signer)->scopes) {
            neoc_transaction_signer_free(*signer);
            *signer = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy((*signer)->scopes, scopes, scopes_count * sizeof(neoc_witness_scope_t));
        (*signer)->scopes_count = scopes_count;
    }

    if (allowed_contracts_count > 0 && allowed_contracts) {
        err = copy_string_array(allowed_contracts, allowed_contracts_count, &(*signer)->allowed_contracts);
        if (err != NEOC_SUCCESS) {
            neoc_transaction_signer_free(*signer);
            *signer = NULL;
            return err;
        }
        (*signer)->allowed_contracts_count = allowed_contracts_count;
    }

    if (allowed_groups_count > 0 && allowed_groups) {
        err = copy_string_array(allowed_groups, allowed_groups_count, &(*signer)->allowed_groups);
        if (err != NEOC_SUCCESS) {
            neoc_transaction_signer_free(*signer);
            *signer = NULL;
            return err;
        }
        (*signer)->allowed_groups_count = allowed_groups_count;
    }

    if (rules_count > 0 && rules) {
        err = copy_rules(rules, rules_count, &(*signer)->rules);
        if (err != NEOC_SUCCESS) {
            neoc_transaction_signer_free(*signer);
            *signer = NULL;
            return err;
        }
        (*signer)->rules_count = rules_count;
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_signer_add_scope(neoc_transaction_signer_t *signer,
                                               neoc_witness_scope_t scope) {
    if (!signer) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    size_t new_count = signer->scopes_count + 1;
    neoc_witness_scope_t *new_scopes =
        neoc_realloc(signer->scopes, new_count * sizeof(neoc_witness_scope_t));
    if (!new_scopes) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    new_scopes[signer->scopes_count] = scope;
    signer->scopes = new_scopes;
    signer->scopes_count = new_count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_signer_add_allowed_contract(neoc_transaction_signer_t *signer,
                                                          const char *contract_hash) {
    if (!signer || !contract_hash) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    size_t new_count = signer->allowed_contracts_count + 1;
    char **arr = neoc_realloc(signer->allowed_contracts, new_count * sizeof(char *));
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    arr[signer->allowed_contracts_count] = dup_string(contract_hash);
    if (!arr[signer->allowed_contracts_count]) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    signer->allowed_contracts = arr;
    signer->allowed_contracts_count = new_count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_signer_add_allowed_group(neoc_transaction_signer_t *signer,
                                                       const char *group_key) {
    if (!signer || !group_key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    size_t new_count = signer->allowed_groups_count + 1;
    char **arr = neoc_realloc(signer->allowed_groups, new_count * sizeof(char *));
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    arr[signer->allowed_groups_count] = dup_string(group_key);
    if (!arr[signer->allowed_groups_count]) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    signer->allowed_groups = arr;
    signer->allowed_groups_count = new_count;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_scope_array(neoc_json_t *array,
                                      neoc_witness_scope_t **scopes_out,
                                      size_t *count_out) {
    if (!scopes_out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *scopes_out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }
    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_witness_scope_t *scopes = neoc_calloc(count, sizeof(neoc_witness_scope_t));
    if (!scopes) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        const char *name = entry ? neoc_json_get_string(entry, "value") : NULL;
        if (!name && entry) {
            name = neoc_json_get_string(entry, "scope");
        }
        if (!name && entry) {
            name = neoc_json_get_string(entry, NULL);
        }
        const char *scope_str = name;
        if (!scope_str) {
            neoc_free(scopes);
            return NEOC_ERROR_INVALID_FORMAT;
        }
        if (neoc_witness_scope_from_json(scope_str, &scopes[i]) != NEOC_SUCCESS) {
            neoc_free(scopes);
            return NEOC_ERROR_INVALID_FORMAT;
        }
    }

    *scopes_out = scopes;
    *count_out = count;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_string_array(neoc_json_t *array,
                                       char ***out,
                                       size_t *count_out) {
    if (!out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }

    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    char **arr = neoc_calloc(count, sizeof(char *));
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        const char *val = entry ? neoc_json_get_string(entry, NULL) : NULL;
        if (!val && entry) {
            val = neoc_json_get_string(entry, "value");
        }
        if (!val) {
            val = "";
        }
        arr[i] = dup_string(val);
        if (!arr[i]) {
            for (size_t j = 0; j <= i; j++) {
                neoc_free(arr[j]);
            }
            neoc_free(arr);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *out = arr;
    *count_out = count;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_rules_array(neoc_json_t *array,
                                      neoc_witness_rule_t ***rules_out,
                                      size_t *count_out) {
    if (!rules_out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *rules_out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }
    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_witness_rule_t **rules = neoc_calloc(count, sizeof(neoc_witness_rule_t *));
    if (!rules) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        if (!entry) {
            free_rules_array(rules, i);
            return NEOC_ERROR_INVALID_FORMAT;
        }
        char *entry_json = neoc_json_to_string(entry);
        neoc_witness_rule_t *rule = NULL;
        if (!entry_json || neoc_witness_rule_from_json(entry_json, &rule) != NEOC_SUCCESS) {
            neoc_free(entry_json);
            free_rules_array(rules, i);
            return NEOC_ERROR_INVALID_FORMAT;
        }
        neoc_free(entry_json);
        rules[i] = rule;
    }

    *rules_out = rules;
    *count_out = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_signer_from_json(const char *json_str,
                                               neoc_transaction_signer_t **signer) {
    if (!json_str || !signer) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *signer = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    const char *account_hex = neoc_json_get_string(json, "account");
    if (!account_hex) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }
    neoc_hash160_t account_hash;
    if (neoc_hash160_from_hex(&account_hash, account_hex) != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_witness_scope_t *scopes = NULL;
    size_t scopes_count = 0;
    parse_scope_array(neoc_json_get_array(json, "scopes"), &scopes, &scopes_count);

    char **allowed_contracts = NULL;
    size_t allowed_contracts_count = 0;
    parse_string_array(neoc_json_get_array(json, "allowedcontracts"),
                       &allowed_contracts,
                       &allowed_contracts_count);

    char **allowed_groups = NULL;
    size_t allowed_groups_count = 0;
    parse_string_array(neoc_json_get_array(json, "allowedgroups"),
                       &allowed_groups,
                       &allowed_groups_count);

    neoc_witness_rule_t **rules = NULL;
    size_t rules_count = 0;
    parse_rules_array(neoc_json_get_array(json, "rules"),
                      &rules,
                      &rules_count);

    neoc_error_t err = neoc_transaction_signer_create_full(&account_hash,
                                                           scopes,
                                                           scopes_count,
                                                           (const char **)allowed_contracts,
                                                           allowed_contracts_count,
                                                           (const char **)allowed_groups,
                                                           allowed_groups_count,
                                                           (const neoc_witness_rule_t **)rules,
                                                           rules_count,
                                                           signer);
    neoc_free(scopes);
    free_string_array(allowed_contracts, allowed_contracts_count);
    free_string_array(allowed_groups, allowed_groups_count);
    free_rules_array(rules, rules_count);
    neoc_json_free(json);
    return err;
}

static neoc_error_t scopes_to_json_array(const neoc_witness_scope_t *scopes,
                                         size_t count,
                                         neoc_json_t **out) {
    if (!out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out = neoc_json_create_array();
    if (!*out) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        const char *scope_str = neoc_witness_scope_to_json(scopes[i]);
        if (scope_str) {
            neoc_json_t *item = neoc_json_create_object();
            if (!item) {
                neoc_json_free(*out);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            neoc_json_add_string(item, "value", scope_str);
            neoc_json_array_add(*out, item);
        }
    }
    return NEOC_SUCCESS;
}

static neoc_error_t string_array_to_json(const char *key,
                                         char **array,
                                         size_t count,
                                         neoc_json_t *parent) {
    if (!parent || !key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!array || count == 0) {
        return NEOC_SUCCESS;
    }
    neoc_json_t *arr = neoc_json_create_array();
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        if (array[i]) {
            neoc_json_t *item = neoc_json_create_object();
            if (!item) {
                neoc_json_free(arr);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            neoc_json_add_string(item, "value", array[i]);
            neoc_json_array_add(arr, item);
        }
    }
    neoc_json_add_object(parent, key, arr);
    return NEOC_SUCCESS;
}

static neoc_error_t rules_to_json_array(neoc_witness_rule_t **rules,
                                        size_t count,
                                        neoc_json_t *parent) {
    if (!parent) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!rules || count == 0) {
        return NEOC_SUCCESS;
    }
    neoc_json_t *arr = neoc_json_create_array();
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        if (!rules[i]) {
            continue;
        }
        char *rule_json = NULL;
        if (neoc_witness_rule_to_json(rules[i], &rule_json) == NEOC_SUCCESS && rule_json) {
            neoc_json_t *entry = neoc_json_parse(rule_json);
            neoc_free(rule_json);
            if (entry) {
                neoc_json_array_add(arr, entry);
            }
        }
    }
    neoc_json_add_object(parent, "rules", arr);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_signer_to_json(const neoc_transaction_signer_t *signer,
                                             char **json_str) {
    if (!signer || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (signer->account) {
        char hash_hex[NEOC_HASH160_STRING_LENGTH] = {0};
        if (neoc_hash160_to_hex(signer->account, hash_hex, sizeof(hash_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(root, "account", hash_hex);
        }
    }

    neoc_json_t *scopes_arr = NULL;
    scopes_to_json_array(signer->scopes, signer->scopes_count, &scopes_arr);
    if (scopes_arr) {
        neoc_json_add_object(root, "scopes", scopes_arr);
    }

    string_array_to_json("allowedcontracts", signer->allowed_contracts, signer->allowed_contracts_count, root);
    string_array_to_json("allowedgroups", signer->allowed_groups, signer->allowed_groups_count, root);
    rules_to_json_array(signer->rules, signer->rules_count, root);

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_transaction_signer_copy(const neoc_transaction_signer_t *src,
                                          neoc_transaction_signer_t **dest) {
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    return neoc_transaction_signer_create_full(src->account,
                                               src->scopes,
                                               src->scopes_count,
                                               (const char **)src->allowed_contracts,
                                               src->allowed_contracts_count,
                                               (const char **)src->allowed_groups,
                                               src->allowed_groups_count,
                                               (const neoc_witness_rule_t **)src->rules,
                                               src->rules_count,
                                               dest);
}

bool neoc_transaction_signer_equals(const neoc_transaction_signer_t *signer1,
                                    const neoc_transaction_signer_t *signer2) {
    if (signer1 == signer2) {
        return true;
    }
    if (!signer1 || !signer2 || !signer1->account || !signer2->account) {
        return false;
    }
    if (memcmp(signer1->account->data, signer2->account->data, NEOC_HASH160_SIZE) != 0) {
        return false;
    }
    if (signer1->scopes_count != signer2->scopes_count ||
        signer1->allowed_contracts_count != signer2->allowed_contracts_count ||
        signer1->allowed_groups_count != signer2->allowed_groups_count ||
        signer1->rules_count != signer2->rules_count) {
        return false;
    }
    for (size_t i = 0; i < signer1->scopes_count; i++) {
        if (signer1->scopes[i] != signer2->scopes[i]) {
            return false;
        }
    }
    for (size_t i = 0; i < signer1->allowed_contracts_count; i++) {
        const char *a1 = signer1->allowed_contracts[i] ? signer1->allowed_contracts[i] : "";
        const char *a2 = signer2->allowed_contracts[i] ? signer2->allowed_contracts[i] : "";
        if (strcmp(a1, a2) != 0) {
            return false;
        }
    }
    for (size_t i = 0; i < signer1->allowed_groups_count; i++) {
        const char *g1 = signer1->allowed_groups[i] ? signer1->allowed_groups[i] : "";
        const char *g2 = signer2->allowed_groups[i] ? signer2->allowed_groups[i] : "";
        if (strcmp(g1, g2) != 0) {
            return false;
        }
    }
    for (size_t i = 0; i < signer1->rules_count; i++) {
        if (!neoc_witness_rule_equals(signer1->rules[i], signer2->rules[i])) {
            return false;
        }
    }
    return true;
}
