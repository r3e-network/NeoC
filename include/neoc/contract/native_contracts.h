/**
 * @file native_contracts.h
 * @brief Neo native contracts (NEO, GAS, etc.)
 */

#ifndef NEOC_NATIVE_CONTRACTS_H
#define NEOC_NATIVE_CONTRACTS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

// Native contract hashes (mainnet)
#define NATIVE_NEO_HASH     "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"
#define NATIVE_GAS_HASH     "0xd2a4cff31913016155e38e474a2c06d08be276cf"
#define NATIVE_POLICY_HASH  "0xcc5e4edd9f5f8dba8bb65734541df7a1c081c67b"
#define NATIVE_ORACLE_HASH  "0xfe924b7cfe89ddd271abaf7210a80a7e11178758"
#define NATIVE_LEDGER_HASH  "0xda65b600f7124ce6c79950c1772a36403104f2be"
#define NATIVE_ROLE_HASH    "0x49cf4e5378ffcd4dec034fd98a174c5491e395e2"
#define NATIVE_CRYPTO_HASH  "0x726cb6e0cd8628a1350a611384688911ab75f51b"
#define NATIVE_STD_HASH     "0xacce6fd80d44e1796aa0c2c625e9e4e0ce39efc0"
#define NATIVE_MANAGEMENT_HASH "0xfffdc93764dbaddd97c48f252a53ea4643faa3fd"

// Native contract IDs
typedef enum {
    NATIVE_CONTRACT_NEO = -5,
    NATIVE_CONTRACT_GAS = -6,
    NATIVE_CONTRACT_POLICY = -7,
    NATIVE_CONTRACT_ORACLE = -8,
    NATIVE_CONTRACT_LEDGER = -9,
    NATIVE_CONTRACT_ROLE = -10,
    NATIVE_CONTRACT_CRYPTO = -11,
    NATIVE_CONTRACT_STD = -12,
    NATIVE_CONTRACT_MANAGEMENT = -13
} neoc_native_contract_id_t;

// NEP-17 methods (used by NEO and GAS)
#define NEP17_METHOD_SYMBOL "symbol"
#define NEP17_METHOD_DECIMALS "decimals"
#define NEP17_METHOD_TOTAL_SUPPLY "totalSupply"
#define NEP17_METHOD_BALANCE_OF "balanceOf"
#define NEP17_METHOD_TRANSFER "transfer"

// NEO specific methods
#define NEO_METHOD_UNCLAIMED_GAS "unclaimedGas"
#define NEO_METHOD_REGISTER_CANDIDATE "registerCandidate"
#define NEO_METHOD_UNREGISTER_CANDIDATE "unregisterCandidate"
#define NEO_METHOD_VOTE "vote"
#define NEO_METHOD_GET_CANDIDATES "getCandidates"
#define NEO_METHOD_GET_COMMITTEE "getCommittee"
#define NEO_METHOD_GET_NEXT_BLOCK_VALIDATORS "getNextBlockValidators"
#define NEO_METHOD_GET_GAS_PER_BLOCK "getGasPerBlock"
#define NEO_METHOD_SET_GAS_PER_BLOCK "setGasPerBlock"
#define NEO_METHOD_GET_REGISTER_PRICE "getRegisterPrice"
#define NEO_METHOD_SET_REGISTER_PRICE "setRegisterPrice"
#define NEO_METHOD_GET_ACCOUNT_STATE "getAccountState"

// GAS specific methods
#define GAS_METHOD_REFUEL "refuel"

// Policy contract methods
#define POLICY_METHOD_GET_FEE_PER_BYTE "getFeePerByte"
#define POLICY_METHOD_SET_FEE_PER_BYTE "setFeePerByte"
#define POLICY_METHOD_GET_EXEC_FEE_FACTOR "getExecFeeFactor"
#define POLICY_METHOD_SET_EXEC_FEE_FACTOR "setExecFeeFactor"
#define POLICY_METHOD_GET_STORAGE_PRICE "getStoragePrice"
#define POLICY_METHOD_SET_STORAGE_PRICE "setStoragePrice"
#define POLICY_METHOD_IS_BLOCKED "isBlocked"
#define POLICY_METHOD_BLOCK_ACCOUNT "blockAccount"
#define POLICY_METHOD_UNBLOCK_ACCOUNT "unblockAccount"

/**
 * @brief Get native contract hash by ID
 * 
 * @param contract_id Native contract ID
 * @param hash Output hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_get_hash(neoc_native_contract_id_t contract_id,
                                   neoc_hash160_t *hash);

/**
 * @brief Get native contract hash by name
 * 
 * @param name Contract name (e.g., "NEO", "GAS")
 * @param hash Output hash
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_get_hash_by_name(const char *name,
                                           neoc_hash160_t *hash);

/**
 * @brief Create NEO transfer script
 * 
 * @param from From address hash
 * @param to To address hash
 * @param amount Amount to transfer (in NEO, not fractions)
 * @param data Optional data
 * @param data_len Data length
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_neo_transfer(const neoc_hash160_t *from,
                                       const neoc_hash160_t *to,
                                       uint64_t amount,
                                       const uint8_t *data,
                                       size_t data_len,
                                       uint8_t **script,
                                       size_t *script_len);

/**
 * @brief Create GAS transfer script
 * 
 * @param from From address hash
 * @param to To address hash
 * @param amount Amount to transfer (in GAS fractions, 1 GAS = 10^8 fractions)
 * @param data Optional data
 * @param data_len Data length
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_gas_transfer(const neoc_hash160_t *from,
                                       const neoc_hash160_t *to,
                                       uint64_t amount,
                                       const uint8_t *data,
                                       size_t data_len,
                                       uint8_t **script,
                                       size_t *script_len);

/**
 * @brief Create balance query script for NEO
 * 
 * @param account Account hash
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_neo_balance_of(const neoc_hash160_t *account,
                                         uint8_t **script,
                                         size_t *script_len);

/**
 * @brief Create balance query script for GAS
 * 
 * @param account Account hash
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_gas_balance_of(const neoc_hash160_t *account,
                                         uint8_t **script,
                                         size_t *script_len);

/**
 * @brief Create unclaimed GAS query script
 * 
 * @param account Account hash
 * @param height Block height
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_neo_unclaimed_gas(const neoc_hash160_t *account,
                                            uint32_t height,
                                            uint8_t **script,
                                            size_t *script_len);

/**
 * @brief Create vote script
 * 
 * @param account Account hash
 * @param vote_to Public key to vote for (NULL to cancel vote)
 * @param vote_to_len Public key length
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_neo_vote(const neoc_hash160_t *account,
                                   const uint8_t *vote_to,
                                   size_t vote_to_len,
                                   uint8_t **script,
                                   size_t *script_len);

/**
 * @brief Check if hash is a native contract
 * 
 * @param hash Hash to check
 * @return true if native contract, false otherwise
 */
bool neoc_native_is_native_contract(const neoc_hash160_t *hash);

/**
 * @brief Get native contract name
 * 
 * @param hash Contract hash
 * @return Contract name (do not free), or NULL if not native
 */
const char* neoc_native_get_name(const neoc_hash160_t *hash);

/**
 * @brief Get native contract ID
 * 
 * @param hash Contract hash
 * @param id Output contract ID
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_native_get_id(const neoc_hash160_t *hash,
                                 neoc_native_contract_id_t *id);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NATIVE_CONTRACTS_H
