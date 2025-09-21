/**
 * @file contract_storage_entry.h
 * @brief Contract storage entry structure
 * 
 * Converted from Swift source: protocol/core/response/ContractStorageEntry.swift
 * This header provides access to contract storage entry types defined in contract_response_types.h
 * and adds Swift-compatible functions.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_STORAGE_ENTRY_H
#define NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_STORAGE_ENTRY_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/contract_response_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Note: neoc_contract_storage_entry_t is defined in contract_response_types.h */

/**
 * @brief Create a new contract storage entry (Swift-compatible)
 * 
 * Extended version with Swift string parameter mapping.
 * 
 * @param key Storage key as hex string
 * @param value Storage value as hex string
 * @param entry Output pointer for the created entry (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_create_swift(
    const char *key,
    const char *value,
    neoc_contract_storage_entry_t **entry
);

/**
 * @brief Parse contract storage entry from JSON string (Swift-compatible)
 * 
 * @param json_str JSON string containing storage entry data
 * @param entry Output pointer for the parsed entry (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_from_json_swift(
    const char *json_str,
    neoc_contract_storage_entry_t **entry
);

/**
 * @brief Convert contract storage entry to JSON string (Swift-compatible)
 * 
 * @param entry Storage entry to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_to_json_swift(
    const neoc_contract_storage_entry_t *entry,
    char **json_str
);

/**
 * @brief Create a copy of a contract storage entry (Swift-compatible)
 * 
 * @param src Source entry to copy
 * @param dest Output pointer for the copied entry (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_copy_swift(
    const neoc_contract_storage_entry_t *src,
    neoc_contract_storage_entry_t **dest
);

/**
 * @brief Compare two contract storage entries for equality (Swift-compatible)
 * 
 * @param entry1 First entry
 * @param entry2 Second entry
 * @return true if entries are equal, false otherwise
 */
bool neoc_contract_storage_entry_equals_swift(
    const neoc_contract_storage_entry_t *entry1,
    const neoc_contract_storage_entry_t *entry2
);

/**
 * @brief Get storage key as hex string (Swift-compatible)
 * 
 * @param entry Storage entry
 * @param key_str Output pointer for key string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_get_key_string(
    const neoc_contract_storage_entry_t *entry,
    char **key_str
);

/**
 * @brief Get storage value as hex string (Swift-compatible)
 * 
 * @param entry Storage entry
 * @param value_str Output pointer for value string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_get_value_string(
    const neoc_contract_storage_entry_t *entry,
    char **value_str
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_STORAGE_ENTRY_H */
