/**
 * @file neo_transaction.h
 * @brief NEO blockchain transaction structure and operations
 */

#ifndef NEOC_NEO_TRANSACTION_H
#define NEOC_NEO_TRANSACTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/protocol/response/transaction_attribute.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEOC_TRANSACTION_HEADER_SIZE 25
#define NEOC_MAX_TRANSACTION_SIZE 102400

/**
 * NEO transaction structure
 */
typedef struct neoc_transaction {
    uint8_t version;                           // Transaction version
    uint32_t nonce;                            // Random number
    uint32_t valid_until_block;                // Block height when transaction expires
    neoc_signer_t **signers;                   // Transaction signers
    size_t signers_count;                      // Number of signers
    int64_t system_fee;                        // System fee
    int64_t network_fee;                       // Network fee
    neoc_transaction_attribute_t **attributes; // Transaction attributes
    size_t attributes_count;                   // Number of attributes
    uint8_t *script;                            // Transaction script
    size_t script_len;                          // Script length
    neoc_witness_t **witnesses;                // Transaction witnesses
    size_t witnesses_count;                    // Number of witnesses
    uint32_t block_count_when_sent;            // Block count when sent
    bool sent;                                 // Whether transaction has been sent
} neoc_transaction_t;

/**
 * Create a new transaction
 * @param transaction Output transaction
 * @return Error code
 */
neoc_error_t neoc_transaction_create(neoc_transaction_t **transaction);

/**
 * Initialize transaction with parameters
 * @param transaction The transaction
 * @param version Transaction version
 * @param nonce Random number
 * @param valid_until_block Expiry block height
 * @param signers Array of signers
 * @param signers_count Number of signers
 * @param system_fee System fee
 * @param network_fee Network fee
 * @param script Transaction script
 * @param script_len Script length
 * @return Error code
 */
neoc_error_t neoc_transaction_init(neoc_transaction_t *transaction,
                                    uint8_t version,
                                    uint32_t nonce,
                                    uint32_t valid_until_block,
                                    neoc_signer_t **signers,
                                    size_t signers_count,
                                    int64_t system_fee,
                                    int64_t network_fee,
                                    uint8_t *script,
                                    size_t script_len);

/**
 * Get transaction ID (hash)
 * @param transaction The transaction
 * @param tx_id Output transaction ID
 * @return Error code
 */
neoc_error_t neoc_transaction_get_id(neoc_transaction_t *transaction,
                                      neoc_hash256_t **tx_id);

/**
 * Get transaction sender
 * @param transaction The transaction
 * @param sender Output sender hash
 * @return Error code
 */
neoc_error_t neoc_transaction_get_sender(neoc_transaction_t *transaction,
                                          neoc_hash160_t **sender);

/**
 * Get transaction signers
 * @param transaction The transaction
 * @param signers Output signers array (pointer to internal data, do not free)
 * @param count Output number of signers
 * @return Error code
 */
neoc_error_t neoc_transaction_get_signers(neoc_transaction_t *transaction,
                                           neoc_signer_t ***signers,
                                           size_t *count);

/**
 * Add witness to transaction
 * @param transaction The transaction
 * @param witness Witness to add
 * @return Error code
 */
neoc_error_t neoc_transaction_add_witness(neoc_transaction_t *transaction,
                                           neoc_witness_t *witness);

/**
 * Add multi-sig witness to transaction
 * @param transaction The transaction
 * @param verification_script Multi-sig verification script
 * @param signatures Array of signatures
 * @param sig_count Number of signatures
 * @return Error code
 */
neoc_error_t neoc_transaction_add_multisig_witness(neoc_transaction_t *transaction,
                                                    uint8_t *verification_script,
                                                    size_t script_len,
                                                    uint8_t **signatures,
                                                    size_t sig_count);

/**
 * Add attribute to transaction
 * @param transaction The transaction
 * @param attribute Attribute to add
 * @return Error code
 */
neoc_error_t neoc_transaction_add_attribute(neoc_transaction_t *transaction,
                                             neoc_transaction_attribute_t *attribute);

/**
 * Calculate transaction size
 * @param transaction The transaction
 * @return Transaction size in bytes
 */
size_t neoc_transaction_get_size(neoc_transaction_t *transaction);

/**
 * Get hash data for signing
 * @param transaction The transaction
 * @param network_magic Network magic number
 * @param hash_data Output hash data
 * @param data_len Output data length
 * @return Error code
 */
neoc_error_t neoc_transaction_get_hash_data(neoc_transaction_t *transaction,
                                             uint32_t network_magic,
                                             uint8_t **hash_data,
                                             size_t *data_len);

/**
 * Serialize transaction without witnesses
 * @param transaction The transaction
 * @param data Output serialized data
 * @param data_len Output data length
 * @return Error code
 */
neoc_error_t neoc_transaction_serialize_without_witnesses(neoc_transaction_t *transaction,
                                                           uint8_t **data,
                                                           size_t *data_len);

/**
 * Serialize complete transaction
 * @param transaction The transaction
 * @param data Output serialized data
 * @param data_len Output data length
 * @return Error code
 */
neoc_error_t neoc_transaction_serialize(neoc_transaction_t *transaction,
                                         uint8_t **data,
                                         size_t *data_len);

/**
 * Deserialize transaction from bytes
 * @param data Serialized transaction data
 * @param data_len Data length
 * @param transaction Output transaction
 * @return Error code
 */
neoc_error_t neoc_transaction_deserialize(uint8_t *data,
                                           size_t data_len,
                                           neoc_transaction_t **transaction);

/**
 * Validate transaction
 * @param transaction The transaction
 * @return Error code (NEOC_SUCCESS if valid)
 */
neoc_error_t neoc_transaction_validate(neoc_transaction_t *transaction);

/**
 * Free transaction
 * @param transaction The transaction to free
 */
void neoc_transaction_free(neoc_transaction_t *transaction);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_TRANSACTION_H
