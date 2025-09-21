/**
 * @file contract_parameter.h
 * @brief Header converted from ContractParameter.swift
 */

#ifndef NEOC_CONTRACT_PARAMETER_H
#define NEOC_CONTRACT_PARAMETER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contract parameter types
 */
typedef enum {
    NEOC_CONTRACT_PARAM_TYPE_ANY = 0x00,
    NEOC_CONTRACT_PARAM_TYPE_BOOLEAN = 0x10,
    NEOC_CONTRACT_PARAM_TYPE_INTEGER = 0x11,
    NEOC_CONTRACT_PARAM_TYPE_BYTE_ARRAY = 0x12,
    NEOC_CONTRACT_PARAM_TYPE_STRING = 0x13,
    NEOC_CONTRACT_PARAM_TYPE_HASH160 = 0x14,
    NEOC_CONTRACT_PARAM_TYPE_HASH256 = 0x15,
    NEOC_CONTRACT_PARAM_TYPE_PUBLIC_KEY = 0x16,
    NEOC_CONTRACT_PARAM_TYPE_SIGNATURE = 0x17,
    NEOC_CONTRACT_PARAM_TYPE_ARRAY = 0x20,
    NEOC_CONTRACT_PARAM_TYPE_MAP = 0x22,
    NEOC_CONTRACT_PARAM_TYPE_INTEROP_INTERFACE = 0x30,
    NEOC_CONTRACT_PARAM_TYPE_VOID = 0xFF
} neoc_contract_parameter_type_t;

/**
 * Contract parameter structure
 */
typedef struct neoc_contract_parameter {
    neoc_contract_parameter_type_t type;
    char *name;
    void *value;
    size_t value_size;
} neoc_contract_parameter_t;

/**
 * Create contract parameter
 * @param type Parameter type
 * @param name Parameter name (optional)
 * @param value Parameter value
 * @param value_size Value size
 * @param param Output parameter
 * @return Error code
 */
neoc_error_t neoc_contract_parameter_create(neoc_contract_parameter_type_t type,
                                            const char *name,
                                            const void *value,
                                            size_t value_size,
                                            neoc_contract_parameter_t **param);

/**
 * Free contract parameter
 * @param param Parameter to free
 */
void neoc_contract_parameter_free(neoc_contract_parameter_t *param);

/**
 * Helper functions for creating specific parameter types
 */
neoc_error_t neoc_contract_parameter_create_bool(bool value, neoc_contract_parameter_t **param);
neoc_error_t neoc_contract_parameter_create_int(int64_t value, neoc_contract_parameter_t **param);
neoc_error_t neoc_contract_parameter_create_string(const char *value, neoc_contract_parameter_t **param);
neoc_error_t neoc_contract_parameter_create_bytes(const uint8_t *value, size_t len, neoc_contract_parameter_t **param);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_PARAMETER_H
