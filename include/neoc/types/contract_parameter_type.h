/**
 * @file contract_parameter_type.h
 * @brief Contract parameter type definitions
 */

#ifndef NEOC_CONTRACT_PARAMETER_TYPE_H
#define NEOC_CONTRACT_PARAMETER_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contract parameter types
 */
typedef enum neoc_contract_parameter_type {
    NEOC_PARAM_TYPE_ANY = 0x00,
    NEOC_PARAM_TYPE_BOOLEAN = 0x10,
    NEOC_PARAM_TYPE_INTEGER = 0x11,
    NEOC_PARAM_TYPE_BYTE_ARRAY = 0x12,
    NEOC_PARAM_TYPE_STRING = 0x13,
    NEOC_PARAM_TYPE_HASH160 = 0x14,
    NEOC_PARAM_TYPE_HASH256 = 0x15,
    NEOC_PARAM_TYPE_PUBLIC_KEY = 0x16,
    NEOC_PARAM_TYPE_SIGNATURE = 0x17,
    NEOC_PARAM_TYPE_ARRAY = 0x20,
    NEOC_PARAM_TYPE_MAP = 0x22,
    NEOC_PARAM_TYPE_INTEROP_INTERFACE = 0x30,
    NEOC_PARAM_TYPE_VOID = 0xff
} neoc_contract_parameter_type_t;

/**
 * Get string representation of parameter type
 */
const char* neoc_contract_parameter_type_to_string(neoc_contract_parameter_type_t type);

/**
 * Parse parameter type from string
 */
neoc_error_t neoc_contract_parameter_type_from_string(const char *str,
                                                       neoc_contract_parameter_type_t *type);

/**
 * Check if type is valid
 */
bool neoc_contract_parameter_type_is_valid(neoc_contract_parameter_type_t type);

/**
 * Get byte value of parameter type
 */
uint8_t neoc_contract_parameter_type_to_byte(neoc_contract_parameter_type_t type);

/**
 * Parse parameter type from byte
 */
neoc_error_t neoc_contract_parameter_type_from_byte(uint8_t byte,
                                                     neoc_contract_parameter_type_t *type);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_PARAMETER_TYPE_H
