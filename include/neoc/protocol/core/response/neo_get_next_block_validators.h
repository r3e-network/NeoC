/**
 * @file neo_get_next_block_validators.h
 * @brief Neo next block validators response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetNextBlockValidators.swift
 * Provides functionality for retrieving next block validator information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEXT_BLOCK_VALIDATORS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEXT_BLOCK_VALIDATORS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a block validator
 * 
 * Contains information about a validator including their public key,
 * vote count, and active status.
 */
typedef struct {
    char *public_key;                   /**< Validator public key */
    char *votes;                        /**< Vote count as string */
    bool active;                        /**< Whether validator is active */
} neoc_validator_t;

/**
 * @brief Response structure for getnextblockvalidators RPC call
 * 
 * Contains the list of next block validators returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_validator_t *result;           /**< Array of validators (NULL if error) */
    size_t result_count;                /**< Number of validators */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_next_block_validators_t;

/**
 * @brief Create a new validator structure
 * 
 * @param public_key Validator public key
 * @param votes Vote count as string
 * @param active Whether validator is active
 * @param validator Pointer to store the created validator (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_validator_create(
    const char *public_key,
    const char *votes,
    bool active,
    neoc_validator_t **validator
);

/**
 * @brief Free a validator structure
 * 
 * @param validator Validator to free
 */
void neoc_validator_free(neoc_validator_t *validator);

/**
 * @brief Create a new Neo get next block validators response
 * 
 * @param id Request ID
 * @param validators Array of validators (can be NULL for error responses)
 * @param validators_count Number of validators
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_next_block_validators_create(
    int id,
    neoc_validator_t *validators,
    size_t validators_count,
    const char *error,
    int error_code,
    neoc_neo_get_next_block_validators_t **response
);

/**
 * @brief Free a Neo get next block validators response
 * 
 * @param response Response to free
 */
void neoc_neo_get_next_block_validators_free(neoc_neo_get_next_block_validators_t *response);

/**
 * @brief Parse JSON response into Neo get next block validators structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_next_block_validators_from_json(
    const char *json_str,
    neoc_neo_get_next_block_validators_t **response
);

/**
 * @brief Convert Neo get next block validators response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_next_block_validators_to_json(
    const neoc_neo_get_next_block_validators_t *response,
    char **json_str
);

/**
 * @brief Get validators array from response (convenience function)
 * 
 * @param response Response to get validators from
 * @return Validators array pointer, or NULL if error or no result
 */
neoc_validator_t *neoc_neo_get_next_block_validators_get_validators(
    const neoc_neo_get_next_block_validators_t *response
);

/**
 * @brief Check if the response contains valid validators data
 * 
 * @param response Response to check
 * @return true if response has valid validators, false otherwise
 */
bool neoc_neo_get_next_block_validators_has_validators(
    const neoc_neo_get_next_block_validators_t *response
);

/**
 * @brief Get number of validators in the response
 * 
 * @param response Response to count validators from
 * @return Number of validators, or 0 on error
 */
size_t neoc_neo_get_next_block_validators_get_count(
    const neoc_neo_get_next_block_validators_t *response
);

/**
 * @brief Get validator by index
 * 
 * @param response Response to get validator from
 * @param index Index of the validator to retrieve
 * @return Validator pointer, or NULL if index out of bounds or error
 */
neoc_validator_t *neoc_neo_get_next_block_validators_get_validator(
    const neoc_neo_get_next_block_validators_t *response,
    size_t index
);

/**
 * @brief Find validator by public key
 * 
 * @param response Response to search in
 * @param public_key Public key of the validator to find
 * @return Validator pointer, or NULL if not found or error
 */
neoc_validator_t *neoc_neo_get_next_block_validators_find_validator(
    const neoc_neo_get_next_block_validators_t *response,
    const char *public_key
);

/**
 * @brief Get number of active validators
 * 
 * @param response Response to count active validators from
 * @return Number of active validators, or 0 on error
 */
size_t neoc_neo_get_next_block_validators_get_active_count(
    const neoc_neo_get_next_block_validators_t *response
);

/**
 * @brief Get public key from validator
 * 
 * @param validator Validator to get public key from
 * @return Public key string, or NULL if error
 */
const char *neoc_validator_get_public_key(
    const neoc_validator_t *validator
);

/**
 * @brief Get votes from validator
 * 
 * @param validator Validator to get votes from
 * @return Votes string, or NULL if error
 */
const char *neoc_validator_get_votes(
    const neoc_validator_t *validator
);

/**
 * @brief Check if validator is active
 * 
 * @param validator Validator to check
 * @return true if validator is active, false otherwise
 */
bool neoc_validator_is_active(
    const neoc_validator_t *validator
);

/**
 * @brief Get votes as integer from validator
 * 
 * @param validator Validator to get votes from
 * @param votes Pointer to store the votes value (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_validator_get_votes_int(
    const neoc_validator_t *validator,
    uint64_t *votes
);

/**
 * @brief Copy public key string from validator
 * 
 * @param validator Validator to get public key from
 * @param public_key_copy Pointer to store the copied public key string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_validator_copy_public_key(
    const neoc_validator_t *validator,
    char **public_key_copy
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEXT_BLOCK_VALIDATORS_H */
