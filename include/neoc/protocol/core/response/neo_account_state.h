/**
 * @file neo_account_state.h
 * @brief Neo account state structure
 * 
 * Converted from Swift source: protocol/core/response/NeoAccountState.swift
 * Represents the state of a Neo account including balance and voting information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_ACCOUNT_STATE_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_ACCOUNT_STATE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/ec_public_key.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neo account state structure
 * 
 * Represents the state of a Neo account, including balance, balance height,
 * and voting information (public key of voted candidate).
 */
typedef struct {
    int64_t balance;                           /**< NEO balance */
    int64_t *balance_height;                   /**< Height when balance was last updated (NULL if not set) */
    neoc_ec_public_key_t *public_key;          /**< Public key of voted candidate (NULL if no vote) */
} neoc_neo_account_state_t;

/**
 * @brief JSON-RPC response for Neo account state
 * 
 * Complete response structure for Neo account state queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_neo_account_state_t *result;   /**< Account state result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_neo_account_state_response_t;

/**
 * @brief Create a new Neo account state
 * 
 * @param balance NEO balance
 * @param balance_height Height when balance was last updated (can be NULL)
 * @param public_key Public key of voted candidate (can be NULL)
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_create(
    int64_t balance,
    const int64_t *balance_height,
    const neoc_ec_public_key_t *public_key,
    neoc_neo_account_state_t **state
);

/**
 * @brief Create a Neo account state with no vote
 * 
 * @param balance NEO balance
 * @param update_height Height when balance was last updated
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_create_no_vote(
    int64_t balance,
    int64_t update_height,
    neoc_neo_account_state_t **state
);

/**
 * @brief Create a Neo account state with no balance
 * 
 * @param state Output pointer for the created state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_create_no_balance(
    neoc_neo_account_state_t **state
);

/**
 * @brief Free a Neo account state
 * 
 * @param state State to free (can be NULL)
 */
void neoc_neo_account_state_free(
    neoc_neo_account_state_t *state
);

/**
 * @brief Create a new Neo account state response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Account state result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_response_create(
    const char *jsonrpc,
    int id,
    neoc_neo_account_state_t *result,
    const char *error,
    int error_code,
    neoc_neo_account_state_response_t **response
);

/**
 * @brief Free a Neo account state response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_neo_account_state_response_free(
    neoc_neo_account_state_response_t *response
);

/**
 * @brief Parse Neo account state from JSON string
 * 
 * @param json_str JSON string containing account state data
 * @param state Output pointer for the parsed state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_from_json(
    const char *json_str,
    neoc_neo_account_state_t **state
);

/**
 * @brief Parse Neo account state response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_response_from_json(
    const char *json_str,
    neoc_neo_account_state_response_t **response
);

/**
 * @brief Convert Neo account state to JSON string
 * 
 * @param state Account state to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_to_json(
    const neoc_neo_account_state_t *state,
    char **json_str
);

/**
 * @brief Check if account has voted for a candidate
 * 
 * @param state Account state to check
 * @return true if account has voted, false otherwise
 */
bool neoc_neo_account_state_has_vote(
    const neoc_neo_account_state_t *state
);

/**
 * @brief Check if account has a balance
 * 
 * @param state Account state to check
 * @return true if account has a balance > 0, false otherwise
 */
bool neoc_neo_account_state_has_balance(
    const neoc_neo_account_state_t *state
);

/**
 * @brief Create a copy of a Neo account state
 * 
 * @param src Source state to copy
 * @param dest Output pointer for the copied state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_account_state_copy(
    const neoc_neo_account_state_t *src,
    neoc_neo_account_state_t **dest
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_ACCOUNT_STATE_H */
