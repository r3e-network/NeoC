/**
 * @file neo_get_token_balances.c
 * @brief Implementation of the generic gettokenbalances RPC response.
 */

#include "neoc/protocol/core/response/neo_get_token_balances.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/decode.h"

#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static void neoc_token_balance_dispose(neoc_token_balance_t *balance) {
    if (!balance) {
        return;
    }
    neoc_free(balance->asset_hash);
    balance->asset_hash = NULL;
}

static neoc_error_t neoc_token_balance_set_hash(neoc_token_balance_t *balance,
                                                const neoc_hash160_t *asset_hash) {
    if (!balance || !asset_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balance: invalid hash pointer");
    }
    balance->asset_hash = neoc_malloc(sizeof(neoc_hash160_t));
    if (!balance->asset_hash) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balance: hash allocation failed");
    }
    return neoc_hash160_copy(balance->asset_hash, asset_hash);
}

static void neoc_token_balance_array_free(neoc_token_balance_t *balances, size_t count) {
    if (!balances) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        neoc_token_balance_dispose(&balances[i]);
    }
    neoc_free(balances);
}

neoc_error_t neoc_token_balance_create(const neoc_hash160_t *asset_hash,
                                       neoc_token_balance_t **balance_out) {
    if (!asset_hash || !balance_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balance: invalid arguments");
    }

    *balance_out = NULL;
    neoc_token_balance_t *balance = neoc_calloc(1, sizeof(neoc_token_balance_t));
    if (!balance) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balance: allocation failed");
    }

    neoc_error_t err = neoc_token_balance_set_hash(balance, asset_hash);
    if (err != NEOC_SUCCESS) {
        neoc_token_balance_free(balance);
        return err;
    }

    *balance_out = balance;
    return NEOC_SUCCESS;
}

void neoc_token_balance_free(neoc_token_balance_t *balance) {
    if (!balance) {
        return;
    }
    neoc_token_balance_dispose(balance);
    neoc_free(balance);
}

static neoc_error_t neoc_token_balances_copy_array(neoc_token_balance_t *dest,
                                                   const neoc_token_balance_t *src,
                                                   size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!src[i].asset_hash) {
            continue;
        }
        neoc_error_t err = neoc_token_balance_set_hash(&dest[i], src[i].asset_hash);
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; ++j) {
                neoc_token_balance_dispose(&dest[j]);
            }
            return err;
        }
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_token_balances_create(const char *address,
                                        const neoc_token_balance_t *balances,
                                        size_t balances_count,
                                        neoc_token_balances_t **token_balances_out) {
    if (!token_balances_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances: output pointer invalid");
    }

    *token_balances_out = NULL;

    neoc_token_balances_t *container = neoc_calloc(1, sizeof(neoc_token_balances_t));
    if (!container) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances: allocation failed");
    }

    if (address) {
        container->address = neoc_strdup(address);
        if (!container->address) {
            neoc_token_balances_free(container);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances: address duplication failed");
        }
    }

    if (balances_count > 0 && balances) {
        container->balances = neoc_calloc(balances_count, sizeof(neoc_token_balance_t));
        if (!container->balances) {
            neoc_token_balances_free(container);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances: balances array allocation failed");
        }
        container->balances_count = balances_count;
        neoc_error_t err = neoc_token_balances_copy_array(container->balances, balances, balances_count);
        if (err != NEOC_SUCCESS) {
            neoc_token_balances_free(container);
            return err;
        }
    }

    *token_balances_out = container;
    return NEOC_SUCCESS;
}

void neoc_token_balances_free(neoc_token_balances_t *token_balances) {
    if (!token_balances) {
        return;
    }

    if (token_balances->balances) {
        for (size_t i = 0; i < token_balances->balances_count; ++i) {
            neoc_token_balance_dispose(&token_balances->balances[i]);
        }
        neoc_free(token_balances->balances);
    }
    neoc_free(token_balances->address);
    neoc_free(token_balances);
}

