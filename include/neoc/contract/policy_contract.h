/**
 * @file policy_contract.h
 * @brief Complete implementation for policy_contract
 */

#ifndef NEOC_policy_contract_H_GUARD
#define NEOC_policy_contract_H_GUARD

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Policy contract opaque type
 */
typedef struct neoc_policy_contract neoc_policy_contract_t;

/**
 * @brief Create policy contract instance
 * 
 * @param policy Output policy contract (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_policy_contract_create(neoc_policy_contract_t **policy);

/**
 * @brief Get fee per byte
 * 
 * @param policy Policy contract instance
 * @param fee Output fee per byte
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_policy_get_fee_per_byte(neoc_policy_contract_t *policy,
                                           uint64_t *fee);

/**
 * @brief Get execution fee factor
 * 
 * @param policy Policy contract instance
 * @param factor Output execution fee factor
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_policy_get_exec_fee_factor(neoc_policy_contract_t *policy,
                                              uint32_t *factor);

/**
 * @brief Get storage price
 * 
 * @param policy Policy contract instance
 * @param price Output storage price
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_policy_get_storage_price(neoc_policy_contract_t *policy,
                                            uint32_t *price);

/**
 * @brief Set fee per byte
 *
 * Updates the cached fee per byte value (committee operation placeholder).
 *
 * @param policy Policy contract instance
 * @param fee New fee per byte
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_policy_set_fee_per_byte(neoc_policy_contract_t *policy,
                                           uint64_t fee);

/**
 * @brief Check if account is blocked
 * 
 * @param policy Policy contract instance
 * @param account Account to check
 * @param blocked Output blocked status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_policy_is_blocked(neoc_policy_contract_t *policy,
                                     const neoc_hash160_t *account,
                                     bool *blocked);

/**
 * @brief Free policy contract
 * 
 * @param policy Policy contract to free
 */
void neoc_policy_contract_free(neoc_policy_contract_t *policy);

#ifdef __cplusplus
}
#endif

#endif // NEOC_policy_contract_H_GUARD
