/**
 * @file neo_get_nep11_balances.c
 * @brief Implementation of the getnep11balances RPC response.
 */

#include "neoc/protocol/core/response/neo_get_nep11_balances.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/decode.h"
#include "neoc/types/neoc_hash160.h"

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static char *neoc_strdup_optional(const char *value, neoc_error_t *err_out) {
    if (!value) {
        return NULL;
    }
    char *copy = neoc_strdup(value);
    if (!copy && err_out) {
        *err_out = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances: string duplication failed");
    }
    return copy;
}

neoc_error_t neoc_nep11_token_create(const char *token_id,
                                     const char *amount,
                                     int last_updated_block,
                                     neoc_nep11_token_t **token_out) {
    if (!token_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_token: output pointer invalid");
    }

    *token_out = NULL;

    neoc_nep11_token_t *token = neoc_calloc(1, sizeof(neoc_nep11_token_t));
    if (!token) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_token: allocation failed");
    }

    neoc_error_t err = NEOC_SUCCESS;
    token->token_id = neoc_strdup_optional(token_id, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_token_free(token);
        return err;
    }

    token->amount = neoc_strdup_optional(amount, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_token_free(token);
        return err;
    }

    token->last_updated_block = last_updated_block;

    *token_out = token;
    return NEOC_SUCCESS;
}

void neoc_nep11_token_free(neoc_nep11_token_t *token) {
    if (!token) {
        return;
    }
    neoc_free(token->token_id);
    neoc_free(token->amount);
    neoc_free(token);
}

static void neoc_nep11_token_array_free(neoc_nep11_token_t *tokens, size_t count) {
    if (!tokens) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        neoc_free(tokens[i].token_id);
        neoc_free(tokens[i].amount);
    }
    neoc_free(tokens);
}

neoc_error_t neoc_nep11_balance_create(const char *name,
                                       const char *symbol,
                                       const char *decimals,
                                       const neoc_hash160_t *asset_hash,
                                       neoc_nep11_token_t *tokens,
                                       size_t tokens_count,
                                       neoc_nep11_balance_t **balance_out) {
    if (!asset_hash || !balance_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_balance: invalid arguments");
    }

    *balance_out = NULL;

    neoc_nep11_balance_t *balance = neoc_calloc(1, sizeof(neoc_nep11_balance_t));
    if (!balance) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balance: allocation failed");
    }

    neoc_error_t err = neoc_hash160_copy(&balance->asset_hash, asset_hash);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_balance_free(balance);
        return err;
    }

    balance->name = neoc_strdup_optional(name, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_balance_free(balance);
        return err;
    }

    balance->symbol = neoc_strdup_optional(symbol, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_balance_free(balance);
        return err;
    }

    balance->decimals = neoc_strdup_optional(decimals, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_balance_free(balance);
        return err;
    }

    balance->tokens = tokens;
    balance->tokens_count = tokens_count;

    *balance_out = balance;
    return NEOC_SUCCESS;
}

static void neoc_nep11_balance_dispose(neoc_nep11_balance_t *balance) {
    if (!balance) {
        return;
    }
    neoc_free(balance->name);
    neoc_free(balance->symbol);
    neoc_free(balance->decimals);
    neoc_nep11_token_array_free(balance->tokens, balance->tokens_count);
    balance->tokens = NULL;
    balance->tokens_count = 0;
}

void neoc_nep11_balance_free(neoc_nep11_balance_t *balance) {
    if (!balance) {
        return;
    }
    neoc_nep11_balance_dispose(balance);
    neoc_free(balance);
}

static void neoc_nep11_balance_array_free(neoc_nep11_balance_t *balances, size_t count) {
    if (!balances) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        neoc_nep11_balance_dispose(&balances[i]);
    }
    neoc_free(balances);
}

neoc_error_t neoc_nep11_balances_create(const char *address,
                                        neoc_nep11_balance_t *balances,
                                        size_t balances_count,
                                        neoc_nep11_balances_t **nep11_balances_out) {
    if (!nep11_balances_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_balances: output pointer invalid");
    }

    *nep11_balances_out = NULL;

    neoc_nep11_balances_t *nep11_balances = neoc_calloc(1, sizeof(neoc_nep11_balances_t));
    if (!nep11_balances) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances: allocation failed");
    }

    neoc_error_t err = NEOC_SUCCESS;
    nep11_balances->address = neoc_strdup_optional(address, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_balances_free(nep11_balances);
        return err;
    }

    nep11_balances->balances = balances;
    nep11_balances->balances_count = balances_count;

    *nep11_balances_out = nep11_balances;
    return NEOC_SUCCESS;
}

