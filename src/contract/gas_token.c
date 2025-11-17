/**
 * @file gas_token.c
 * @brief GAS token implementation
 */

#include "neoc/contract/gas_token.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include "neoc/script/script_helper.h"
#include <string.h>

/**
 * GAS native contract script hash (little endian)
 * Matches Neo N3 mainnet deployment.
 */
const neoc_hash160_t NEOC_GAS_TOKEN_HASH = {
    .data = {
        0xd2, 0xa4, 0xcf, 0xf3, 0x19, 0x13, 0x01, 0x61,
        0x55, 0xe3, 0x8e, 0x47, 0x4a, 0x2c, 0x06, 0xd0,
        0x8b, 0xe2, 0x76, 0xcf
    }
};

static const uint64_t NEOC_GAS_TOKEN_INITIAL_SUPPLY = 5200000000000000ULL;

static neoc_error_t neoc_gas_token_validate(neoc_gas_token_t *token) {
    if (!token || !token->fungible) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "GAS token is not initialised");
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_create(neoc_gas_token_t **token) {
    if (!token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Token pointer cannot be NULL");
    }

    neoc_gas_token_t *instance = neoc_calloc(1, sizeof(neoc_gas_token_t));
    if (!instance) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate GAS token wrapper");
    }

    neoc_fungible_token_t *fungible = NULL;
    neoc_hash160_t hash_copy;
    neoc_error_t err = neoc_hash160_copy(&hash_copy, &NEOC_GAS_TOKEN_HASH);
    if (err != NEOC_SUCCESS) {
        neoc_free(instance);
        return err;
    }

    err = neoc_fungible_token_create(&hash_copy, &fungible);
    if (err != NEOC_SUCCESS) {
        neoc_free(instance);
        return err;
    }

    fungible->decimals = NEOC_GAS_TOKEN_DECIMALS;
    fungible->total_supply = NEOC_GAS_TOKEN_INITIAL_SUPPLY;

    if (fungible->base.name) {
        neoc_free(fungible->base.name);
    }
    if (fungible->base.symbol) {
        neoc_free(fungible->base.symbol);
    }

    fungible->base.name = neoc_strdup(NEOC_GAS_TOKEN_NAME);
    fungible->base.symbol = neoc_strdup(NEOC_GAS_TOKEN_SYMBOL);

    if (!fungible->base.name || !fungible->base.symbol) {
        neoc_fungible_token_free(fungible);
        neoc_free(instance);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to duplicate GAS token metadata");
    }

    instance->fungible = fungible;
    *token = instance;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_balance_of(neoc_gas_token_t *token,
                                       neoc_rpc_client_t *rpc_client,
                                       const neoc_hash160_t *account,
                                       uint64_t *balance) {
    if (!token || !rpc_client || !account || !balance) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Token, RPC client, account, and balance output are required");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_nep17_balance_t *balances = NULL;
    size_t balance_count = 0;
    err = neoc_rpc_get_nep17_balances(rpc_client, account, &balances, &balance_count);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint64_t gas_balance = 0;
    for (size_t i = 0; i < balance_count; ++i) {
        if (memcmp(&balances[i].asset_hash, &NEOC_GAS_TOKEN_HASH, sizeof(neoc_hash160_t)) == 0) {
            const char *amount_str = balances[i].amount ? balances[i].amount : "0";
            gas_balance = (uint64_t)strtoull(amount_str, NULL, 10);
            break;
        }
    }

    neoc_rpc_nep17_balances_free(balances, balance_count);
    *balance = gas_balance;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_transfer(neoc_gas_token_t *token,
                                     const neoc_hash160_t *from,
                                     const neoc_hash160_t *to,
                                     uint64_t amount,
                                     const uint8_t *data,
                                     size_t data_len) {
    (void)data;
    (void)data_len;
    if (!from || !to) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "From and to addresses are required");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (amount == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Transfer amount must be greater than zero");
    }

    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "Direct transfer invocation is not implemented. "
                          "Use neoc_gas_token_build_transfer_script instead.");
}

neoc_error_t neoc_gas_token_refuel(neoc_hash160_t *account,
                                   int64_t amount) {
    if (!account || amount <= 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Account and positive amount are required");
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "GAS refuel helper requires RPC integration");
}

