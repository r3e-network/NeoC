/**
 * @file transaction_send_token.h
 * @brief Transaction send token structure for token transfers
 * 
 * Based on Swift source: protocol/core/response/TransactionSendToken.swift
 * Represents a token transfer operation within a transaction
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_SEND_TOKEN_H
#define NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_SEND_TOKEN_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transaction send token structure
 * 
 * Represents a token transfer operation with asset, value, and address
 */
typedef struct {
    neoc_hash160_t *token;              /**< Token/asset hash */
    int64_t value;                      /**< Transfer value */
    char *address;                      /**< Destination address */
} neoc_transaction_send_token_t;

/**
 * @brief Create a new transaction send token
 * 
 * @param send_token Pointer to store the created send token
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_create(
    neoc_transaction_send_token_t **send_token
);

/**
 * @brief Free a transaction send token and its resources
 * 
 * @param send_token Send token to free
 */
void neoc_transaction_send_token_free(
    neoc_transaction_send_token_t *send_token
);

/**
 * @brief Create a transaction send token with values
 * 
 * @param token Token/asset hash
 * @param value Transfer value
 * @param address Destination address
 * @param send_token Pointer to store the created send token
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_create_with_values(
    const neoc_hash160_t *token,
    int64_t value,
    const char *address,
    neoc_transaction_send_token_t **send_token
);

/**
 * @brief Parse JSON into transaction send token
 * 
 * @param json_str JSON string to parse
 * @param send_token Pointer to store the parsed send token
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_from_json(
    const char *json_str,
    neoc_transaction_send_token_t **send_token
);

/**
 * @brief Convert transaction send token to JSON string
 * 
 * @param send_token Send token to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_to_json(
    const neoc_transaction_send_token_t *send_token,
    char **json_str
);

/**
 * @brief Copy a transaction send token
 * 
 * @param src Source send token
 * @param dest Pointer to store the copied send token
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_copy(
    const neoc_transaction_send_token_t *src,
    neoc_transaction_send_token_t **dest
);

/**
 * @brief Compare two transaction send tokens for equality
 * 
 * @param token1 First send token
 * @param token2 Second send token
 * @return true if send tokens are equal, false otherwise
 */
bool neoc_transaction_send_token_equals(
    const neoc_transaction_send_token_t *token1,
    const neoc_transaction_send_token_t *token2
);

/**
 * @brief Set the token hash for a send token
 * 
 * @param send_token Send token to modify
 * @param token Token hash to set
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_set_token(
    neoc_transaction_send_token_t *send_token,
    const neoc_hash160_t *token
);

/**
 * @brief Set the value for a send token
 * 
 * @param send_token Send token to modify
 * @param value Value to set
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_set_value(
    neoc_transaction_send_token_t *send_token,
    int64_t value
);

/**
 * @brief Set the address for a send token
 * 
 * @param send_token Send token to modify
 * @param address Address to set
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_send_token_set_address(
    neoc_transaction_send_token_t *send_token,
    const char *address
);

/**
 * @brief Get the token hash from a send token
 * 
 * @param send_token Send token to query
 * @return Token hash or NULL if not set
 */
const neoc_hash160_t* neoc_transaction_send_token_get_token(
    const neoc_transaction_send_token_t *send_token
);

/**
 * @brief Get the value from a send token
 * 
 * @param send_token Send token to query
 * @return Transfer value
 */
int64_t neoc_transaction_send_token_get_value(
    const neoc_transaction_send_token_t *send_token
);

/**
 * @brief Get the address from a send token
 * 
 * @param send_token Send token to query
 * @return Address string or NULL if not set
 */
const char* neoc_transaction_send_token_get_address(
    const neoc_transaction_send_token_t *send_token
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_SEND_TOKEN_H */
