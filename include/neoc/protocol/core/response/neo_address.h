/**
 * @file neo_address.h
 * @brief Neo address response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoAddress.swift
 * Provides functionality for handling wallet address information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_ADDRESS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_ADDRESS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a Neo wallet address
 * 
 * Contains information about a wallet address including its string representation,
 * key availability, optional label, and watch-only status.
 */
typedef struct {
    char *address;                      /**< Address string */
    bool has_key;                       /**< Whether address has private key */
    char *label;                        /**< Optional address label (can be NULL) */
    bool watch_only;                    /**< Whether address is watch-only */
} neoc_neo_address_t;

/**
 * @brief Create a new Neo address structure
 * 
 * @param address Address string
 * @param has_key Whether address has private key
 * @param label Optional address label (can be NULL)
 * @param watch_only Whether address is watch-only
 * @param neo_address Pointer to store the created address (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_address_create(
    const char *address,
    bool has_key,
    const char *label,
    bool watch_only,
    neoc_neo_address_t **neo_address
);

/**
 * @brief Free a Neo address structure
 * 
 * @param neo_address Address to free
 */
void neoc_neo_address_free(neoc_neo_address_t *neo_address);

/**
 * @brief Copy a Neo address structure
 * 
 * @param source Source address to copy
 * @param copy Pointer to store the copied address (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_address_copy(
    const neoc_neo_address_t *source,
    neoc_neo_address_t **copy
);

/**
 * @brief Get address string from Neo address
 * 
 * @param neo_address Address to get string from
 * @return Address string, or NULL if error
 */
const char *neoc_neo_address_get_address(
    const neoc_neo_address_t *neo_address
);

/**
 * @brief Check if address has private key
 * 
 * @param neo_address Address to check
 * @return true if address has private key, false otherwise
 */
bool neoc_neo_address_has_key(
    const neoc_neo_address_t *neo_address
);

/**
 * @brief Get label from Neo address
 * 
 * @param neo_address Address to get label from
 * @return Label string, or NULL if no label or error
 */
const char *neoc_neo_address_get_label(
    const neoc_neo_address_t *neo_address
);

/**
 * @brief Check if address is watch-only
 * 
 * @param neo_address Address to check
 * @return true if address is watch-only, false otherwise
 */
bool neoc_neo_address_is_watch_only(
    const neoc_neo_address_t *neo_address
);

/**
 * @brief Set label for Neo address
 * 
 * @param neo_address Address to set label for
 * @param label New label (can be NULL to clear label)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_address_set_label(
    neoc_neo_address_t *neo_address,
    const char *label
);

/**
 * @brief Parse JSON into Neo address structure
 * 
 * @param json_str JSON string to parse
 * @param neo_address Pointer to store the parsed address (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_address_from_json(
    const char *json_str,
    neoc_neo_address_t **neo_address
);

/**
 * @brief Convert Neo address to JSON string
 * 
 * @param neo_address Address to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_address_to_json(
    const neoc_neo_address_t *neo_address,
    char **json_str
);

/**
 * @brief Check if two Neo addresses are equal
 * 
 * @param address1 First address to compare
 * @param address2 Second address to compare
 * @return true if addresses are equal, false otherwise
 */
bool neoc_neo_address_equals(
    const neoc_neo_address_t *address1,
    const neoc_neo_address_t *address2
);

/**
 * @brief Copy address string from Neo address
 * 
 * @param neo_address Address to get string from
 * @param address_copy Pointer to store the copied address string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_address_copy_address_string(
    const neoc_neo_address_t *neo_address,
    char **address_copy
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_ADDRESS_H */
