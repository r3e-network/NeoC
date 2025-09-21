/**
 * @file contract_method_token.h
 * @brief Contract method token structure
 * 
 * Converted from Swift source: protocol/core/response/ContractMethodToken.swift
 * This header provides access to contract method token types defined in contract_response_types.h
 * and adds Swift-compatible functions.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_METHOD_TOKEN_H
#define NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_METHOD_TOKEN_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include "neoc/protocol/contract_response_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Note: neoc_contract_method_token_t is defined in contract_response_types.h */

/**
 * @brief Create a new contract method token (Swift-compatible)
 * 
 * Extended version with Swift parameter mapping.
 * 
 * @param hash Contract hash
 * @param method Method name
 * @param param_count Number of parameters
 * @param return_value Whether method has return value
 * @param call_flags Call flags string
 * @param token Output pointer for the created token (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_method_token_create_swift(
    const neoc_hash160_t *hash,
    const char *method,
    int param_count,
    bool return_value,
    const char *call_flags,
    neoc_contract_method_token_t **token
);

/**
 * @brief Parse contract method token from JSON string (Swift-compatible)
 * 
 * @param json_str JSON string containing method token data
 * @param token Output pointer for the parsed token (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_method_token_from_json_swift(
    const char *json_str,
    neoc_contract_method_token_t **token
);

/**
 * @brief Convert contract method token to JSON string (Swift-compatible)
 * 
 * @param token Method token to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_method_token_to_json_swift(
    const neoc_contract_method_token_t *token,
    char **json_str
);

/**
 * @brief Create a copy of a contract method token (Swift-compatible)
 * 
 * @param src Source token to copy
 * @param dest Output pointer for the copied token (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_method_token_copy_swift(
    const neoc_contract_method_token_t *src,
    neoc_contract_method_token_t **dest
);

/**
 * @brief Compare two contract method tokens for equality (Swift-compatible)
 * 
 * @param token1 First token
 * @param token2 Second token
 * @return true if tokens are equal, false otherwise
 */
bool neoc_contract_method_token_equals_swift(
    const neoc_contract_method_token_t *token1,
    const neoc_contract_method_token_t *token2
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_METHOD_TOKEN_H */
