/**
 * @file native_contract.h
 * @brief Base native contract interface for Neo blockchain
 */

#ifndef NEOC_NATIVE_CONTRACT_H
#define NEOC_NATIVE_CONTRACT_H

#include <stdint.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Native contract IDs
 */
typedef enum {
    NATIVE_CONTRACT_MANAGEMENT = -1,
    NATIVE_CONTRACT_LEDGER = -2,
    NATIVE_CONTRACT_NEO = -5,
    NATIVE_CONTRACT_GAS = -6,
    NATIVE_CONTRACT_POLICY = -7,
    NATIVE_CONTRACT_ROLE_MANAGEMENT = -8,
    NATIVE_CONTRACT_ORACLE = -9,
    NATIVE_CONTRACT_CRYPTO = -10,
    NATIVE_CONTRACT_STD_LIB = -11
} neoc_native_contract_id_t;

/**
 * @brief Base native contract structure
 */
typedef struct {
    int32_t contract_id;
    char name[64];
    neoc_hash160_t script_hash;
    uint32_t update_history[16];
    size_t update_count;
} neoc_native_contract_t;

/**
 * @brief Initialize native contract
 * 
 * @param contract Native contract to initialize
 * @param id Contract ID
 * @param name Contract name
 * @param script_hash_hex Script hash in hex format
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_contract_init(neoc_native_contract_t *contract,
                                        int32_t id,
                                        const char *name,
                                        const char *script_hash_hex);

/**
 * @brief Get native contract by ID
 * 
 * @param id Contract ID
 * @param contract Output contract
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_contract_get_by_id(int32_t id,
                                             neoc_native_contract_t **contract);

/**
 * @brief Get native contract by name
 * 
 * @param name Contract name
 * @param contract Output contract
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_contract_get_by_name(const char *name,
                                               neoc_native_contract_t **contract);

/**
 * @brief Get all native contracts
 * 
 * @param contracts Output contracts array (caller must free)
 * @param count Output number of contracts
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_contract_get_all(neoc_native_contract_t ***contracts,
                                           size_t *count);

/**
 * @brief Free native contract
 * 
 * @param contract Contract to free
 */
void neoc_native_contract_free(neoc_native_contract_t *contract);

/**
 * @brief Free native contracts array
 * 
 * @param contracts Contracts array
 * @param count Number of contracts
 */
void neoc_native_contracts_free(neoc_native_contract_t **contracts, size_t count);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NATIVE_CONTRACT_H
