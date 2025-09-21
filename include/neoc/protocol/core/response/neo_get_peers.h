/**
 * @file neo_get_peers.h
 * @brief Neo peers response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetPeers.swift
 * Provides functionality for retrieving network peer information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_PEERS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_PEERS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a network address entry for a peer
 * 
 * This structure contains the IP address and port information for a peer node.
 */
typedef struct {
    char *address;                      /**< IP address of the peer */
    int port;                           /**< Port number of the peer */
} neoc_address_entry_t;

/**
 * @brief Represents peer network information
 * 
 * This structure contains categorized lists of network peers including
 * connected, bad, and unconnected peers.
 */
typedef struct {
    neoc_address_entry_t *connected;    /**< Array of connected peers */
    size_t connected_count;             /**< Number of connected peers */
    neoc_address_entry_t *bad;          /**< Array of bad peers */
    size_t bad_count;                   /**< Number of bad peers */
    neoc_address_entry_t *unconnected;  /**< Array of unconnected peers */
    size_t unconnected_count;           /**< Number of unconnected peers */
} neoc_peers_t;

/**
 * @brief Response structure for getpeers RPC call
 * 
 * Contains the peer information returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_peers_t *result;               /**< Peers information (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_peers_t;

/**
 * @brief Create a new address entry structure
 * 
 * @param address IP address of the peer
 * @param port Port number of the peer
 * @param entry Pointer to store the created entry (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_address_entry_create(
    const char *address,
    int port,
    neoc_address_entry_t **entry
);

/**
 * @brief Free an address entry structure
 * 
 * @param entry Entry to free
 */
void neoc_address_entry_free(neoc_address_entry_t *entry);

/**
 * @brief Create a new peers structure
 * 
 * @param connected Array of connected peer entries
 * @param connected_count Number of connected peers
 * @param bad Array of bad peer entries
 * @param bad_count Number of bad peers
 * @param unconnected Array of unconnected peer entries
 * @param unconnected_count Number of unconnected peers
 * @param peers Pointer to store the created peers structure (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_peers_create(
    neoc_address_entry_t *connected,
    size_t connected_count,
    neoc_address_entry_t *bad,
    size_t bad_count,
    neoc_address_entry_t *unconnected,
    size_t unconnected_count,
    neoc_peers_t **peers
);

/**
 * @brief Free a peers structure
 * 
 * @param peers Peers structure to free
 */
void neoc_peers_free(neoc_peers_t *peers);

/**
 * @brief Create a new Neo get peers response
 * 
 * @param id Request ID
 * @param peers Peers data (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_peers_create(
    int id,
    neoc_peers_t *peers,
    const char *error,
    int error_code,
    neoc_neo_get_peers_t **response
);

/**
 * @brief Free a Neo get peers response
 * 
 * @param response Response to free
 */
void neoc_neo_get_peers_free(neoc_neo_get_peers_t *response);

/**
 * @brief Parse JSON response into Neo get peers structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_peers_from_json(
    const char *json_str,
    neoc_neo_get_peers_t **response
);

/**
 * @brief Convert Neo get peers response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_peers_to_json(
    const neoc_neo_get_peers_t *response,
    char **json_str
);

/**
 * @brief Get peers data from response (convenience function)
 * 
 * @param response Response to get peers from
 * @return Peers data pointer, or NULL if error or no result
 */
neoc_peers_t *neoc_neo_get_peers_get_peers(
    const neoc_neo_get_peers_t *response
);

/**
 * @brief Check if the response contains valid peers data
 * 
 * @param response Response to check
 * @return true if response has valid peers, false otherwise
 */
bool neoc_neo_get_peers_has_peers(
    const neoc_neo_get_peers_t *response
);

/**
 * @brief Get number of connected peers
 * 
 * @param response Response to count connected peers from
 * @return Number of connected peers, or 0 on error
 */
size_t neoc_neo_get_peers_get_connected_count(
    const neoc_neo_get_peers_t *response
);

/**
 * @brief Get number of bad peers
 * 
 * @param response Response to count bad peers from
 * @return Number of bad peers, or 0 on error
 */
size_t neoc_neo_get_peers_get_bad_count(
    const neoc_neo_get_peers_t *response
);

/**
 * @brief Get number of unconnected peers
 * 
 * @param response Response to count unconnected peers from
 * @return Number of unconnected peers, or 0 on error
 */
size_t neoc_neo_get_peers_get_unconnected_count(
    const neoc_neo_get_peers_t *response
);

/**
 * @brief Get total number of known peers
 * 
 * @param response Response to count peers from
 * @return Total number of peers (connected + bad + unconnected), or 0 on error
 */
size_t neoc_neo_get_peers_get_total_count(
    const neoc_neo_get_peers_t *response
);

/**
 * @brief Get connected peer by index
 * 
 * @param response Response to get peer from
 * @param index Index of the connected peer
 * @return Address entry pointer, or NULL if index out of bounds or error
 */
neoc_address_entry_t *neoc_neo_get_peers_get_connected_peer(
    const neoc_neo_get_peers_t *response,
    size_t index
);

/**
 * @brief Get bad peer by index
 * 
 * @param response Response to get peer from
 * @param index Index of the bad peer
 * @return Address entry pointer, or NULL if index out of bounds or error
 */
neoc_address_entry_t *neoc_neo_get_peers_get_bad_peer(
    const neoc_neo_get_peers_t *response,
    size_t index
);

/**
 * @brief Get unconnected peer by index
 * 
 * @param response Response to get peer from
 * @param index Index of the unconnected peer
 * @return Address entry pointer, or NULL if index out of bounds or error
 */
neoc_address_entry_t *neoc_neo_get_peers_get_unconnected_peer(
    const neoc_neo_get_peers_t *response,
    size_t index
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_PEERS_H */
