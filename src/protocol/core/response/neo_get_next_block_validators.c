/**
 * @file neo_get_next_block_validators.c
 * @brief Implementation of the getnextblockvalidators RPC response.
 */

#include "neoc/protocol/core/response/neo_get_next_block_validators.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/decode.h"

#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static char *neoc_validator_strdup(const char *value, neoc_error_t *err_out) {
    if (!value) {
        return NULL;
    }
    char *copy = neoc_strdup(value);
    if (!copy && err_out) {
        *err_out = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators: string duplication failed");
    }
    return copy;
}

neoc_error_t neoc_validator_create(const char *public_key,
                                   const char *votes,
                                   bool active,
                                   neoc_validator_t **validator_out) {
    if (!validator_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "validators: output pointer invalid");
    }

    *validator_out = NULL;

    neoc_validator_t *validator = neoc_calloc(1, sizeof(neoc_validator_t));
    if (!validator) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators: allocation failed");
    }

    neoc_error_t err = NEOC_SUCCESS;
    validator->public_key = neoc_validator_strdup(public_key, &err);
    if (err != NEOC_SUCCESS) {
        neoc_validator_free(validator);
        return err;
    }

    validator->votes = neoc_validator_strdup(votes, &err);
    if (err != NEOC_SUCCESS) {
        neoc_validator_free(validator);
        return err;
    }

    validator->active = active;

    *validator_out = validator;
    return NEOC_SUCCESS;
}

void neoc_validator_free(neoc_validator_t *validator) {
    if (!validator) {
        return;
    }
    neoc_free(validator->public_key);
    neoc_free(validator->votes);
    neoc_free(validator);
}

static void neoc_validator_array_free(neoc_validator_t *validators, size_t count) {
    if (!validators) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        neoc_free(validators[i].public_key);
        neoc_free(validators[i].votes);
    }
    neoc_free(validators);
}

neoc_error_t neoc_neo_get_next_block_validators_create(int id,
                                                        neoc_validator_t *validators,
                                                        size_t validators_count,
                                                        const char *error,
                                                        int error_code,
                                                        neoc_neo_get_next_block_validators_t **response_out) {
    if (!response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "validators_response: output pointer invalid");
    }

    *response_out = NULL;

    neoc_neo_get_next_block_validators_t *response = neoc_calloc(1, sizeof(neoc_neo_get_next_block_validators_t));
    if (!response) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: allocation failed");
    }

    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        neoc_neo_get_next_block_validators_free(response);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: jsonrpc allocation failed");
    }

    response->id = id;
    response->result = validators;
    response->result_count = validators_count;
    response->error_code = error_code;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_neo_get_next_block_validators_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: error duplication failed");
        }
    }

    *response_out = response;
    return NEOC_SUCCESS;
}

void neoc_neo_get_next_block_validators_free(neoc_neo_get_next_block_validators_t *response) {
    if (!response) {
        return;
    }

    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    neoc_validator_array_free(response->result, response->result_count);
    neoc_free(response);
}

