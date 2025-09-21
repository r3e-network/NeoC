#ifndef NEOC_RPC_RESPONSE_TYPES_H
#define NEOC_RPC_RESPONSE_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/serialization/neo_serializable.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/protocol/stack_item.h"
#include "neoc/transaction/witness.h"
#include "neoc/transaction/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file rpc_response_types.h
 * @brief Neo RPC response types for protocol communication
 * 
 * This header defines all the response types used in Neo RPC protocol communication.
 * These types represent the various data structures returned by Neo RPC calls including
 * blocks, transactions, account information, network data, and system states.
 * 
 * All types implement proper memory management and include serialization capabilities
 * where appropriate. The structures closely match the JSON responses from Neo RPC
 * but are optimized for C usage with proper error handling and memory safety.
 * 
 * @author NeoC SDK Team
 * @version 1.0.0
 * @date 2024
 */

/**
 * @brief Neo Protocol information
 * 
 * Contains protocol-level configuration and network parameters.
 */
typedef struct neoc_protocol_info {
    uint32_t network;                           /**< Network magic number */
    uint32_t validators_count;                  /**< Number of validators (optional) */
    uint32_t ms_per_block;                      /**< Milliseconds per block */
    uint32_t max_valid_until_block_increment;   /**< Max valid until block increment */
    uint32_t max_traceable_blocks;              /**< Maximum traceable blocks */
    uint8_t address_version;                    /**< Address version byte */
    uint32_t max_transactions_per_block;        /**< Max transactions per block */
    uint32_t memory_pool_max_transactions;      /**< Memory pool max transactions */
    uint64_t initial_gas_distribution;          /**< Initial GAS distribution */
} neoc_protocol_info_t;

/**
 * @brief Neo Version information
 * 
 * Contains version and network information from getversion RPC call.
 */
typedef struct neoc_version_info {
    uint32_t tcp_port;              /**< TCP port (optional) */
    uint32_t ws_port;               /**< WebSocket port (optional) */
    uint64_t nonce;                 /**< Node nonce */
    char *user_agent;               /**< User agent string */
    neoc_protocol_info_t protocol;  /**< Protocol information */
    bool has_tcp_port;              /**< Whether TCP port is set */
    bool has_ws_port;               /**< Whether WebSocket port is set */
} neoc_version_info_t;

/**
 * @brief Neo Block information
 * 
 * Represents a complete block from the Neo blockchain.
 */
typedef struct neoc_block {
    neoc_hash256_t hash;                /**< Block hash */
    uint32_t size;                      /**< Block size in bytes */
    uint32_t version;                   /**< Block version */
    neoc_hash256_t previous_block_hash; /**< Previous block hash */
    neoc_hash256_t merkle_root_hash;    /**< Merkle root hash */
    uint64_t timestamp;                 /**< Block timestamp */
    uint32_t index;                     /**< Block index */
    uint32_t primary;                   /**< Primary consensus node index */
    char *next_consensus;               /**< Next consensus address */
    neoc_witness_t *witnesses;          /**< Block witnesses */
    size_t witness_count;               /**< Number of witnesses */
    neoc_transaction_t *transactions;   /**< Block transactions */
    size_t transaction_count;           /**< Number of transactions */
    uint32_t confirmations;             /**< Number of confirmations */
    neoc_hash256_t next_block_hash;     /**< Next block hash (optional) */
    bool has_primary;                   /**< Whether primary is set */
    bool has_next_block_hash;           /**< Whether next block hash is set */
} neoc_block_t;

/**
 * @brief Transaction verification information
 * 
 * Contains information about transaction verification and status.
 */
typedef struct neoc_send_raw_transaction_result {
    neoc_hash256_t hash;    /**< Transaction hash */
} neoc_send_raw_transaction_result_t;

/**
 * @brief Network fee information
 * 
 * Contains information about network fees for transactions.
 */
typedef struct neoc_network_fee_result {
    uint64_t network_fee;   /**< Calculated network fee */
} neoc_network_fee_result_t;

/**
 * @brief Address validation result
 * 
 * Contains information about address validation.
 */
typedef struct neoc_validate_address_result {
    char *address;          /**< The address that was validated */
    bool valid;             /**< Whether the address is valid */
} neoc_validate_address_result_t;

/**
 * @brief NEP-17 balance information
 * 
 * Represents a balance for a NEP-17 token.
 */
typedef struct neoc_nep17_balance {
    neoc_hash160_t asset_hash;  /**< Asset contract hash */
    char *amount;               /**< Balance amount as string */
    uint64_t last_updated_block; /**< Last updated block */
} neoc_nep17_balance_t;

/**
 * @brief NEP-17 balances result
 * 
 * Contains all NEP-17 token balances for an address.
 */
typedef struct neoc_nep17_balances_result {
    char *address;                      /**< Account address */
    neoc_nep17_balance_t *balances;     /**< Token balances */
    size_t balance_count;               /**< Number of balances */
} neoc_nep17_balances_result_t;

/**
 * @brief NEP-17 transfer information
 * 
 * Represents a single NEP-17 token transfer.
 */
