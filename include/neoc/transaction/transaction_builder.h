/**
 * @file transaction_builder.h
 * @brief Neo transaction builder for constructing and signing transactions
 */

#ifndef NEOC_TRANSACTION_BUILDER_H
#define NEOC_TRANSACTION_BUILDER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/transaction/transaction.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness.h"
#include "neoc/wallet/account.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct neoc_tx_builder_t neoc_tx_builder_t;
typedef neoc_tx_builder_t neoc_transaction_builder_t;
#ifndef NEOC_RPC_CLIENT_FORWARD_DECLARED
#define NEOC_RPC_CLIENT_FORWARD_DECLARED
typedef struct neoc_rpc_client_t neoc_rpc_client_t;
#endif

// Transaction builder constants
#define TX_BUILDER_MAX_SCRIPT_SIZE 65536
#define TX_BUILDER_MAX_SIGNERS 16
#define TX_BUILDER_MAX_ATTRIBUTES 16
#define TX_BUILDER_MAX_WITNESSES 16
#define TX_DEFAULT_VALID_UNTIL_BLOCK 1000000

// Fee constants (in GAS fractions)
#define TX_DEFAULT_NETWORK_FEE 100000      // 0.001 GAS
#define TX_DEFAULT_SYSTEM_FEE 0            // Variable based on script
#define TX_PRIORITY_NETWORK_FEE 500000     // 0.005 GAS for high priority

// Transaction attribute types and structures are defined in transaction.h

