#ifndef NEOC_SIGNER_H
#define NEOC_SIGNER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/transaction/witness_scope.h"
#include "neoc/serialization/binary_writer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transaction signer structure
 */
typedef struct {
    neoc_hash160_t account;           // Signer account script hash
    uint8_t scopes;                   // Witness scopes (combination of flags)
    neoc_hash160_t *allowed_contracts; // Allowed contracts (when CUSTOM_CONTRACTS)
    size_t allowed_contracts_count;   // Number of allowed contracts
    uint8_t **allowed_groups;         // Allowed groups (when CUSTOM_GROUPS)
    size_t *allowed_groups_sizes;     // Sizes of group public keys
    size_t allowed_groups_count;      // Number of allowed groups
    void *rules;                       // Witness rules (when WITNESS_RULES)
    size_t rules_count;                // Number of witness rules
} neoc_signer_t;

/**
 * @brief Create a new signer
 * 
 * @param account Account script hash
 * @param scopes Witness scopes
 * @param signer Output signer (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_create(const neoc_hash160_t *account,
                                 uint8_t scopes,
                                 neoc_signer_t **signer);

/**
 * @brief Create signer with global scope
 * 
 * @param account Account script hash
 * @param signer Output signer (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_create_global(const neoc_hash160_t *account,
                                        neoc_signer_t **signer);

/**
 * @brief Create signer with called-by-entry scope
 * 
 * @param account Account script hash
 * @param signer Output signer (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_create_called_by_entry(const neoc_hash160_t *account,
                                                 neoc_signer_t **signer);

/**
 * @brief Add allowed contract to signer
 * 
 * @param signer The signer
 * @param contract Contract script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_add_allowed_contract(neoc_signer_t *signer,
                                               const neoc_hash160_t *contract);

/**
 * @brief Add allowed group to signer
 * 
 * @param signer The signer
 * @param group_pubkey Group public key
 * @param pubkey_size Size of public key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_add_allowed_group(neoc_signer_t *signer,
                                            const uint8_t *group_pubkey,
                                            size_t pubkey_size);

/**
 * @brief Check if signer has global scope
 * 
 * @param signer The signer
 * @return true if has global scope
 */
bool neoc_signer_has_global_scope(const neoc_signer_t *signer);

/**
 * @brief Check if signer has called-by-entry scope
 * 
 * @param signer The signer
 * @return true if has called-by-entry scope
 */
bool neoc_signer_has_called_by_entry_scope(const neoc_signer_t *signer);

/**
 * @brief Check if signer has custom contracts scope
 * 
 * @param signer The signer
 * @return true if has custom contracts scope
 */
bool neoc_signer_has_custom_contracts_scope(const neoc_signer_t *signer);

/**
 * @brief Check if signer has custom groups scope
 * 
 * @param signer The signer
 * @return true if has custom groups scope
 */
bool neoc_signer_has_custom_groups_scope(const neoc_signer_t *signer);

/**
 * @brief Get signer size for serialization
 * 
 * @param signer The signer
 * @return Size in bytes
 */
size_t neoc_signer_get_size(const neoc_signer_t *signer);

/**
 * @brief Copy a signer
 * 
 * @param source The source signer
 * @param dest Output destination signer (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_copy(const neoc_signer_t *source, neoc_signer_t **dest);

/**
 * @brief Get signer account hash
 * 
 * @param signer The signer
 * @param account Output account hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_get_account(const neoc_signer_t *signer, neoc_hash160_t *account);

/**
 * @brief Serialize signer to binary writer
 * 
 * @param signer The signer
 * @param writer Binary writer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signer_serialize(const neoc_signer_t *signer, neoc_binary_writer_t *writer);

/**
 * @brief Free a signer
 * 
 * @param signer The signer to free
 */
void neoc_signer_free(neoc_signer_t *signer);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SIGNER_H
