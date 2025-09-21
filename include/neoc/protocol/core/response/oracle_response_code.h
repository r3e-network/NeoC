/**
 * @file oracle_response_code.h
 * @brief Oracle response code enumeration
 * 
 * Converted from Swift source: protocol/core/response/OracleResponseCode.swift
 * Provides Oracle response status codes with string and byte representations.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_ORACLE_RESPONSE_CODE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_ORACLE_RESPONSE_CODE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Oracle response code enumeration
 * 
 * Represents the various response codes that can be returned by the Neo Oracle service.
 * Each code has both a string representation and a byte value.
 */
typedef enum {
    NEOC_ORACLE_RESPONSE_SUCCESS = 0x00,                    /**< Success */
    NEOC_ORACLE_RESPONSE_PROTOCOL_NOT_SUPPORTED = 0x10,     /**< Protocol not supported */
    NEOC_ORACLE_RESPONSE_CONSENSUS_UNREACHABLE = 0x12,      /**< Consensus unreachable */
    NEOC_ORACLE_RESPONSE_NOT_FOUND = 0x14,                  /**< Not found */
    NEOC_ORACLE_RESPONSE_TIMEOUT = 0x16,                    /**< Timeout */
    NEOC_ORACLE_RESPONSE_FORBIDDEN = 0x18,                  /**< Forbidden */
    NEOC_ORACLE_RESPONSE_RESPONSE_TOO_LARGE = 0x1a,         /**< Response too large */
    NEOC_ORACLE_RESPONSE_INSUFFICIENT_FUNDS = 0x1c,         /**< Insufficient funds */
    NEOC_ORACLE_RESPONSE_CONTENT_TYPE_NOT_SUPPORTED = 0x1f, /**< Content type not supported */
    NEOC_ORACLE_RESPONSE_ERROR = 0xff                       /**< General error */
} neoc_oracle_response_code_t;

/**
 * @brief Convert Oracle response code to string representation
 * 
 * @param code Oracle response code
 * @return String representation ("Success", "NotFound", etc.), or NULL for invalid code
 */
const char *neoc_oracle_response_code_to_string(
    neoc_oracle_response_code_t code
);

/**
 * @brief Convert string to Oracle response code
 * 
 * @param string String representation ("Success", "NotFound", etc.)
 * @param code Output pointer for the parsed code
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_response_code_from_string(
    const char *string,
    neoc_oracle_response_code_t *code
);

/**
 * @brief Convert Oracle response code to byte value
 * 
 * @param code Oracle response code
 * @return Byte value, or 0xFF for invalid code
 */
uint8_t neoc_oracle_response_code_to_byte(
    neoc_oracle_response_code_t code
);

/**
 * @brief Convert byte value to Oracle response code
 * 
 * @param byte Byte value
 * @param code Output pointer for the parsed code
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_response_code_from_byte(
    uint8_t byte,
    neoc_oracle_response_code_t *code
);

/**
 * @brief Check if Oracle response code is valid
 * 
 * @param code Oracle response code to validate
 * @return true if valid, false otherwise
 */
bool neoc_oracle_response_code_is_valid(
    neoc_oracle_response_code_t code
);

/**
 * @brief Check if Oracle response code indicates success
 * 
 * @param code Oracle response code to check
 * @return true if success, false otherwise
 */
bool neoc_oracle_response_code_is_success(
    neoc_oracle_response_code_t code
);

/**
 * @brief Check if Oracle response code indicates an error
 * 
 * @param code Oracle response code to check
 * @return true if error, false otherwise
 */
bool neoc_oracle_response_code_is_error(
    neoc_oracle_response_code_t code
);

/**
 * @brief Get all supported Oracle response codes
 * 
 * @param codes Output array of supported codes (caller must free)
 * @param count Output count of supported codes
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_oracle_response_code_get_all_supported(
    neoc_oracle_response_code_t **codes,
    size_t *count
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_ORACLE_RESPONSE_CODE_H */
