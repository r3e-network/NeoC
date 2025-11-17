/**
 * @file neo_get_mem_pool.c
 * @brief Implementation of Neo get memory pool response handling.
 *
 * Mirrors NeoSwift's `NeoGetMemPool` response by providing helpers to parse,
 * materialise, and query the mem-pool details returned by JSON-RPC.
 */

#include "neoc/protocol/core/response/neo_get_mem_pool.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/decode.h"
#include "neoc/types/neoc_hash256.h"

#include <string.h>

neoc_error_t neoc_mem_pool_details_create(int height,
                                          const neoc_hash256_t *verified,
                                          size_t verified_count,
                                          const neoc_hash256_t *unverified,
                                          size_t unverified_count,
                                          neoc_mem_pool_details_t **details_out) {
    if (!details_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_details: output pointer invalid");
    }

    *details_out = NULL;

    neoc_mem_pool_details_t *details = neoc_calloc(1, sizeof(neoc_mem_pool_details_t));
    if (!details) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_details: allocation failed");
    }

    details->height = height;

    if (verified_count > 0 && verified) {
        details->verified = neoc_calloc(verified_count, sizeof(neoc_hash256_t));
        if (!details->verified) {
            neoc_mem_pool_details_free(details);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_details: verified allocation failed");
        }
        details->verified_count = verified_count;
        memcpy(details->verified, verified, verified_count * sizeof(neoc_hash256_t));
    }

    if (unverified_count > 0 && unverified) {
        details->unverified = neoc_calloc(unverified_count, sizeof(neoc_hash256_t));
        if (!details->unverified) {
            neoc_mem_pool_details_free(details);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_details: unverified allocation failed");
        }
        details->unverified_count = unverified_count;
        memcpy(details->unverified, unverified, unverified_count * sizeof(neoc_hash256_t));
    }

    *details_out = details;
    return NEOC_SUCCESS;
}

void neoc_mem_pool_details_free(neoc_mem_pool_details_t *details) {
    if (!details) {
        return;
    }

    neoc_free(details->verified);
    neoc_free(details->unverified);

    neoc_free(details);
}

static neoc_neo_get_mem_pool_t *neoc_mem_pool_response_allocate(void) {
    neoc_neo_get_mem_pool_t *response = neoc_calloc(1, sizeof(neoc_neo_get_mem_pool_t));
    if (!response) {
        neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: allocation failed");
        return NULL;
    }
    return response;
}

neoc_error_t neoc_neo_get_mem_pool_create(int id,
                                          neoc_mem_pool_details_t *mem_pool_details,
                                          const char *error,
                                          int error_code,
                                          neoc_neo_get_mem_pool_t **response_out) {
    if (!response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_response: output pointer invalid");
    }

    *response_out = NULL;

    neoc_neo_get_mem_pool_t *response = neoc_mem_pool_response_allocate();
    if (!response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        neoc_neo_get_mem_pool_free(response);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: jsonrpc allocation failed");
    }

    response->id = id;
    response->result = mem_pool_details;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_neo_get_mem_pool_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: error allocation failed");
        }
    }

    response->error_code = error_code;
    *response_out = response;

    return NEOC_SUCCESS;
}

void neoc_neo_get_mem_pool_free(neoc_neo_get_mem_pool_t *response) {
    if (!response) {
        return;
    }

    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    neoc_mem_pool_details_free(response->result);
    neoc_free(response);
}

neoc_mem_pool_details_t *neoc_neo_get_mem_pool_get_details(const neoc_neo_get_mem_pool_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_mem_pool_has_details(const neoc_neo_get_mem_pool_t *response) {
    return response && response->result != NULL;
}

neoc_error_t neoc_neo_get_mem_pool_get_height(const neoc_neo_get_mem_pool_t *response,
                                              int *height) {
    if (!height) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_response: height output invalid");
    }
    *height = 0;

    if (!response || !response->result) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "mem_pool_response: no details available");
    }

    *height = response->result->height;
    return NEOC_SUCCESS;
}

size_t neoc_neo_get_mem_pool_get_total_count(const neoc_neo_get_mem_pool_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->verified_count + response->result->unverified_count;
}

