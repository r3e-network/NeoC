/**
 * @file neo_get_nep11_transfers.h
 * @brief NEP-11 NFT transfers response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetNep11Transfers.swift
 * Provides functionality for retrieving NEP-11 (Non-Fungible Token) transfer history.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP11_TRANSFERS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP11_TRANSFERS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/hash256.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a single NEP-11 token transfer
 * 
 * Contains detailed information about an individual NFT transfer including
 * token ID, timestamps, addresses, amounts, and blockchain references.
 */
typedef struct {
    char *token_id;                     /**< Unique NFT token identifier */
    int64_t timestamp;                  /**< Transfer timestamp */
    neoc_hash160_t asset_hash;          /**< NFT contract hash */
    char *transfer_address;             /**< Transfer counterpart address */
    int64_t amount;                     /**< Transfer amount */
    int block_index;                    /**< Block index where transfer occurred */
    int transfer_notify_index;          /**< Transfer notification index */
    neoc_hash256_t tx_hash;             /**< Transaction hash */
} neoc_nep11_transfer_t;

/**
 * @brief NEP-11 transfers collection for an address
 * 
 * Contains sent and received NFT transfers for a specific address, providing
 * a complete view of NFT transfer activity.
 */
typedef struct {
    neoc_nep11_transfer_t *sent;        /**< Array of sent transfers */
    size_t sent_count;                  /**< Number of sent transfers */
    neoc_nep11_transfer_t *received;    /**< Array of received transfers */
    size_t received_count;              /**< Number of received transfers */
    char *transfer_address;             /**< Address for which transfers are retrieved */
} neoc_nep11_transfers_t;

/**
 * @brief Response structure for getnep11transfers RPC call
 * 
 * Contains the NEP-11 transfer history returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_nep11_transfers_t *result;     /**< NEP-11 transfers result (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_nep11_transfers_t;

/**
 * @brief Create a new NEP-11 transfer structure
 * 
 * @param token_id NFT token identifier
 * @param timestamp Transfer timestamp
 * @param asset_hash NFT contract hash
 * @param transfer_address Transfer counterpart address
 * @param amount Transfer amount
 * @param block_index Block index
 * @param transfer_notify_index Transfer notification index
 * @param tx_hash Transaction hash
 * @param transfer Pointer to store the created transfer (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep11_transfer_create(
    const char *token_id,
    int64_t timestamp,
    const neoc_hash160_t *asset_hash,
    const char *transfer_address,
    int64_t amount,
    int block_index,
    int transfer_notify_index,
    const neoc_hash256_t *tx_hash,
    neoc_nep11_transfer_t **transfer
);

/**
 * @brief Free a NEP-11 transfer structure
 * 
 * @param transfer Transfer to free
 */
void neoc_nep11_transfer_free(neoc_nep11_transfer_t *transfer);

/**
 * @brief Create a new NEP-11 transfers collection
 * 
 * @param sent Array of sent transfers
 * @param sent_count Number of sent transfers
 * @param received Array of received transfers
 * @param received_count Number of received transfers
 * @param transfer_address Address for transfers
 * @param transfers Pointer to store the created collection (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep11_transfers_create(
    neoc_nep11_transfer_t *sent,
    size_t sent_count,
    neoc_nep11_transfer_t *received,
    size_t received_count,
    const char *transfer_address,
    neoc_nep11_transfers_t **transfers
);

/**
 * @brief Free a NEP-11 transfers collection
 * 
 * @param transfers Collection to free
 */
void neoc_nep11_transfers_free(neoc_nep11_transfers_t *transfers);

/**
 * @brief Create a new Neo get NEP-11 transfers response
 * 
 * @param id Request ID
 * @param nep11_transfers NEP-11 transfers data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_transfers_create(
    int id,
    neoc_nep11_transfers_t *nep11_transfers,
    const char *error,
    int error_code,
    neoc_neo_get_nep11_transfers_t **response
);

/**
 * @brief Free a Neo get NEP-11 transfers response
 * 
 * @param response Response to free
 */
