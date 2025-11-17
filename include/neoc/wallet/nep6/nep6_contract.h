/**
 * @file nep6_contract.h
 * @brief NEP-6 contract structure
 * 
 * Based on Swift source: wallet/nep6/NEP6Contract.swift
 * Represents a contract in NEP-6 wallet format
 */

#ifndef NEOC_WALLET_NEP6_NEP6_CONTRACT_H
#define NEOC_WALLET_NEP6_NEP6_CONTRACT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/contract_parameter_type.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NEP-6 contract parameter
 * 
 * Represents a parameter definition for a contract method
 */
typedef struct {
    char *name;                             /**< Parameter name */
    neoc_contract_parameter_type_t type;    /**< Parameter type */
} neoc_nep6_parameter_t;

/**
 * @brief NEP-6 contract structure
 * 
 * Represents a contract in NEP-6 standard wallet format.
 * Based on Swift NEP6Contract structure.
 */
typedef struct {
    char *script;                           /**< Contract script (nullable) */
    neoc_nep6_parameter_t *parameters;      /**< Array of contract parameters */
    size_t parameter_count;                 /**< Number of parameters */
    bool is_deployed;                       /**< Whether contract is deployed */
} neoc_nep6_contract_t;

const char* neoc_nep6_contract_get_script(const neoc_nep6_contract_t *contract);
size_t neoc_nep6_contract_get_parameter_count(const neoc_nep6_contract_t *contract);
neoc_nep6_parameter_t* neoc_nep6_contract_get_parameter(const neoc_nep6_contract_t *contract,
                                                        size_t index);
bool neoc_nep6_contract_is_deployed(const neoc_nep6_contract_t *contract);

/**
 * @brief Create a new NEP-6 contract
 * 
 * @param script Contract script (can be NULL)
 * @param parameters Array of contract parameters (can be NULL)
 * @param parameter_count Number of parameters
 * @param is_deployed Whether the contract is deployed
 * @param contract Pointer to store the created contract
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_contract_create(
    const char *script,
    const neoc_nep6_parameter_t *parameters,
    size_t parameter_count,
    bool is_deployed,
    neoc_nep6_contract_t **contract
);

/**
 * @brief Free a NEP-6 contract and its resources
 * 
 * @param contract Contract to free
 */
void neoc_nep6_contract_free(
    neoc_nep6_contract_t *contract
);

/**
 * @brief Parse JSON into NEP-6 contract
 * 
 * @param json_str JSON string to parse
 * @param contract Pointer to store the parsed contract
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_contract_from_json(
    const char *json_str,
    neoc_nep6_contract_t **contract
);

/**
 * @brief Convert NEP-6 contract to JSON string
 * 
 * @param contract Contract to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_contract_to_json(
    const neoc_nep6_contract_t *contract,
    char **json_str
);

/**
 * @brief Create a NEP-6 parameter
 * 
 * @param name Parameter name
 * @param type Parameter type
 * @param parameter Pointer to store the created parameter
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep6_parameter_create(
    const char *name,
    neoc_contract_parameter_type_t type,
    neoc_nep6_parameter_t **parameter
);

const char* neoc_nep6_parameter_get_name(const neoc_nep6_parameter_t *parameter);
neoc_contract_parameter_type_t neoc_nep6_parameter_get_type(const neoc_nep6_parameter_t *parameter);

/**
 * @brief Free a NEP-6 parameter
 * 
 * @param parameter Parameter to free
 */
void neoc_nep6_parameter_free(
    neoc_nep6_parameter_t *parameter
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_WALLET_NEP6_NEP6_CONTRACT_H */
