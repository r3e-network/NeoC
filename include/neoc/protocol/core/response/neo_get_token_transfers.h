/**
 * @file neo_get_token_transfers.h
 * @brief Neo token transfer history response
 * 
 * Based on Swift source: protocol/core/response/NeoGetTokenTransfers.swift
 * Response structure for generic token transfer history queries
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_TOKEN_TRANSFERS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_TOKEN_TRANSFERS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include "neoc/types/hash256.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Token transfer record
 * 
 * Represents a single token transfer operation
 */
typedef struct {
    uint64_t timestamp;                 /**< Transfer timestamp */
    neoc_hash160_t *asset_hash;         /**< Asset/token hash */
    char *transfer_address;             /**< Transfer address (from/to) */
    char *amount;                       /**< Transfer amount as string */
    uint32_t block_index;               /**< Block index where transfer occurred */
    uint32_t transfer_notify_index;     /**< Transfer notification index */
    neoc_hash256_t *tx_hash;            /**< Transaction hash */
} neoc_token_transfer_t;

/**
 * @brief Token transfers collection
 * 
 * Contains sent and received transfers for an address
 */
typedef struct {
    neoc_token_transfer_t **sent;       /**< Array of sent transfers */
    size_t sent_count;                  /**< Number of sent transfers */
    neoc_token_transfer_t **received;   /**< Array of received transfers */
    size_t received_count;              /**< Number of received transfers */
    char *transfer_address;             /**< Address being queried */
} neoc_token_transfers_t;

/**
 * @brief GetTokenTransfers JSON-RPC response
 * 
 * Response for general token transfer queries
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                            /**< Request ID */
    neoc_token_transfers_t *result;     /**< Token transfers result */
    
    /* Error fields */
    int error_code;                     /**< Error code if request failed */
    char *error_message;                /**< Error message if request failed */
} neoc_get_token_transfers_response_t;

/**
 * @brief Create a new token transfer
 * 
 * @param transfer Pointer to store the created transfer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_transfer_create(
    neoc_token_transfer_t **transfer
);

/**
 * @brief Free a token transfer and its resources
 * 
 * @param transfer Transfer to free
 */
void neoc_token_transfer_free(
    neoc_token_transfer_t *transfer
);

/**
 * @brief Create a new token transfers collection
 * 
 * @param transfers Pointer to store the created transfers
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_transfers_create(
    neoc_token_transfers_t **transfers
);

/**
 * @brief Free a token transfers collection and its resources
 * 
 * @param transfers Transfers collection to free
 */
void neoc_token_transfers_free(
    neoc_token_transfers_t *transfers
);

/**
 * @brief Create a new GetTokenTransfers response
 * 
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_token_transfers_response_create(
    neoc_get_token_transfers_response_t **response
);

/**
 * @brief Free a GetTokenTransfers response and its resources
 * 
 * @param response Response to free
 */
void neoc_get_token_transfers_response_free(
    neoc_get_token_transfers_response_t *response
);

/**
 * @brief Parse JSON into GetTokenTransfers response
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_token_transfers_response_from_json(
    const char *json_str,
    neoc_get_token_transfers_response_t **response
);

/**
 * @brief Convert GetTokenTransfers response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_token_transfers_response_to_json(
    const neoc_get_token_transfers_response_t *response,
    char **json_str
);

/**
 * @brief Add a sent transfer to the collection
 * 
 * @param transfers Transfers collection to modify
 * @param transfer Transfer to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_transfers_add_sent(
    neoc_token_transfers_t *transfers,
    neoc_token_transfer_t *transfer
);

/**
 * @brief Add a received transfer to the collection
 * 
 * @param transfers Transfers collection to modify
 * @param transfer Transfer to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_transfers_add_received(
    neoc_token_transfers_t *transfers,
    neoc_token_transfer_t *transfer
);

/**
 * @brief Set the transfer address for the collection
 * 
 * @param transfers Transfers collection to modify
 * @param address Address string
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_transfers_set_address(
    neoc_token_transfers_t *transfers,
    const char *address
);

/**
 * @brief Create a token transfer with full details
 * 
 * @param timestamp Transfer timestamp
 * @param asset_hash Asset/token hash
 * @param transfer_address Transfer address
 * @param amount Transfer amount as string
 * @param block_index Block index
 * @param transfer_notify_index Transfer notification index
 * @param tx_hash Transaction hash
 * @param transfer Pointer to store the created transfer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_token_transfer_create_full(
    uint64_t timestamp,
    const neoc_hash160_t *asset_hash,
    const char *transfer_address,
    const char *amount,
    uint32_t block_index,
    uint32_t transfer_notify_index,
    const neoc_hash256_t *tx_hash,
    neoc_token_transfer_t **transfer
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_TOKEN_TRANSFERS_H */