neoc_error_t neoc_neo_get_token_balances_response_create(const char *jsonrpc,
                                                         int id,
                                                         neoc_token_balances_t *result,
                                                         const char *error,
                                                         int error_code,
                                                         neoc_neo_get_token_balances_response_t **response_out) {
    if (!response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances_response: output pointer invalid");
    }

    *response_out = NULL;

    neoc_neo_get_token_balances_response_t *response = neoc_calloc(1, sizeof(neoc_neo_get_token_balances_response_t));
    if (!response) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances_response: allocation failed");
    }

    if (jsonrpc) {
        response->jsonrpc = neoc_strdup(jsonrpc);
        if (!response->jsonrpc) {
            neoc_neo_get_token_balances_response_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances_response: jsonrpc duplication failed");
        }
    }

    response->id = id;
    response->result = result;
    response->error_code = error_code;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_neo_get_token_balances_response_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances_response: error duplication failed");
        }
    }

    *response_out = response;
    return NEOC_SUCCESS;
}

void neoc_neo_get_token_balances_response_free(neoc_neo_get_token_balances_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    neoc_token_balances_free(response->result);
    neoc_free(response);
}

neoc_error_t neoc_token_balances_find_asset(const neoc_token_balances_t *token_balances,
                                            const neoc_hash160_t *asset_hash,
                                            const neoc_token_balance_t **balance_out) {
    if (!token_balances || !asset_hash || !balance_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances: find_asset invalid arguments");
    }

    *balance_out = NULL;
    if (!token_balances->balances) {
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "token_balances: no balances present");
    }

    for (size_t i = 0; i < token_balances->balances_count; ++i) {
        const neoc_token_balance_t *candidate = &token_balances->balances[i];
        if (candidate->asset_hash && neoc_hash160_equal(candidate->asset_hash, asset_hash)) {
            *balance_out = candidate;
            return NEOC_SUCCESS;
        }
    }

    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "token_balances: asset not found");
}

#ifdef HAVE_CJSON

static cJSON *neoc_token_balance_to_json_object(const neoc_token_balance_t *balance,
                                                neoc_error_t *error_out) {
    if (error_out) {
        *error_out = NEOC_SUCCESS;
    }

    if (!balance) {
        if (error_out) {
            *error_out = neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                                        "token_balances: null balance during serialization");
        }
        return NULL;
    }

    cJSON *object = cJSON_CreateObject();
    if (!object) {
        if (error_out) {
            *error_out = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                                        "token_balances: failed to allocate balance JSON");
        }
        return NULL;
    }

    if (balance->asset_hash) {
        char hash_buffer[NEOC_HASH160_STRING_LENGTH] = {0};
        neoc_error_t err = neoc_hash160_to_string(balance->asset_hash,
                                                  hash_buffer,
                                                  sizeof(hash_buffer));
        if (err != NEOC_SUCCESS) {
            if (error_out) {
                *error_out = neoc_error_set(err,
                                            "token_balances: failed to encode asset hash");
            }
            cJSON_Delete(object);
            return NULL;
        }
        cJSON_AddStringToObject(object, "assethash", hash_buffer);
    }

    return object;
}

