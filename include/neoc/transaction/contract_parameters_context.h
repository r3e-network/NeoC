/**
 * @file contract_parameters_context.h
 * @brief Contract parameters context for multi-signature transactions
 */

#ifndef NEOC_CONTRACT_PARAMETERS_CONTEXT_H_GUARD
#define NEOC_CONTRACT_PARAMETERS_CONTEXT_H_GUARD

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/transaction/witness.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct neoc_transaction neoc_transaction_t;

/**
 * Context item for contract parameters
 */
typedef struct {
    char *script;                           ///< Verification script
    neoc_contract_parameter_t **parameters; ///< Contract parameters
    size_t parameter_count;                 ///< Number of parameters
    char **signatures;                      ///< Signature strings
    size_t signature_count;                 ///< Number of signatures
} neoc_context_item_t;

/**
 * Contract parameters context structure
 * Used for collecting signatures for multi-signature transactions
 */
typedef struct {
    neoc_transaction_t *transaction;         ///< The transaction being signed
    neoc_hash160_t *script_hashes;           ///< Script hashes array
    size_t script_hash_count;                ///< Number of script hashes
    neoc_contract_parameter_t ***parameters; ///< 2D array of contract parameters
    size_t *parameter_counts;                ///< Array of parameter counts per script hash
    bool *completed;                         ///< Array of completion status per script hash
    int network;                             ///< Network ID
} neoc_contract_parameters_context_t;

/**
 * Create contract parameters context
 * @param transaction The transaction to create context for
 * @param context Output context (caller must free with neoc_contract_parameters_context_free)
 * @return Error code
 */
neoc_error_t neoc_contract_parameters_context_create(neoc_transaction_t *transaction,
                                                      neoc_contract_parameters_context_t **context);

/**
 * Free contract parameters context
 * @param context Context to free
 */
void neoc_contract_parameters_context_free(neoc_contract_parameters_context_t *context);

/**
 * Add signature to context
 * @param ctx The context
 * @param script_hash The script hash
 * @param signature The signature
 * @return Error code
 */
neoc_error_t neoc_contract_parameters_context_add_signature(neoc_contract_parameters_context_t *ctx,
                                                             const neoc_hash160_t *script_hash,
                                                             const char *signature);

/**
 * Check if context is complete (all required signatures collected)
 * @param context The context
 * @param is_complete Output completion status
 * @return Error code
 */
neoc_error_t neoc_contract_parameters_context_is_complete(const neoc_contract_parameters_context_t *context,
                                                           bool *is_complete);

/**
 * Get witnesses from context
 * @param context The context
 * @param witnesses Output witnesses array (caller must free)
 * @param count Output number of witnesses
 * @return Error code
 */
neoc_error_t neoc_contract_parameters_context_get_witnesses(const neoc_contract_parameters_context_t *context,
                                                             neoc_witness_t ***witnesses,
                                                             size_t *count);

/**
 * Convert context to JSON string
 * @param context The context
 * @param json_str Output JSON string (caller must free)
 * @return Error code
 */
neoc_error_t neoc_contract_parameters_context_to_json(const neoc_contract_parameters_context_t *context,
                                                       char **json_str);

/**
 * Create context from JSON string
 * @param json_str The JSON string
 * @param context Output context (caller must free)
 * @return Error code
 */
neoc_error_t neoc_contract_parameters_context_from_json(const char *json_str,
                                                         neoc_contract_parameters_context_t **context);

/**
 * Create context item
 * @param script Verification script
 * @param item Output context item (caller must free)
 * @return Error code
 */
neoc_error_t neoc_context_item_create(const char *script, neoc_context_item_t **item);

/**
 * Free context item
 * @param item Item to free
 */
void neoc_context_item_free(neoc_context_item_t *item);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_PARAMETERS_CONTEXT_H_GUARD
