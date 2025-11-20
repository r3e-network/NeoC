/**
 * @file nep17_contract.c
 * @brief NEP-17 contract metadata implementation
 */

#include "neoc/protocol/core/response/nep17_contract.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/json.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_nep17_contract_create(const neoc_hash160_t *script_hash,
                                        const char *symbol,
                                        int decimals,
                                        neoc_nep17_contract_t **contract) {
    if (!script_hash || !symbol || !contract) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *contract = neoc_calloc(1, sizeof(neoc_nep17_contract_t));
    if (!*contract) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*contract)->script_hash = neoc_malloc(sizeof(neoc_hash160_t));
    if (!(*contract)->script_hash) {
        neoc_nep17_contract_free(*contract);
        *contract = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    *(*contract)->script_hash = *script_hash;

    (*contract)->symbol = dup_string(symbol);
    if (!(*contract)->symbol) {
        neoc_nep17_contract_free(*contract);
        *contract = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*contract)->decimals = decimals;
    return NEOC_SUCCESS;
}

void neoc_nep17_contract_free(neoc_nep17_contract_t *contract) {
    if (!contract) {
        return;
    }
    if (contract->script_hash) {
        neoc_free(contract->script_hash);
    }
    neoc_free(contract->symbol);
    neoc_free(contract);
}

neoc_error_t neoc_nep17_contract_from_json(const char *json_str,
                                           neoc_nep17_contract_t **contract) {
    if (!json_str || !contract) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *contract = NULL;

    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    const char *hash_hex = neoc_json_get_string(json, "hash");
    if (!hash_hex) {
        hash_hex = neoc_json_get_string(json, "scriptHash");
    }
    const char *symbol = neoc_json_get_string(json, "symbol");
    int64_t decimals_val = 0;
    neoc_json_get_int(json, "decimals", &decimals_val);

    if (!hash_hex || !symbol) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_hash160_t hash;
    if (neoc_hash160_from_hex(&hash, hash_hex) != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_error_t err = neoc_nep17_contract_create(&hash, symbol, (int)decimals_val, contract);
    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_nep17_contract_to_json(const neoc_nep17_contract_t *contract,
                                         char **json_str) {
    if (!contract || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (contract->script_hash) {
        char hash_hex[NEOC_HASH160_STRING_LENGTH] = {0};
        if (neoc_hash160_to_hex(contract->script_hash, hash_hex, sizeof(hash_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(json, "hash", hash_hex);
        }
    }
    if (contract->symbol) {
        neoc_json_add_string(json, "symbol", contract->symbol);
    }
    neoc_json_add_int(json, "decimals", contract->decimals);

    *json_str = neoc_json_to_string(json);
    neoc_json_free(json);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_nep17_contract_copy(const neoc_nep17_contract_t *src,
                                      neoc_nep17_contract_t **dest) {
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    return neoc_nep17_contract_create(src->script_hash, src->symbol, src->decimals, dest);
}

bool neoc_nep17_contract_equals(const neoc_nep17_contract_t *contract1,
                                const neoc_nep17_contract_t *contract2) {
    if (contract1 == contract2) {
        return true;
    }
    if (!contract1 || !contract2 || !contract1->script_hash || !contract2->script_hash) {
        return false;
    }
    if (contract1->decimals != contract2->decimals) {
        return false;
    }
    if ((contract1->symbol && !contract2->symbol) || (!contract1->symbol && contract2->symbol)) {
        return false;
    }
    if (contract1->symbol && contract2->symbol && strcmp(contract1->symbol, contract2->symbol) != 0) {
        return false;
    }
    return memcmp(contract1->script_hash->data, contract2->script_hash->data, NEOC_HASH160_SIZE) == 0;
}

neoc_error_t neoc_nep17_contract_get_min_unit(const neoc_nep17_contract_t *contract,
                                              double *min_unit) {
    if (!contract || !min_unit) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *min_unit = pow(10.0, -(double)contract->decimals);
    return NEOC_SUCCESS;
}