void neoc_neo_get_nep11_transfers_free(neoc_neo_get_nep11_transfers_t *response);

/**
 * @brief Parse JSON response into Neo get NEP-11 transfers structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_transfers_from_json(
    const char *json_str,
    neoc_neo_get_nep11_transfers_t **response
);

/**
 * @brief Convert Neo get NEP-11 transfers response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_transfers_to_json(
    const neoc_neo_get_nep11_transfers_t *response,
    char **json_str
);

/**
 * @brief Get NEP-11 transfers from response (convenience function)
 * 
 * @param response Response to get transfers from
 * @return NEP-11 transfers pointer, or NULL if error or no result
 */
neoc_nep11_transfers_t *neoc_neo_get_nep11_transfers_get_transfers(
    const neoc_neo_get_nep11_transfers_t *response
);

/**
 * @brief Check if the response contains valid NEP-11 transfers data
 * 
 * @param response Response to check
 * @return true if response has valid transfers, false otherwise
 */
bool neoc_neo_get_nep11_transfers_has_transfers(
    const neoc_neo_get_nep11_transfers_t *response
);

/**
 * @brief Get number of sent NFT transfers
 * 
 * @param response Response to count sent transfers from
 * @return Number of sent transfers, or 0 on error
 */
size_t neoc_neo_get_nep11_transfers_get_sent_count(
    const neoc_neo_get_nep11_transfers_t *response
);

/**
 * @brief Get number of received NFT transfers
 * 
 * @param response Response to count received transfers from
 * @return Number of received transfers, or 0 on error
 */
size_t neoc_neo_get_nep11_transfers_get_received_count(
    const neoc_neo_get_nep11_transfers_t *response
);

/**
 * @brief Get total number of NFT transfers (sent + received)
 * 
 * @param response Response to count transfers from
 * @return Total number of transfers, or 0 on error
 */
size_t neoc_neo_get_nep11_transfers_get_total_count(
    const neoc_neo_get_nep11_transfers_t *response
);

/**
 * @brief Get sent NFT transfer by index
 * 
 * @param response Response to get transfer from
 * @param index Index of the sent transfer
 * @return Transfer pointer, or NULL if index out of bounds or error
 */
neoc_nep11_transfer_t *neoc_neo_get_nep11_transfers_get_sent(
    const neoc_neo_get_nep11_transfers_t *response,
    size_t index
);

/**
 * @brief Get received NFT transfer by index
 * 
 * @param response Response to get transfer from
 * @param index Index of the received transfer
 * @return Transfer pointer, or NULL if index out of bounds or error
 */
neoc_nep11_transfer_t *neoc_neo_get_nep11_transfers_get_received(
    const neoc_neo_get_nep11_transfers_t *response,
    size_t index
);

/**
 * @brief Get transfer address from response
 * 
 * @param response Response to get address from
 * @return Transfer address string, or NULL if error or no result
 */
const char *neoc_neo_get_nep11_transfers_get_address(
    const neoc_neo_get_nep11_transfers_t *response
);

/**
 * @brief Find transfers for specific NFT token
 * 
 * @param response Response to search in
 * @param token_id Token ID to find transfers for
 * @param sent_transfers Pointer to store array of sent transfers (output)
 * @param sent_count Pointer to store count of sent transfers (output)
 * @param received_transfers Pointer to store array of received transfers (output)
 * @param received_count Pointer to store count of received transfers (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_nep11_transfers_find_token_transfers(
    const neoc_neo_get_nep11_transfers_t *response,
    const char *token_id,
    neoc_nep11_transfer_t ***sent_transfers,
    size_t *sent_count,
    neoc_nep11_transfer_t ***received_transfers,
    size_t *received_count
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_NEP11_TRANSFERS_H */
