/**
 * @file transaction.h
 * @brief Transaction response structure for Neo RPC calls
 * 
 * This file implements the Transaction response structure based on the Swift source:
 * protocol/core/response/Transaction.swift
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_H
#define NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness.h"
#include "neoc/protocol/response/transaction_attribute.h"
#include "neoc/types/neoc_vm_state_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transaction response structure from Neo RPC calls
 * 
 * This structure represents a transaction as returned by Neo RPC calls,
 * including all relevant information about the transaction and its execution state.
 */
typedef struct neoc_transaction_response {
    neoc_hash256_t hash;                /**< Transaction hash */
    uint32_t size;                      /**< Transaction size in bytes */
    uint32_t version;                   /**< Transaction version */
    uint32_t nonce;                     /**< Random nonce */
    char *sender;                       /**< Sender address */
    char *sys_fee;                      /**< System fee as string */
    char *net_fee;                      /**< Network fee as string */
    uint32_t valid_until_block;         /**< Block height when transaction expires */
    neoc_signer_t **signers;            /**< Transaction signers */
    size_t signers_count;               /**< Number of signers */
    neoc_transaction_attribute_t **attributes; /**< Transaction attributes */
    size_t attributes_count;            /**< Number of attributes */
    char *script;                       /**< Transaction script as hex string */
    neoc_witness_t **witnesses;         /**< Transaction witnesses */
    size_t witnesses_count;             /**< Number of witnesses */
    neoc_hash256_t *block_hash;         /**< Block hash (optional) */
    uint32_t *confirmations;            /**< Number of confirmations (optional) */
    uint64_t *block_time;               /**< Block time (optional) */
    neoc_vm_state_t *vm_state;          /**< VM execution state (optional) */
} neoc_transaction_response_t;

/**
 * @brief Create a new transaction response structure
 * 
 * @param response Output transaction response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_create(neoc_transaction_response_t **response);

/**
 * @brief Free a transaction response structure
 * 
 * @param response The transaction response to free
 */
void neoc_transaction_response_free(neoc_transaction_response_t *response);

/**
 * @brief Initialize a transaction response with parameters
 * 
 * @param response The transaction response to initialize
 * @param hash Transaction hash
 * @param size Transaction size
 * @param version Transaction version
 * @param nonce Random nonce
 * @param sender Sender address
 * @param sys_fee System fee string
 * @param net_fee Network fee string
 * @param valid_until_block Expiry block height
 * @param signers Array of signers
 * @param signers_count Number of signers
 * @param attributes Array of attributes
 * @param attributes_count Number of attributes
 * @param script Transaction script
 * @param witnesses Array of witnesses
 * @param witnesses_count Number of witnesses
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_init(neoc_transaction_response_t *response,
                                           const neoc_hash256_t *hash,
                                           uint32_t size,
                                           uint32_t version,
                                           uint32_t nonce,
                                           const char *sender,
                                           const char *sys_fee,
                                           const char *net_fee,
                                           uint32_t valid_until_block,
                                           neoc_signer_t **signers,
                                           size_t signers_count,
                                           neoc_transaction_attribute_t **attributes,
                                           size_t attributes_count,
                                           const char *script,
                                           neoc_witness_t **witnesses,
                                           size_t witnesses_count);

/**
 * @brief Set optional block hash for transaction response
 * 
 * @param response The transaction response
 * @param block_hash Block hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_set_block_hash(neoc_transaction_response_t *response,
                                                     const neoc_hash256_t *block_hash);

/**
 * @brief Set optional confirmations for transaction response
 * 
 * @param response The transaction response
 * @param confirmations Number of confirmations
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_set_confirmations(neoc_transaction_response_t *response,
                                                        uint32_t confirmations);

/**
 * @brief Set optional block time for transaction response
 * 
 * @param response The transaction response
 * @param block_time Block time
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_set_block_time(neoc_transaction_response_t *response,
                                                     uint64_t block_time);

/**
 * @brief Set optional VM state for transaction response
 * 
 * @param response The transaction response
 * @param vm_state VM execution state
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_set_vm_state(neoc_transaction_response_t *response,
                                                   neoc_vm_state_t vm_state);

/**
 * @brief Clone a transaction response structure
 * 
 * @param source Source transaction response
 * @param dest Output destination transaction response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_clone(const neoc_transaction_response_t *source,
                                            neoc_transaction_response_t **dest);

/**
 * @brief Parse transaction response from JSON string
 * 
 * @param json_str JSON string containing transaction data
 * @param response Output transaction response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_from_json(const char *json_str,
                                                neoc_transaction_response_t **response);

/**
 * @brief Convert transaction response to JSON string
 * 
 * @param response The transaction response
 * @param json_str Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_response_to_json(const neoc_transaction_response_t *response,
                                              char **json_str);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_H */