typedef struct neoc_nep17_transfer {
    uint64_t timestamp;             /**< Transfer timestamp */
    neoc_hash160_t asset_hash;      /**< Asset contract hash */
    char *transfer_address;         /**< From/To address */
    char *amount;                   /**< Transfer amount */
    uint32_t block_index;           /**< Block index */
    neoc_hash256_t transfer_notify_index; /**< Transfer notification index */
    neoc_hash256_t tx_hash;         /**< Transaction hash */
} neoc_nep17_transfer_t;

/**
 * @brief NEP-17 transfers result
 * 
 * Contains transfer history for an address.
 */
typedef struct neoc_nep17_transfers_result {
    char *address;                      /**< Account address */
    neoc_nep17_transfer_t *sent;        /**< Sent transfers */
    size_t sent_count;                  /**< Number of sent transfers */
    neoc_nep17_transfer_t *received;    /**< Received transfers */
    size_t received_count;              /**< Number of received transfers */
} neoc_nep17_transfers_result_t;

/**
 * @brief Unclaimed GAS information
 * 
 * Contains information about unclaimed GAS.
 */
typedef struct neoc_unclaimed_gas_result {
    char *unclaimed;                /**< Unclaimed GAS amount */
    char *address;                  /**< Account address */
} neoc_unclaimed_gas_result_t;

/**
 * @brief Next block validators result
 * 
 * Contains the list of next block validators.
 */
typedef struct neoc_next_block_validators_result {
    char **validators;              /**< Array of validator public keys */
    size_t validator_count;         /**< Number of validators */
} neoc_next_block_validators_result_t;

/**
 * @brief Committee members result
 * 
 * Contains the list of committee members.
 */
typedef struct neoc_committee_result {
    char **members;                 /**< Array of committee member public keys */
    size_t member_count;            /**< Number of committee members */
} neoc_committee_result_t;

/**
 * @brief Connection information for a peer
 * 
 * Contains information about a connected peer.
 */
typedef struct neoc_peer_connection {
    char *address;                  /**< Peer address */
    uint32_t port;                  /**< Peer port */
} neoc_peer_connection_t;

/**
 * @brief Peer information
 * 
 * Contains detailed information about a network peer.
 */
typedef struct neoc_peer_info {
    char *address;                  /**< Peer address */
    uint32_t port;                  /**< Peer port */
} neoc_peer_info_t;

/**
 * @brief Network peers result
 * 
 * Contains information about network peers.
 */
typedef struct neoc_peers_result {
    neoc_peer_info_t *unconnected;  /**< Unconnected peers */
    size_t unconnected_count;       /**< Number of unconnected peers */
    neoc_peer_connection_t *connected; /**< Connected peers */
    size_t connected_count;         /**< Number of connected peers */
    neoc_peer_info_t *bad;          /**< Bad peers */
    size_t bad_count;               /**< Number of bad peers */
} neoc_peers_result_t;

/**
 * @brief Memory pool result
 * 
 * Contains information about the memory pool.
 */
typedef struct neoc_memory_pool_result {
    uint32_t height;                /**< Current block height */
    neoc_hash256_t *verified;       /**< Verified transaction hashes */
    size_t verified_count;          /**< Number of verified transactions */
    neoc_hash256_t *unverified;     /**< Unverified transaction hashes */
    size_t unverified_count;        /**< Number of unverified transactions */
} neoc_memory_pool_result_t;

/**
 * @brief State height result
 * 
 * Contains information about state height.
 */
typedef struct neoc_state_height_result {
    uint32_t local_root_index;      /**< Local root index */
    uint32_t validated_root_index;  /**< Validated root index */
} neoc_state_height_result_t;

/**
 * @brief State root information
 * 
 * Contains information about state root.
 */
typedef struct neoc_state_root_result {
    uint32_t version;               /**< State root version */
    uint32_t index;                 /**< State root index */
    neoc_hash256_t root_hash;       /**< Root hash */
    neoc_witness_t *witnesses;      /**< State root witnesses */
    size_t witness_count;           /**< Number of witnesses */
} neoc_state_root_result_t;

/**
 * @brief Plugin information
 * 
 * Contains information about a loaded plugin.
 */
typedef struct neoc_plugin_info {
    char *name;                     /**< Plugin name */
    char *version;                  /**< Plugin version */
    char **interfaces;              /**< Implemented interfaces */
    size_t interface_count;         /**< Number of interfaces */
} neoc_plugin_info_t;

/**
 * @brief List plugins result
 * 
 * Contains the list of loaded plugins.
 */
typedef struct neoc_list_plugins_result {
    neoc_plugin_info_t *plugins;    /**< Plugin information */
    size_t plugin_count;            /**< Number of plugins */
} neoc_list_plugins_result_t;

/**
 * @brief Application log entry
 * 
 * Represents an entry in an application log.
 */
typedef struct neoc_application_log_entry {
    char *trigger;                  /**< Log trigger type */
    char *vm_state;                 /**< VM state */
    char *exception;                /**< Exception message (if any) */
    uint64_t gas_consumed;          /**< Gas consumed */
    neoc_stack_item_t **stack;      /**< Result stack */
    size_t stack_count;             /**< Number of stack items */
    // Notifications array (when implemented)
} neoc_application_log_entry_t;

