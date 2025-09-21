#include "neoc/protocol/core/response/contract_method_token.h"

neoc_error_t neoc_contract_method_token_create_swift(
    const neoc_hash160_t *hash,
    const char *method,
    int param_count,
    bool return_value,
    const char *call_flags,
    neoc_contract_method_token_t **token) {
    (void)hash;
    (void)method;
    (void)param_count;
    (void)return_value;
    (void)call_flags;
    if (token) {
        *token = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_method_token_create_swift not implemented");
}

neoc_error_t neoc_contract_method_token_from_json_swift(
    const char *json_str,
    neoc_contract_method_token_t **token) {
    (void)json_str;
    if (token) {
        *token = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_method_token_from_json_swift not implemented");
}

neoc_error_t neoc_contract_method_token_to_json_swift(
    const neoc_contract_method_token_t *token,
    char **json_str) {
    (void)token;
    if (json_str) {
        *json_str = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_method_token_to_json_swift not implemented");
}

neoc_error_t neoc_contract_method_token_copy_swift(
    const neoc_contract_method_token_t *src,
    neoc_contract_method_token_t **dest) {
    (void)src;
    if (dest) {
        *dest = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_method_token_copy_swift not implemented");
}

bool neoc_contract_method_token_equals_swift(
    const neoc_contract_method_token_t *token1,
    const neoc_contract_method_token_t *token2) {
    (void)token1;
    (void)token2;
    return false;
}