static neoc_error_t neoc_token_balances_to_json_object_internal(
    const neoc_token_balances_t *token_balances,
    cJSON **object_out) {
    if (!token_balances || !object_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "token_balances: to_json_object invalid arguments");
    }

    *object_out = NULL;

    cJSON *object = cJSON_CreateObject();
    if (!object) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                              "token_balances: failed to allocate JSON object");
    }

    if (token_balances->address) {
        cJSON_AddStringToObject(object, "address", token_balances->address);
    }

    cJSON *balance_array = cJSON_CreateArray();
    if (!balance_array) {
        cJSON_Delete(object);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                              "token_balances: failed to allocate balance array");
    }
    cJSON_AddItemToObject(object, "balance", balance_array);

    if (token_balances->balances && token_balances->balances_count > 0) {
        for (size_t i = 0; i < token_balances->balances_count; ++i) {
            neoc_error_t entry_err = NEOC_SUCCESS;
            cJSON *entry = neoc_token_balance_to_json_object(&token_balances->balances[i], &entry_err);
            if (!entry) {
                cJSON_Delete(object);
                if (entry_err == NEOC_SUCCESS) {
                    entry_err = neoc_error_set(NEOC_ERROR_INVALID_STATE,
                                               "token_balances: balance serialization failed");
                }
                return entry_err;
            }
            cJSON_AddItemToArray(balance_array, entry);
        }
    }

    *object_out = object;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_token_balance_parse_array(const cJSON *array,
                                                   neoc_token_balance_t **balances_out,
                                                   size_t *count_out) {
    if (!balances_out || !count_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances: parse array outputs invalid");
    }

    *balances_out = NULL;
    *count_out = 0;

    if (!array || !cJSON_IsArray(array) || cJSON_GetArraySize(array) == 0) {
        return NEOC_SUCCESS;
    }

    int count = cJSON_GetArraySize(array);
    neoc_token_balance_t *balances = neoc_calloc((size_t)count, sizeof(neoc_token_balance_t));
    if (!balances) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances: balances allocation failed");
    }

    for (int i = 0; i < count; ++i) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        if (!item || !cJSON_IsObject(item)) {
            continue;
        }

        const cJSON *asset_hash_item = cJSON_GetObjectItemCaseSensitive(item, "assethash");
        if (asset_hash_item && cJSON_IsString(asset_hash_item) && asset_hash_item->valuestring) {
            neoc_hash160_t hash;
            neoc_error_t err = neoc_hash160_from_string(asset_hash_item->valuestring, &hash);
            if (err != NEOC_SUCCESS) {
                neoc_token_balance_array_free(balances, (size_t)count);
                return err;
            }
            err = neoc_token_balance_set_hash(&balances[i], &hash);
            if (err != NEOC_SUCCESS) {
                neoc_token_balance_array_free(balances, (size_t)count);
                return err;
            }
        }
    }

    *balances_out = balances;
    *count_out = (size_t)count;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_token_balances_from_json_object(const cJSON *object,
                                                         neoc_token_balances_t **token_balances_out) {
    if (!object || !token_balances_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances: from_json_object invalid arguments");
    }

    *token_balances_out = NULL;

    neoc_token_balances_t *container = neoc_calloc(1, sizeof(neoc_token_balances_t));
    if (!container) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances: container allocation failed");
    }

    const cJSON *address_item = cJSON_GetObjectItemCaseSensitive(object, "address");
    if (address_item && cJSON_IsString(address_item) && address_item->valuestring) {
        container->address = neoc_strdup(address_item->valuestring);
        if (!container->address) {
            neoc_token_balances_free(container);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances: address duplication failed");
        }
    }

    neoc_token_balance_t *balances = NULL;
    size_t balances_count = 0;
    neoc_error_t err = neoc_token_balance_parse_array(cJSON_GetObjectItemCaseSensitive(object, "balance"),
                                                      &balances,
                                                      &balances_count);
    if (err != NEOC_SUCCESS) {
        neoc_token_balances_free(container);
        return err;
    }

    container->balances = balances;
    container->balances_count = balances_count;

    *token_balances_out = container;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_token_balances_from_json(const char *json_str,
                                           neoc_token_balances_t **token_balances_out) {
    if (!json_str || !token_balances_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances: from_json invalid arguments");
    }

    *token_balances_out = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "token_balances: JSON root must be object");
    }

    neoc_error_t err = neoc_token_balances_from_json_object(root, token_balances_out);
    cJSON_Delete(root);
    return err;
}

