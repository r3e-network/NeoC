/**
 * @file neo_send_raw_transaction.c
 * @brief SendRawTransaction RPC response implementation
 */

#include "neoc/protocol/core/response/neo_send_raw_transaction.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_raw_transaction_result_create(const neoc_hash256_t *hash,
                                                neoc_raw_transaction_result_t **result) {
    if (!hash || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *result = neoc_calloc(1, sizeof(neoc_raw_transaction_result_t));
    if (!*result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    (*result)->hash = *hash;
    return NEOC_SUCCESS;
}

void neoc_raw_transaction_result_free(neoc_raw_transaction_result_t *result) {
    neoc_free(result);
}

neoc_error_t neoc_neo_send_raw_transaction_create(int id,
                                                  neoc_raw_transaction_result_t *transaction_result,
                                                  const char *error,
                                                  int error_code,
                                                  neoc_neo_send_raw_transaction_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *response = neoc_calloc(1, sizeof(neoc_neo_send_raw_transaction_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->jsonrpc = dup_string("2.0");
    if (!(*response)->jsonrpc) {
        neoc_free(*response);
        *response = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->id = id;
    (*response)->result = transaction_result;
    (*response)->error_code = error_code;

    if (error) {
        (*response)->error = dup_string(error);
        if (!(*response)->error) {
            neoc_neo_send_raw_transaction_free(*response);
            *response = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    return NEOC_SUCCESS;
}

void neoc_neo_send_raw_transaction_free(neoc_neo_send_raw_transaction_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_raw_transaction_result_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_neo_send_raw_transaction_from_json(const char *json_str,
                                                     neoc_neo_send_raw_transaction_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_send_raw_transaction_t *parsed = neoc_calloc(1, sizeof(neoc_neo_send_raw_transaction_t));
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

    const char *hash_hex = neoc_json_get_string(result_obj, "hash");
    if (hash_hex) {
        neoc_hash256_t hash;
        if (neoc_hash256_from_hex(&hash, hash_hex) == NEOC_SUCCESS) {
            neoc_raw_transaction_result_t *result = NULL;
            if (neoc_raw_transaction_result_create(&hash, &result) == NEOC_SUCCESS) {
                parsed->result = result;
                parsed->error_code = 0;
            }
        }
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_send_raw_transaction_to_json(const neoc_neo_send_raw_transaction_t *response,
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
        neoc_json_t *result_obj = neoc_json_create_object();
        if (!result_obj) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }

        char hash_hex[NEOC_HASH256_STRING_LENGTH] = {0};
        if (neoc_hash256_to_hex(&response->result->hash, hash_hex, sizeof(hash_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(result_obj, "hash", hash_hex);
        }
        neoc_json_add_object(root, "result", result_obj);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_raw_transaction_result_t *neoc_neo_send_raw_transaction_get_result(const neoc_neo_send_raw_transaction_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_send_raw_transaction_has_result(const neoc_neo_send_raw_transaction_t *response) {
    return response && response->result && response->error == NULL && response->error_code == 0;
}

neoc_error_t neoc_neo_send_raw_transaction_get_hash(const neoc_neo_send_raw_transaction_t *response,
                                                    neoc_hash256_t *hash) {
    if (!response || !hash) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *hash = response->result->hash;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_send_raw_transaction_get_hash_string(const neoc_neo_send_raw_transaction_t *response,
                                                           char **hash_str) {
    if (!response || !hash_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *hash_str = NULL;
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }

    char hex[NEOC_HASH256_STRING_LENGTH] = {0};
    neoc_error_t err = neoc_hash256_to_hex(&response->result->hash, hex, sizeof(hex), false);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    *hash_str = dup_string(hex);
    return *hash_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

bool neoc_neo_send_raw_transaction_is_successful(const neoc_neo_send_raw_transaction_t *response) {
    return response && response->result && response->error == NULL && response->error_code == 0;
}