neoc_validator_t *neoc_neo_get_next_block_validators_get_validators(const neoc_neo_get_next_block_validators_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_next_block_validators_has_validators(const neoc_neo_get_next_block_validators_t *response) {
    return response && response->result != NULL;
}

size_t neoc_neo_get_next_block_validators_get_count(const neoc_neo_get_next_block_validators_t *response) {
    if (!response) {
        return 0;
    }
    return response->result_count;
}

neoc_validator_t *neoc_neo_get_next_block_validators_get_validator(const neoc_neo_get_next_block_validators_t *response,
                                                                   size_t index) {
    if (!response || index >= response->result_count) {
        return NULL;
    }
    return &response->result[index];
}

neoc_validator_t *neoc_neo_get_next_block_validators_find_validator(const neoc_neo_get_next_block_validators_t *response,
                                                                    const char *public_key) {
    if (!response || !public_key) {
        return NULL;
    }
    for (size_t i = 0; i < response->result_count; ++i) {
        neoc_validator_t *validator = &response->result[i];
        if (validator->public_key && strcmp(validator->public_key, public_key) == 0) {
            return validator;
        }
    }
    return NULL;
}

size_t neoc_neo_get_next_block_validators_get_active_count(const neoc_neo_get_next_block_validators_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    size_t count = 0;
    for (size_t i = 0; i < response->result_count; ++i) {
        if (response->result[i].active) {
            ++count;
        }
    }
    return count;
}

const char *neoc_validator_get_public_key(const neoc_validator_t *validator) {
    return validator ? validator->public_key : NULL;
}

const char *neoc_validator_get_votes(const neoc_validator_t *validator) {
    return validator ? validator->votes : NULL;
}

bool neoc_validator_is_active(const neoc_validator_t *validator) {
    return validator ? validator->active : false;
}

#ifdef HAVE_CJSON

static bool neoc_validator_parse_bool(const cJSON *item) {
    if (!item) {
        return false;
    }
    if (cJSON_IsBool(item)) {
        return cJSON_IsTrue(item);
    }
    if (cJSON_IsString(item) && item->valuestring) {
        return strcmp(item->valuestring, "true") == 0 || strcmp(item->valuestring, "1") == 0;
    }
    return false;
}

static neoc_error_t neoc_validator_parse_array(const cJSON *array,
                                               neoc_validator_t **validators_out,
                                               size_t *count_out) {
    if (!validators_out || !count_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "validators: parse array outputs invalid");
    }

    *validators_out = NULL;
    *count_out = 0;

    if (!array || !cJSON_IsArray(array) || cJSON_GetArraySize(array) == 0) {
        return NEOC_SUCCESS;
    }

    int count = cJSON_GetArraySize(array);
    neoc_validator_t *validators = neoc_calloc((size_t)count, sizeof(neoc_validator_t));
    if (!validators) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators: allocation failed");
    }

    for (int i = 0; i < count; ++i) {
        cJSON *entry = cJSON_GetArrayItem(array, i);
        if (!entry || !cJSON_IsObject(entry)) {
            continue;
        }

        const cJSON *public_key_item = cJSON_GetObjectItemCaseSensitive(entry, "publickey");
        if (public_key_item && cJSON_IsString(public_key_item) && public_key_item->valuestring) {
            validators[i].public_key = neoc_strdup(public_key_item->valuestring);
            if (!validators[i].public_key) {
                neoc_validator_array_free(validators, (size_t)count);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators: public key duplication failed");
            }
        }

        const cJSON *votes_item = cJSON_GetObjectItemCaseSensitive(entry, "votes");
        if (votes_item && cJSON_IsString(votes_item) && votes_item->valuestring) {
            validators[i].votes = neoc_strdup(votes_item->valuestring);
            if (!validators[i].votes) {
                neoc_validator_array_free(validators, (size_t)count);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators: votes duplication failed");
            }
        }

        validators[i].active = neoc_validator_parse_bool(cJSON_GetObjectItemCaseSensitive(entry, "active"));
    }

    *validators_out = validators;
    *count_out = (size_t)count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_next_block_validators_from_json(const char *json_str,
                                                          neoc_neo_get_next_block_validators_t **response_out) {
    if (!json_str || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "validators_response: from_json invalid arguments");
    }

    *response_out = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "validators_response: root must be object");
    }

    neoc_neo_get_next_block_validators_t *response = neoc_calloc(1, sizeof(neoc_neo_get_next_block_validators_t));
    if (!response) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: allocation failed");
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_neo_get_next_block_validators_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: jsonrpc duplication failed");
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
                neoc_neo_get_next_block_validators_free(response);
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: error duplication failed");
            }
        }
    }

    if (!response->error_code) {
        const cJSON *result_array = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_array && cJSON_IsArray(result_array)) {
            neoc_validator_t *validators = NULL;
            size_t validators_count = 0;
            neoc_error_t err = neoc_validator_parse_array(result_array, &validators, &validators_count);
            if (err != NEOC_SUCCESS) {
                neoc_neo_get_next_block_validators_free(response);
                cJSON_Delete(root);
                return err;
            }

            response->result = validators;
            response->result_count = validators_count;
        }
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_next_block_validators_to_json(const neoc_neo_get_next_block_validators_t *response,
                                                         char **json_str_out) {
    if (!response || !json_str_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "validators_response: to_json invalid arguments");
    }

    *json_str_out = NULL;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: JSON object allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_code || response->error) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: error object allocation failed");
        }
        cJSON_AddItemToObject(root, "error", error_obj);
        cJSON_AddNumberToObject(error_obj, "code", response->error_code);
        if (response->error) {
            cJSON_AddStringToObject(error_obj, "message", response->error);
        }
    } else if (response->result) {
        cJSON *result_array = cJSON_CreateArray();
        if (!result_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: result array allocation failed");
        }
        cJSON_AddItemToObject(root, "result", result_array);

        for (size_t i = 0; i < response->result_count; ++i) {
            neoc_validator_t *validator = &response->result[i];
            cJSON *entry = cJSON_CreateObject();
            if (!entry) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: entry allocation failed");
            }

            if (validator->public_key) cJSON_AddStringToObject(entry, "publickey", validator->public_key);
            if (validator->votes) cJSON_AddStringToObject(entry, "votes", validator->votes);
            cJSON_AddBoolToObject(entry, "active", validator->active);

            cJSON_AddItemToArray(result_array, entry);
        }
    }

    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!rendered) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: JSON render failed");
    }

    *json_str_out = neoc_strdup(rendered);
    cJSON_free(rendered);

    if (!*json_str_out) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "validators_response: JSON duplication failed");
    }

    return NEOC_SUCCESS;
}

#else

neoc_error_t neoc_neo_get_next_block_validators_from_json(const char *json_str,
                                                          neoc_neo_get_next_block_validators_t **response_out) {
    (void)json_str;
    (void)response_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "validators_response: JSON parsing requires cJSON");
}

neoc_error_t neoc_neo_get_next_block_validators_to_json(const neoc_neo_get_next_block_validators_t *response,
                                                         char **json_str_out) {
    (void)response;
    (void)json_str_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "validators_response: JSON serialization requires cJSON");
}

#endif /* HAVE_CJSON */
