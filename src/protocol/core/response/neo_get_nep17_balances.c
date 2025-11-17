/**
 * @file neo_get_nep17_balances.c
 * @brief Implementation for the getnep17balances RPC response.
 */

#include "neoc/protocol/core/response/neo_get_nep17_balances.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/decode.h"

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static char *neoc_strdup_or_fail(const char *source, neoc_error_t *err_out) {
    if (!source) {
        return NULL;
    }
    char *copy = neoc_strdup(source);
    if (!copy && err_out) {
        *err_out = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances: failed to duplicate string");
    }
    return copy;
}

neoc_error_t neoc_nep17_balance_create(const neoc_hash160_t *asset_hash,
                                       const char *name,
                                       const char *symbol,
                                       const char *decimals,
                                       const char *amount,
                                       uint64_t last_updated_block,
                                       neoc_nep17_balance_t **balance_out) {
    if (!asset_hash || !amount || !balance_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balance: invalid arguments");
    }

    *balance_out = NULL;

    neoc_nep17_balance_t *balance = neoc_calloc(1, sizeof(neoc_nep17_balance_t));
    if (!balance) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balance: allocation failed");
    }

    neoc_error_t err = neoc_hash160_copy(&balance->asset_hash, asset_hash);
    if (err != NEOC_SUCCESS) {
        neoc_nep17_balance_free(balance);
        return err;
    }

    balance->name = neoc_strdup_or_fail(name, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep17_balance_free(balance);
        return err;
    }

    balance->symbol = neoc_strdup_or_fail(symbol, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep17_balance_free(balance);
        return err;
    }

    balance->decimals = neoc_strdup_or_fail(decimals, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep17_balance_free(balance);
        return err;
    }

    balance->amount = neoc_strdup_or_fail(amount, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep17_balance_free(balance);
        return err;
    }

    balance->last_updated_block = last_updated_block;

    *balance_out = balance;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep17_balances_create(const char *address,
                                        neoc_nep17_balances_t **balances_out) {
    if (!balances_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balances: output pointer invalid");
    }

    *balances_out = NULL;

    neoc_nep17_balances_t *balances = neoc_calloc(1, sizeof(neoc_nep17_balances_t));
    if (!balances) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances: allocation failed");
    }

    neoc_error_t err = NEOC_SUCCESS;
    balances->address = neoc_strdup_or_fail(address, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep17_balances_free(balances);
        return err;
    }

    *balances_out = balances;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep17_balances_add_balance(neoc_nep17_balances_t *balances,
                                             neoc_nep17_balance_t *balance) {
    if (!balances || !balance) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balances: add_balance invalid arguments");
    }

    neoc_nep17_balance_t **new_array = neoc_realloc(
        balances->balances,
        sizeof(neoc_nep17_balance_t *) * (balances->balance_count + 1));
    if (!new_array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances: reallocation failed");
    }

    balances->balances = new_array;
    balances->balances[balances->balance_count] = balance;
    balances->balance_count += 1;

    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep17_balances_response_create(int id,
                                                         neoc_nep17_balances_t *result,
                                                         const char *error,
                                                         int error_code,
                                                         neoc_neo_get_nep17_balances_response_t **response_out) {
    if (!response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balances_response: output pointer invalid");
    }

    *response_out = NULL;

    neoc_neo_get_nep17_balances_response_t *response =
        neoc_calloc(1, sizeof(neoc_neo_get_nep17_balances_response_t));
    if (!response) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: allocation failed");
    }

    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        neoc_neo_get_nep17_balances_response_free(response);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: jsonrpc allocation failed");
    }

    response->id = id;
    response->result = result;
    response->error_code = error_code;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_neo_get_nep17_balances_response_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: error allocation failed");
        }
    }

    *response_out = response;
    return NEOC_SUCCESS;
}

static void neoc_nep17_balance_dispose(neoc_nep17_balance_t *balance) {
    if (!balance) {
        return;
    }
    neoc_free(balance->name);
    neoc_free(balance->symbol);
    neoc_free(balance->decimals);
    neoc_free(balance->amount);
}