void neoc_nep11_balances_free(neoc_nep11_balances_t *nep11_balances) {
    if (!nep11_balances) {
        return;
    }

    neoc_free(nep11_balances->address);
    neoc_nep11_balance_array_free(nep11_balances->balances, nep11_balances->balances_count);
    neoc_free(nep11_balances);
}

neoc_error_t neoc_neo_get_nep11_balances_create(int id,
                                                neoc_nep11_balances_t *nep11_balances,
                                                const char *error,
                                                int error_code,
                                                neoc_neo_get_nep11_balances_t **response_out) {
    if (!response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_balances_response: output pointer invalid");
    }

    *response_out = NULL;

    neoc_neo_get_nep11_balances_t *response = neoc_calloc(1, sizeof(neoc_neo_get_nep11_balances_t));
    if (!response) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: allocation failed");
    }

    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        neoc_neo_get_nep11_balances_free(response);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: jsonrpc allocation failed");
    }

    response->id = id;
    response->result = nep11_balances;
    response->error_code = error_code;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_neo_get_nep11_balances_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: error allocation failed");
        }
    }

    *response_out = response;
    return NEOC_SUCCESS;
}

void neoc_neo_get_nep11_balances_free(neoc_neo_get_nep11_balances_t *response) {
    if (!response) {
        return;
    }

    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    neoc_nep11_balances_free(response->result);
    neoc_free(response);
}

