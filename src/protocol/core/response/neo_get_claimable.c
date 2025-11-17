/**
 * @file neo_get_claimable.c
 * @brief neo_get_claimable implementation
 * 
 * Based on Swift source: protocol/core/response/NeoGetClaimable.swift
 */

#include "neoc/protocol/core/response/neo_get_claimable.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include "neoc/utils/decode.h"

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

#include <string.h>

static void neoc_claimable_claim_dispose(neoc_claimable_claim_t *claim) {
    if (!claim) {
        return;
    }
    neoc_free(claim->tx_id);
    neoc_free(claim->generated_gas);
    neoc_free(claim->system_fee);
    neoc_free(claim->unclaimed_gas);
}

static void neoc_claimables_dispose(neoc_claimables_t *claimables) {
    if (!claimables) {
        return;
    }
    if (claimables->claims) {
        for (size_t i = 0; i < claimables->claims_count; ++i) {
            neoc_claimable_claim_dispose(&claimables->claims[i]);
        }
        neoc_free(claimables->claims);
    }
    neoc_free(claimables->address);
    neoc_free(claimables->total_unclaimed);
}

static void neoc_neo_get_claimable_dispose(neoc_neo_get_claimable_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_claimables_dispose(response->result);
        neoc_free(response->result);
    }
}

neoc_error_t neoc_claimable_claim_create(const char *tx_id,
                                         int index,
                                         int neo_value,
                                         int start_height,
                                         int end_height,
                                         const char *generated_gas,
                                         const char *system_fee,
                                         const char *unclaimed_gas,
                                         neoc_claimable_claim_t **claim) {
    if (!claim) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid claim pointer");
    }
    *claim = NULL;

    neoc_claimable_claim_t *result = neoc_calloc(1, sizeof(neoc_claimable_claim_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate claim");
    }

    if (tx_id) {
        result->tx_id = neoc_strdup(tx_id);
        if (!result->tx_id) {
            neoc_claimable_claim_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate tx_id");
        }
    }

    result->index = index;
    result->neo_value = neo_value;
    result->start_height = start_height;
    result->end_height = end_height;

    if (generated_gas) {
        result->generated_gas = neoc_strdup(generated_gas);
        if (!result->generated_gas) {
            neoc_claimable_claim_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate generated_gas");
        }
    }
    if (system_fee) {
        result->system_fee = neoc_strdup(system_fee);
        if (!result->system_fee) {
            neoc_claimable_claim_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate system_fee");
        }
    }
    if (unclaimed_gas) {
        result->unclaimed_gas = neoc_strdup(unclaimed_gas);
        if (!result->unclaimed_gas) {
            neoc_claimable_claim_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate unclaimed_gas");
        }
    }

    *claim = result;
    return NEOC_SUCCESS;
}

void neoc_claimable_claim_free(neoc_claimable_claim_t *claim) {
    if (!claim) {
        return;
    }
    neoc_claimable_claim_dispose(claim);
    neoc_free(claim);
}

neoc_error_t neoc_claimables_create(neoc_claimable_claim_t *claims,
                                    size_t claims_count,
                                    const char *address,
                                    const char *total_unclaimed,
                                    neoc_claimables_t **claimables) {
    if (!claimables) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid claimables pointer");
    }
    *claimables = NULL;

    neoc_claimables_t *result = neoc_calloc(1, sizeof(neoc_claimables_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate claimables");
    }

    if (claims_count > 0) {
        result->claims = neoc_calloc(claims_count, sizeof(neoc_claimable_claim_t));
        if (!result->claims) {
            neoc_claimables_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate claims array");
        }

        for (size_t i = 0; i < claims_count; ++i) {
            neoc_claimable_claim_t *dest = &result->claims[i];
            neoc_claimable_claim_t *src = &claims[i];

            if (src->tx_id) {
                dest->tx_id = neoc_strdup(src->tx_id);
                if (!dest->tx_id) {
                    neoc_claimables_dispose(result);
                    neoc_free(result);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate claim tx_id");
                }
            }
            dest->index = src->index;
            dest->neo_value = src->neo_value;
            dest->start_height = src->start_height;
            dest->end_height = src->end_height;

            if (src->generated_gas) {
                dest->generated_gas = neoc_strdup(src->generated_gas);
                if (!dest->generated_gas) {
                    neoc_claimables_dispose(result);
                    neoc_free(result);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate generated_gas");
                }
            }
            if (src->system_fee) {
                dest->system_fee = neoc_strdup(src->system_fee);
                if (!dest->system_fee) {
                    neoc_claimables_dispose(result);
                    neoc_free(result);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate system_fee");
                }
            }
            if (src->unclaimed_gas) {
                dest->unclaimed_gas = neoc_strdup(src->unclaimed_gas);
                if (!dest->unclaimed_gas) {
                    neoc_claimables_dispose(result);
                    neoc_free(result);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate unclaimed_gas");
                }
            }
        }
    }
    result->claims_count = claims_count;

    if (address) {
        result->address = neoc_strdup(address);
        if (!result->address) {
            neoc_claimables_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate address");
        }
    }

    if (total_unclaimed) {
        result->total_unclaimed = neoc_strdup(total_unclaimed);
        if (!result->total_unclaimed) {
            neoc_claimables_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate total_unclaimed");
        }
    }

    *claimables = result;
    return NEOC_SUCCESS;
}

