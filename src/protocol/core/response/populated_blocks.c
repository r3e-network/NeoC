/**
 * @file populated_blocks.c
 * @brief PopulatedBlocks RPC response implementation
 */

#include "neoc/protocol/core/response/populated_blocks.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_populated_blocks_create(const char *cache_id,
                                          const int *blocks,
                                          size_t blocks_count,
                                          neoc_populated_blocks_t **populated_blocks) {
    if (!cache_id || !populated_blocks) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *populated_blocks = neoc_calloc(1, sizeof(neoc_populated_blocks_t));
    if (!*populated_blocks) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*populated_blocks)->cache_id = dup_string(cache_id);
    if (!(*populated_blocks)->cache_id) {
        neoc_populated_blocks_free(*populated_blocks);
        *populated_blocks = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (blocks && blocks_count > 0) {
        (*populated_blocks)->blocks = neoc_calloc(blocks_count, sizeof(int));
        if (!(*populated_blocks)->blocks) {
            neoc_populated_blocks_free(*populated_blocks);
            *populated_blocks = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy((*populated_blocks)->blocks, blocks, blocks_count * sizeof(int));
        (*populated_blocks)->blocks_count = blocks_count;
    }

    return NEOC_SUCCESS;
}

void neoc_populated_blocks_free(neoc_populated_blocks_t *populated_blocks) {
    if (!populated_blocks) {
        return;
    }
    neoc_free(populated_blocks->cache_id);
    neoc_free(populated_blocks->blocks);
    neoc_free(populated_blocks);
}

neoc_error_t neoc_populated_blocks_response_create(const char *jsonrpc,
                                                   int id,
                                                   neoc_populated_blocks_t *result,
                                                   const char *error,
                                                   int error_code,
                                                   neoc_populated_blocks_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = neoc_calloc(1, sizeof(neoc_populated_blocks_response_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->jsonrpc = dup_string(jsonrpc ? jsonrpc : "2.0");
    if (!(*response)->jsonrpc) {
        neoc_populated_blocks_response_free(*response);
        *response = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->id = id;
    (*response)->result = result;
    (*response)->error_code = error_code;

    if (error) {
        (*response)->error = dup_string(error);
        if (!(*response)->error) {
            neoc_populated_blocks_response_free(*response);
            *response = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    return NEOC_SUCCESS;
}

void neoc_populated_blocks_response_free(neoc_populated_blocks_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_populated_blocks_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_populated_blocks_from_json(const char *json_str,
                                             neoc_populated_blocks_t **populated_blocks) {
    if (!json_str || !populated_blocks) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *populated_blocks = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    const char *cache_id = neoc_json_get_string(json, "cacheid");
    if (!cache_id) {
        cache_id = neoc_json_get_string(json, "cache_id");
    }

    neoc_json_t *blocks_arr = neoc_json_get_array(json, "blocks");
    size_t count = blocks_arr ? neoc_json_array_size(blocks_arr) : 0;
    int *blocks = NULL;

    if (count > 0) {
        blocks = neoc_calloc(count, sizeof(int));
        if (!blocks) {
            neoc_json_free(json);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        for (size_t i = 0; i < count; i++) {
            neoc_json_t *entry = neoc_json_array_get(blocks_arr, i);
            int64_t val = 0;
            if (neoc_json_get_int(entry ? entry : json, "value", &val) != NEOC_SUCCESS &&
                neoc_json_get_int(entry ? entry : json, "", &val) != NEOC_SUCCESS &&
                neoc_json_get_int(entry, "index", &val) != NEOC_SUCCESS) {
                double dval = 0.0;
                if (neoc_json_get_number(entry ? entry : json, "", &dval) == NEOC_SUCCESS) {
                    val = (int64_t)dval;
                }
            }
            blocks[i] = (int)val;
        }
    }

    neoc_error_t err = neoc_populated_blocks_create(cache_id ? cache_id : "", blocks, count, populated_blocks);
    neoc_free(blocks);
    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_populated_blocks_response_from_json(const char *json_str,
                                                      neoc_populated_blocks_response_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_populated_blocks_response_t *parsed = neoc_calloc(1, sizeof(neoc_populated_blocks_response_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_string(neoc_json_get_string(json, "jsonrpc"));
    if (!parsed->jsonrpc) {
        parsed->jsonrpc = dup_string("2.0");
    }

    int64_t id_val = 0;
    if (neoc_json_get_int(json, "id", &id_val) == NEOC_SUCCESS) {
        parsed->id = (int)id_val;
    }

    neoc_json_t *error_obj = neoc_json_get_object(json, "error");
    if (error_obj) {
        int64_t code = 0;
        if (neoc_json_get_int(error_obj, "code", &code) == NEOC_SUCCESS) {
            parsed->error_code = (int)code;
        }
        const char *message = neoc_json_get_string(error_obj, "message");
        if (message) {
            parsed->error = dup_string(message);
        }
    }

    neoc_json_t *result = neoc_json_get_object(json, "result");
    if (!result) {
        result = json;
    }

    char *result_json = neoc_json_to_string(result);
    if (result_json) {
        neoc_populated_blocks_t *pb = NULL;
        if (neoc_populated_blocks_from_json(result_json, &pb) == NEOC_SUCCESS) {
            parsed->result = pb;
            parsed->error_code = 0;
        }
        neoc_free(result_json);
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_populated_blocks_to_json(const neoc_populated_blocks_t *populated_blocks,
                                           char **json_str) {
    if (!populated_blocks || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (populated_blocks->cache_id) {
        neoc_json_add_string(root, "cacheid", populated_blocks->cache_id);
    }

    if (populated_blocks->blocks && populated_blocks->blocks_count > 0) {
        neoc_json_t *arr = neoc_json_create_array();
        if (!arr) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        for (size_t i = 0; i < populated_blocks->blocks_count; i++) {
            neoc_json_t *entry = neoc_json_create_object();
            if (!entry) {
                neoc_json_free(arr);
                neoc_json_free(root);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            neoc_json_add_int(entry, "value", populated_blocks->blocks[i]);
            neoc_json_array_add(arr, entry);
        }
        neoc_json_add_object(root, "blocks", arr);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

bool neoc_populated_blocks_contains_block(const neoc_populated_blocks_t *populated_blocks,
                                          int block_index) {
    if (!populated_blocks || !populated_blocks->blocks) {
        return false;
    }
    for (size_t i = 0; i < populated_blocks->blocks_count; i++) {
        if (populated_blocks->blocks[i] == block_index) {
            return true;
        }
    }
    return false;
}
