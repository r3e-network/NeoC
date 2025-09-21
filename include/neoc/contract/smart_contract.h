#ifndef NEOC_SMART_CONTRACT_H
#define NEOC_SMART_CONTRACT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/contract/contract_parameter.h"
#include "neoc/script/script_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Smart contract structure
 */
typedef struct {
    neoc_hash160_t script_hash;         // Contract script hash
    char *name;                          // Contract name (optional)
    uint8_t *nef;                        // NEF (NEO Executable Format) data
    size_t nef_len;                      // NEF data length
    char *manifest;                      // Contract manifest JSON
} neoc_smart_contract_t;

/**
 * @brief Contract invocation structure
 */
typedef struct {
    neoc_hash160_t script_hash;         // Contract to invoke
    char *operation;                     // Operation/method name
    neoc_contract_parameter_t **params; // Parameters
    size_t param_count;                  // Number of parameters
} neoc_contract_invocation_t;

/**
 * @brief Native contract identifiers
 */
typedef enum {
    NEOC_NATIVE_CONTRACT_NEO = 0,
    NEOC_NATIVE_CONTRACT_GAS,
    NEOC_NATIVE_CONTRACT_POLICY,
    NEOC_NATIVE_CONTRACT_ROLE_MANAGEMENT,
    NEOC_NATIVE_CONTRACT_ORACLE,
    NEOC_NATIVE_CONTRACT_LEDGER,
    NEOC_NATIVE_CONTRACT_MANAGEMENT,
    NEOC_NATIVE_CONTRACT_CRYPTO,
    NEOC_NATIVE_CONTRACT_STD_LIB
} neoc_native_contract_t;

/**
 * @brief Create a smart contract reference
 * 
 * @param script_hash Contract script hash
 * @param name Optional contract name
 * @param contract Output contract (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_smart_contract_create(const neoc_hash160_t *script_hash,
                                         const char *name,
                                         neoc_smart_contract_t **contract);

/**
 * @brief Create contract invocation
 * 
 * @param script_hash Contract to invoke
 * @param operation Operation/method name
 * @param params Parameters (can be NULL)
 * @param param_count Number of parameters
 * @param invocation Output invocation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_invocation_create(const neoc_hash160_t *script_hash,
                                              const char *operation,
                                              neoc_contract_parameter_t **params,
                                              size_t param_count,
                                              neoc_contract_invocation_t **invocation);

/**
 * @brief Build invocation script for contract call
 * 
 * @param invocation The invocation details
 * @param builder Script builder to append to
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_build_invocation_script(const neoc_contract_invocation_t *invocation,
                                                    neoc_script_builder_t *builder);

/**
 * @brief Get native contract script hash
 * 
 * @param contract Native contract identifier
 * @param hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_contract_get_hash(neoc_native_contract_t contract,
                                            neoc_hash160_t *hash);

/**
 * @brief Get NEO token script hash
 * 
 * @param hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_get_neo_hash(neoc_hash160_t *hash);

/**
 * @brief Get GAS token script hash
 * 
 * @param hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_get_gas_hash(neoc_hash160_t *hash);

/**
 * @brief Create NEP-17 transfer invocation
 * 
 * @param token_hash Token contract hash
 * @param from Sender address
 * @param to Receiver address
 * @param amount Amount to transfer (in smallest unit)
 * @param data Optional data parameter
 * @param invocation Output invocation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_transfer_create(const neoc_hash160_t *token_hash,
                                         const neoc_hash160_t *from,
                                         const neoc_hash160_t *to,
                                         int64_t amount,
                                         neoc_contract_parameter_t *data,
                                         neoc_contract_invocation_t **invocation);

/**
 * @brief Create NEP-17 balance query invocation
 * 
 * @param token_hash Token contract hash
 * @param account Account to query
 * @param invocation Output invocation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_balance_of_create(const neoc_hash160_t *token_hash,
                                           const neoc_hash160_t *account,
                                           neoc_contract_invocation_t **invocation);

/**
 * @brief Create NEP-17 symbol query invocation
 * 
 * @param token_hash Token contract hash
 * @param invocation Output invocation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_symbol_create(const neoc_hash160_t *token_hash,
                                       neoc_contract_invocation_t **invocation);

/**
 * @brief Create NEP-17 decimals query invocation
 * 
 * @param token_hash Token contract hash
 * @param invocation Output invocation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_decimals_create(const neoc_hash160_t *token_hash,
                                         neoc_contract_invocation_t **invocation);

/**
 * @brief Create NEP-17 total supply query invocation
 * 
 * @param token_hash Token contract hash
 * @param invocation Output invocation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep17_total_supply_create(const neoc_hash160_t *token_hash,
                                             neoc_contract_invocation_t **invocation);

/**
 * @brief Get the script hash from a smart contract
 * 
 * @param contract The smart contract
 * @param script_hash Output script hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_smart_contract_get_script_hash(const neoc_smart_contract_t *contract,
                                                  neoc_hash160_t *script_hash);

/**
 * @brief Free a smart contract
 * 
 * @param contract The contract to free
 */
void neoc_smart_contract_free(neoc_smart_contract_t *contract);

/**
 * @brief Free a contract invocation
 * 
 * @param invocation The invocation to free
 */
void neoc_contract_invocation_free(neoc_contract_invocation_t *invocation);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SMART_CONTRACT_H