void neoc_claimables_free(neoc_claimables_t *claimables) {
    if (!claimables) {
        return;
    }
    neoc_claimables_dispose(claimables);
    neoc_free(claimables);
}

neoc_error_t neoc_neo_get_claimable_create(int id,
                                           neoc_claimables_t *claimables,
                                           const char *error,
                                           int error_code,
                                           neoc_neo_get_claimable_t **response) {
    if (!response) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid response pointer");
    }
    *response = NULL;

    neoc_neo_get_claimable_t *result = neoc_calloc(1, sizeof(neoc_neo_get_claimable_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate response");
    }

    result->jsonrpc = neoc_strdup("2.0");
    if (!result->jsonrpc) {
        neoc_neo_get_claimable_dispose(result);
        neoc_free(result);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to set jsonrpc");
    }

    result->id = id;
    result->result = claimables;

    if (error) {
        result->error = neoc_strdup(error);
        if (!result->error) {
            neoc_neo_get_claimable_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate error");
        }
    }
    result->error_code = error_code;

    *response = result;
    return NEOC_SUCCESS;
}

void neoc_neo_get_claimable_free(neoc_neo_get_claimable_t *response) {
    if (!response) {
        return;
    }
    neoc_neo_get_claimable_dispose(response);
    neoc_free(response);
}

neoc_claimables_t *neoc_neo_get_claimable_get_claimables(const neoc_neo_get_claimable_t *response) {
    return response ? response->result : NULL;
}

bool neoc_neo_get_claimable_has_claimables(const neoc_neo_get_claimable_t *response) {
    return response && response->result != NULL && response->error == NULL;
}

size_t neoc_neo_get_claimable_get_claims_count(const neoc_neo_get_claimable_t *response) {
    if (!neoc_neo_get_claimable_has_claimables(response)) {
        return 0;
    }
    return response->result->claims_count;
}

neoc_claimable_claim_t *neoc_neo_get_claimable_get_claim(const neoc_neo_get_claimable_t *response,
                                                         size_t index) {
    if (!neoc_neo_get_claimable_has_claimables(response)) {
        return NULL;
    }
    if (index >= response->result->claims_count) {
        return NULL;
    }
    return &response->result->claims[index];
}

#ifdef HAVE_CJSON

static cJSON *neoc_claimable_claim_to_json_object(const neoc_claimable_claim_t *claim) {
    cJSON *node = cJSON_CreateObject();
    if (!node) {
        return NULL;
    }

    if (claim->tx_id) cJSON_AddStringToObject(node, "txid", claim->tx_id);
    cJSON_AddNumberToObject(node, "n", claim->index);
    cJSON_AddNumberToObject(node, "neo", claim->neo_value);
    cJSON_AddNumberToObject(node, "start", claim->start_height);
    cJSON_AddNumberToObject(node, "end", claim->end_height);
    if (claim->generated_gas) cJSON_AddStringToObject(node, "generated", claim->generated_gas);
    if (claim->system_fee) cJSON_AddStringToObject(node, "systemfee", claim->system_fee);
    if (claim->unclaimed_gas) cJSON_AddStringToObject(node, "unclaimed", claim->unclaimed_gas);

    return node;
}

static cJSON *neoc_claimables_to_json_object(const neoc_claimables_t *claimables) {
    cJSON *node = cJSON_CreateObject();
    if (!node) {
        return NULL;
    }

    if (claimables->address) cJSON_AddStringToObject(node, "address", claimables->address);
    if (claimables->total_unclaimed) cJSON_AddStringToObject(node, "unclaimed", claimables->total_unclaimed);

    cJSON *claims_array = cJSON_CreateArray();
    if (!claims_array) {
        cJSON_Delete(node);
        return NULL;
    }

    for (size_t i = 0; i < claimables->claims_count; ++i) {
        cJSON *claim_json = neoc_claimable_claim_to_json_object(&claimables->claims[i]);
        if (!claim_json) {
            cJSON_Delete(claims_array);
            cJSON_Delete(node);
            return NULL;
        }
        cJSON_AddItemToArray(claims_array, claim_json);
    }

    cJSON_AddItemToObject(node, "claimable", claims_array);
    return node;
}