size_t neoc_neo_get_mem_pool_get_verified_count(const neoc_neo_get_mem_pool_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->verified_count;
}

size_t neoc_neo_get_mem_pool_get_unverified_count(const neoc_neo_get_mem_pool_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->unverified_count;
}

static neoc_error_t neoc_mem_pool_get_transaction(const neoc_hash256_t *array,
                                                  size_t count,
                                                  size_t index,
                                                  neoc_hash256_t *hash_out) {
    if (!array || !hash_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_response: transaction arguments invalid");
    }
    if (index >= count) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_BOUNDS, "mem_pool_response: transaction index out of range");
    }
    return neoc_hash256_copy(hash_out, &array[index]);
}

neoc_error_t neoc_neo_get_mem_pool_get_verified_transaction(const neoc_neo_get_mem_pool_t *response,
                                                            size_t index,
                                                            neoc_hash256_t *hash_out) {
    if (!response || !response->result) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "mem_pool_response: no details available");
    }
    return neoc_mem_pool_get_transaction(response->result->verified,
                                         response->result->verified_count,
                                         index,
                                         hash_out);
}

neoc_error_t neoc_neo_get_mem_pool_get_unverified_transaction(const neoc_neo_get_mem_pool_t *response,
                                                              size_t index,
                                                              neoc_hash256_t *hash_out) {
    if (!response || !response->result) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "mem_pool_response: no details available");
    }
    return neoc_mem_pool_get_transaction(response->result->unverified,
                                         response->result->unverified_count,
                                         index,
                                         hash_out);
}

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>