neoc_error_t neoc_gas_token_get_name(neoc_gas_token_t *token, char **name) {
    if (!name) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Name output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    const char *source = token->fungible->base.name ? token->fungible->base.name : NEOC_GAS_TOKEN_NAME;
    *name = neoc_strdup(source);
    if (!*name) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate GAS token name");
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_get_symbol(neoc_gas_token_t *token, char **symbol) {
    if (!symbol) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Symbol output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    const char *source = token->fungible->base.symbol ? token->fungible->base.symbol : NEOC_GAS_TOKEN_SYMBOL;
    *symbol = neoc_strdup(source);
    if (!*symbol) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate GAS token symbol");
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_get_decimals(neoc_gas_token_t *token, int *decimals) {
    if (!decimals) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Decimals output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    *decimals = token->fungible->decimals;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_get_script_hash(neoc_gas_token_t *token,
                                            neoc_hash160_t *script_hash) {
    if (!script_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Script hash output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    return neoc_hash160_copy(script_hash, &NEOC_GAS_TOKEN_HASH);
}

neoc_error_t neoc_gas_token_get_total_supply(neoc_gas_token_t *token,
                                             uint64_t *total_supply) {
    if (!total_supply) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Total supply output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    *total_supply = token->fungible->total_supply > 0
                        ? (uint64_t)token->fungible->total_supply
                        : NEOC_GAS_TOKEN_INITIAL_SUPPLY;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_build_transfer_script(
    neoc_gas_token_t *token,
    const neoc_hash160_t *from,
    const neoc_hash160_t *to,
    uint64_t amount,
    const uint8_t *data,
    size_t data_len,
    uint8_t **script,
    size_t *script_len) {
    if (!from || !to || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transfer script arguments");
    }
    if (amount == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Transfer amount must be greater than zero");
    }

    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_script_create_nep17_transfer(&NEOC_GAS_TOKEN_HASH,
                                             from,
                                             to,
                                             amount,
                                             data,
                                             data_len,
                                             script,
                                             script_len);
}

neoc_error_t neoc_gas_token_build_multi_transfer_script(
    neoc_gas_token_t *token,
    const neoc_hash160_t *from,
    const neoc_gas_token_transfer_request_t *transfers,
    size_t transfer_count,
    uint8_t **script,
    size_t *script_len) {
    if (!from || !transfers || transfer_count == 0 || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid multi-transfer arguments");
    }

    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint8_t *combined = NULL;
    size_t combined_len = 0;

    for (size_t i = 0; i < transfer_count; i++) {
        const neoc_gas_token_transfer_request_t *request = &transfers[i];
        if (request->amount == 0) {
            neoc_free(combined);
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Transfer amount must be greater than zero");
        }

        uint8_t *single_script = NULL;
        size_t single_len = 0;
        err = neoc_script_create_nep17_transfer(&NEOC_GAS_TOKEN_HASH,
                                                from,
                                                &request->to,
                                                request->amount,
                                                request->data,
                                                request->data_len,
                                                &single_script,
                                                &single_len);
        if (err != NEOC_SUCCESS) {
            neoc_free(combined);
            return err;
        }

        uint8_t *resized = neoc_realloc(combined, combined_len + single_len);
        if (!resized) {
            neoc_free(single_script);
            neoc_free(combined);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to expand multi-transfer script buffer");
        }

        memcpy(resized + combined_len, single_script, single_len);
        combined = resized;
        combined_len += single_len;
        neoc_free(single_script);
    }

    *script = combined;
    *script_len = combined_len;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_is_native_contract(neoc_gas_token_t *token,
                                               bool *is_native) {
    if (!is_native) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "is_native output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    *is_native = true;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_get_contract_version(neoc_gas_token_t *token,
                                                 int *version) {
    if (!version) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Version output pointer cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    *version = 0;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_gas_token_get_supported_methods(neoc_gas_token_t *token,
                                                  char ***methods,
                                                  size_t *method_count) {
    if (!methods || !method_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Methods output parameters cannot be NULL");
    }
    neoc_error_t err = neoc_gas_token_validate(token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    static const char *kMethods[] = {
        "symbol",
        "decimals",
        "totalSupply",
        "balanceOf",
        "transfer",
        "refuel"
    };
    const size_t count = sizeof(kMethods) / sizeof(kMethods[0]);

    char **result = neoc_calloc(count, sizeof(char *));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate methods array");
    }

    for (size_t i = 0; i < count; i++) {
        result[i] = neoc_strdup(kMethods[i]);
        if (!result[i]) {
            for (size_t j = 0; j < i; j++) {
                neoc_free(result[j]);
            }
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to duplicate method name");
        }
    }

    *methods = result;
    *method_count = count;
    return NEOC_SUCCESS;
}

void neoc_gas_token_free(neoc_gas_token_t *token) {
    if (!token) {
        return;
    }

    if (token->fungible) {
        neoc_fungible_token_free(token->fungible);
        token->fungible = NULL;
    }
    neoc_free(token);
}
