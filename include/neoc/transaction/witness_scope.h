/**
 * @file witness_scope.h
 * @brief Witness scope definitions for transaction signers
 */

#ifndef NEOC_WITNESS_SCOPE_H
#define NEOC_WITNESS_SCOPE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Witness scope enumeration
 * Defines the scope in which a witness/signature is valid
 */
typedef enum neoc_witness_scope {
    NEOC_WITNESS_SCOPE_NONE = 0x00,            // Signature is disabled in contracts
    NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY = 0x01, // Only entry point contract can use witness
    NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS = 0x10, // Specific contracts allowed
    NEOC_WITNESS_SCOPE_CUSTOM_GROUPS = 0x20,    // Specific contract groups allowed
    NEOC_WITNESS_SCOPE_WITNESS_RULES = 0x40,    // Witness rules must be met
    NEOC_WITNESS_SCOPE_GLOBAL = 0x80            // Global witness scope
} neoc_witness_scope_t;

/**
 * Get JSON string representation of witness scope
 * @param scope The witness scope
 * @return JSON string representation
 */
const char* neoc_witness_scope_to_json(neoc_witness_scope_t scope);

/**
 * Parse witness scope from JSON string
 * @param json JSON string representation
 * @param scope Output witness scope
 * @return Error code
 */
neoc_error_t neoc_witness_scope_from_json(const char *json, 
                                           neoc_witness_scope_t *scope);

/**
 * Combine multiple witness scopes into a single byte
 * @param scopes Array of witness scopes
 * @param count Number of scopes in array
 * @return Combined scope byte
 */
uint8_t neoc_witness_scope_combine(const neoc_witness_scope_t *scopes, 
                                    size_t count);

/**
 * Extract individual scopes from a combined scope byte
 * @param combined Combined scope byte
 * @param scopes Output array of witness scopes (must be allocated)
 * @param max_count Maximum number of scopes that can be stored
 * @param count Output actual number of scopes extracted
 * @return Error code
 */
neoc_error_t neoc_witness_scope_extract(uint8_t combined,
                                         neoc_witness_scope_t *scopes,
                                         size_t max_count,
                                         size_t *count);

/**
 * Check if a combined scope contains a specific scope
 * @param combined Combined scope byte
 * @param scope Scope to check for
 * @return True if scope is present
 */
bool neoc_witness_scope_has(uint8_t combined, neoc_witness_scope_t scope);

/**
 * Validate witness scope value
 * @param scope Scope to validate
 * @return True if valid
 */
bool neoc_witness_scope_is_valid(neoc_witness_scope_t scope);

#ifdef __cplusplus
}
#endif

#endif // NEOC_WITNESS_SCOPE_H
