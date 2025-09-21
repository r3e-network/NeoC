/**
 * @file deployment.h
 * @brief Smart contract deployment utilities for Neo
 */

#ifndef NEOC_DEPLOYMENT_H
#define NEOC_DEPLOYMENT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/contract/nef_file.h"
#include "neoc/contract/contract_manifest.h"
#include "neoc/transaction/transaction.h"
#include "neoc/protocol/rpc_client.h"
#include "neoc/wallet/account.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Contract deployment parameters
 */
typedef struct {
    neoc_nef_file_t *nef;               /**< NEF file */
    neoc_contract_manifest_t *manifest;  /**< Contract manifest */
    uint8_t *data;                       /**< Optional deployment data */
    size_t data_size;                    /**< Size of deployment data */
} neoc_deployment_params_t;

/**
 * @brief Contract update parameters
 */
typedef struct {
    neoc_hash160_t contract_hash;       /**< Contract to update */
    neoc_nef_file_t *nef;               /**< New NEF file (optional) */
    neoc_contract_manifest_t *manifest;  /**< New manifest (optional) */
    uint8_t *data;                       /**< Optional update data */
    size_t data_size;                    /**< Size of update data */
} neoc_update_params_t;

/**
 * @brief Deploy a smart contract
 * 
 * @param client RPC client
 * @param params Deployment parameters
 * @param account Deployer account
 * @param tx Output transaction
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_deploy(neoc_rpc_client_t *client,
                                   const neoc_deployment_params_t *params,
                                   neoc_account_t *account,
                                   neoc_transaction_t **tx);

/**
 * @brief Update a smart contract
 * 
 * @param client RPC client
 * @param params Update parameters
 * @param account Contract owner account
 * @param tx Output transaction
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_update(neoc_rpc_client_t *client,
                                   const neoc_update_params_t *params,
                                   neoc_account_t *account,
                                   neoc_transaction_t **tx);

/**
 * @brief Destroy a smart contract
 * 
 * @param client RPC client
 * @param contract_hash Contract to destroy
 * @param account Contract owner account
 * @param tx Output transaction
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_destroy(neoc_rpc_client_t *client,
                                    const neoc_hash160_t *contract_hash,
                                    neoc_account_t *account,
                                    neoc_transaction_t **tx);

/**
 * @brief Calculate deployment costs
 * 
 * @param nef NEF file
 * @param manifest Contract manifest
 * @param storage_fee Output storage fee
 * @param cpu_fee Output CPU fee
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_calculate_deploy_cost(const neoc_nef_file_t *nef,
                                                  const neoc_contract_manifest_t *manifest,
                                                  uint64_t *storage_fee,
                                                  uint64_t *cpu_fee);

/**
 * @brief Build deployment script
 * 
 * @param params Deployment parameters
 * @param script Output script buffer
 * @param script_size Size of script buffer
 * @param actual_size Actual script size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_build_deploy_script(const neoc_deployment_params_t *params,
                                                uint8_t *script,
                                                size_t script_size,
                                                size_t *actual_size);

/**
 * @brief Build update script
 * 
 * @param params Update parameters
 * @param script Output script buffer
 * @param script_size Size of script buffer
 * @param actual_size Actual script size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_build_update_script(const neoc_update_params_t *params,
                                                uint8_t *script,
                                                size_t script_size,
                                                size_t *actual_size);

/**
 * @brief Verify contract deployment
 * 
 * @param client RPC client
 * @param contract_hash Contract hash to verify
 * @param expected_nef Expected NEF
 * @param expected_manifest Expected manifest
 * @return NEOC_SUCCESS if deployment matches, error code otherwise
 */
neoc_error_t neoc_contract_verify_deployment(neoc_rpc_client_t *client,
                                              const neoc_hash160_t *contract_hash,
                                              const neoc_nef_file_t *expected_nef,
                                              const neoc_contract_manifest_t *expected_manifest);

/**
 * @brief Free deployment parameters
 * 
 * @param params Deployment parameters to free
 */
void neoc_deployment_params_free(neoc_deployment_params_t *params);

/**
 * @brief Free update parameters
 * 
 * @param params Update parameters to free
 */
void neoc_update_params_free(neoc_update_params_t *params);

#ifdef __cplusplus
}
#endif

#endif // NEOC_DEPLOYMENT_H