static neoc_error_t neoc_mem_pool_parse_array(const cJSON *array,
                                              neoc_hash256_t **out,
                                              size_t *count_out) {
    if (!out || !count_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_response: array outputs invalid");
    }

    *out = NULL;
    *count_out = 0;

    if (!array || !cJSON_IsArray(array) || cJSON_GetArraySize(array) == 0) {
        return NEOC_SUCCESS;
    }

    const int count = cJSON_GetArraySize(array);
    neoc_hash256_t *hashes = neoc_calloc((size_t)count, sizeof(neoc_hash256_t));
    if (!hashes) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: hash array allocation failed");
    }

    neoc_error_t err = NEOC_SUCCESS;
    for (int i = 0; i < count; ++i) {
        const cJSON *item = cJSON_GetArrayItem(array, i);
        if (!item || !cJSON_IsString(item) || !item->valuestring) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "mem_pool_response: transaction hash invalid");
            break;
        }

        err = neoc_hash256_from_string(item->valuestring, &hashes[i]);
        if (err != NEOC_SUCCESS) {
            err = neoc_error_set(err, "mem_pool_response: failed to parse hash");
            break;
        }
    }

    if (err != NEOC_SUCCESS) {
        neoc_free(hashes);
        return err;
    }

    *out = hashes;
    *count_out = (size_t)count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_mem_pool_from_json(const char *json_str,
                                             neoc_neo_get_mem_pool_t **response_out) {
    if (!json_str || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_response: from_json arguments invalid");
    }

    *response_out = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "mem_pool_response: root must be JSON object");
    }

    neoc_neo_get_mem_pool_t *response = neoc_mem_pool_response_allocate();
    if (!response) {
        cJSON_Delete(root);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_neo_get_mem_pool_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: jsonrpc copy failed");
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
                neoc_neo_get_mem_pool_free(response);
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: error copy failed");
            }
        }
    }

    if (!response->error_code) {
        const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_obj && cJSON_IsObject(result_obj)) {
            int height = 0;
            const cJSON *height_item = cJSON_GetObjectItemCaseSensitive(result_obj, "height");
            if (height_item) {
                if (cJSON_IsNumber(height_item)) {
                    height = height_item->valueint;
                } else if (cJSON_IsString(height_item) && height_item->valuestring) {
                    neoc_decode_int_from_string(height_item->valuestring, &height);
                }
            }

            neoc_hash256_t *verified = NULL;
            size_t verified_count = 0;
            neoc_error_t err = neoc_mem_pool_parse_array(
                cJSON_GetObjectItemCaseSensitive(result_obj, "verified"),
                &verified,
                &verified_count);
            if (err != NEOC_SUCCESS) {
                neoc_neo_get_mem_pool_free(response);
                cJSON_Delete(root);
                return err;
            }

            neoc_hash256_t *unverified = NULL;
            size_t unverified_count = 0;
            err = neoc_mem_pool_parse_array(
                cJSON_GetObjectItemCaseSensitive(result_obj, "unverified"),
                &unverified,
                &unverified_count);
            if (err != NEOC_SUCCESS) {
                neoc_free(verified);
                neoc_neo_get_mem_pool_free(response);
                cJSON_Delete(root);
                return err;
            }

            err = neoc_mem_pool_details_create(height,
                                               verified,
                                               verified_count,
                                               unverified,
                                               unverified_count,
                                               &response->result);

            neoc_free(verified);
            neoc_free(unverified);

            if (err != NEOC_SUCCESS) {
                neoc_neo_get_mem_pool_free(response);
                cJSON_Delete(root);
                return err;
            }
        }
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_mem_pool_to_json(const neoc_neo_get_mem_pool_t *response,
                                           char **json_str_out) {
    if (!response || !json_str_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mem_pool_response: to_json arguments invalid");
    }

    *json_str_out = NULL;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: JSON object allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_code || response->error) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: error object allocation failed");
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
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: result object allocation failed");
        }
        cJSON_AddItemToObject(root, "result", result_obj);
        cJSON_AddNumberToObject(result_obj, "height", response->result->height);

        cJSON *verified_array = cJSON_CreateArray();
        if (!verified_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: verified array allocation failed");
        }
        for (size_t i = 0; i < response->result->verified_count; ++i) {
            char hex_buffer[NEOC_HASH256_STRING_LENGTH];
            char prefixed_buffer[NEOC_HASH256_STRING_LENGTH + 2];
            if (neoc_hash256_to_string(&response->result->verified[i], hex_buffer, sizeof(hex_buffer)) == NEOC_SUCCESS) {
                prefixed_buffer[0] = '0';
                prefixed_buffer[1] = 'x';
                memcpy(prefixed_buffer + 2, hex_buffer, sizeof(hex_buffer));
                cJSON_AddItemToArray(verified_array, cJSON_CreateString(prefixed_buffer));
            }
        }
        cJSON_AddItemToObject(result_obj, "verified", verified_array);

        cJSON *unverified_array = cJSON_CreateArray();
        if (!unverified_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: unverified array allocation failed");
        }
        for (size_t i = 0; i < response->result->unverified_count; ++i) {
            char hex_buffer[NEOC_HASH256_STRING_LENGTH];
            char prefixed_buffer[NEOC_HASH256_STRING_LENGTH + 2];
            if (neoc_hash256_to_string(&response->result->unverified[i], hex_buffer, sizeof(hex_buffer)) == NEOC_SUCCESS) {
                prefixed_buffer[0] = '0';
                prefixed_buffer[1] = 'x';
                memcpy(prefixed_buffer + 2, hex_buffer, sizeof(hex_buffer));
                cJSON_AddItemToArray(unverified_array, cJSON_CreateString(prefixed_buffer));
            }
        }
        cJSON_AddItemToObject(result_obj, "unverified", unverified_array);
    }

    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!rendered) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: JSON render failed");
    }

    *json_str_out = neoc_strdup(rendered);
    cJSON_free(rendered);

    if (!*json_str_out) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "mem_pool_response: JSON duplication failed");
    }

    return NEOC_SUCCESS;
}

#else /* HAVE_CJSON */

neoc_error_t neoc_neo_get_mem_pool_from_json(const char *json_str,
                                             neoc_neo_get_mem_pool_t **response_out) {
    (void)json_str;
    (void)response_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "mem_pool_response: JSON parsing requires cJSON");
}

neoc_error_t neoc_neo_get_mem_pool_to_json(const neoc_neo_get_mem_pool_t *response,
                                           char **json_str_out) {
    (void)response;
    (void)json_str_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "mem_pool_response: JSON serialization requires cJSON");
}

#endif /* HAVE_CJSON */
