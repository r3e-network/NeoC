/**
 * @file record_type.h
 * @brief Record type enumeration for DNS-like records
 * 
 * Converted from Swift source: protocol/core/RecordType.swift
 * Provides record types for NNS (Neo Name Service) operations.
 */

#ifndef NEOC_PROTOCOL_CORE_RECORD_TYPE_H
#define NEOC_PROTOCOL_CORE_RECORD_TYPE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Record type enumeration
 * 
 * Represents different types of DNS-like records supported by NNS.
 * Each type has both a numeric byte value and string representation.
 */
typedef enum {
    NEOC_RECORD_TYPE_A = 1,         /**< Address record (IPv4) */
    NEOC_RECORD_TYPE_CNAME = 5,     /**< Canonical name record */
    NEOC_RECORD_TYPE_TXT = 16,      /**< Text record */
    NEOC_RECORD_TYPE_AAAA = 28      /**< IPv6 address record */
} neoc_record_type_t;

/**
 * @brief Convert record type to string representation
 * 
 * @param type Record type
 * @return String representation ("A", "CNAME", "TXT", "AAAA"), or NULL for invalid type
 */
const char *neoc_record_type_to_string(
    neoc_record_type_t type
);

/**
 * @brief Convert string to record type
 * 
 * @param string String representation ("A", "CNAME", "TXT", "AAAA")
 * @param type Output pointer for the parsed type
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_type_from_string(
    const char *string,
    neoc_record_type_t *type
);

/**
 * @brief Convert record type to byte value
 * 
 * @param type Record type
 * @return Byte value, or 0 for invalid type
 */
uint8_t neoc_record_type_to_byte(
    neoc_record_type_t type
);

/**
 * @brief Convert byte value to record type
 * 
 * @param byte Byte value
 * @param type Output pointer for the parsed type
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_type_from_byte(
    uint8_t byte,
    neoc_record_type_t *type
);

/**
 * @brief Check if record type is valid
 * 
 * @param type Record type to validate
 * @return true if valid, false otherwise
 */
bool neoc_record_type_is_valid(
    neoc_record_type_t type
);

/**
 * @brief Get all supported record types
 * 
 * @param types Output array of supported types (caller must free)
 * @param count Output count of supported types
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_record_type_get_all_supported(
    neoc_record_type_t **types,
    size_t *count
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RECORD_TYPE_H */
