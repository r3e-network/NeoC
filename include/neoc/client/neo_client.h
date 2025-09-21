/**
 * @file neo_client.h
 * @brief Main Neo blockchain client interface
 */

#ifndef NEOC_NEO_CLIENT_H
#define NEOC_NEO_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neo client configuration
 */
typedef struct {
    neoc_hash160_t nns_resolver;               ///< NeoNameService resolver script hash
    uint32_t block_interval;                   ///< Block production interval in milliseconds
    uint32_t polling_interval;                 ///< Polling interval for blockchain observation
    uint32_t max_valid_until_block_increment;  ///< Maximum valid block increment for transactions
    bool allow_transmission_on_fault;          ///< Allow sending transactions that fault
    uint32_t network_magic;                    ///< Network magic number
    bool network_magic_set;                    ///< Whether network magic has been set
} neoc_client_config_t;

/**
 * @brief Neo client structure
 */
typedef struct neoc_client neoc_client_t;

/**
 * @brief RPC version information
 */
typedef struct {
    uint32_t protocol;
    uint32_t network;
    char *user_agent;
    uint32_t rpc_version;
} neoc_rpc_version_t;

/**
 * @brief Block structure
 */
typedef struct {
    uint32_t version;
    neoc_hash256_t hash;
    neoc_hash256_t prev_hash;
    neoc_hash256_t merkle_root;
    uint64_t timestamp;
    uint32_t index;
    uint8_t primary_index;
    neoc_hash160_t next_consensus;
    // Additional fields as needed
} neoc_block_t;

/**
 * @brief Contract state structure
 */
typedef struct {
    int32_t id;
    uint16_t update_counter;
    neoc_hash160_t hash;
    uint8_t *nef;
    size_t nef_len;
    char *manifest;
} neoc_contract_state_t;

/**
 * @brief Native contract structure
 */
typedef struct {
    int32_t id;
    neoc_hash160_t hash;
    char *name;
    char *manifest;
} neoc_native_contract_t;

/**
 * @brief Peer information
 */
typedef struct {
    char *address;
    uint16_t port;
} neoc_peer_t;

/**
 * @brief Contract parameter for invocation
 */
typedef struct {
    uint8_t type;
    void *value;
} neoc_contract_parameter_t;

/**
 * @brief Transaction signer
 */
typedef struct neoc_signer neoc_signer_t;

/**
 * @brief Transaction structure
 */
typedef struct neoc_transaction neoc_transaction_t;

/**
 * @brief Invocation result
 */
typedef struct {
    char *script;
    char *state;
    uint64_t gas_consumed;
    char *exception;
    // Stack items would go here
} neoc_invocation_result_t;

/**
 * @brief NEP-17 balance
 */
typedef struct {
    neoc_hash160_t asset_hash;
    uint64_t amount;
    uint32_t last_updated_block;
} neoc_nep17_balance_t;

/**
 * @brief NEP-17 transfer
 */
typedef struct {
    uint64_t timestamp;
    neoc_hash160_t asset_hash;
    neoc_hash160_t from;
    neoc_hash160_t to;
    uint64_t amount;
    uint32_t block_index;
    neoc_hash256_t tx_hash;
} neoc_nep17_transfer_t;

/**
 * @brief State root
 */
typedef struct {
    uint32_t version;
    uint32_t index;
    neoc_hash256_t root_hash;
    // Witness would go here
} neoc_state_root_t;

/**
 * @brief State height
 */
typedef struct {
    uint32_t local_root_index;
    uint32_t validated_root_index;
} neoc_state_height_t;

/**
 * @brief Proof structure
 */
typedef struct {
    uint8_t *proof_bytes;
    size_t proof_len;
} neoc_proof_t;

/**
 * @brief Application log
 */
typedef struct {
    neoc_hash256_t tx_hash;
    char *trigger;
    char *vm_state;
    uint64_t gas_consumed;
    // Notifications and stack would go here
} neoc_application_log_t;

/**
 * @brief Wallet address
 */
typedef struct {
    char *address;
    bool has_key;
    char *label;
    bool watch_only;
} neoc_wallet_address_t;

/**
 * @brief Send token structure for send_many
 */
typedef struct {
    neoc_hash160_t token_hash;
    neoc_hash160_t to;
    uint64_t amount;
} neoc_send_token_t;

// Configuration functions

/**
 * @brief Create a new client configuration with defaults
 */
neoc_error_t neoc_client_config_create(neoc_client_config_t **config);

/**
 * @brief Set the NNS resolver script hash
 */
neoc_error_t neoc_client_config_set_nns_resolver(neoc_client_config_t *config,
                                                  const neoc_hash160_t *resolver);

/**
 * @brief Set the network magic number
 */