void neoc_nep17_balance_free(neoc_nep17_balance_t *balance) {
    if (!balance) {
        return;
    }
    neoc_nep17_balance_dispose(balance);
    neoc_free(balance);
}

void neoc_nep17_balances_free(neoc_nep17_balances_t *balances) {
    if (!balances) {
        return;
    }

    if (balances->balances) {
        for (size_t i = 0; i < balances->balance_count; ++i) {
            neoc_nep17_balance_free(balances->balances[i]);
        }
        neoc_free(balances->balances);
    }

    neoc_free(balances->address);
    neoc_free(balances);
}

void neoc_neo_get_nep17_balances_response_free(neoc_neo_get_nep17_balances_response_t *response) {
    if (!response) {
        return;
    }

    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    neoc_nep17_balances_free(response->result);
    neoc_free(response);
}

bool neoc_neo_get_nep17_balances_response_is_success(const neoc_neo_get_nep17_balances_response_t *response) {
    return response && response->error_code == 0 && response->error == NULL && response->result != NULL;
}

size_t neoc_nep17_balances_get_token_count(const neoc_nep17_balances_t *balances) {
    return balances ? balances->balance_count : 0;
}

neoc_error_t neoc_nep17_balances_get_balance(const neoc_nep17_balances_t *balances,
                                             const neoc_hash160_t *asset_hash,
                                             const neoc_nep17_balance_t **balance_out) {
    if (!balances || !asset_hash || !balance_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balances: get_balance invalid arguments");
    }

    *balance_out = NULL;

    for (size_t i = 0; i < balances->balance_count; ++i) {
        neoc_nep17_balance_t *candidate = balances->balances[i];
        if (candidate && neoc_hash160_equal(&candidate->asset_hash, asset_hash)) {
            *balance_out = candidate;
            return NEOC_SUCCESS;
        }
    }

    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "nep17_balances: asset hash not found");
}

neoc_error_t neoc_nep17_balance_copy(const neoc_nep17_balance_t *source,
                                     neoc_nep17_balance_t **copy_out) {
    if (!source || !copy_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balance: copy invalid arguments");
    }

    return neoc_nep17_balance_create(&source->asset_hash,
                                     source->name,
                                     source->symbol,
                                     source->decimals,
                                     source->amount,
                                     source->last_updated_block,
                                     copy_out);
}

#ifdef HAVE_CJSON

static uint64_t neoc_nep17_parse_last_updated_block(const cJSON *item) {
    if (!item) {
        return 0;
    }

    if (cJSON_IsNumber(item)) {
        if (item->valuedouble < 0) {
            return 0;
        }
        return (uint64_t)item->valuedouble;
    }

    if (cJSON_IsString(item) && item->valuestring) {
        return (uint64_t)strtoull(item->valuestring, NULL, 10);
    }

    return 0;
}

static neoc_error_t neoc_nep17_parse_amount_string(const cJSON *item, char **amount_out) {
    if (!item || !amount_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balance: amount parse invalid arguments");
    }

    *amount_out = NULL;

    if (cJSON_IsString(item) && item->valuestring) {
        *amount_out = neoc_strdup(item->valuestring);
        if (!*amount_out) {
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balance: amount duplication failed");
        }
        return NEOC_SUCCESS;
    }

    if (cJSON_IsNumber(item)) {
        char buffer[64];
        int printed = snprintf(buffer, sizeof(buffer), "%.0f", item->valuedouble);
        if (printed < 0 || (size_t)printed >= sizeof(buffer)) {
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "nep17_balance: amount conversion failed");
        }
        *amount_out = neoc_strdup(buffer);
        if (!*amount_out) {
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balance: amount duplication failed");
        }
        return NEOC_SUCCESS;
    }

    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "nep17_balance: amount must be string or number");
}

static char *neoc_nep17_optional_string(const cJSON *object, const char *key) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);
    if (item && cJSON_IsString(item) && item->valuestring) {
        return neoc_strdup(item->valuestring);
    }
    return NULL;
}

