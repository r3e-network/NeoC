/**
 * @file neo_get_state_height.c
 * @brief Neo state height response implementation
 *
 * Converted from Swift source: protocol/core/response/NeoGetStateHeight.swift
 */

#include "neoc/protocol/core/response/neo_get_state_height.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *dup_or_default(const char *value, const char *fallback) {
    if (value) {
        return neoc_strdup(value);
    }
    return fallback ? neoc_strdup(fallback) : NULL;
}

neoc_error_t neoc_state_height_create(int local_root_index,
                                      int validated_root_index,
                                      neoc_state_height_t **state_height) {
    if (!state_height) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *state_height = NULL;
    neoc_state_height_t *created = neoc_malloc(sizeof(neoc_state_height_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->local_root_index = local_root_index;
    created->validated_root_index = validated_root_index;
    *state_height = created;
    return NEOC_SUCCESS;
}

void neoc_state_height_free(neoc_state_height_t *state_height) {
    if (!state_height) {
        return;
    }
    neoc_free(state_height);
}

neoc_error_t neoc_neo_get_state_height_create(int id,
                                              neoc_state_height_t *state_height,
                                              const char *error,
                                              int error_code,
                                              neoc_neo_get_state_height_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_get_state_height_t *created = neoc_calloc(1, sizeof(neoc_neo_get_state_height_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_or_default("2.0", "2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = state_height;
    created->error_code = error_code;

    if (error) {
        created->error = neoc_strdup(error);
        if (!created->error) {
            neoc_neo_get_state_height_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_get_state_height_free(neoc_neo_get_state_height_t *response) {
    if (!response) {
        return;
    }

    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
    }
    if (response->error) {
        neoc_free(response->error);
    }
    if (response->result) {
        neoc_state_height_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_neo_get_state_height_from_json(const char *json_str,
                                                 neoc_neo_get_state_height_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_get_state_height_t *parsed = neoc_calloc(1, sizeof(neoc_neo_get_state_height_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_or_default(neoc_json_get_string(json, "jsonrpc"), "2.0");
    int64_t parsed_id = 0;
    if (neoc_json_get_int(json, "id", &parsed_id) == NEOC_SUCCESS) {
        parsed->id = (int)parsed_id;
    }

    neoc_json_t *error_obj = neoc_json_get_object(json, "error");
    if (error_obj) {
        int64_t code = 0;
        if (neoc_json_get_int(error_obj, "code", &code) == NEOC_SUCCESS) {
            parsed->error_code = (int)code;
        }
        const char *message = neoc_json_get_string(error_obj, "message");
        if (message) {
            parsed->error = neoc_strdup(message);
        }
    }

    neoc_json_t *result = neoc_json_get_object(json, "result");
    if (!result) {
        /* Some nodes may return the fields at the top level */
        result = json;
    }

    int64_t local_idx = 0;
    int64_t validated_idx = 0;
    if (neoc_json_get_int(result, "localrootindex", &local_idx) == NEOC_SUCCESS &&
        neoc_json_get_int(result, "validatedrootindex", &validated_idx) == NEOC_SUCCESS) {
        neoc_state_height_t *state_height = NULL;
        neoc_error_t err = neoc_state_height_create((int)local_idx, (int)validated_idx, &state_height);
        if (err == NEOC_SUCCESS) {
            parsed->result = state_height;
            parsed->error_code = 0;
        } else {
            neoc_neo_get_state_height_free(parsed);
            neoc_json_free(json);
            return err;
        }
    }

    neoc_json_free(json);
    *response = parsed;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_state_height_to_json(const neoc_neo_get_state_height_t *response,
                                               char **json_str) {
    if (!response || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *json_str = NULL;
    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (response->jsonrpc) {
        neoc_json_add_string(root, "jsonrpc", response->jsonrpc);
    }
    neoc_json_add_int(root, "id", response->id);

    if (response->error) {
        neoc_json_t *error_obj = neoc_json_create_object();
        if (!error_obj) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        neoc_json_add_int(error_obj, "code", response->error_code);
        neoc_json_add_string(error_obj, "message", response->error);
        neoc_json_add_object(root, "error", error_obj);
    } else if (response->result) {
        neoc_json_t *result = neoc_json_create_object();
        if (!result) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        neoc_json_add_int(result, "localrootindex", response->result->local_root_index);
        neoc_json_add_int(result, "validatedrootindex", response->result->validated_root_index);
        neoc_json_add_object(root, "result", result);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

neoc_state_height_t *neoc_neo_get_state_height_get_state_height(const neoc_neo_get_state_height_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_state_height_has_state_height(const neoc_neo_get_state_height_t *response) {
    if (!response) {
        return false;
    }
    return response->result != NULL && response->error == NULL && response->error_code == 0;
}

neoc_error_t neoc_neo_get_state_height_get_local_root_index(const neoc_neo_get_state_height_t *response,
                                                            int *local_root_index) {
    if (!response || !local_root_index) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *local_root_index = response->result->local_root_index;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_state_height_get_validated_root_index(const neoc_neo_get_state_height_t *response,
                                                                int *validated_root_index) {
    if (!response || !validated_root_index) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *validated_root_index = response->result->validated_root_index;
    return NEOC_SUCCESS;
}

bool neoc_neo_get_state_height_is_synchronized(const neoc_neo_get_state_height_t *response) {
    if (!response || !response->result) {
        return false;
    }
    return response->result->local_root_index == response->result->validated_root_index;
}

neoc_error_t neoc_neo_get_state_height_get_lag(const neoc_neo_get_state_height_t *response,
                                               int *lag) {
    if (!response || !lag) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *lag = response->result->validated_root_index - response->result->local_root_index;
    return NEOC_SUCCESS;
}
