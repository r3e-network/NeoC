/**
 * @file contract_signer.h
 * @brief Contract signer for smart contract verification
 */

#ifndef NEOC_CONTRACT_SIGNER_H
#define NEOC_CONTRACT_SIGNER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness_scope.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/contract_parameter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contract signer structure
 * Represents a smart contract for transaction verification
 */
typedef struct neoc_contract_signer {
    neoc_signer_t base;                       // Base signer
    neoc_contract_parameter_t **verify_params; // Parameters for verify() method
    size_t verify_params_count;               // Number of verify parameters
} neoc_contract_signer_t;

/**
 * Create a contract signer with called-by-entry scope
 * Only allows the entry point contract to use this signer's witness
 * @param contract_hash The script hash of the contract
 * @param verify_params Array of parameters for verify() method
 * @param params_count Number of verify parameters
 * @param signer Output contract signer
 * @return Error code
 */
neoc_error_t neoc_contract_signer_called_by_entry(neoc_hash160_t *contract_hash,
                                                   neoc_contract_parameter_t **verify_params,
                                                   size_t params_count,
                                                   neoc_contract_signer_t **signer);

/**
 * Create a contract signer with global witness scope
 * @param contract_hash The script hash of the contract
 * @param verify_params Array of parameters for verify() method
 * @param params_count Number of verify parameters
 * @param signer Output contract signer
 * @return Error code
 */
neoc_error_t neoc_contract_signer_global(neoc_hash160_t *contract_hash,
                                          neoc_contract_parameter_t **verify_params,
                                          size_t params_count,
                                          neoc_contract_signer_t **signer);

/**
 * Create a contract signer with no witness scope
 * @param contract_hash The script hash of the contract
 * @param verify_params Array of parameters for verify() method
 * @param params_count Number of verify parameters
 * @param signer Output contract signer
 * @return Error code
 */
neoc_error_t neoc_contract_signer_none(neoc_hash160_t *contract_hash,
                                        neoc_contract_parameter_t **verify_params,
                                        size_t params_count,
                                        neoc_contract_signer_t **signer);

/**
 * Get verify parameters from a contract signer
 * @param signer The contract signer
 * @param params Output parameters array
 * @param count Output parameter count
 * @return Error code
 */
neoc_error_t neoc_contract_signer_get_verify_params(neoc_contract_signer_t *signer,
                                                     neoc_contract_parameter_t ***params,
                                                     size_t *count);

/**
 * Add verify parameters to a contract signer
 * @param signer The contract signer
 * @param params Parameters to add
 * @param count Number of parameters to add
 * @return Error code
 */
neoc_error_t neoc_contract_signer_add_verify_params(neoc_contract_signer_t *signer,
                                                     neoc_contract_parameter_t **params,
                                                     size_t count);

/**
 * Free a contract signer
 * @param signer The signer to free
 */
void neoc_contract_signer_free(neoc_contract_signer_t *signer);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_SIGNER_H
