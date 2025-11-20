/**
 * @file neo_get_wallet_balance.c
 * @brief Neo getwalletbalance response implementation
 */

#include "neoc/protocol/core/response/neo_get_wallet_balance.h"

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

neoc_error_t neoc_wallet_balance_create(const char *balance,
                                        neoc_wallet_balance_t **wallet_balance) {
    if (!balance || !wallet_balance) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *wallet_balance = NULL;
    neoc_wallet_balance_t *created = neoc_calloc(1, sizeof(neoc_wallet_balance_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->balance = neoc_strdup(balance);
    if (!created->balance) {
        neoc_wallet_balance_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    *wallet_balance = created;
    return NEOC_SUCCESS;
}

void neoc_wallet_balance_free(neoc_wallet_balance_t *wallet_balance) {
    if (!wallet_balance) {
        return;
    }
    neoc_free(wallet_balance->balance);
    neoc_free(wallet_balance);
}

neoc_error_t neoc_neo_get_wallet_balance_create(int id,
                                                neoc_wallet_balance_t *wallet_balance,
                                                const char *error,
                                                int error_code,
                                                neoc_neo_get_wallet_balance_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_get_wallet_balance_t *created = neoc_calloc(1, sizeof(neoc_neo_get_wallet_balance_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_or_default("2.0", "2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = wallet_balance;
    created->error_code = error_code;

    if (error) {
        created->error = neoc_strdup(error);
        if (!created->error) {
            neoc_neo_get_wallet_balance_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_get_wallet_balance_free(neoc_neo_get_wallet_balance_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_wallet_balance_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_neo_get_wallet_balance_from_json(const char *json_str,
                                                   neoc_neo_get_wallet_balance_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_get_wallet_balance_t *parsed = neoc_calloc(1, sizeof(neoc_neo_get_wallet_balance_t));
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

    const char *balance = neoc_json_get_string(result, "balance");
    if (!balance) {
        balance = neoc_json_get_string(result, "Balance");
    }

    if (balance) {
        neoc_wallet_balance_t *data = NULL;
        neoc_error_t err = neoc_wallet_balance_create(balance, &data);
        if (err == NEOC_SUCCESS) {
            parsed->result = data;
            parsed->error_code = 0;
        } else {
            neoc_neo_get_wallet_balance_free(parsed);
            neoc_json_free(json);
            return err;
        }
    }

    neoc_json_free(json);
    *response = parsed;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_wallet_balance_to_json(const neoc_neo_get_wallet_balance_t *response,
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
        neoc_json_add_string(result, "balance", response->result->balance);
        neoc_json_add_object(root, "result", result);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

neoc_wallet_balance_t *neoc_neo_get_wallet_balance_get_balance(const neoc_neo_get_wallet_balance_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_wallet_balance_has_balance(const neoc_neo_get_wallet_balance_t *response) {
    if (!response) {
        return false;
    }
    return response->result != NULL && response->error == NULL && response->error_code == 0;
}

const char *neoc_neo_get_wallet_balance_get_balance_string(const neoc_neo_get_wallet_balance_t *response) {
    if (!response || !response->result) {
        return NULL;
    }
    return response->result->balance;
}

neoc_error_t neoc_neo_get_wallet_balance_get_balance_double(const neoc_neo_get_wallet_balance_t *response,
                                                            double *balance) {
    if (!response || !balance) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    if (!string_to_double(response->result->balance, balance)) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    return NEOC_SUCCESS;
}

bool neoc_neo_get_wallet_balance_has_funds(const neoc_neo_get_wallet_balance_t *response) {
    double bal = 0.0;
    if (neoc_neo_get_wallet_balance_get_balance_double(response, &bal) != NEOC_SUCCESS) {
        return false;
    }
    return bal > 0.0;
}

neoc_error_t neoc_neo_get_wallet_balance_copy_balance_string(const neoc_neo_get_wallet_balance_t *response,
                                                             char **balance_copy) {
    if (!response || !balance_copy) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *balance_copy = NULL;
    if (!response->result || !response->result->balance) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *balance_copy = neoc_strdup(response->result->balance);
    return *balance_copy ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}