/**
 * @brief Create a new transaction builder
 * 
 * @param builder Output transaction builder
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_create(neoc_tx_builder_t **builder);
neoc_error_t neoc_transaction_builder_create(neoc_transaction_builder_t **builder);

/**
 * @brief Set the transaction version
 * 
 * @param builder Transaction builder
 * @param version Version number (default: 0)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_version(neoc_tx_builder_t *builder, uint8_t version);
neoc_error_t neoc_transaction_builder_set_version(neoc_transaction_builder_t *builder, uint8_t version);

/**
 * @brief Set the transaction nonce
 * 
 * @param builder Transaction builder
 * @param nonce Nonce value (random by default)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_nonce(neoc_tx_builder_t *builder, uint32_t nonce);
neoc_error_t neoc_transaction_builder_set_nonce(neoc_transaction_builder_t *builder, uint32_t nonce);

/**
 * @brief Set the valid until block
 * 
 * @param builder Transaction builder
 * @param block_height Block height until which transaction is valid (must be > 0)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_valid_until_block(neoc_tx_builder_t *builder, 
                                                     uint32_t block_height);
neoc_error_t neoc_transaction_builder_set_valid_until_block(neoc_transaction_builder_t *builder,
                                                            uint32_t block_height);

/**
 * @brief Set valid until block using current chain height from RPC
 *
 * @param builder Transaction builder
 * @param client RPC client to query current block height
 * @param increment Blocks to add to current height (default 1000 if 0)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_valid_until_block_from_rpc(neoc_tx_builder_t *builder,
                                                            neoc_rpc_client_t *client,
                                                            uint32_t increment);
neoc_error_t neoc_transaction_builder_set_valid_until_block_from_rpc(neoc_transaction_builder_t *builder,
                                                                     neoc_rpc_client_t *client,
                                                                     uint32_t increment);

/**
 * @brief Set the transaction script
 * 
 * @param builder Transaction builder
 * @param script Script bytes
 * @param script_size Script size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_script(neoc_tx_builder_t *builder,
                                         const uint8_t *script,
                                         size_t script_size);
neoc_error_t neoc_transaction_builder_set_script(neoc_transaction_builder_t *builder,
                                                 const uint8_t *script,
                                                 size_t script_size);

/**
 * @brief Add a signer to the transaction
 * 
 * @param builder Transaction builder
 * @param signer Signer to add
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_add_signer(neoc_tx_builder_t *builder,
                                         const neoc_signer_t *signer);
neoc_error_t neoc_transaction_builder_add_signer(neoc_transaction_builder_t *builder,
                                                 const neoc_signer_t *signer);

/**
 * @brief Add a signer from account
 * 
 * @param builder Transaction builder
 * @param account Account to add as signer
 * @param scope Witness scope for the signer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_add_signer_from_account(neoc_tx_builder_t *builder,
                                                      const neoc_account_t *account,
                                                      neoc_witness_scope_t scope);
neoc_error_t neoc_transaction_builder_add_signer_from_account(neoc_transaction_builder_t *builder,
                                                              const neoc_account_t *account,
                                                              neoc_witness_scope_t scope);

/**
 * @brief Set the first signer (fee payer)
 * 
 * @param builder Transaction builder
 * @param account Account to set as first signer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_first_signer(neoc_tx_builder_t *builder,
                                               const neoc_account_t *account);
neoc_error_t neoc_transaction_builder_set_first_signer(neoc_transaction_builder_t *builder,
                                                       const neoc_account_t *account);

/**
 * @brief Add a transaction attribute
 * 
 * @param builder Transaction builder
 * @param attribute Attribute to add
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_add_attribute(neoc_tx_builder_t *builder,
                                           const neoc_tx_attribute_t *attribute);

/**
 * @brief Set high priority for the transaction
 * 
 * @param builder Transaction builder
 * @param high_priority True for high priority
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_set_high_priority(neoc_tx_builder_t *builder,
                                               bool high_priority);

/**
 * @brief Add additional network fee
 * 
 * @param builder Transaction builder
 * @param fee Additional network fee in GAS fractions
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_add_network_fee(neoc_tx_builder_t *builder,
                                              uint64_t fee);
neoc_error_t neoc_transaction_builder_add_network_fee(neoc_transaction_builder_t *builder,
                                                      uint64_t fee);

/**
 * @brief Add additional system fee
 * 
 * @param builder Transaction builder
 * @param fee Additional system fee in GAS fractions
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_add_system_fee(neoc_tx_builder_t *builder,
                                             uint64_t fee);
neoc_error_t neoc_transaction_builder_add_system_fee(neoc_transaction_builder_t *builder,
                                                     uint64_t fee);

/**
 * @brief Calculate fees for the transaction (requires RPC client)
 * 
 * @param builder Transaction builder
 * @param client RPC client for fee calculation
 * @param network_fee Output network fee
 * @param system_fee Output system fee
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_calculate_fees(neoc_tx_builder_t *builder,
                                            neoc_rpc_client_t *client,
                                            uint64_t *network_fee,
                                            uint64_t *system_fee);

/**
 * @brief Build the unsigned transaction
 * 
 * @param builder Transaction builder
 * @param transaction Output transaction
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_build_unsigned(neoc_tx_builder_t *builder,
                                            neoc_transaction_t **transaction);
neoc_error_t neoc_transaction_builder_build(neoc_transaction_builder_t *builder,
                                            neoc_transaction_t **transaction);

/**
 * @brief Sign the transaction with accounts
 * 
 * @param builder Transaction builder
 * @param accounts Array of accounts for signing
 * @param account_count Number of accounts
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_sign(neoc_tx_builder_t *builder,
                                   neoc_account_t **accounts,
                                   size_t account_count);
neoc_error_t neoc_transaction_builder_sign(neoc_transaction_builder_t *builder,
                                           neoc_account_t **accounts,
                                           size_t account_count);

/**
 * @brief Build and sign the transaction
 * 
 * @param builder Transaction builder
 * @param accounts Array of accounts for signing
 * @param account_count Number of accounts
 * @param transaction Output signed transaction
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_build_and_sign(neoc_tx_builder_t *builder,
                                             neoc_account_t **accounts,
                                             size_t account_count,
                                             neoc_transaction_t **transaction);
neoc_error_t neoc_transaction_builder_build_and_sign(neoc_transaction_builder_t *builder,
                                                     neoc_account_t **accounts,
                                                     size_t account_count,
                                                     neoc_transaction_t **transaction);

/**
 * @brief Get the transaction hash
 * 
 * @param builder Transaction builder
 * @param hash Output transaction hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_get_hash(const neoc_tx_builder_t *builder,
                                       neoc_hash256_t *hash);

/**
 * @brief Serialize transaction to bytes
 * 
 * @param builder Transaction builder
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @param serialized_size Output serialized size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_serialize(const neoc_tx_builder_t *builder,
                                        uint8_t *buffer,
                                        size_t buffer_size,
                                        size_t *serialized_size);
neoc_error_t neoc_transaction_builder_serialize(const neoc_transaction_builder_t *builder,
                                                uint8_t *buffer,
                                                size_t buffer_size,
                                                size_t *serialized_size);

/**
 * @brief Free transaction builder
 * 
 * @param builder Transaction builder to free
 */
void neoc_tx_builder_free(neoc_tx_builder_t *builder);
void neoc_transaction_builder_free(neoc_transaction_builder_t *builder);

// Helper functions for common transaction patterns

/**
 * @brief Create a NEP-17 transfer transaction
 * 
 * @param token_hash Token contract hash
 * @param from_account From account
 * @param to_address To address
 * @param amount Transfer amount
 * @param data Additional data (can be NULL)
 * @param data_size Data size
 * @param builder Output transaction builder
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_create_nep17_transfer(const neoc_hash160_t *token_hash,
                                                    const neoc_account_t *from_account,
                                                    const char *to_address,
                                                    uint64_t amount,
                                                    const uint8_t *data,
                                                    size_t data_size,
                                                    neoc_tx_builder_t **builder);

/**
 * @brief Create a contract invocation transaction
 * 
 * @param contract_hash Contract hash
 * @param method Method name
 * @param params Parameters (JSON array string)
 * @param signers Array of signers
 * @param signer_count Number of signers
 * @param builder Output transaction builder
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_builder_create_contract_call(const neoc_hash160_t *contract_hash,
                                                   const char *method,
                                                   const char *params,
                                                   neoc_signer_t **signers,
                                                   size_t signer_count,
                                                   neoc_tx_builder_t **builder);

#ifdef __cplusplus
}
#endif

#endif // NEOC_TRANSACTION_BUILDER_H