static neoc_error_t neoc_nep17_parse_balance(const cJSON *item,
                                             neoc_nep17_balance_t **balance_out) {
    if (!item || !balance_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balance: parse invalid arguments");
    }

    const cJSON *asset_hash_item = cJSON_GetObjectItemCaseSensitive(item, "assethash");
    if (!asset_hash_item || !cJSON_IsString(asset_hash_item) || !asset_hash_item->valuestring) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "nep17_balance: assethash missing");
    }

    neoc_hash160_t asset_hash;
    neoc_error_t err = neoc_hash160_from_string(asset_hash_item->valuestring, &asset_hash);
    if (err != NEOC_SUCCESS) {
        return neoc_error_set(err, "nep17_balance: invalid asset hash");
    }

    char *amount = NULL;
    err = neoc_nep17_parse_amount_string(cJSON_GetObjectItemCaseSensitive(item, "amount"), &amount);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    char *name = neoc_nep17_optional_string(item, "name");
    char *symbol = neoc_nep17_optional_string(item, "symbol");
    char *decimals = neoc_nep17_optional_string(item, "decimals");

    uint64_t last_updated_block = neoc_nep17_parse_last_updated_block(
        cJSON_GetObjectItemCaseSensitive(item, "lastupdatedblock"));

    neoc_nep17_balance_t *balance = NULL;
    err = neoc_nep17_balance_create(&asset_hash,
                                    name,
                                    symbol,
                                    decimals,
                                    amount,
                                    last_updated_block,
                                    &balance);

    neoc_free(name);
    neoc_free(symbol);
    neoc_free(decimals);
    neoc_free(amount);

    if (err != NEOC_SUCCESS) {
        return err;
    }

    *balance_out = balance;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep17_balances_response_from_json(const char *json_string,
                                                            neoc_neo_get_nep17_balances_response_t **response_out) {
    if (!json_string || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balances_response: from_json invalid arguments");
    }

    *response_out = NULL;

    cJSON *root = cJSON_Parse(json_string);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "nep17_balances_response: JSON root must be object");
    }

    neoc_neo_get_nep17_balances_response_t *response =
        neoc_calloc(1, sizeof(neoc_neo_get_nep17_balances_response_t));
    if (!response) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: allocation failed");
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_neo_get_nep17_balances_response_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: jsonrpc duplication failed");
        }
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (id_item) {
        if (cJSON_IsNumber(id_item)) {
            response->id = id_item->valueint;
        } else if (cJSON_IsString(id_item) && id_item->valuestring) {
            neoc_decode_int_from_string(id_item->valuestring, &response->id);
        }
    }

    const cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (error_obj && cJSON_IsObject(error_obj)) {
        const cJSON *code_item = cJSON_GetObjectItemCaseSensitive(error_obj, "code");
        if (code_item && cJSON_IsNumber(code_item)) {
            response->error_code = code_item->valueint;
        }

        const cJSON *message_item = cJSON_GetObjectItemCaseSensitive(error_obj, "message");
        if (message_item && cJSON_IsString(message_item) && message_item->valuestring) {
            response->error = neoc_strdup(message_item->valuestring);
            if (!response->error) {
                neoc_neo_get_nep17_balances_response_free(response);
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: error duplication failed");
            }
        }
    }

    if (!response->error_code) {
        const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_obj && cJSON_IsObject(result_obj)) {
            const cJSON *address_item = cJSON_GetObjectItemCaseSensitive(result_obj, "address");
            const char *address_value = (address_item && cJSON_IsString(address_item)) ? address_item->valuestring : NULL;

            neoc_nep17_balances_t *balances = NULL;
            neoc_error_t err = neoc_nep17_balances_create(address_value, &balances);
            if (err != NEOC_SUCCESS) {
                neoc_neo_get_nep17_balances_response_free(response);
                cJSON_Delete(root);
                return err;
            }

            const cJSON *balance_array = cJSON_GetObjectItemCaseSensitive(result_obj, "balance");
            if (balance_array && cJSON_IsArray(balance_array)) {
                const int count = cJSON_GetArraySize(balance_array);
                for (int i = 0; i < count; ++i) {
                    const cJSON *entry = cJSON_GetArrayItem(balance_array, i);
                    if (!entry || !cJSON_IsObject(entry)) {
                        continue;
                    }

                    neoc_nep17_balance_t *balance = NULL;
                    err = neoc_nep17_parse_balance(entry, &balance);
                    if (err != NEOC_SUCCESS) {
                        neoc_nep17_balances_free(balances);
                        neoc_neo_get_nep17_balances_response_free(response);
                        cJSON_Delete(root);
                        return err;
                    }

                    err = neoc_nep17_balances_add_balance(balances, balance);
                    if (err != NEOC_SUCCESS) {
                        neoc_nep17_balance_free(balance);
                        neoc_nep17_balances_free(balances);
                        neoc_neo_get_nep17_balances_response_free(response);
                        cJSON_Delete(root);
                        return err;
                    }
                }
            }

            response->result = balances;
        }
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep17_balances_response_to_json(const neoc_neo_get_nep17_balances_response_t *response,
                                                          char **json_string_out) {
    if (!response || !json_string_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep17_balances_response: to_json invalid arguments");
    }

    *json_string_out = NULL;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: JSON object allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_code || response->error) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: error object allocation failed");
        }
        cJSON_AddItemToObject(root, "error", error_obj);
        cJSON_AddNumberToObject(error_obj, "code", response->error_code);
        if (response->error) {
            cJSON_AddStringToObject(error_obj, "message", response->error);
        }
    } else if (response->result) {
        cJSON *result_obj = cJSON_CreateObject();
        if (!result_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: result object allocation failed");
        }
        cJSON_AddItemToObject(root, "result", result_obj);
        if (response->result->address) {
            cJSON_AddStringToObject(result_obj, "address", response->result->address);
        }

        cJSON *balance_array = cJSON_CreateArray();
        if (!balance_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: balance array allocation failed");
        }
        cJSON_AddItemToObject(result_obj, "balance", balance_array);

        for (size_t i = 0; i < response->result->balance_count; ++i) {
            const neoc_nep17_balance_t *balance = response->result->balances[i];
            if (!balance) {
                continue;
            }

            cJSON *entry = cJSON_CreateObject();
            if (!entry) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: balance entry allocation failed");
            }

            char hash_buffer[NEOC_HASH160_STRING_LENGTH];
            char prefixed_hash[NEOC_HASH160_STRING_LENGTH + 2];
            if (neoc_hash160_to_string(&balance->asset_hash, hash_buffer, sizeof(hash_buffer)) == NEOC_SUCCESS) {
                prefixed_hash[0] = '0';
                prefixed_hash[1] = 'x';
                memcpy(prefixed_hash + 2, hash_buffer, sizeof(hash_buffer));
                cJSON_AddStringToObject(entry, "assethash", prefixed_hash);
            }

            if (balance->name) cJSON_AddStringToObject(entry, "name", balance->name);
            if (balance->symbol) cJSON_AddStringToObject(entry, "symbol", balance->symbol);
            if (balance->decimals) cJSON_AddStringToObject(entry, "decimals", balance->decimals);
            if (balance->amount) cJSON_AddStringToObject(entry, "amount", balance->amount);
            cJSON_AddNumberToObject(entry, "lastupdatedblock", (double)balance->last_updated_block);

            cJSON_AddItemToArray(balance_array, entry);
        }
    }

    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!rendered) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: JSON render failed");
    }

    *json_string_out = neoc_strdup(rendered);
    cJSON_free(rendered);

    if (!*json_string_out) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep17_balances_response: JSON duplication failed");
    }

    return NEOC_SUCCESS;
}

#else

neoc_error_t neoc_neo_get_nep17_balances_response_from_json(const char *json_string,
                                                            neoc_neo_get_nep17_balances_response_t **response_out) {
    (void)json_string;
    (void)response_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "nep17_balances_response: JSON parsing requires cJSON");
}

neoc_error_t neoc_neo_get_nep17_balances_response_to_json(const neoc_neo_get_nep17_balances_response_t *response,
                                                          char **json_string_out) {
    (void)response;
    (void)json_string_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "nep17_balances_response: JSON serialization requires cJSON");
}

#endif /* HAVE_CJSON */
