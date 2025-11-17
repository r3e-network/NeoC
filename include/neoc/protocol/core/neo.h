/**
 * @file neo.h
 * @brief Core Neo protocol interface and types
 * 
 * Implements the core Neo blockchain protocol interface for RPC communication,
 * smart contract interaction, and blockchain data retrieval. Based on the
 * Swift Neo protocol implementation.
 */

#ifndef NEOC_PROTOCOL_CORE_NEO_H
#define NEOC_PROTOCOL_CORE_NEO_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/contract/contract_parameter.h"
#include "neoc/transaction/signer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for protocol types
typedef struct neoc_request neoc_request_t;
struct neoc_rpc_block;
struct neoc_rpc_transaction;

typedef struct neoc_rpc_block neoc_block_t;
#ifndef NEOC_TRANSACTION_H
typedef struct neoc_rpc_transaction neoc_transaction_t;
#endif
typedef struct neoc_contract_state neoc_contract_state_t;
typedef struct neoc_invocation_result neoc_invocation_result_t;
typedef struct neoc_rpc_client_t neoc_rpc_client_t;

/**
 * @brief Neo protocol client structure
 */
typedef struct {
    char *rpc_url;           /**< RPC endpoint URL */
    uint32_t network_magic;  /**< Network magic number */
    bool network_magic_set;  /**< Whether the magic number has been resolved */
    neoc_rpc_client_t *rpc_client; /**< Underlying JSON-RPC client */
    void *context;           /**< Additional context data */
} neoc_neo_client_t;

/**
 * @brief Create a new Neo protocol client
 * @param rpc_url The RPC endpoint URL
 * @param network_magic The network magic number
 * @param client Pointer to store the created client
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_client_create(const char *rpc_url, uint32_t network_magic, neoc_neo_client_t **client);

/**
 * @brief Free Neo protocol client
 * @param client The client to free
 */
void neoc_neo_client_free(neoc_neo_client_t *client);

// MARK: Blockchain Methods

/**
 * @brief Get the hash of the latest block
 * @param client The Neo client
 * @param block_hash Pointer to store the block hash
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_best_block_hash(neoc_neo_client_t *client, neoc_hash256_t *block_hash);

/**
 * @brief Get block hash by index
 * @param client The Neo client
 * @param block_index The block index
 * @param block_hash Pointer to store the block hash
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_hash(neoc_neo_client_t *client, uint32_t block_index, neoc_hash256_t *block_hash);

/**
 * @brief Get block by hash
 * @param client The Neo client
 * @param block_hash The block hash
 * @param full_transactions Whether to return full transaction objects
 * @param block Pointer to store the block
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_by_hash(neoc_neo_client_t *client, const neoc_hash256_t *block_hash, bool full_transactions, neoc_block_t **block);

/**
 * @brief Get block by index
 * @param client The Neo client
 * @param block_index The block index
 * @param full_transactions Whether to return full transaction objects
 * @param block Pointer to store the block
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_by_index(neoc_neo_client_t *client, uint32_t block_index, bool full_transactions, neoc_block_t **block);

/**
 * @brief Get block count
 * @param client The Neo client
 * @param count Pointer to store the block count
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_count(neoc_neo_client_t *client, uint32_t *count);

/**
 * @brief Get contract state
 * @param client The Neo client
 * @param contract_hash The contract hash
 * @param state Pointer to store the contract state
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_contract_state(neoc_neo_client_t *client, const neoc_hash160_t *contract_hash, neoc_contract_state_t **state);

/**
 * @brief Get transaction by hash
 * @param client The Neo client
 * @param tx_hash The transaction hash
 * @param transaction Pointer to store the transaction
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_transaction(neoc_neo_client_t *client, const neoc_hash256_t *tx_hash, neoc_transaction_t **transaction);

// MARK: Node Methods

/**
 * @brief Get connection count
 * @param client The Neo client
 * @param count Pointer to store the connection count
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_connection_count(neoc_neo_client_t *client, uint32_t *count);

/**
 * @brief Send raw transaction
 * @param client The Neo client
 * @param raw_transaction_hex The raw transaction in hex format
 * @param tx_hash Pointer to store the transaction hash
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_send_raw_transaction(neoc_neo_client_t *client, const char *raw_transaction_hex, neoc_hash256_t *tx_hash);

// MARK: Smart Contract Methods

/**
 * @brief Invoke contract function
 * @param client The Neo client
 * @param contract_hash The contract hash
 * @param function_name The function name to invoke
 * @param params Array of contract parameters (can be NULL)
 * @param param_count Number of parameters
 * @param signers Array of signers (can be NULL)
 * @param signer_count Number of signers
 * @param result Pointer to store the invocation result
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_invoke_function(neoc_neo_client_t *client, 
                                     const neoc_hash160_t *contract_hash,
                                     const char *function_name,
                                     const neoc_contract_parameter_t *params,
                                     size_t param_count,
                                     const neoc_signer_t *signers,
                                     size_t signer_count,
                                     neoc_invocation_result_t **result);

/**
 * @brief Invoke script
 * @param client The Neo client
 * @param script_hex The script in hex format
 * @param signers Array of signers (can be NULL)
 * @param signer_count Number of signers
 * @param result Pointer to store the invocation result
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_invoke_script(neoc_neo_client_t *client,
                                   const char *script_hex,
                                   const neoc_signer_t *signers,
                                   size_t signer_count,
                                   neoc_invocation_result_t **result);

// MARK: Utilities Methods

/**
 * @brief Validate Neo address
 * @param client The Neo client
 * @param address The address to validate
 * @param is_valid Pointer to store the validation result
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_validate_address(neoc_neo_client_t *client, const char *address, bool *is_valid);

/**
 * @brief Calculate network fee for transaction
 * @param client The Neo client
 * @param transaction_hex The transaction in hex format
 * @param network_fee Pointer to store the calculated network fee
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_calculate_network_fee(neoc_neo_client_t *client, const char *transaction_hex, uint64_t *network_fee);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_NEO_H */
