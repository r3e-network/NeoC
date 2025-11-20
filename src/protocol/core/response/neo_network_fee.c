/**
 * @file neo_network_fee.c
 * @brief calculate network fee RPC response implementation
 */

#include "neoc/protocol/core/response/neo_network_fee.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

static bool parse_int64_from_string(const char *str, int64_t *out) {
    if (!str || !out) {
        return false;
    }
    errno = 0;
    char *endptr = NULL;
    long long val = strtoll(str, &endptr, 10);
    if (errno != 0 || endptr == str) {
        return false;
    }
    *out = (int64_t)val;
    return true;
}

neoc_error_t neoc_neo_network_fee_create(int64_t network_fee,
                                         neoc_neo_network_fee_t **fee_struct) {
    if (!fee_struct) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *fee_struct = neoc_calloc(1, sizeof(neoc_neo_network_fee_t));
    if (!*fee_struct) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    (*fee_struct)->network_fee = network_fee;
    return NEOC_SUCCESS;
}

void neoc_neo_network_fee_free(neoc_neo_network_fee_t *fee_struct) {
    if (!fee_struct) {
        return;
    }
    neoc_free(fee_struct);
}

neoc_error_t neoc_neo_calculate_network_fee_create(int id,
                                                   neoc_neo_network_fee_t *network_fee,
                                                   const char *error,
                                                   int error_code,
                                                   neoc_neo_calculate_network_fee_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_calculate_network_fee_t *created = neoc_calloc(1, sizeof(neoc_neo_calculate_network_fee_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_string("2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = network_fee;
    created->error_code = error_code;

    if (error) {
        created->error = dup_string(error);
        if (!created->error) {
            neoc_neo_calculate_network_fee_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_calculate_network_fee_free(neoc_neo_calculate_network_fee_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_neo_network_fee_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_neo_calculate_network_fee_from_json(const char *json_str,
                                                     neoc_neo_calculate_network_fee_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_calculate_network_fee_t *parsed = neoc_calloc(1, sizeof(neoc_neo_calculate_network_fee_t));
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

    const char *fee_str = neoc_json_get_string(result_obj, "networkfee");
    int64_t fee_val = 0;
    if (fee_str) {
        if (!parse_int64_from_string(fee_str, &fee_val)) {
            neoc_neo_calculate_network_fee_free(parsed);
            neoc_json_free(json);
            return NEOC_ERROR_INVALID_FORMAT;
        }
    } else {
        neoc_json_get_int(result_obj, "networkfee", &fee_val);
    }

    neoc_neo_network_fee_t *fee = NULL;
    if (neoc_neo_network_fee_create(fee_val, &fee) == NEOC_SUCCESS) {
        parsed->result = fee;
        parsed->error_code = 0;
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_calculate_network_fee_to_json(const neoc_neo_calculate_network_fee_t *response,
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

        char fee_buf[32];
        snprintf(fee_buf, sizeof(fee_buf), "%lld", (long long)response->result->network_fee);
        neoc_json_add_string(result, "networkfee", fee_buf);
        neoc_json_add_object(root, "result", result);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_neo_network_fee_t *neoc_neo_calculate_network_fee_get_fee(const neoc_neo_calculate_network_fee_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_calculate_network_fee_has_fee(const neoc_neo_calculate_network_fee_t *response) {
    return response && response->result && response->error == NULL && response->error_code == 0;
}
