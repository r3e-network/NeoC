/**
 * @file neo_response_aliases.h
 * @brief Neo response type aliases and convenience definitions
 * 
 * Converted from Swift source: protocol/core/response/NeoResponseAliases.swift
 * Provides type aliases and convenience structures for common Neo RPC responses.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_RESPONSE_ALIASES_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_RESPONSE_ALIASES_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash256.h"
#include "neoc/protocol/core/response/neo_address.h"
#include "neoc/transaction/transaction.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Response structure for block count queries
 * 
 * Simple integer response for blockchain height queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    int *result;                        /**< Block count result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_block_count_t;

/**
 * @brief Response structure for block hash queries
 * 
 * Hash256 response for block hash queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_hash256_t *result;             /**< Block hash result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_block_hash_t;

/**
 * @brief Response structure for connection count queries
 * 
 * Integer response for network connection count.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    int *result;                        /**< Connection count result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_connection_count_t;

/**
 * @brief Response structure for boolean operations
 * 
 * Generic boolean response for wallet operations.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    bool *result;                       /**< Boolean result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_boolean_response_t;

/**
 * @brief Response structure for string operations
 * 
 * Generic string response for various operations.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    char *result;                       /**< String result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_string_response_t;

/**
 * @brief Response structure for transaction operations
 * 
 * Transaction response for send operations.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_transaction_t *result;         /**< Transaction result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_transaction_response_t;

/**
 * @brief Response structure for address list operations
 * 
 * Array of addresses response for wallet operations.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_neo_address_t *result;         /**< Array of addresses (NULL if error) */
    size_t result_count;                /**< Number of addresses */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_address_list_response_t;

/* Type aliases for common response patterns */

/** Alias for block count response */
typedef neoc_neo_block_count_t neoc_neo_get_block_count_t;

/** Alias for connection count response (also used for block header count) */
typedef neoc_neo_connection_count_t neoc_neo_get_connection_count_t;
typedef neoc_neo_connection_count_t neoc_neo_block_header_count_t;

/** Alias for wallet operations */
typedef neoc_neo_boolean_response_t neoc_neo_close_wallet_t;
typedef neoc_neo_boolean_response_t neoc_neo_open_wallet_t;
typedef neoc_neo_boolean_response_t neoc_neo_submit_block_t;
typedef neoc_neo_boolean_response_t neoc_neo_terminate_session_t;

/** Alias for string operations */
typedef neoc_neo_string_response_t neoc_neo_dump_priv_key_t;
typedef neoc_neo_string_response_t neoc_neo_get_new_address_t;
typedef neoc_neo_string_response_t neoc_neo_get_raw_block_t;
typedef neoc_neo_string_response_t neoc_neo_get_raw_transaction_t;
typedef neoc_neo_string_response_t neoc_neo_get_storage_t;
typedef neoc_neo_string_response_t neoc_neo_get_state_t;
typedef neoc_neo_string_response_t neoc_neo_get_proof_t;
typedef neoc_neo_string_response_t neoc_neo_verify_proof_t;
typedef neoc_neo_string_response_t neoc_neo_get_wallet_unclaimed_gas_t;

/** Alias for transaction operations */
typedef neoc_neo_transaction_response_t neoc_neo_send_from_t;
typedef neoc_neo_transaction_response_t neoc_neo_send_many_t;
typedef neoc_neo_transaction_response_t neoc_neo_send_to_address_t;

/** Alias for address operations */
typedef neoc_neo_address_list_response_t neoc_neo_list_address_t;

/** Height-related aliases */
typedef neoc_neo_block_count_t neoc_neo_get_wallet_height_t;
typedef neoc_neo_get_wallet_height_t neoc_neo_get_transaction_height_t;

/**
 * @brief Create a generic integer response
 * 
 * @param id Request ID
 * @param result Integer result (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_create_int_response(
    int id,
    const int *result,
    const char *error,
    int error_code,
    neoc_neo_block_count_t **response
);

/**
 * @brief Create a generic boolean response
 * 
 * @param id Request ID
 * @param result Boolean result (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_create_bool_response(
    int id,
    const bool *result,
    const char *error,
    int error_code,
    neoc_neo_boolean_response_t **response
);

/**
 * @brief Create a generic string response
 * 
 * @param id Request ID
 * @param result String result (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_create_string_response(
    int id,
    const char *result,
    const char *error,
    int error_code,
    neoc_neo_string_response_t **response
);

/**
 * @brief Free any Neo response structure
 * 
 * Generic free function for common response types.
 * 
 * @param response Response to free
 */
void neoc_neo_response_free(void *response);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_RESPONSE_ALIASES_H */
