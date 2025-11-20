/**
 * @file neo_validate_address.c
 * @brief validateaddress RPC response handling
 */

#include "neoc/protocol/core/response/neo_validate_address.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_neo_validate_address_result_create(const char *address,
                                                     bool valid,
                                                     neoc_neo_validate_address_result_t **result) {
    if (!address || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *result = NULL;
    neoc_neo_validate_address_result_t *created = neoc_calloc(1, sizeof(neoc_neo_validate_address_result_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->address = dup_string(address);
    if (!created->address) {
        neoc_neo_validate_address_result_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->valid = valid;
    *result = created;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_validate_address_result_set_properties(neoc_neo_validate_address_result_t *result,
                                                             bool is_script_hash,
                                                             bool is_multisig,
                                                             bool is_standard,
                                                             const char *script_hash) {
    if (!result) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    result->is_script_hash = is_script_hash;
    result->is_multisig = is_multisig;
    result->is_standard = is_standard;

    if (script_hash) {
        char *copy = dup_string(script_hash);
        if (!copy) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        if (result->script_hash) {
            neoc_free(result->script_hash);
        }
        result->script_hash = copy;
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_validate_address_result_copy(const neoc_neo_validate_address_result_t *source,
                                                   neoc_neo_validate_address_result_t **copy_out) {
    if (!source || !copy_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *copy_out = NULL;
    neoc_neo_validate_address_result_t *copy = neoc_calloc(1, sizeof(neoc_neo_validate_address_result_t));
    if (!copy) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    copy->valid = source->valid;
    copy->is_script_hash = source->is_script_hash;
    copy->is_multisig = source->is_multisig;
    copy->is_standard = source->is_standard;

    if (source->address) {
        copy->address = dup_string(source->address);
        if (!copy->address) {
            neoc_neo_validate_address_result_free(copy);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    if (source->script_hash) {
        copy->script_hash = dup_string(source->script_hash);
        if (!copy->script_hash) {
            neoc_neo_validate_address_result_free(copy);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *copy_out = copy;
    return NEOC_SUCCESS;
}

void neoc_neo_validate_address_result_free(neoc_neo_validate_address_result_t *result) {
    if (!result) {
        return;
    }
    neoc_free(result->address);
    neoc_free(result->script_hash);
    neoc_free(result);
}

neoc_error_t neoc_neo_validate_address_response_create(int id,
                                                       neoc_neo_validate_address_result_t *result,
                                                       const char *error,
                                                       int error_code,
                                                       neoc_neo_validate_address_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_validate_address_response_t *created = neoc_calloc(1, sizeof(neoc_neo_validate_address_response_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_string("2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = result; /* ownership transferred */
    created->error_code = error_code;

    if (error) {
        created->error = dup_string(error);
        if (!created->error) {
            neoc_neo_validate_address_response_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

static neoc_neo_validate_address_result_t *parse_result(neoc_json_t *result_obj) {
    if (!result_obj) {
        return NULL;
    }

    const char *address = neoc_json_get_string(result_obj, "address");
    bool valid = false;
    neoc_json_get_bool(result_obj, "isvalid", &valid);

    neoc_neo_validate_address_result_t *result = NULL;
    if (neoc_neo_validate_address_result_create(address ? address : "", valid, &result) != NEOC_SUCCESS) {
        return NULL;
    }

    bool is_script_hash = false;
    bool is_multisig = false;
    bool is_standard = false;
    neoc_json_get_bool(result_obj, "isscripthash", &is_script_hash);
    neoc_json_get_bool(result_obj, "ismultisig", &is_multisig);
    neoc_json_get_bool(result_obj, "isstandard", &is_standard);

    const char *script_hash = neoc_json_get_string(result_obj, "scripthash");
    if (neoc_neo_validate_address_result_set_properties(result,
                                                        is_script_hash,
                                                        is_multisig,
                                                        is_standard,
                                                        script_hash) != NEOC_SUCCESS) {
        neoc_neo_validate_address_result_free(result);
        return NULL;
    }

    return result;
}

neoc_error_t neoc_neo_validate_address_response_from_json(const char *json_string,
                                                          neoc_neo_validate_address_response_t **response) {
    if (!json_string || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_string);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_validate_address_response_t *parsed = neoc_calloc(1, sizeof(neoc_neo_validate_address_response_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_string(neoc_json_get_string(json, "jsonrpc"));
    if (!parsed->jsonrpc) {
        parsed->jsonrpc = dup_string("2.0");
    }

    int64_t id_value = 0;
    if (neoc_json_get_int(json, "id", &id_value) == NEOC_SUCCESS) {
        parsed->id = (int)id_value;
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

    neoc_neo_validate_address_result_t *result = parse_result(result_obj);
    parsed->result = result;
    if (result) {
        parsed->error_code = 0;
    }

    neoc_json_free(json);
    *response = parsed;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_validate_address_response_to_json(const neoc_neo_validate_address_response_t *response,
                                                        char **json_string) {
    if (!response || !json_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *json_string = NULL;
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
        neoc_json_add_string(result, "address", response->result->address);
        neoc_json_add_bool(result, "isvalid", response->result->valid);
        neoc_json_add_bool(result, "isscripthash", response->result->is_script_hash);
        neoc_json_add_bool(result, "ismultisig", response->result->is_multisig);
        neoc_json_add_bool(result, "isstandard", response->result->is_standard);
        if (response->result->script_hash) {
            neoc_json_add_string(result, "scripthash", response->result->script_hash);
        }
        neoc_json_add_object(root, "result", result);
    }

    *json_string = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!*json_string) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

bool neoc_neo_validate_address_response_is_success(const neoc_neo_validate_address_response_t *response) {
    return response && response->error == NULL && response->error_code == 0;
}

bool neoc_neo_validate_address_response_is_valid_address(const neoc_neo_validate_address_response_t *response) {
    return neoc_neo_validate_address_response_is_success(response) &&
           response->result &&
           response->result->valid;
}

neoc_error_t neoc_neo_validate_address_response_get_script_hash(const neoc_neo_validate_address_response_t *response,
                                                                const char **script_hash) {
    if (!response || !script_hash) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result || !response->result->script_hash) {
        *script_hash = NULL;
        return NEOC_ERROR_INVALID_STATE;
    }
    *script_hash = response->result->script_hash;
    return NEOC_SUCCESS;
}

bool neoc_neo_validate_address_response_is_multisig(const neoc_neo_validate_address_response_t *response) {
    return response && response->result && response->result->is_multisig;
}

bool neoc_neo_validate_address_response_is_standard(const neoc_neo_validate_address_response_t *response) {
    return response && response->result && response->result->is_standard;
}

void neoc_neo_validate_address_response_free(neoc_neo_validate_address_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_neo_validate_address_result_free(response->result);
    }
    neoc_free(response);
}
