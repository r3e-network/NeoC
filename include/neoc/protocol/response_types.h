/**
 * @file response_types.h
 * @brief Neo RPC response type definitions
 *
 * This file contains all RPC response type structures used by the Neo blockchain.
 * These correspond to the response types in the original NeoSwift SDK.
 */

#ifndef NEOC_PROTOCOL_RESPONSE_TYPES_H
#define NEOC_PROTOCOL_RESPONSE_TYPES_H

#include "../neoc_error.h"
#include "../types/neoc_hash160.h"
#include "../types/neoc_hash256.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ResponseTypes RPC Response Types
 * @brief Data structures for Neo RPC responses
 * @{
 */

// Forward declarations
typedef struct neoc_witness neoc_witness_t;
typedef struct neoc_transaction neoc_transaction_t;

/**
 * @brief Base response structure
 */
typedef struct {
    char* jsonrpc;        /**< JSON-RPC version */
    int id;              /**< Request ID */
    char* error;         /**< Error message if any */
} neoc_response_base_t;

/**
 * @brief Neo protocol information
 */
typedef struct {
    int network;                           /**< Network identifier */
    int validators_count;                  /**< Number of validators */
    int ms_per_block;                     /**< Milliseconds per block */
    int max_valid_until_block_increment;  /**< Max valid until block increment */
    int max_traceable_blocks;             /**< Max traceable blocks */
    int address_version;                  /**< Address version */
    int max_transactions_per_block;       /**< Max transactions per block */
    int memory_pool_max_transactions;     /**< Memory pool max transactions */
    uint64_t initial_gas_distribution;   /**< Initial GAS distribution */
} neoc_protocol_t;

/**
 * @brief Neo version information
 */
typedef struct {
    int tcp_port;                /**< TCP port */
    int ws_port;                 /**< WebSocket port */
    uint32_t nonce;             /**< Node nonce */
    char* user_agent;           /**< User agent string */
    neoc_protocol_t* protocol;  /**< Protocol information */
} neoc_version_t;

/**
 * @brief GetVersion response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_version_t* result;
} neoc_get_version_response_t;

/**
 * @brief Block information
 */
typedef struct {
    neoc_hash256_t hash;                  /**< Block hash */
    int size;                             /**< Block size in bytes */
    int version;                          /**< Block version */
    neoc_hash256_t prev_block_hash;       /**< Previous block hash */
    neoc_hash256_t merkle_root_hash;      /**< Merkle root hash */
    uint64_t time;                        /**< Block timestamp */
    uint64_t index;                       /**< Block index */
    int primary;                          /**< Primary consensus node index */
    char* next_consensus;                 /**< Next consensus address */
    neoc_witness_t** witnesses;          /**< Block witnesses */
    size_t witness_count;                 /**< Number of witnesses */
    neoc_transaction_t** transactions;   /**< Block transactions */
    size_t transaction_count;             /**< Number of transactions */
    int confirmations;                    /**< Number of confirmations */
    neoc_hash256_t* next_block_hash;      /**< Next block hash (optional) */
} neoc_block_t;

/**
 * @brief GetBlock response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_block_t* result;
} neoc_get_block_response_t;

/**
 * @brief NEP-17 balance information
 */
typedef struct {
    char* name;                    /**< Token name */
    char* symbol;                  /**< Token symbol */
    char* decimals;                /**< Token decimals */
    char* amount;                  /**< Balance amount */
    double last_updated_block;     /**< Last updated block */
    neoc_hash160_t asset_hash;     /**< Asset hash */
} neoc_nep17_balance_t;

/**
 * @brief NEP-17 balances collection
 */
typedef struct {
    char* address;                        /**< Account address */
    neoc_nep17_balance_t** balances;      /**< Balance array */
    size_t balance_count;                 /**< Number of balances */
} neoc_nep17_balances_t;

/**
 * @brief GetNep17Balances response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_nep17_balances_t* result;
} neoc_get_nep17_balances_response_t;

/**
 * @brief NEP-17 transfer information
 */
typedef struct {
    uint64_t timestamp;             /**< Transfer timestamp */
    neoc_hash160_t asset_hash;      /**< Asset hash */
    char* transfer_address;         /**< Transfer from/to address */
    char* amount;                   /**< Transfer amount */
    uint64_t block_index;           /**< Block index */
    neoc_hash256_t transfer_id;     /**< Transfer transaction hash */
} neoc_nep17_transfer_t;

/**
 * @brief NEP-17 transfers collection
 */
typedef struct {
    char* address;                         /**< Account address */
    neoc_nep17_transfer_t** sent;         /**< Sent transfers */
    size_t sent_count;                     /**< Number of sent transfers */
    neoc_nep17_transfer_t** received;     /**< Received transfers */
    size_t received_count;                 /**< Number of received transfers */
} neoc_nep17_transfers_t;

/**
 * @brief GetNep17Transfers response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_nep17_transfers_t* result;
} neoc_get_nep17_transfers_response_t;

/**
 * @brief Peer information
 */
typedef struct {
    char* address;      /**< Peer address */
    int port;           /**< Peer port */
} neoc_peer_t;

/**
 * @brief GetPeers response
 */
typedef struct {
    neoc_response_base_t base;
    struct {
        neoc_peer_t** unconnected;        /**< Unconnected peers */
        size_t unconnected_count;          /**< Number of unconnected peers */
        neoc_peer_t** bad;                 /**< Bad peers */
        size_t bad_count;                  /**< Number of bad peers */
        neoc_peer_t** connected;           /**< Connected peers */
        size_t connected_count;            /**< Number of connected peers */
    }* result;
} neoc_get_peers_response_t;

/**
 * @brief Memory pool information
 */
typedef struct {
    int height;                     /**< Current blockchain height */
    int verified;                   /**< Number of verified transactions */
    int unverified;                 /**< Number of unverified transactions */
} neoc_mempool_t;

/**
 * @brief GetMemPool response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_mempool_t* result;
} neoc_get_mempool_response_t;

/**
 * @brief Contract state information
 */
typedef struct {
    int id;                        /**< Contract ID */
    char* hash;                    /**< Contract hash */
    char* nef;                     /**< NEF file data */
    char* manifest;                /**< Contract manifest */
    uint64_t update_counter;       /**< Update counter */
} neoc_contract_state_t;

/**
 * @brief GetContractState response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_contract_state_t* result;
} neoc_get_contract_state_response_t;

/**
 * @brief Application log entry
 */
typedef struct {
    neoc_hash256_t tx_id;          /**< Transaction ID */
    char* trigger;                 /**< Trigger type */
    char* vm_state;                /**< VM state */
    char* exception;               /**< Exception message */
    uint64_t gas_consumed;         /**< GAS consumed */
    char** stack;                  /**< Result stack */
    size_t stack_count;            /**< Stack size */
    char** notifications;          /**< Notifications */
    size_t notification_count;     /**< Number of notifications */
} neoc_app_log_entry_t;

/**
 * @brief GetApplicationLog response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_app_log_entry_t* result;
} neoc_get_app_log_response_t;

/**
 * @brief Unclaimed GAS information
 */
typedef struct {
    char* unclaimed;               /**< Unclaimed GAS amount */
    char* address;                 /**< Account address */
} neoc_unclaimed_gas_t;

/**
 * @brief GetUnclaimedGas response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_unclaimed_gas_t* result;
} neoc_get_unclaimed_gas_response_t;

/**
 * @brief Next block validators
 */
typedef struct {
    char** validators;             /**< Validator public keys */
    size_t validator_count;        /**< Number of validators */
} neoc_next_block_validators_t;

/**
 * @brief GetNextBlockValidators response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_next_block_validators_t* result;
} neoc_get_next_block_validators_response_t;

/**
 * @brief Committee information
 */
typedef struct {
    char** members;                /**< Committee member public keys */
    size_t member_count;           /**< Number of members */
} neoc_committee_t;

/**
 * @brief GetCommittee response
 */
typedef struct {
    neoc_response_base_t base;
    neoc_committee_t* result;
} neoc_get_committee_response_t;

/**
 * @defgroup ResponseTypeFunctions Response Type Functions
 * @brief Functions for creating and managing response types
 * @{
 */

/**
 * @brief Create a new GetVersion response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_version_response_create(neoc_get_version_response_t** response);

/**
 * @brief Free a GetVersion response
 * @param response Response to free
 */
void neoc_get_version_response_free(neoc_get_version_response_t* response);

/**
 * @brief Create a new GetBlock response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_block_response_create(neoc_get_block_response_t** response);

/**
 * @brief Free a GetBlock response
 * @param response Response to free
 */
void neoc_get_block_response_free(neoc_get_block_response_t* response);

/**
 * @brief Create a new GetNep17Balances response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_nep17_balances_response_create(neoc_get_nep17_balances_response_t** response);

/**
 * @brief Free a GetNep17Balances response
 * @param response Response to free
 */
void neoc_get_nep17_balances_response_free(neoc_get_nep17_balances_response_t* response);

/**
 * @brief Create a new GetNep17Transfers response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_nep17_transfers_response_create(neoc_get_nep17_transfers_response_t** response);

/**
 * @brief Free a GetNep17Transfers response
 * @param response Response to free
 */
void neoc_get_nep17_transfers_response_free(neoc_get_nep17_transfers_response_t* response);

/**
 * @brief Create a new GetPeers response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_peers_response_create(neoc_get_peers_response_t** response);

/**
 * @brief Free a GetPeers response
 * @param response Response to free
 */
void neoc_get_peers_response_free(neoc_get_peers_response_t* response);

/**
 * @brief Create a new GetMemPool response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_mempool_response_create(neoc_get_mempool_response_t** response);

/**
 * @brief Free a GetMemPool response
 * @param response Response to free
 */
void neoc_get_mempool_response_free(neoc_get_mempool_response_t* response);

/**
 * @brief Create a new GetContractState response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_contract_state_response_create(neoc_get_contract_state_response_t** response);

/**
 * @brief Free a GetContractState response
 * @param response Response to free
 */
void neoc_get_contract_state_response_free(neoc_get_contract_state_response_t* response);

/**
 * @brief Create a new GetApplicationLog response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_app_log_response_create(neoc_get_app_log_response_t** response);

/**
 * @brief Free a GetApplicationLog response
 * @param response Response to free
 */
void neoc_get_app_log_response_free(neoc_get_app_log_response_t* response);

/**
 * @brief Create a new GetUnclaimedGas response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_unclaimed_gas_response_create(neoc_get_unclaimed_gas_response_t** response);

/**
 * @brief Free a GetUnclaimedGas response
 * @param response Response to free
 */
void neoc_get_unclaimed_gas_response_free(neoc_get_unclaimed_gas_response_t* response);

/**
 * @brief Create a new GetNextBlockValidators response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_next_block_validators_response_create(neoc_get_next_block_validators_response_t** response);

/**
 * @brief Free a GetNextBlockValidators response
 * @param response Response to free
 */
void neoc_get_next_block_validators_response_free(neoc_get_next_block_validators_response_t* response);

/**
 * @brief Create a new GetCommittee response
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_committee_response_create(neoc_get_committee_response_t** response);

/**
 * @brief Free a GetCommittee response
 * @param response Response to free
 */
void neoc_get_committee_response_free(neoc_get_committee_response_t* response);

/** @} */ // end of ResponseTypeFunctions
/** @} */ // end of ResponseTypes

#ifdef __cplusplus
}
#endif

#endif // NEOC_PROTOCOL_RESPONSE_TYPES_H
