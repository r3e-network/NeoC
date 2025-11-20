/**
 * @file neo_get_unclaimed_gas.c
 * @brief Neo getunclaimedgas response implementation
 */

#include "neoc/protocol/core/response/neo_get_unclaimed_gas.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static char *dup_or_default(const char *value, const char *fallback) {
    if (value) {
        return neoc_strdup(value);
    }
    return fallback ? neoc_strdup(fallback) : NULL;
}

static bool string_to_double(const char *input, double *out) {
    if (!input || !out) {
        return false;
    }
    errno = 0;
    char *endptr = NULL;
    double val = strtod(input, &endptr);
    if (errno != 0 || endptr == input) {
        return false;
    }
    *out = val;
    return true;
}

neoc_error_t neoc_get_unclaimed_gas_create(const char *unclaimed,
                                           const char *address,
                                           neoc_get_unclaimed_gas_t **unclaimed_gas) {
    if (!unclaimed || !address || !unclaimed_gas) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *unclaimed_gas = NULL;
    neoc_get_unclaimed_gas_t *created = neoc_calloc(1, sizeof(neoc_get_unclaimed_gas_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->unclaimed = neoc_strdup(unclaimed);
    created->address = neoc_strdup(address);
    if (!created->unclaimed || !created->address) {
        neoc_get_unclaimed_gas_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    *unclaimed_gas = created;
    return NEOC_SUCCESS;
}

void neoc_get_unclaimed_gas_free(neoc_get_unclaimed_gas_t *unclaimed_gas) {
    if (!unclaimed_gas) {
        return;
    }
    neoc_free(unclaimed_gas->unclaimed);
    neoc_free(unclaimed_gas->address);
    neoc_free(unclaimed_gas);
}

neoc_error_t neoc_neo_get_unclaimed_gas_create(int id,
                                               neoc_get_unclaimed_gas_t *unclaimed_gas,
                                               const char *error,
                                               int error_code,
                                               neoc_neo_get_unclaimed_gas_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_get_unclaimed_gas_t *created = neoc_calloc(1, sizeof(neoc_neo_get_unclaimed_gas_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_or_default("2.0", "2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = unclaimed_gas;
    created->error_code = error_code;

    if (error) {
        created->error = neoc_strdup(error);
        if (!created->error) {
            neoc_neo_get_unclaimed_gas_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_get_unclaimed_gas_free(neoc_neo_get_unclaimed_gas_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_get_unclaimed_gas_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_neo_get_unclaimed_gas_from_json(const char *json_str,
                                                  neoc_neo_get_unclaimed_gas_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_get_unclaimed_gas_t *parsed = neoc_calloc(1, sizeof(neoc_neo_get_unclaimed_gas_t));
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
        result = json;
    }

    const char *unclaimed = neoc_json_get_string(result, "unclaimed");
    const char *address = neoc_json_get_string(result, "address");
    if (unclaimed && address) {
        neoc_get_unclaimed_gas_t *data = NULL;
        neoc_error_t err = neoc_get_unclaimed_gas_create(unclaimed, address, &data);
        if (err == NEOC_SUCCESS) {
            parsed->result = data;
            parsed->error_code = 0;
        } else {
            neoc_neo_get_unclaimed_gas_free(parsed);
            neoc_json_free(json);
            return err;
        }
    }

    neoc_json_free(json);
    *response = parsed;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_unclaimed_gas_to_json(const neoc_neo_get_unclaimed_gas_t *response,
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
        neoc_json_add_string(result, "unclaimed", response->result->unclaimed);
        neoc_json_add_string(result, "address", response->result->address);
        neoc_json_add_object(root, "result", result);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

neoc_get_unclaimed_gas_t *neoc_neo_get_unclaimed_gas_get_data(const neoc_neo_get_unclaimed_gas_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_unclaimed_gas_has_data(const neoc_neo_get_unclaimed_gas_t *response) {
    if (!response) {
        return false;
    }
    return response->result != NULL && response->error == NULL && response->error_code == 0;
}

const char *neoc_neo_get_unclaimed_gas_get_amount_string(const neoc_neo_get_unclaimed_gas_t *response) {
    if (!response || !response->result) {
        return NULL;
    }
    return response->result->unclaimed;
}

const char *neoc_neo_get_unclaimed_gas_get_address(const neoc_neo_get_unclaimed_gas_t *response) {
    if (!response || !response->result) {
        return NULL;
    }
    return response->result->address;
}

neoc_error_t neoc_neo_get_unclaimed_gas_get_amount_double(const neoc_neo_get_unclaimed_gas_t *response,
                                                          double *amount) {
    if (!response || !amount) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    if (!string_to_double(response->result->unclaimed, amount)) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    return NEOC_SUCCESS;
}

bool neoc_neo_get_unclaimed_gas_has_unclaimed(const neoc_neo_get_unclaimed_gas_t *response) {
    double amount = 0.0;
    if (neoc_neo_get_unclaimed_gas_get_amount_double(response, &amount) != NEOC_SUCCESS) {
        return false;
    }
    return amount > 0.0;
}

neoc_error_t neoc_neo_get_unclaimed_gas_copy_amount_string(const neoc_neo_get_unclaimed_gas_t *response,
                                                           char **amount_copy) {
    if (!response || !amount_copy) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *amount_copy = NULL;
    if (!response->result || !response->result->unclaimed) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *amount_copy = neoc_strdup(response->result->unclaimed);
    return *amount_copy ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_neo_get_unclaimed_gas_copy_address_string(const neoc_neo_get_unclaimed_gas_t *response,
                                                            char **address_copy) {
    if (!response || !address_copy) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *address_copy = NULL;
    if (!response->result || !response->result->address) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *address_copy = neoc_strdup(response->result->address);
    return *address_copy ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}