/**
 * @brief Application log result
 * 
 * Contains application execution logs.
 */
typedef struct neoc_application_log_result {
    neoc_hash256_t tx_id;           /**< Transaction ID */
    neoc_application_log_entry_t *executions; /**< Execution logs */
    size_t execution_count;         /**< Number of executions */
} neoc_application_log_result_t;

/**
 * @brief Invocation result
 * 
 * Contains the result of a contract invocation.
 */
typedef struct neoc_invocation_result {
    char *script;                   /**< Invocation script */
    char *state;                    /**< VM state */
    uint64_t gas_consumed;          /**< Gas consumed */
    char *exception;                /**< Exception message (if any) */
    neoc_stack_item_t **stack;      /**< Result stack */
    size_t stack_count;             /**< Number of stack items */
    // Notifications and diagnostics (when implemented)
} neoc_invocation_result_t;

// Creation functions

/**
 * @brief Create version info structure
 * 
 * @param version_info Output version info (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_version_info_create(neoc_version_info_t **version_info);

/**
 * @brief Create block structure
 * 
 * @param block Output block (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_create(neoc_block_t **block);

/**
 * @brief Create NEP-17 balances result
 * 
 * @param result Output balances result (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_nep17_balances_result_create(neoc_nep17_balances_result_t **result);

/**
 * @brief Create invocation result
 * 
 * @param result Output invocation result (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_invocation_result_create(neoc_invocation_result_t **result);

// Utility functions

/**
 * @brief Parse block from JSON string
 * 
 * @param json_str JSON string containing block data
 * @param block Output block (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_from_json(const char *json_str, neoc_block_t **block);

/**
 * @brief Parse version info from JSON string
 * 
 * @param json_str JSON string containing version data
 * @param version_info Output version info (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_version_info_from_json(const char *json_str, neoc_version_info_t **version_info);

/**
 * @brief Convert block to JSON string
 * 
 * @param block Block to convert
 * @param json_str Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_to_json(const neoc_block_t *block, char **json_str);

// Free functions

/**
 * @brief Free protocol info
 * 
 * @param protocol_info Protocol info to free
 */
void neoc_protocol_info_free(neoc_protocol_info_t *protocol_info);

/**
 * @brief Free version info
 * 
 * @param version_info Version info to free
 */
void neoc_version_info_free(neoc_version_info_t *version_info);

/**
 * @brief Free block
 * 
 * @param block Block to free
 */
void neoc_block_free(neoc_block_t *block);

/**
 * @brief Free send raw transaction result
 * 
 * @param result Result to free
 */
void neoc_send_raw_transaction_result_free(neoc_send_raw_transaction_result_t *result);

/**
 * @brief Free network fee result
 * 
 * @param result Result to free
 */
void neoc_network_fee_result_free(neoc_network_fee_result_t *result);

/**
 * @brief Free validate address result
 * 
 * @param result Result to free
 */
void neoc_validate_address_result_free(neoc_validate_address_result_t *result);

/**
 * @brief Free NEP-17 balances result
 * 
 * @param result Result to free
 */
void neoc_nep17_balances_result_free(neoc_nep17_balances_result_t *result);

/**
 * @brief Free NEP-17 transfers result
 * 
 * @param result Result to free
 */
void neoc_nep17_transfers_result_free(neoc_nep17_transfers_result_t *result);

/**
 * @brief Free unclaimed GAS result
 * 
 * @param result Result to free
 */
void neoc_unclaimed_gas_result_free(neoc_unclaimed_gas_result_t *result);

/**
 * @brief Free next block validators result
 * 
 * @param result Result to free
 */
void neoc_next_block_validators_result_free(neoc_next_block_validators_result_t *result);

/**
 * @brief Free committee result
 * 
 * @param result Result to free
 */
void neoc_committee_result_free(neoc_committee_result_t *result);

/**
 * @brief Free peers result
 * 
 * @param result Result to free
 */
void neoc_peers_result_free(neoc_peers_result_t *result);

/**
 * @brief Free memory pool result
 * 
 * @param result Result to free
 */
void neoc_memory_pool_result_free(neoc_memory_pool_result_t *result);

/**
 * @brief Free state height result
 * 
 * @param result Result to free
 */
void neoc_state_height_result_free(neoc_state_height_result_t *result);

/**
 * @brief Free state root result
 * 
 * @param result Result to free
 */
void neoc_state_root_result_free(neoc_state_root_result_t *result);

/**
 * @brief Free list plugins result
 * 
 * @param result Result to free
 */
void neoc_list_plugins_result_free(neoc_list_plugins_result_t *result);

/**
 * @brief Free application log result
 * 
 * @param result Result to free
 */
void neoc_application_log_result_free(neoc_application_log_result_t *result);

/**
 * @brief Free invocation result
 * 
 * @param result Result to free
 */
void neoc_invocation_result_free(neoc_invocation_result_t *result);

#ifdef __cplusplus
}
#endif

#endif // NEOC_RPC_RESPONSE_TYPES_H
