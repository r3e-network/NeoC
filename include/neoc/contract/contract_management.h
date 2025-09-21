/**
 * @file contract_management.h
 * @brief Contract management interface
 */

#ifndef NEOC_CONTRACT_MANAGEMENT_H
#define NEOC_CONTRACT_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/protocol/contract_response_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations - actual definitions in protocol/contract_response_types.h
typedef struct neoc_contract_nef neoc_contract_nef_t;
typedef struct neoc_contract_manifest neoc_contract_manifest_t;
typedef struct neoc_contract_state neoc_contract_state_t;

/**
 * Contract management native contract
 */
typedef struct neoc_contract_management neoc_contract_management_t;

/**
 * Create contract management instance
 */
neoc_error_t neoc_contract_management_create(neoc_contract_management_t **mgmt);

/**
 * Deploy a contract
 */
neoc_error_t neoc_contract_management_deploy(neoc_contract_management_t *mgmt,
                                               const neoc_contract_nef_t *nef,
                                               const neoc_contract_manifest_t *manifest,
                                               neoc_contract_state_t **contract);

/**
 * Update a contract
 */
neoc_error_t neoc_contract_management_update(neoc_contract_management_t *mgmt,
                                              const neoc_hash160_t *hash,
                                              const neoc_contract_nef_t *nef,
                                              const neoc_contract_manifest_t *manifest);

/**
 * Destroy a contract
 */
neoc_error_t neoc_contract_management_destroy(neoc_contract_management_t *mgmt,
                                               const neoc_hash160_t *hash);

/**
 * Get contract by hash
 */
neoc_error_t neoc_contract_management_get_contract(neoc_contract_management_t *mgmt,
                                                    const neoc_hash160_t *hash,
                                                    neoc_contract_state_t **contract);

/**
 * Check if contract exists
 */
neoc_error_t neoc_contract_management_has_contract(neoc_contract_management_t *mgmt,
                                                    const neoc_hash160_t *hash,
                                                    bool *exists);

/**
 * Free contract state
 */
void neoc_contract_state_free(neoc_contract_state_t *contract);

/**
 * Free contract management
 */
void neoc_contract_management_free(neoc_contract_management_t *mgmt);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_MANAGEMENT_H
