/**
 * @file record_state.c
 * @brief RecordState RPC model implementation
 */

#include "neoc/protocol/core/response/record_state.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/core/record_type.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_record_state_create(const char *name,
                                      neoc_record_type_t record_type,
                                      const char *data,
                                      neoc_record_state_t **state) {
    if (!name || !data || !state) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *state = neoc_calloc(1, sizeof(neoc_record_state_t));
    if (!*state) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*state)->name = dup_string(name);
    (*state)->data = dup_string(data);
    (*state)->record_type = record_type;

    if (!(*state)->name || !(*state)->data) {
        neoc_record_state_free(*state);
        *state = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    return NEOC_SUCCESS;
}

void neoc_record_state_free(neoc_record_state_t *state) {
    if (!state) {
        return;
    }
    neoc_free(state->name);
    neoc_free(state->data);
    neoc_free(state);
}

neoc_error_t neoc_record_state_response_create(const char *jsonrpc,
                                               int id,
                                               neoc_record_state_t *result,
                                               const char *error,
                                               int error_code,
                                               neoc_record_state_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = neoc_calloc(1, sizeof(neoc_record_state_response_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->jsonrpc = dup_string(jsonrpc ? jsonrpc : "2.0");
    if (!(*response)->jsonrpc) {
        neoc_record_state_response_free(*response);
        *response = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->id = id;
    (*response)->result = result;
    (*response)->error_code = error_code;

    if (error) {
        (*response)->error = dup_string(error);
        if (!(*response)->error) {
            neoc_record_state_response_free(*response);
            *response = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    return NEOC_SUCCESS;
}

void neoc_record_state_response_free(neoc_record_state_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_record_state_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_record_state_from_json(const char *json_str,
                                         neoc_record_state_t **state) {
    if (!json_str || !state) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *state = NULL;

    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    const char *name = neoc_json_get_string(json, "name");
    const char *data = neoc_json_get_string(json, "data");
    const char *type_str = neoc_json_get_string(json, "type");

    neoc_record_type_t record_type = 0;
    if (!type_str || neoc_record_type_from_string(type_str, &record_type) != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_error_t err = (name && data)
        ? neoc_record_state_create(name, record_type, data, state)
        : NEOC_ERROR_INVALID_FORMAT;

    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_record_state_response_from_json(const char *json_str,
                                                  neoc_record_state_response_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_record_state_response_t *parsed = neoc_calloc(1, sizeof(neoc_record_state_response_t));
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

    neoc_json_t *result_obj = neoc_json_get_object(json, "result");
    if (!result_obj) {
        result_obj = json;
    }

    char *result_json = neoc_json_to_string(result_obj);
    if (result_json) {
        neoc_record_state_t *state = NULL;
        if (neoc_record_state_from_json(result_json, &state) == NEOC_SUCCESS) {
            parsed->result = state;
            parsed->error_code = 0;
        }
        neoc_free(result_json);
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_record_state_to_json(const neoc_record_state_t *state,
                                       char **json_str) {
    if (!state || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    const char *type_str = neoc_record_type_to_string(state->record_type);
    if (!type_str) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (state->name) {
        neoc_json_add_string(root, "name", state->name);
    }
    if (state->data) {
        neoc_json_add_string(root, "data", state->data);
    }
    neoc_json_add_string(root, "type", type_str);

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_record_state_copy(const neoc_record_state_t *src,
                                    neoc_record_state_t **dest) {
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    return neoc_record_state_create(src->name, src->record_type, src->data, dest);
}

bool neoc_record_state_equals(const neoc_record_state_t *state1,
                              const neoc_record_state_t *state2) {
    if (state1 == state2) {
        return true;
    }
    if (!state1 || !state2) {
        return false;
    }
    const char *name1 = state1->name ? state1->name : "";
    const char *name2 = state2->name ? state2->name : "";
    const char *data1 = state1->data ? state1->data : "";
    const char *data2 = state2->data ? state2->data : "";

    return state1->record_type == state2->record_type &&
           strcmp(name1, name2) == 0 &&
           strcmp(data1, data2) == 0;
}
