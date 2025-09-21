/**
 * @file neo_witness.h
 * @brief Neo witness structure for transaction verification
 * 
 * Converted from Swift source: protocol/core/response/NeoWitness.swift
 * Represents a witness containing invocation and verification scripts.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_WITNESS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_WITNESS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/transaction/witness.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neo witness structure
 * 
 * Represents a witness with invocation and verification scripts
 * encoded as base64 strings for JSON serialization.
 */
typedef struct {
    char *invocation;                   /**< Invocation script as base64 string */
    char *verification;                 /**< Verification script as base64 string */
} neoc_neo_witness_t;

/**
 * @brief Create a new Neo witness
 * 
 * @param invocation Invocation script as base64 string
 * @param verification Verification script as base64 string
 * @param witness Output pointer for the created witness (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_create(
    const char *invocation,
    const char *verification,
    neoc_neo_witness_t **witness
);

/**
 * @brief Create a Neo witness from a Witness structure
 * 
 * Converts a regular witness to a Neo witness with base64-encoded scripts.
 * 
 * @param witness Source witness structure
 * @param neo_witness Output pointer for the created Neo witness (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_from_witness(
    const neoc_witness_t *witness,
    neoc_neo_witness_t **neo_witness
);

/**
 * @brief Free a Neo witness
 * 
 * @param witness Witness to free (can be NULL)
 */
void neoc_neo_witness_free(
    neoc_neo_witness_t *witness
);

/**
 * @brief Parse Neo witness from JSON string
 * 
 * @param json_str JSON string containing witness data
 * @param witness Output pointer for the parsed witness (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_from_json(
    const char *json_str,
    neoc_neo_witness_t **witness
);

/**
 * @brief Convert Neo witness to JSON string
 * 
 * @param witness Witness to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_to_json(
    const neoc_neo_witness_t *witness,
    char **json_str
);

/**
 * @brief Create a copy of a Neo witness
 * 
 * @param src Source witness to copy
 * @param dest Output pointer for the copied witness (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_copy(
    const neoc_neo_witness_t *src,
    neoc_neo_witness_t **dest
);

/**
 * @brief Compare two Neo witnesses for equality
 * 
 * @param witness1 First witness
 * @param witness2 Second witness
 * @return true if witnesses are equal, false otherwise
 */
bool neoc_neo_witness_equals(
    const neoc_neo_witness_t *witness1,
    const neoc_neo_witness_t *witness2
);

/**
 * @brief Convert Neo witness to regular Witness structure
 * 
 * Decodes the base64 scripts and creates a regular witness structure.
 * 
 * @param neo_witness Source Neo witness
 * @param witness Output pointer for the created witness (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_to_witness(
    const neoc_neo_witness_t *neo_witness,
    neoc_witness_t **witness
);

/**
 * @brief Get invocation script size in bytes
 * 
 * @param witness Neo witness
 * @param size Output pointer for script size
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_get_invocation_size(
    const neoc_neo_witness_t *witness,
    size_t *size
);

/**
 * @brief Get verification script size in bytes
 * 
 * @param witness Neo witness
 * @param size Output pointer for script size
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_witness_get_verification_size(
    const neoc_neo_witness_t *witness,
    size_t *size
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_WITNESS_H */