neoc_error_t neoc_client_config_set_network_magic(neoc_client_config_t *config,
                                                   uint32_t magic);

/**
 * @brief Allow or prevent transmission of faulting scripts
 */
neoc_error_t neoc_client_config_allow_fault_transmission(neoc_client_config_t *config,
                                                         bool allow);

/**
 * @brief Free configuration
 */
void neoc_client_config_free(neoc_client_config_t *config);

// Client lifecycle

/**
 * @brief Create a new Neo client
 * 
 * @param client Output client pointer
 * @param node_url URL of the Neo node
 * @param config Optional configuration (NULL for defaults)
 * @return Error code
 */
neoc_error_t neoc_client_create(neoc_client_t **client,
                                const char *node_url,
                                neoc_client_config_t *config);

/**
 * @brief Get the network magic number
 */
neoc_error_t neoc_client_get_network_magic(neoc_client_t *client,
                                           uint32_t *magic);

/**
 * @brief Get the network magic number as bytes
 */
neoc_error_t neoc_client_get_network_magic_bytes(neoc_client_t *client,
                                                 uint8_t *magic_bytes,
                                                 size_t *bytes_len);

// Blockchain methods

/**
 * @brief Get the hash of the best (latest) block
 */
neoc_error_t neoc_client_get_best_block_hash(neoc_client_t *client,
                                             neoc_hash256_t *hash);

/**
 * @brief Get block hash by index
 */
neoc_error_t neoc_client_get_block_hash(neoc_client_t *client,
                                        uint32_t index,
                                        neoc_hash256_t *hash);

/**
 * @brief Get block count
 */
neoc_error_t neoc_client_get_block_count(neoc_client_t *client,
                                         uint32_t *count);

/**
 * @brief Get block by hash
 */
neoc_error_t neoc_client_get_block_by_hash(neoc_client_t *client,
                                           const neoc_hash256_t *hash,
                                           bool verbose,
                                           neoc_block_t **block);

/**
 * @brief Get block by index
 */
neoc_error_t neoc_client_get_block_by_index(neoc_client_t *client,
                                            uint32_t index,
                                            bool verbose,
                                            neoc_block_t **block);

/**
 * @brief Get block header count
 */
neoc_error_t neoc_client_get_block_header_count(neoc_client_t *client,
                                                uint32_t *count);

// Contract methods

/**
 * @brief Get contract state
 */
neoc_error_t neoc_client_get_contract_state(neoc_client_t *client,
                                            const neoc_hash160_t *script_hash,
                                            neoc_contract_state_t **state);

/**
 * @brief Get native contracts
 */
neoc_error_t neoc_client_get_native_contracts(neoc_client_t *client,
                                              neoc_native_contract_t ***contracts,
                                              size_t *count);

// Node methods

/**
 * @brief Get connection count
 */
neoc_error_t neoc_client_get_connection_count(neoc_client_t *client,
                                              uint32_t *count);

/**
 * @brief Get peers
 */
neoc_error_t neoc_client_get_peers(neoc_client_t *client,
                                   neoc_peer_t ***peers,
                                   size_t *count);

/**
 * @brief Get version
 */
neoc_error_t neoc_client_get_version(neoc_client_t *client,
                                     neoc_rpc_version_t **version);

// SmartContract methods

/**
 * @brief Invoke a smart contract function
 */
neoc_error_t neoc_client_invoke_function(neoc_client_t *client,
                                         const neoc_hash160_t *script_hash,
                                         const char *function,
                                         neoc_contract_parameter_t **params,
                                         size_t param_count,
                                         neoc_signer_t **signers,
                                         size_t signer_count,
                                         neoc_invocation_result_t **result);

/**
 * @brief Invoke a script
 */
neoc_error_t neoc_client_invoke_script(neoc_client_t *client,
                                       const uint8_t *script,
                                       size_t script_len,
                                       neoc_signer_t **signers,
                                       size_t signer_count,
                                       neoc_invocation_result_t **result);

/**
 * @brief Get unclaimed gas
 */
neoc_error_t neoc_client_get_unclaimed_gas(neoc_client_t *client,
                                           const neoc_hash160_t *address,
                                           uint64_t *unclaimed);

// Transaction methods

/**
 * @brief Send raw transaction
 */
neoc_error_t neoc_client_send_raw_transaction(neoc_client_t *client,
                                              const uint8_t *tx_bytes,
                                              size_t tx_len,
                                              neoc_hash256_t *tx_hash);

/**
 * @brief Get transaction
 */
neoc_error_t neoc_client_get_transaction(neoc_client_t *client,
                                         const neoc_hash256_t *tx_hash,
                                         bool verbose,
                                         neoc_transaction_t **transaction);

/**
 * @brief Get transaction height
 */