static neoc_error_t neoc_claimable_claim_from_json(const cJSON *json, neoc_claimable_claim_t *out) {
    if (!json || !out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid claim JSON");
    }

    memset(out, 0, sizeof(neoc_claimable_claim_t));

    cJSON *txid = cJSON_GetObjectItemCaseSensitive(json, "txid");
    cJSON *index = cJSON_GetObjectItemCaseSensitive(json, "n");
    cJSON *neo = cJSON_GetObjectItemCaseSensitive(json, "neo");
    cJSON *start = cJSON_GetObjectItemCaseSensitive(json, "start");
    cJSON *end = cJSON_GetObjectItemCaseSensitive(json, "end");
    cJSON *generated = cJSON_GetObjectItemCaseSensitive(json, "generated");
    cJSON *systemfee = cJSON_GetObjectItemCaseSensitive(json, "systemfee");
    cJSON *unclaimed = cJSON_GetObjectItemCaseSensitive(json, "unclaimed");

    neoc_error_t err = NEOC_SUCCESS;

    if (txid && cJSON_IsString(txid)) {
        out->tx_id = neoc_strdup(txid->valuestring);
        if (!out->tx_id) {
            err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy txid");
            goto error;
        }
    }
    if (index) {
        if (cJSON_IsNumber(index)) {
            out->index = index->valueint;
        } else if (cJSON_IsString(index) && index->valuestring) {
            neoc_decode_int_from_string(index->valuestring, &out->index);
        }
    }
    if (neo) {
        if (cJSON_IsNumber(neo)) {
            out->neo_value = neo->valueint;
        } else if (cJSON_IsString(neo) && neo->valuestring) {
            neoc_decode_int_from_string(neo->valuestring, &out->neo_value);
        }
    }
    if (start) {
        if (cJSON_IsNumber(start)) {
            out->start_height = start->valueint;
        } else if (cJSON_IsString(start) && start->valuestring) {
            neoc_decode_int_from_string(start->valuestring, &out->start_height);
        }
    }
    if (end) {
        if (cJSON_IsNumber(end)) {
            out->end_height = end->valueint;
        } else if (cJSON_IsString(end) && end->valuestring) {
            neoc_decode_int_from_string(end->valuestring, &out->end_height);
        }
    }
    if (generated && cJSON_IsString(generated)) {
        out->generated_gas = neoc_strdup(generated->valuestring);
        if (!out->generated_gas) {
            err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy generated");
            goto error;
        }
    }
    if (systemfee && cJSON_IsString(systemfee)) {
        out->system_fee = neoc_strdup(systemfee->valuestring);
        if (!out->system_fee) {
            err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy systemfee");
            goto error;
        }
    }
    if (unclaimed && cJSON_IsString(unclaimed)) {
        out->unclaimed_gas = neoc_strdup(unclaimed->valuestring);
        if (!out->unclaimed_gas) {
            err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy unclaimed");
            goto error;
        }
    }

    return NEOC_SUCCESS;

error:
    neoc_claimable_claim_dispose(out);
    memset(out, 0, sizeof(neoc_claimable_claim_t));
    return err;
}

