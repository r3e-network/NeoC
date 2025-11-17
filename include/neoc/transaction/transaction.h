#ifndef NEOC_TRANSACTION_H
#define NEOC_TRANSACTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness.h"
#include "neoc/wallet/account.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transaction attribute types
 */
typedef enum {
    NEOC_TX_ATTR_HIGH_PRIORITY = 0x01,
    NEOC_TX_ATTR_ORACLE_RESPONSE = 0x11,
    NEOC_TX_ATTR_NOT_VALID_BEFORE = 0x20,
    NEOC_TX_ATTR_CONFLICTS = 0x21
} neoc_tx_attribute_type_t;

/**
 * @brief Transaction attribute structure
 */
typedef struct {
    neoc_tx_attribute_type_t type;
    uint8_t *data;
    size_t data_len;
} neoc_tx_attribute_t;

/**
 * @brief NEO transaction structure
 */
typedef struct {
    uint8_t version;                    // Transaction version
    uint32_t nonce;                      // Random nonce
    uint64_t system_fee;                 // System fee in GAS
    uint64_t network_fee;                // Network fee in GAS
    uint32_t valid_until_block;          // Valid until block height
    neoc_signer_t **signers;             // Transaction signers
    size_t signer_count;                 // Number of signers
    neoc_tx_attribute_t **attributes;   // Transaction attributes
    size_t attribute_count;              // Number of attributes
    uint8_t *script;                     // Transaction script
    size_t script_len;                   // Script length
    neoc_witness_t **witnesses;          // Transaction witnesses
    size_t witness_count;                // Number of witnesses
    neoc_hash256_t hash;                 // Transaction hash (computed)
} neoc_transaction_t;

/**
 * @brief Create a new empty transaction
 * 
 * @param transaction Output transaction (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_create(neoc_transaction_t **transaction);

/**
 * @brief Set transaction version
 * 
 * @param transaction The transaction
 * @param version Version number
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_set_version(neoc_transaction_t *transaction, uint8_t version);

/**
 * @brief Set transaction nonce
 * 
 * @param transaction The transaction
 * @param nonce Nonce value
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_set_nonce(neoc_transaction_t *transaction, uint32_t nonce);

/**
 * @brief Set system fee
 * 
 * @param transaction The transaction
 * @param fee System fee in GAS (smallest unit)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_set_system_fee(neoc_transaction_t *transaction, uint64_t fee);

/**
 * @brief Set network fee
 * 
 * @param transaction The transaction
 * @param fee Network fee in GAS (smallest unit)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_set_network_fee(neoc_transaction_t *transaction, uint64_t fee);

/**
 * @brief Set valid until block
 * 
 * @param transaction The transaction
 * @param block Valid until block height
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_set_valid_until_block(neoc_transaction_t *transaction, uint32_t block);

/**
 * @brief Set transaction script
 * 
 * @param transaction The transaction
 * @param script Script bytes
 * @param script_len Script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_set_script(neoc_transaction_t *transaction,
                                          const uint8_t *script,
                                          size_t script_len);
neoc_error_t neoc_transaction_get_script(const neoc_transaction_t *transaction,
                                         uint8_t **script,
                                         size_t *script_len);
const uint8_t* neoc_transaction_get_script_ptr(const neoc_transaction_t *transaction,
                                               size_t *script_len);

/**
 * @brief Add a signer to the transaction
 * 
 * @param transaction The transaction
 * @param signer The signer to add (transaction takes ownership)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_add_signer(neoc_transaction_t *transaction,
                                          neoc_signer_t *signer);

/**
 * @brief Add an attribute to the transaction
 * 
 * @param transaction The transaction
 * @param attribute The attribute to add (transaction takes ownership)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_add_attribute(neoc_transaction_t *transaction,
                                             neoc_tx_attribute_t *attribute);

/**
 * @brief Add a witness to the transaction
 * 
 * @param transaction The transaction
 * @param witness The witness to add (transaction takes ownership)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_add_witness(neoc_transaction_t *transaction,
                                           neoc_witness_t *witness);

/**
 * @brief Calculate transaction hash
 * 
 * @param transaction The transaction
 * @param hash Output hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_calculate_hash(neoc_transaction_t *transaction,
                                              neoc_hash256_t *hash);

/**
 * @brief Get transaction hash
 * 
 * @param transaction The transaction
 * @param hash Output hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_get_hash(const neoc_transaction_t *transaction,
                                        neoc_hash256_t *hash);

/**
 * @brief Sign transaction with account
 * 
 * @param transaction The transaction to sign
 * @param account The account to sign with
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_sign(neoc_transaction_t *transaction,
                                    neoc_account_t *account);

/**
 * @brief Serialize transaction to bytes
 * 
 * @param transaction The transaction
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @param serialized_size Output serialized size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_serialize(const neoc_transaction_t *transaction,
                                         uint8_t *buffer,
                                         size_t buffer_size,
                                         size_t *serialized_size);

/**
 * @brief Deserialize transaction from bytes
 * 
 * @param bytes Transaction bytes
 * @param bytes_len Length of bytes
 * @param transaction Output transaction (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_deserialize(const uint8_t *bytes,
                                           size_t bytes_len,
                                           neoc_transaction_t **transaction);

/**
 * @brief Sign transaction with multiple accounts
 * 
 * @param transaction The transaction to sign
 * @param accounts Array of accounts
 * @param account_count Number of accounts
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_transaction_sign_multi(neoc_transaction_t *transaction,
                                          const neoc_account_t **accounts,
                                          size_t account_count);

/**
 * @brief Get transaction size in bytes
 * 
 * @param transaction The transaction
 * @return Size in bytes
 */
size_t neoc_transaction_get_size(const neoc_transaction_t *transaction);

/**
 * @brief Verify transaction signatures
 * 
 * @param transaction The transaction
 * @return true if all signatures are valid, false otherwise
 */
bool neoc_transaction_verify(const neoc_transaction_t *transaction);

/**
 * @brief Free a transaction
 * 
 * @param transaction The transaction to free
 */
void neoc_transaction_free(neoc_transaction_t *transaction);

neoc_error_t neoc_transaction_clone(const neoc_transaction_t *source,
                                     neoc_transaction_t **clone);

neoc_transaction_t* neoc_transaction_from_json(const char* json_str);

char* neoc_transaction_to_json(const neoc_transaction_t* tx);

neoc_transaction_t* neoc_transaction_deserialize_simple(const uint8_t *bytes,
                                                        size_t bytes_len,
                                                        size_t *consumed);

/**
 * @brief Create a transaction attribute
 * 
 * @param type Attribute type
 * @param data Attribute data
 * @param data_len Data length
 * @param attribute Output attribute (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_tx_attribute_create(neoc_tx_attribute_type_t type,
                                       const uint8_t *data,
                                       size_t data_len,
                                       neoc_tx_attribute_t **attribute);

/**
 * @brief Free a transaction attribute
 * 
 * @param attribute The attribute to free
 */
void neoc_tx_attribute_free(neoc_tx_attribute_t *attribute);

#ifdef __cplusplus
}
#endif

#endif // NEOC_TRANSACTION_H