neoc_error_t neoc_client_get_transaction_height(neoc_client_t *client,
                                                const neoc_hash256_t *tx_hash,
                                                uint32_t *height);

// NEP-17 Token methods

/**
 * @brief Get NEP-17 balances
 */
neoc_error_t neoc_client_get_nep17_balances(neoc_client_t *client,
                                            const neoc_hash160_t *address,
                                            neoc_nep17_balance_t ***balances,
                                            size_t *count);

/**
 * @brief Get NEP-17 transfers
 */
neoc_error_t neoc_client_get_nep17_transfers(neoc_client_t *client,
                                             const neoc_hash160_t *address,
                                             uint64_t timestamp_start,
                                             uint64_t timestamp_end,
                                             neoc_nep17_transfer_t ***transfers,
                                             size_t *count);

// State methods

/**
 * @brief Get state root
 */
neoc_error_t neoc_client_get_state_root(neoc_client_t *client,
                                        uint32_t block_index,
                                        neoc_state_root_t **root);

/**
 * @brief Get state height
 */
neoc_error_t neoc_client_get_state_height(neoc_client_t *client,
                                          neoc_state_height_t **height);

/**
 * @brief Get proof
 */
neoc_error_t neoc_client_get_proof(neoc_client_t *client,
                                   const neoc_hash256_t *root_hash,
                                   const neoc_hash160_t *script_hash,
                                   const uint8_t *storage_key,
                                   size_t key_len,
                                   neoc_proof_t **proof);

/**
 * @brief Verify proof
 */
neoc_error_t neoc_client_verify_proof(neoc_client_t *client,
                                      const neoc_hash256_t *root_hash,
                                      const neoc_proof_t *proof,
                                      bool *is_valid);

// Application logs

/**
 * @brief Get application log
 */
neoc_error_t neoc_client_get_application_log(neoc_client_t *client,
                                             const neoc_hash256_t *tx_hash,
                                             neoc_application_log_t **log);

// Utility methods

/**
 * @brief Validate address
 */
neoc_error_t neoc_client_validate_address(neoc_client_t *client,
                                          const char *address,
                                          bool *is_valid);

/**
 * @brief Close wallet
 */
neoc_error_t neoc_client_close_wallet(neoc_client_t *client);

/**
 * @brief Dump private key
 */
neoc_error_t neoc_client_dump_private_key(neoc_client_t *client,
                                          const neoc_hash160_t *script_hash,
                                          char **wif);

/**
 * @brief Get new address
 */
neoc_error_t neoc_client_get_new_address(neoc_client_t *client,
                                         char **address);

/**
 * @brief Get wallet balance
 */
neoc_error_t neoc_client_get_wallet_balance(neoc_client_t *client,
                                            const neoc_hash160_t *token_hash,
                                            uint64_t *balance);

/**
 * @brief Get wallet unclaimed gas
 */
neoc_error_t neoc_client_get_wallet_unclaimed_gas(neoc_client_t *client,
                                                  uint64_t *unclaimed);

/**
 * @brief Import private key
 */
neoc_error_t neoc_client_import_private_key(neoc_client_t *client,
                                            const char *wif);

/**
 * @brief List addresses
 */
neoc_error_t neoc_client_list_address(neoc_client_t *client,
                                      neoc_wallet_address_t ***addresses,
                                      size_t *count);

/**
 * @brief Open wallet
 */
neoc_error_t neoc_client_open_wallet(neoc_client_t *client,
                                     const char *path,
                                     const char *password);

/**
 * @brief Send from specific address
 */
neoc_error_t neoc_client_send_from(neoc_client_t *client,
                                   const neoc_hash160_t *token_hash,
                                   const neoc_hash160_t *from,
                                   const neoc_hash160_t *to,
                                   uint64_t amount,
                                   neoc_hash256_t *tx_hash);

/**
 * @brief Send many tokens
 */
neoc_error_t neoc_client_send_many(neoc_client_t *client,
                                   const neoc_hash160_t *from,
                                   neoc_send_token_t **tokens,
                                   size_t token_count,
                                   neoc_hash256_t *tx_hash);

/**
 * @brief Send to address
 */
neoc_error_t neoc_client_send_to_address(neoc_client_t *client,
                                         const neoc_hash160_t *token_hash,
                                         const neoc_hash160_t *to,
                                         uint64_t amount,
                                         neoc_hash256_t *tx_hash);

// Cleanup

/**
 * @brief Free Neo client
 */
void neoc_client_free(neoc_client_t *client);

/**
 * @brief Free RPC version
 */
void neoc_rpc_version_free(neoc_rpc_version_t *version);

/**
 * @brief Free block
 */
void neoc_block_free(neoc_block_t *block);

/**
 * @brief Free contract state
 */
void neoc_contract_state_free(neoc_contract_state_t *state);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_CLIENT_H
