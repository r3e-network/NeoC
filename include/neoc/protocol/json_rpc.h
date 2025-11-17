/**
 * @file json_rpc.h
 * @brief JSON-RPC 2.0 client implementation for Neo blockchain
 */

#ifndef NEOC_JSON_RPC_H
#define NEOC_JSON_RPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/protocol/contract_response_types.h"

/**
 * @brief JSON-RPC client configuration
 */
typedef struct {
    char* url;                              ///< RPC endpoint URL
    int timeout_ms;                         ///< Request timeout in milliseconds
    int max_retries;                        ///< Maximum number of retries
    bool allow_transmission_on_fault;       ///< Allow transmission of faulted scripts
    uint32_t network_magic;                 ///< Network magic number
    neoc_hash160_t nns_resolver;           ///< NNS resolver contract hash
    int block_interval_ms;                 ///< Block interval in milliseconds
    int polling_interval_ms;                ///< Polling interval in milliseconds
    int max_valid_until_block_increment;   ///< Max valid until block increment
} neoc_rpc_config_t;

/**
 * @brief JSON-RPC client handle
 */
typedef struct neoc_rpc_client_t neoc_rpc_client_t;

/**
 * @brief JSON-RPC request structure
 */
typedef struct {
    const char* method;     ///< RPC method name
    const char* params;     ///< JSON parameters (can be NULL)
    int id;                 ///< Request ID
} neoc_rpc_request_t;

/**
 * @brief JSON-RPC response structure
 */
typedef struct {
    int id;                 ///< Response ID
    char* result;           ///< JSON result (NULL if error)
    char* error_message;    ///< Error message (NULL if success)
    int error_code;         ///< Error code (0 if success)
    char* raw_response;     ///< Raw JSON response
} neoc_rpc_response_t;

/**
 * @brief Block information
 */
typedef struct {
    neoc_hash256_t hash;
    uint32_t version;
    neoc_hash256_t prev_hash;
    neoc_hash256_t merkle_root;
    uint64_t timestamp;
    uint64_t nonce;
    uint32_t index;
    uint8_t primary_index;
    neoc_hash160_t next_consensus;
    size_t tx_count;
    neoc_hash256_t* tx_hashes;     ///< Array of transaction hashes
    char* witness_json;             ///< Witness as JSON string
} neoc_block_t;

/**
 * @brief Transaction information
 */
typedef struct {
    neoc_hash256_t hash;
    uint32_t version;
    uint32_t nonce;
    neoc_hash160_t sender;
    uint64_t system_fee;
    uint64_t network_fee;
    uint32_t valid_until_block;
    char* script;                   ///< Script in hex
    size_t signer_count;
    char* signers_json;             ///< Signers as JSON string
    size_t witness_count;
    char* witnesses_json;           ///< Witnesses as JSON string
    size_t attribute_count;
    char* attributes_json;          ///< Attributes as JSON string
} neoc_transaction_info_t;

/**
 * @brief Invocation result
 */
typedef struct {
    char* script;                   ///< Executed script
    char* state;                    ///< VM state (HALT/FAULT/BREAK)
    uint64_t gas_consumed;          ///< GAS consumed
    char* exception;                ///< Exception message if faulted
    char* stack_json;               ///< Stack items as JSON
    char* notifications_json;       ///< Notifications as JSON
    char* session_id;               ///< Session ID for iterators
    char* diagnostics_json;         ///< Diagnostics as JSON
} neoc_invocation_result_t;

/**
 * @brief Version information
 */
typedef struct {
    uint32_t tcp_port;
    uint32_t ws_port;
    uint32_t nonce;
    char* user_agent;
    uint32_t protocol_version;
    uint32_t protocol_network;
    uint32_t protocol_milliseconds_per_block;
    uint32_t protocol_max_trace_blocks;
    uint32_t protocol_max_transactions_per_block;
    uint32_t protocol_memory_pool_max_transactions;
    uint32_t protocol_max_valid_until_block_increment;
    uint64_t protocol_initial_gas_distribution;
    bool protocol_hardforks_enabled;
} neoc_version_info_t;

// Client Management

/**
 * @brief Create a new RPC client
 * 
 * @param client Output RPC client handle
 * @param config Configuration for the client
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_client_create(neoc_rpc_client_t** client,
                                     const neoc_rpc_config_t* config);

/**
 * @brief Free an RPC client
 * 
 * @param client The client to free
 */
void neoc_rpc_client_free(neoc_rpc_client_t* client);

/**
 * @brief Set a custom header for all requests
 * 
 * @param client The RPC client
 * @param key Header key
 * @param value Header value
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_set_header(neoc_rpc_client_t* client,
                                  const char* key,
                                  const char* value);

// Request/Response Management

/**
 * @brief Send a raw JSON-RPC request
 * 
 * @param client The RPC client
 * @param request The request to send
 * @param response Output response
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_send_request(neoc_rpc_client_t* client,
                                    const neoc_rpc_request_t* request,
                                    neoc_rpc_response_t** response);

/**
 * @brief Free an RPC response
 * 
 * @param response The response to free
 */
void neoc_rpc_response_free(neoc_rpc_response_t* response);

// Blockchain Methods

