/**
 * @file transaction_signer.h
 * @brief Transaction signer configuration for Neo transactions
 * 
 * Based on Swift source: protocol/core/response/TransactionSigner.swift
 * Represents a transaction signer with witness scopes and permissions
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_SIGNER_H
#define NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_SIGNER_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include "neoc/transaction/witness_scope.h"
#include "neoc/protocol/core/witnessrule/witness_rule.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transaction signer configuration
 * 
 * Defines how a transaction should be signed with witness scope restrictions
 */
typedef struct {
    neoc_hash160_t *account;            /**< Signing account hash */
    neoc_witness_scope_t *scopes;       /**< Array of witness scopes */
    size_t scopes_count;                /**< Number of witness scopes */
    char **allowed_contracts;           /**< Array of allowed contract hashes (nullable) */
    size_t allowed_contracts_count;     /**< Number of allowed contracts */
    char **allowed_groups;              /**< Array of allowed group public keys (nullable) */
    size_t allowed_groups_count;        /**< Number of allowed groups */
    neoc_witness_rule_t **rules;        /**< Array of witness rules (nullable) */
    size_t rules_count;                 /**< Number of witness rules */
} neoc_transaction_signer_t;

/**
 * @brief Create a new transaction signer
 * 
 * @param signer Pointer to store the created signer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_create(
    neoc_transaction_signer_t **signer
);

/**
 * @brief Free a transaction signer and its resources
 * 
 * @param signer Signer to free
 */
void neoc_transaction_signer_free(
    neoc_transaction_signer_t *signer
);

/**
 * @brief Create a transaction signer with basic scope
 * 
 * @param account Account hash to sign with
 * @param scopes Array of witness scopes
 * @param scopes_count Number of witness scopes
 * @param signer Pointer to store the created signer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_create_with_scopes(
    const neoc_hash160_t *account,
    const neoc_witness_scope_t *scopes,
    size_t scopes_count,
    neoc_transaction_signer_t **signer
);

/**
 * @brief Create a transaction signer with full configuration
 * 
 * @param account Account hash to sign with
 * @param scopes Array of witness scopes
 * @param scopes_count Number of witness scopes
 * @param allowed_contracts Array of allowed contract hashes (can be NULL)
 * @param allowed_contracts_count Number of allowed contracts
 * @param allowed_groups Array of allowed group public keys (can be NULL)
 * @param allowed_groups_count Number of allowed groups
 * @param rules Array of witness rules (can be NULL)
 * @param rules_count Number of witness rules
 * @param signer Pointer to store the created signer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_create_full(
    const neoc_hash160_t *account,
    const neoc_witness_scope_t *scopes,
    size_t scopes_count,
    const char **allowed_contracts,
    size_t allowed_contracts_count,
    const char **allowed_groups,
    size_t allowed_groups_count,
    const neoc_witness_rule_t **rules,
    size_t rules_count,
    neoc_transaction_signer_t **signer
);

/**
 * @brief Add a witness scope to the signer
 * 
 * @param signer Signer to modify
 * @param scope Witness scope to add
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_add_scope(
    neoc_transaction_signer_t *signer,
    neoc_witness_scope_t scope
);

/**
 * @brief Add an allowed contract to the signer
 * 
 * @param signer Signer to modify
 * @param contract_hash Contract hash to add
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_add_allowed_contract(
    neoc_transaction_signer_t *signer,
    const char *contract_hash
);

/**
 * @brief Add an allowed group to the signer
 * 
 * @param signer Signer to modify
 * @param group_pubkey Group public key to add
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_add_allowed_group(
    neoc_transaction_signer_t *signer,
    const char *group_pubkey
);

/**
 * @brief Add a witness rule to the signer
 * 
 * @param signer Signer to modify
 * @param rule Witness rule to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_add_rule(
    neoc_transaction_signer_t *signer,
    neoc_witness_rule_t *rule
);

/**
 * @brief Parse JSON into transaction signer
 * 
 * @param json_str JSON string to parse
 * @param signer Pointer to store the parsed signer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_from_json(
    const char *json_str,
    neoc_transaction_signer_t **signer
);

/**
 * @brief Convert transaction signer to JSON string
 * 
 * @param signer Signer to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_transaction_signer_to_json(
    const neoc_transaction_signer_t *signer,
    char **json_str
);

/**
 * @brief Check if signer has a specific scope
 * 
 * @param signer Signer to check
 * @param scope Scope to look for
 * @return true if scope is present, false otherwise
 */
bool neoc_transaction_signer_has_scope(
    const neoc_transaction_signer_t *signer,
    neoc_witness_scope_t scope
);

/**
 * @brief Check if contract is allowed by signer
 * 
 * @param signer Signer to check
 * @param contract_hash Contract hash to check
 * @return true if contract is allowed, false otherwise
 */
bool neoc_transaction_signer_is_contract_allowed(
    const neoc_transaction_signer_t *signer,
    const char *contract_hash
);

/**
 * @brief Check if group is allowed by signer
 * 
 * @param signer Signer to check
 * @param group_pubkey Group public key to check
 * @return true if group is allowed, false otherwise
 */
bool neoc_transaction_signer_is_group_allowed(
    const neoc_transaction_signer_t *signer,
    const char *group_pubkey
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_TRANSACTION_SIGNER_H */