static neoc_error_t neoc_claimables_from_json(const cJSON *json, neoc_claimables_t **out_claimables) {
    if (!json || !out_claimables) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid claimables JSON");
    }

    neoc_claimables_t *claimables = neoc_calloc(1, sizeof(neoc_claimables_t));
    if (!claimables) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate claimables");
    }

    cJSON *address = cJSON_GetObjectItemCaseSensitive(json, "address");
    if (address && cJSON_IsString(address)) {
        claimables->address = neoc_strdup(address->valuestring);
        if (!claimables->address) {
            neoc_claimables_dispose(claimables);
            neoc_free(claimables);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy address");
        }
    }

    cJSON *unclaimed = cJSON_GetObjectItemCaseSensitive(json, "unclaimed");
    if (unclaimed && cJSON_IsString(unclaimed)) {
        claimables->total_unclaimed = neoc_strdup(unclaimed->valuestring);
        if (!claimables->total_unclaimed) {
            neoc_claimables_dispose(claimables);
            neoc_free(claimables);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy total unclaimed");
        }
    }

    cJSON *claimable_array = cJSON_GetObjectItemCaseSensitive(json, "claimable");
    if (claimable_array && cJSON_IsArray(claimable_array)) {
        size_t count = (size_t)cJSON_GetArraySize(claimable_array);
        if (count > 0) {
            claimables->claims = neoc_calloc(count, sizeof(neoc_claimable_claim_t));
            if (!claimables->claims) {
                neoc_claimables_dispose(claimables);
                neoc_free(claimables);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate claims");
            }
            claimables->claims_count = count;

            cJSON *item = NULL;
            size_t idx = 0;
            cJSON_ArrayForEach(item, claimable_array) {
                neoc_error_t err = neoc_claimable_claim_from_json(item, &claimables->claims[idx]);
                if (err != NEOC_SUCCESS) {
                    neoc_claimables_dispose(claimables);
                    neoc_free(claimables);
                    return err;
                }
                idx++;
            }
        }
    }

    *out_claimables = claimables;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_claimable_to_json(const neoc_neo_get_claimable_t *response,
                                            char **json_str) {
    if (!response || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to create JSON root");
    }

    if (response->jsonrpc) cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error) {
        cJSON *err_obj = cJSON_CreateObject();
        if (!err_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to create error object");
        }
        cJSON_AddNumberToObject(err_obj, "code", response->error_code);
        cJSON_AddStringToObject(err_obj, "message", response->error);
        cJSON_AddItemToObject(root, "error", err_obj);
    } else if (response->result) {
        cJSON *result_obj = neoc_claimables_to_json_object(response->result);
        if (!result_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to serialize result");
        }
        cJSON_AddItemToObject(root, "result", result_obj);
    }

    char *printed = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!printed) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to print JSON");
    }

    *json_str = neoc_strdup(printed);
    cJSON_free(printed);
    if (!*json_str) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate JSON output");
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_claimable_from_json(const char *json_str,
                                              neoc_neo_get_claimable_t **response) {
    if (!json_str || !response) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    *response = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON format");
    }

    neoc_neo_get_claimable_t *result = neoc_calloc(1, sizeof(neoc_neo_get_claimable_t));
    if (!result) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate response");
    }

    cJSON *jsonrpc = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc && cJSON_IsString(jsonrpc)) {
        result->jsonrpc = neoc_strdup(jsonrpc->valuestring);
        if (!result->jsonrpc) {
            cJSON_Delete(root);
            neoc_neo_get_claimable_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy jsonrpc");
        }
    } else {
        result->jsonrpc = neoc_strdup("2.0");
        if (!result->jsonrpc) {
            cJSON_Delete(root);
            neoc_neo_get_claimable_dispose(result);
            neoc_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to set jsonrpc");
        }
    }

    cJSON *id = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (id && cJSON_IsNumber(id)) {
        result->id = id->valueint;
    }

    cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (error_obj && cJSON_IsObject(error_obj)) {
        cJSON *code = cJSON_GetObjectItemCaseSensitive(error_obj, "code");
        cJSON *message = cJSON_GetObjectItemCaseSensitive(error_obj, "message");
        if (code && cJSON_IsNumber(code)) result->error_code = code->valueint;
        if (message && cJSON_IsString(message)) {
            result->error = neoc_strdup(message->valuestring);
            if (!result->error) {
                cJSON_Delete(root);
                neoc_neo_get_claimable_dispose(result);
                neoc_free(result);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy error message");
            }
        }
    } else {
        cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_obj && cJSON_IsObject(result_obj)) {
            neoc_claimables_t *claimables = NULL;
            neoc_error_t err = neoc_claimables_from_json(result_obj, &claimables);
            if (err != NEOC_SUCCESS) {
                cJSON_Delete(root);
                neoc_neo_get_claimable_dispose(result);
                neoc_free(result);
                return err;
            }
            result->result = claimables;
        }
    }

    cJSON_Delete(root);
    *response = result;
    return NEOC_SUCCESS;
}

#else /* HAVE_CJSON */

neoc_error_t neoc_neo_get_claimable_to_json(const neoc_neo_get_claimable_t *response,
                                            char **json_str) {
    (void)response;
    (void)json_str;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support not compiled in");
}

neoc_error_t neoc_neo_get_claimable_from_json(const char *json_str,
                                              neoc_neo_get_claimable_t **response) {
    (void)json_str;
    (void)response;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support not compiled in");
}

#endif /* HAVE_CJSON */