/**
 * @brief Get the hash of the best (latest) block
 * 
 * @param client The RPC client
 * @param hash Output block hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_best_block_hash(neoc_rpc_client_t* client,
                                           neoc_hash256_t* hash);

/**
 * @brief Get block hash by index
 * 
 * @param client The RPC client
 * @param index Block index
 * @param hash Output block hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_block_hash(neoc_rpc_client_t* client,
                                      uint32_t index,
                                      neoc_hash256_t* hash);

/**
 * @brief Get block by hash
 * 
 * @param client The RPC client
 * @param hash Block hash
 * @param full_tx Whether to include full transaction objects
 * @param block Output block information
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_block(neoc_rpc_client_t* client,
                                 const neoc_hash256_t* hash,
                                 bool full_tx,
                                 neoc_block_t** block);

/**
 * @brief Get block count
 * 
 * @param client The RPC client
 * @param count Output block count
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_block_count(neoc_rpc_client_t* client,
                                       uint32_t* count);

/**
 * @brief Get transaction by hash
 * 
 * @param client The RPC client
 * @param hash Transaction hash
 * @param tx Output transaction information
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_transaction(neoc_rpc_client_t* client,
                                       const neoc_hash256_t* hash,
                                       neoc_transaction_info_t** tx);

/**
 * @brief Get contract state
 * 
 * @param client The RPC client
 * @param hash Contract hash
 * @param state Output contract state
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_contract_state(neoc_rpc_client_t* client,
                                          const neoc_hash160_t* hash,
                                          neoc_contract_state_t** state);

/**
 * @brief Get storage value
 * 
 * @param client The RPC client
 * @param contract Contract hash
 * @param key Storage key (hex string)
 * @param value Output value (hex string, caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_storage(neoc_rpc_client_t* client,
                                   const neoc_hash160_t* contract,
                                   const char* key,
                                   char** value);

// Node Methods

/**
 * @brief Get node version information
 * 
 * @param client The RPC client
 * @param version Output version information
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_version(neoc_rpc_client_t* client,
                                   neoc_version_info_t** version);

/**
 * @brief Get connection count
 * 
 * @param client The RPC client
 * @param count Output connection count
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_connection_count(neoc_rpc_client_t* client,
                                            uint32_t* count);

/**
 * @brief Send raw transaction
 * 
 * @param client The RPC client
 * @param tx_hex Transaction in hex format
 * @param tx_hash Output transaction hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_send_raw_transaction(neoc_rpc_client_t* client,
                                            const char* tx_hex,
                                            neoc_hash256_t* tx_hash);

// Smart Contract Methods

/**
 * @brief Invoke a contract function
 * 
 * @param client The RPC client
 * @param contract Contract hash
 * @param method Method name
 * @param params_json Parameters as JSON array (can be NULL)
 * @param signers_json Signers as JSON array (can be NULL)
 * @param result Output invocation result
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_invoke_function(neoc_rpc_client_t* client,
                                       const neoc_hash160_t* contract,
                                       const char* method,
                                       const char* params_json,
                                       const char* signers_json,
                                       neoc_invocation_result_t** result);

/**
 * @brief Invoke a script
 * 
 * @param client The RPC client
 * @param script_hex Script in hex format
 * @param signers_json Signers as JSON array (can be NULL)
 * @param result Output invocation result
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_invoke_script(neoc_rpc_client_t* client,
                                     const char* script_hex,
                                     const char* signers_json,
                                     neoc_invocation_result_t** result);

// NEP-17 Token Methods

/**
 * @brief Get NEP-17 token balance
 * 
 * @param client The RPC client
 * @param account Account script hash
 * @param balances_json Output balances as JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_nep17_balances(neoc_rpc_client_t* client,
                                          const neoc_hash160_t* account,
                                          char** balances_json);

/**
 * @brief Get NEP-17 transfers
 * 
 * @param client The RPC client
 * @param account Account script hash
 * @param from_timestamp Start timestamp (0 for all)
 * @param to_timestamp End timestamp (0 for all)
 * @param transfers_json Output transfers as JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_get_nep17_transfers(neoc_rpc_client_t* client,
                                           const neoc_hash160_t* account,
                                           uint64_t from_timestamp,
                                           uint64_t to_timestamp,
                                           char** transfers_json);

// Utility Functions

/**
 * @brief Validate an address
 * 
 * @param client The RPC client
 * @param address Address to validate
 * @param is_valid Output validation result
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_validate_address(neoc_rpc_client_t* client,
                                        const char* address,
                                        bool* is_valid);

/**
 * @brief Calculate network fee for a transaction
 * 
 * @param client The RPC client
 * @param tx_hex Transaction in hex format
 * @param fee Output network fee
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_rpc_calculate_network_fee(neoc_rpc_client_t* client,
                                             const char* tx_hex,
                                             uint64_t* fee);

// Helper Functions

/**
 * @brief Free block information
 * 
 * @param block Block to free
 */
void neoc_block_free(neoc_block_t* block);

/**
 * @brief Free transaction information
 * 
 * @param tx Transaction to free
 */
void neoc_transaction_info_free(neoc_transaction_info_t* tx);

/**
 * @brief Free contract state
 * 
 * @param state Contract state to free
 */
void neoc_contract_state_free(neoc_contract_state_t* state);

/**
 * @brief Free invocation result
 * 
 * @param result Invocation result to free
 */
void neoc_invocation_result_free(neoc_invocation_result_t* result);

/**
 * @brief Free version information
 * 
 * @param version Version information to free
 */
void neoc_version_info_free(neoc_version_info_t* version);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_JSON_RPC_H */