neoc_error_t neoc_neo_get_token_balances_response_from_json(const char *json_str,
                                                            neoc_neo_get_token_balances_response_t **response_out) {
    if (!json_str || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "token_balances_response: from_json invalid arguments");
    }

    *response_out = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "token_balances_response: JSON root must be object");
    }

    neoc_neo_get_token_balances_response_t *response = neoc_calloc(1, sizeof(neoc_neo_get_token_balances_response_t));
    if (!response) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances_response: allocation failed");
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_neo_get_token_balances_response_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances_response: jsonrpc duplication failed");
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
                neoc_neo_get_token_balances_response_free(response);
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "token_balances_response: error duplication failed");
            }
        }
    }

    if (!response->error_code) {
        const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_obj && cJSON_IsObject(result_obj)) {
            neoc_token_balances_t *balances = NULL;
            neoc_error_t err = neoc_token_balances_from_json_object(result_obj, &balances);
            if (err != NEOC_SUCCESS) {
                neoc_neo_get_token_balances_response_free(response);
                cJSON_Delete(root);
                return err;
            }
            response->result = balances;
        }
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_token_balances_to_json(const neoc_token_balances_t *token_balances,
                                         char **json_string_out) {
    if (!json_string_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "token_balances: to_json output invalid");
    }

    *json_string_out = NULL;

    if (!token_balances) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "token_balances: to_json requires valid container");
    }

    cJSON *object = NULL;
    neoc_error_t err = neoc_token_balances_to_json_object_internal(token_balances, &object);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    char *json_string = cJSON_PrintUnformatted(object);
    if (!json_string) {
        cJSON_Delete(object);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                              "token_balances: failed to serialize JSON");
    }

    *json_string_out = json_string;
    cJSON_Delete(object);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_token_balances_response_to_json(
    const neoc_neo_get_token_balances_response_t *response,
    char **json_string_out) {
    if (!response || !json_string_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "token_balances_response: to_json invalid arguments");
    }

    *json_string_out = NULL;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                              "token_balances_response: JSON root allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_code || response->error) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                                  "token_balances_response: error object allocation failed");
        }
        cJSON_AddItemToObject(root, "error", error_obj);
        cJSON_AddNumberToObject(error_obj, "code", response->error_code);
        if (response->error) {
            cJSON_AddStringToObject(error_obj, "message", response->error);
        }
    } else if (response->result) {
        cJSON *result_obj = NULL;
        neoc_error_t err = neoc_token_balances_to_json_object_internal(response->result, &result_obj);
        if (err != NEOC_SUCCESS) {
            cJSON_Delete(root);
            return err;
        }
        cJSON_AddItemToObject(root, "result", result_obj);
    }

    char *json_string = cJSON_PrintUnformatted(root);
    if (!json_string) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                              "token_balances_response: failed to serialize JSON");
    }

    *json_string_out = json_string;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

#else

neoc_error_t neoc_token_balances_from_json(const char *json_str,
                                           neoc_token_balances_t **token_balances_out) {
    (void)json_str;
    (void)token_balances_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "token_balances: JSON parsing requires cJSON");
}

neoc_error_t neoc_token_balances_to_json(const neoc_token_balances_t *token_balances,
                                         char **json_string_out) {
    (void)token_balances;
    (void)json_string_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "token_balances: JSON serialization requires cJSON");
}

neoc_error_t neoc_neo_get_token_balances_response_from_json(const char *json_str,
                                                            neoc_neo_get_token_balances_response_t **response_out) {
    (void)json_str;
    (void)response_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "token_balances_response: JSON parsing requires cJSON");
}

neoc_error_t neoc_neo_get_token_balances_response_to_json(
    const neoc_neo_get_token_balances_response_t *response,
    char **json_string_out) {
    (void)response;
    (void)json_string_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "token_balances_response: JSON serialization requires cJSON");
}

#endif /* HAVE_CJSON */