neoc_nep11_balances_t *neoc_neo_get_nep11_balances_get_balances(const neoc_neo_get_nep11_balances_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_nep11_balances_has_balances(const neoc_neo_get_nep11_balances_t *response) {
    return response && response->result != NULL;
}

size_t neoc_neo_get_nep11_balances_get_contracts_count(const neoc_neo_get_nep11_balances_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->balances_count;
}

size_t neoc_neo_get_nep11_balances_get_total_tokens_count(const neoc_neo_get_nep11_balances_t *response) {
    if (!response || !response->result) {
        return 0;
    }

    size_t total = 0;
    for (size_t i = 0; i < response->result->balances_count; ++i) {
        total += response->result->balances[i].tokens_count;
    }
    return total;
}

neoc_nep11_balance_t *neoc_neo_get_nep11_balances_find_contract(const neoc_neo_get_nep11_balances_t *response,
                                                                const neoc_hash160_t *asset_hash) {
    if (!response || !response->result || !asset_hash) {
        return NULL;
    }

    for (size_t i = 0; i < response->result->balances_count; ++i) {
        neoc_nep11_balance_t *balance = &response->result->balances[i];
        if (neoc_hash160_equal(&balance->asset_hash, asset_hash)) {
            return balance;
        }
    }
    return NULL;
}

neoc_nep11_balance_t *neoc_neo_get_nep11_balances_get_balance(const neoc_neo_get_nep11_balances_t *response,
                                                              size_t index) {
    if (!response || !response->result || index >= response->result->balances_count) {
        return NULL;
    }
    return &response->result->balances[index];
}

#ifdef HAVE_CJSON

static neoc_error_t neoc_nep11_parse_tokens(const cJSON *tokens_array,
                                            neoc_nep11_token_t **tokens_out,
                                            size_t *count_out) {
    if (!tokens_out || !count_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_tokens: outputs invalid");
    }

    *tokens_out = NULL;
    *count_out = 0;

    if (!tokens_array || !cJSON_IsArray(tokens_array) || cJSON_GetArraySize(tokens_array) == 0) {
        return NEOC_SUCCESS;
    }

    int count = cJSON_GetArraySize(tokens_array);
    neoc_nep11_token_t *tokens = neoc_calloc((size_t)count, sizeof(neoc_nep11_token_t));
    if (!tokens) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_tokens: allocation failed");
    }

    for (int i = 0; i < count; ++i) {
        cJSON *item = cJSON_GetArrayItem(tokens_array, i);
        if (!item || !cJSON_IsObject(item)) {
            continue;
        }

        cJSON *token_id = cJSON_GetObjectItemCaseSensitive(item, "tokenid");
        if (token_id && cJSON_IsString(token_id) && token_id->valuestring) {
            tokens[i].token_id = neoc_strdup(token_id->valuestring);
            if (!tokens[i].token_id) {
                neoc_nep11_token_array_free(tokens, (size_t)count);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_tokens: token id duplication failed");
            }
        }

        cJSON *amount = cJSON_GetObjectItemCaseSensitive(item, "amount");
        if (amount && cJSON_IsString(amount) && amount->valuestring) {
            tokens[i].amount = neoc_strdup(amount->valuestring);
            if (!tokens[i].amount) {
                neoc_nep11_token_array_free(tokens, (size_t)count);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_tokens: amount duplication failed");
            }
        }

        cJSON *last_block = cJSON_GetObjectItemCaseSensitive(item, "lastupdatedblock");
        if (last_block) {
            if (cJSON_IsNumber(last_block)) {
                tokens[i].last_updated_block = (int64_t)last_block->valuedouble;
            } else if (cJSON_IsString(last_block) && last_block->valuestring) {
                tokens[i].last_updated_block = (int64_t)strtoll(last_block->valuestring, NULL, 10);
            }
        }
    }

    *tokens_out = tokens;
    *count_out = (size_t)count;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_nep11_parse_balances(const cJSON *balance_array,
                                              neoc_nep11_balance_t **balances_out,
                                              size_t *count_out) {
    if (!balances_out || !count_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_balances: outputs invalid");
    }

    *balances_out = NULL;
    *count_out = 0;

    if (!balance_array || !cJSON_IsArray(balance_array) || cJSON_GetArraySize(balance_array) == 0) {
        return NEOC_SUCCESS;
    }

    int count = cJSON_GetArraySize(balance_array);
    neoc_nep11_balance_t *balances = neoc_calloc((size_t)count, sizeof(neoc_nep11_balance_t));
    if (!balances) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances: allocation failed");
    }

    for (int i = 0; i < count; ++i) {
        cJSON *item = cJSON_GetArrayItem(balance_array, i);
        if (!item || !cJSON_IsObject(item)) {
            continue;
        }

        neoc_error_t err = NEOC_SUCCESS;
        balances[i].name = neoc_strdup_optional(cJSON_GetObjectItemCaseSensitive(item, "name") ? cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring : NULL, &err);
        if (err != NEOC_SUCCESS) {
            neoc_nep11_balance_array_free(balances, (size_t)count);
            return err;
        }

        balances[i].symbol = neoc_strdup_optional(cJSON_GetObjectItemCaseSensitive(item, "symbol") ? cJSON_GetObjectItemCaseSensitive(item, "symbol")->valuestring : NULL, &err);
        if (err != NEOC_SUCCESS) {
            neoc_nep11_balance_array_free(balances, (size_t)count);
            return err;
        }

        balances[i].decimals = neoc_strdup_optional(cJSON_GetObjectItemCaseSensitive(item, "decimals") ? cJSON_GetObjectItemCaseSensitive(item, "decimals")->valuestring : NULL, &err);
        if (err != NEOC_SUCCESS) {
            neoc_nep11_balance_array_free(balances, (size_t)count);
            return err;
        }

        const cJSON *asset_hash_item = cJSON_GetObjectItemCaseSensitive(item, "assethash");
        if (asset_hash_item && cJSON_IsString(asset_hash_item) && asset_hash_item->valuestring) {
            neoc_hash160_from_string(asset_hash_item->valuestring, &balances[i].asset_hash);
        }

        neoc_nep11_token_t *tokens = NULL;
        size_t tokens_count = 0;
        err = neoc_nep11_parse_tokens(cJSON_GetObjectItemCaseSensitive(item, "tokens"), &tokens, &tokens_count);
        if (err != NEOC_SUCCESS) {
            neoc_nep11_balance_array_free(balances, (size_t)count);
            return err;
        }
        balances[i].tokens = tokens;
        balances[i].tokens_count = tokens_count;
    }

    *balances_out = balances;
    *count_out = (size_t)count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep11_balances_from_json(const char *json_str,
                                                   neoc_neo_get_nep11_balances_t **response_out) {
    if (!json_str || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_balances_response: from_json invalid arguments");
    }

    *response_out = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "nep11_balances_response: root must be object");
    }

    neoc_neo_get_nep11_balances_t *response = neoc_calloc(1, sizeof(neoc_neo_get_nep11_balances_t));
    if (!response) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: allocation failed");
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_neo_get_nep11_balances_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: jsonrpc duplication failed");
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
                neoc_neo_get_nep11_balances_free(response);
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: error duplication failed");
            }
        }
    }

    if (!response->error_code) {
        const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_obj && cJSON_IsObject(result_obj)) {
            neoc_nep11_balance_t *balances = NULL;
            size_t balances_count = 0;
            neoc_error_t err = neoc_nep11_parse_balances(cJSON_GetObjectItemCaseSensitive(result_obj, "balance"),
                                                         &balances,
                                                         &balances_count);
            if (err != NEOC_SUCCESS) {
                neoc_neo_get_nep11_balances_free(response);
                cJSON_Delete(root);
                return err;
            }

            const cJSON *address_item = cJSON_GetObjectItemCaseSensitive(result_obj, "address");
            const char *address_value = (address_item && cJSON_IsString(address_item)) ? address_item->valuestring : NULL;

            neoc_nep11_balances_t *balances_container = NULL;
            err = neoc_nep11_balances_create(address_value, balances, balances_count, &balances_container);
            if (err != NEOC_SUCCESS) {
                neoc_nep11_balance_array_free(balances, balances_count);
                neoc_neo_get_nep11_balances_free(response);
                cJSON_Delete(root);
                return err;
            }

            response->result = balances_container;
        }
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep11_balances_to_json(const neoc_neo_get_nep11_balances_t *response,
                                                 char **json_str_out) {
    if (!response || !json_str_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_balances_response: to_json invalid arguments");
    }

    *json_str_out = NULL;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: JSON object allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_code || response->error) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: error object allocation failed");
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
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: result object allocation failed");
        }
        cJSON_AddItemToObject(root, "result", result_obj);
        if (response->result->address) {
            cJSON_AddStringToObject(result_obj, "address", response->result->address);
        }

        cJSON *balances_array = cJSON_CreateArray();
        if (!balances_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: balances array allocation failed");
        }
        cJSON_AddItemToObject(result_obj, "balance", balances_array);

        for (size_t i = 0; i < response->result->balances_count; ++i) {
            neoc_nep11_balance_t *balance = &response->result->balances[i];
            cJSON *entry = cJSON_CreateObject();
            if (!entry) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: balance entry allocation failed");
            }

            if (balance->name) cJSON_AddStringToObject(entry, "name", balance->name);
            if (balance->symbol) cJSON_AddStringToObject(entry, "symbol", balance->symbol);
            if (balance->decimals) cJSON_AddStringToObject(entry, "decimals", balance->decimals);

            char hash_buffer[NEOC_HASH160_STRING_LENGTH];
            char prefixed_hash[NEOC_HASH160_STRING_LENGTH + 2];
            if (neoc_hash160_to_string(&balance->asset_hash, hash_buffer, sizeof(hash_buffer)) == NEOC_SUCCESS) {
                prefixed_hash[0] = '0';
                prefixed_hash[1] = 'x';
                memcpy(prefixed_hash + 2, hash_buffer, sizeof(hash_buffer));
                cJSON_AddStringToObject(entry, "assethash", prefixed_hash);
            }

            cJSON *tokens_array = cJSON_CreateArray();
            if (!tokens_array) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: tokens array allocation failed");
            }
            cJSON_AddItemToObject(entry, "tokens", tokens_array);

            for (size_t j = 0; j < balance->tokens_count; ++j) {
                neoc_nep11_token_t *token = &balance->tokens[j];
                cJSON *token_obj = cJSON_CreateObject();
                if (!token_obj) {
                    cJSON_Delete(root);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: token entry allocation failed");
                }

                if (token->token_id) cJSON_AddStringToObject(token_obj, "tokenid", token->token_id);
                if (token->amount) cJSON_AddStringToObject(token_obj, "amount", token->amount);
                cJSON_AddNumberToObject(token_obj, "lastupdatedblock", (double)token->last_updated_block);

                cJSON_AddItemToArray(tokens_array, token_obj);
            }

            cJSON_AddItemToArray(balances_array, entry);
        }
    }

    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!rendered) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: JSON render failed");
    }

    *json_str_out = neoc_strdup(rendered);
    cJSON_free(rendered);

    if (!*json_str_out) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_balances_response: JSON duplication failed");
    }

    return NEOC_SUCCESS;
}

#else

neoc_error_t neoc_neo_get_nep11_balances_from_json(const char *json_str,
                                                   neoc_neo_get_nep11_balances_t **response_out) {
    (void)json_str;
    (void)response_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "nep11_balances_response: JSON parsing requires cJSON");
}

neoc_error_t neoc_neo_get_nep11_balances_to_json(const neoc_neo_get_nep11_balances_t *response,
                                                 char **json_str_out) {
    (void)response;
    (void)json_str_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "nep11_balances_response: JSON serialization requires cJSON");
}

#endif /* HAVE_CJSON */
