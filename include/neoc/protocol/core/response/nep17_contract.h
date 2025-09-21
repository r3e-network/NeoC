/**
 * @file nep17_contract.h
 * @brief NEP-17 contract information structure
 * 
 * Converted from Swift source: protocol/core/response/Nep17Contract.swift
 * Represents NEP-17 (fungible token) contract metadata.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEP17_CONTRACT_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEP17_CONTRACT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NEP-17 contract structure
 * 
 * Represents a NEP-17 (fungible token) contract with script hash,
 * symbol, and decimal precision.
 */
typedef struct {
    neoc_hash160_t *script_hash;        /**< Contract script hash */
    char *symbol;                       /**< Token symbol (e.g., "GAS", "NEO") */
    int decimals;                       /**< Decimal precision */
} neoc_nep17_contract_t;

/**
 * @brief Create a new NEP-17 contract
 * 
 * @param script_hash Contract script hash
 * @param symbol Token symbol
 * @param decimals Decimal precision
 * @param contract Output pointer for the created contract (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_create(
    const neoc_hash160_t *script_hash,
    const char *symbol,
    int decimals,
    neoc_nep17_contract_t **contract
);

/**
 * @brief Free a NEP-17 contract
 * 
 * @param contract Contract to free (can be NULL)
 */
void neoc_nep17_contract_free(
    neoc_nep17_contract_t *contract
);

/**
 * @brief Parse NEP-17 contract from JSON string
 * 
 * @param json_str JSON string containing contract data
 * @param contract Output pointer for the parsed contract (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_from_json(
    const char *json_str,
    neoc_nep17_contract_t **contract
);

/**
 * @brief Convert NEP-17 contract to JSON string
 * 
 * @param contract Contract to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_to_json(
    const neoc_nep17_contract_t *contract,
    char **json_str
);

/**
 * @brief Create a copy of a NEP-17 contract
 * 
 * @param src Source contract to copy
 * @param dest Output pointer for the copied contract (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_copy(
    const neoc_nep17_contract_t *src,
    neoc_nep17_contract_t **dest
);

/**
 * @brief Compare two NEP-17 contracts for equality
 * 
 * @param contract1 First contract
 * @param contract2 Second contract
 * @return true if contracts are equal, false otherwise
 */
bool neoc_nep17_contract_equals(
    const neoc_nep17_contract_t *contract1,
    const neoc_nep17_contract_t *contract2
);

/**
 * @brief Calculate minimum unit value based on decimals
 * 
 * Returns the minimum transferable amount (1 / 10^decimals).
 * 
 * @param contract NEP-17 contract
 * @param min_unit Output pointer for minimum unit value
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_get_min_unit(
    const neoc_nep17_contract_t *contract,
    double *min_unit
);

/**
 * @brief Convert raw amount to decimal representation
 * 
 * Converts a raw token amount to its decimal representation.
 * 
 * @param contract NEP-17 contract
 * @param raw_amount Raw amount (in smallest units)
 * @param decimal_amount Output string for decimal amount (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_raw_to_decimal(
    const neoc_nep17_contract_t *contract,
    const char *raw_amount,
    char **decimal_amount
);

/**
 * @brief Convert decimal amount to raw representation
 * 
 * Converts a decimal token amount to its raw representation.
 * 
 * @param contract NEP-17 contract
 * @param decimal_amount Decimal amount string
 * @param raw_amount Output string for raw amount (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_contract_decimal_to_raw(
    const neoc_nep17_contract_t *contract,
    const char *decimal_amount,
    char **raw_amount
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEP17_CONTRACT_H */
