/**
 * @file contract_error.h
 * @brief Contract-specific error definitions for NeoC SDK
 * 
 * Provides error codes and structures for contract operations including
 * name service validation, contract invocation errors, and domain resolution.
 */

#ifndef NEOC_CONTRACT_ERROR_H
#define NEOC_CONTRACT_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "neoc/neoc_error.h"

/**
 * @brief Contract-specific error codes
 */
typedef enum {
    /* Contract errors start at -100 */
    NEOC_CONTRACT_ERROR_INVALID_NEO_NAME = -100,           ///< Invalid NNS name format
    NEOC_CONTRACT_ERROR_INVALID_NNS_ROOT = -101,           ///< Invalid NNS root domain
    NEOC_CONTRACT_ERROR_UNEXPECTED_RETURN_TYPE = -102,     ///< Unexpected stack item return type
    NEOC_CONTRACT_ERROR_UNRESOLVABLE_DOMAIN = -103,        ///< Domain name could not be resolved
    NEOC_CONTRACT_ERROR_INVALID_MANIFEST = -104,           ///< Invalid contract manifest
    NEOC_CONTRACT_ERROR_INVALID_NEF = -105,               ///< Invalid NEF file format
    NEOC_CONTRACT_ERROR_INSUFFICIENT_FUNDS = -106,         ///< Insufficient funds for operation
    NEOC_CONTRACT_ERROR_INVALID_TOKEN_AMOUNT = -107,       ///< Invalid token amount or decimals
} neoc_contract_error_t;

/**
 * @brief Contract error context structure
 */
typedef struct {
    neoc_contract_error_t error_code;      ///< Specific contract error code
    char name[256];                        ///< Associated name or identifier
    char expected_types[512];              ///< Expected types (for return type errors)
    char actual_type[64];                  ///< Actual type received
} neoc_contract_error_context_t;

/**
 * @brief Get human-readable description for contract error
 * 
 * @param error_code The contract error code
 * @param context Optional error context for detailed messages
 * @param buffer Buffer to store error message
 * @param buffer_size Size of buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_error_get_description(
    neoc_contract_error_t error_code,
    const neoc_contract_error_context_t* context,
    char* buffer,
    size_t buffer_size
);

/**
 * @brief Validate NNS name format
 * 
 * @param name The name to validate
 * @return true if valid, false otherwise
 */
bool neoc_contract_is_valid_nns_name(const char* name);

/**
 * @brief Validate NNS root format
 * 
 * @param root The root to validate
 * @return true if valid, false otherwise
 */
bool neoc_contract_is_valid_nns_root(const char* root);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CONTRACT_ERROR_H */
