#include "neoc/protocol/core/response/contract_method_token.h"

#include "neoc/neo_constants.h"
#include "neoc/types/call_flags.h"
#include "neoc/utils/json.h"

#include <string.h>

static neoc_contract_method_token_t *neoc_contract_method_token_alloc(void) {
    return (neoc_contract_method_token_t *)neoc_calloc(
        1, sizeof(neoc_contract_method_token_t));
}

static void neoc_contract_method_token_free_internal(
    neoc_contract_method_token_t *token) {
    if (!token) {
        return;
    }
    if (token->method) {
        neoc_free(token->method);
    }
    neoc_free(token);
}

neoc_error_t neoc_contract_method_token_create_swift(
    const neoc_hash160_t *hash,
    const char *method,
    int param_count,
    bool return_value,
    const char *call_flags,
    neoc_contract_method_token_t **token) {
    if (!hash || !method || !call_flags || !token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid method token arguments");
    }
    if (param_count < 0 || param_count > 0xFFFF) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Parameter count out of range");
    }

    neoc_call_flags_t flags_enum = NEOC_CALL_FLAG_NONE;
    neoc_error_t err = neoc_call_flags_from_string(call_flags, &flags_enum);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_contract_method_token_t *result = neoc_contract_method_token_alloc();
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to allocate contract method token");
    }

    result->hash = *hash;
    result->method = neoc_strdup(method);
    if (!result->method) {
        neoc_contract_method_token_free_internal(result);
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to copy method name");
    }
    result->params_count = (uint16_t)param_count;
    result->has_return_value = return_value;
    result->call_flags = neoc_call_flags_to_byte(flags_enum);

    *token = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_method_token_from_json_swift(
    const char *json_str,
    neoc_contract_method_token_t **token) {
    if (!json_str || !token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid JSON method token arguments");
    }

    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT,
                              "Failed to parse method token JSON");
    }

    const char *hash_str = neoc_json_get_string(json, "hash");
    const char *method = neoc_json_get_string(json, "method");
    const char *call_flags = neoc_json_get_string(json, "callflags");

    if (!hash_str || !method || !call_flags) {
        neoc_json_free(json);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT,
                              "Method token JSON missing required fields");
    }

    double param_count_double = 0.0;
    neoc_error_t err = neoc_json_get_number(json, "paramcount",
                                            &param_count_double);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return err;
    }

    bool has_return_value = false;
    err = neoc_json_get_bool(json, "hasreturnvalue", &has_return_value);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return err;
    }

    neoc_hash160_t hash = {0};
    err = neoc_hash160_from_string(hash_str, &hash);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return err;
    }

    err = neoc_contract_method_token_create_swift(
        &hash,
        method,
        (int)param_count_double,
        has_return_value,
        call_flags,
        token);

    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_contract_method_token_to_json_swift(
    const neoc_contract_method_token_t *token,
    char **json_str) {
    if (!token || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid method token for JSON conversion");
    }

    *json_str = NULL;
    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                              "JSON support not available");
    }

    char hash_hex[NEOC_HASH160_STRING_LENGTH] = {0};
    neoc_error_t err = neoc_hash160_to_hex(&token->hash, hash_hex,
                                           sizeof(hash_hex), false);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    char *call_flags_str =
        neoc_call_flags_to_string((neoc_call_flags_t)token->call_flags);
    if (!call_flags_str) {
        neoc_json_free(root);
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to encode call flags string");
    }

    err = neoc_json_add_string(root, "hash", hash_hex);
    if (err == NEOC_SUCCESS) {
        err = neoc_json_add_string(root, "method", token->method);
    }
    if (err == NEOC_SUCCESS) {
        err = neoc_json_add_int(root, "paramcount",
                                (int64_t)token->params_count);
    }
    if (err == NEOC_SUCCESS) {
        err = neoc_json_add_bool(root, "hasreturnvalue",
                                 token->has_return_value);
    }
    if (err == NEOC_SUCCESS) {
        err = neoc_json_add_string(root, "callflags", call_flags_str);
    }

    neoc_free(call_flags_str);

    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    char *serialized = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!serialized) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to serialize method token JSON");
    }

    *json_str = serialized;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_method_token_copy_swift(
    const neoc_contract_method_token_t *src,
    neoc_contract_method_token_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid method token copy arguments");
    }

    char *flags_str =
        neoc_call_flags_to_string((neoc_call_flags_t)src->call_flags);
    if (!flags_str) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to convert call flags to string");
    }

    neoc_error_t err = neoc_contract_method_token_create_swift(
        &src->hash,
        src->method,
        src->params_count,
        src->has_return_value,
        flags_str,
        dest);

    neoc_free(flags_str);
    return err;
}

bool neoc_contract_method_token_equals_swift(
    const neoc_contract_method_token_t *token1,
    const neoc_contract_method_token_t *token2) {
    if (token1 == token2) {
        return true;
    }
    if (!token1 || !token2) {
        return false;
    }

    if (memcmp(&token1->hash, &token2->hash, sizeof(neoc_hash160_t)) != 0) {
        return false;
    }
    if ((token1->method == NULL) != (token2->method == NULL)) {
        return false;
    }
    if (token1->method &&
        strcmp(token1->method, token2->method) != 0) {
        return false;
    }
    if (token1->params_count != token2->params_count ||
        token1->has_return_value != token2->has_return_value ||
        token1->call_flags != token2->call_flags) {
        return false;
    }

    return true;
}
