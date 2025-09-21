#ifndef NEOC_CONTRACT_RESPONSE_TYPES_H
#define NEOC_CONTRACT_RESPONSE_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/serialization/neo_serializable.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/protocol/stack_item.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file contract_response_types.h
 * @brief Neo contract response types for RPC protocol
 * 
 * This header defines all the contract-related response types used in the Neo RPC protocol.
 * These types represent the various contract structures returned by Neo RPC calls including
 * contract manifests, contract states, NEF files, and other contract metadata.
 * 
 * All types implement the NeoSerializable interface for binary serialization and include
 * proper memory management with reference counting where appropriate.
 * 
 * @author NeoC SDK Team
 * @version 1.0.0
 * @date 2024
 */

/**
 * @brief Contract Group structure
 * 
 * Represents a group that can interact with a contract.
 * Groups are defined by a public key and signature.
 */
typedef struct neoc_contract_group {
    char *pub_key;              /**< Public key in hex format (33 bytes compressed) */
    char *signature;            /**< Base64-encoded signature */
} neoc_contract_group_t;

/**
 * @brief Contract Method Parameter
 * 
 * Represents a parameter definition for a contract method.
 */
typedef struct neoc_contract_method_parameter {
    char *name;                 /**< Parameter name */
    uint8_t type;               /**< Parameter type (ContractParameterType) */
} neoc_contract_method_parameter_t;

/**
 * @brief Contract Method
 * 
 * Represents a method defined in a contract's ABI.
 */
typedef struct neoc_contract_method {
    char *name;                                     /**< Method name */
    neoc_contract_method_parameter_t *parameters;   /**< Method parameters */
    size_t parameter_count;                         /**< Number of parameters */
    int32_t offset;                                 /**< Method offset in contract */
    uint8_t return_type;                            /**< Return type (ContractParameterType) */
    bool safe;                                      /**< Whether method is safe to call */
} neoc_contract_method_t;

/**
 * @brief Contract Event
 * 
 * Represents an event defined in a contract's ABI.
 */
typedef struct neoc_contract_event {
    char *name;                                     /**< Event name */
    neoc_contract_method_parameter_t *parameters;   /**< Event parameters */
    size_t parameter_count;                         /**< Number of parameters */
} neoc_contract_event_t;

/**
 * @brief Contract ABI
 * 
 * Represents the Application Binary Interface of a contract.
 */
typedef struct neoc_contract_abi {
    neoc_contract_method_t *methods;    /**< Contract methods */
    size_t method_count;                /**< Number of methods */
    neoc_contract_event_t *events;      /**< Contract events */
    size_t event_count;                 /**< Number of events */
} neoc_contract_abi_t;

/**
 * @brief Contract Permission
 * 
 * Represents a permission that a contract requests.
 */
typedef struct neoc_contract_permission {
    char *contract;         /**< Contract hash or wildcard */
    char **methods;         /**< Allowed methods (NULL for all) */
    size_t method_count;    /**< Number of allowed methods */
    bool is_wildcard;       /**< Whether this is a wildcard permission */
} neoc_contract_permission_t;

/**
 * @brief Contract Manifest
 * 
 * Represents the complete manifest of a Neo smart contract.
 * The manifest describes the contract's interface, permissions, and metadata.
 */
typedef struct neoc_contract_manifest {
    neoc_serializable_t base;                   /**< Base serializable interface */
    
    char *name;                                 /**< Contract name */
    neoc_contract_group_t *groups;              /**< Contract groups */
    size_t group_count;                         /**< Number of groups */
    
    char **supported_standards;                 /**< Supported NEP standards */
    size_t supported_standards_count;           /**< Number of supported standards */
    
    neoc_contract_abi_t abi;                    /**< Contract ABI */
    
    neoc_contract_permission_t *permissions;    /**< Contract permissions */
    size_t permission_count;                    /**< Number of permissions */
    
    char **trusts;                              /**< Trusted contracts */
    size_t trust_count;                         /**< Number of trusted contracts */
    bool trust_wildcard;                        /**< Whether trust is wildcard */
    
    char *extra;                                /**< Extra JSON metadata */
} neoc_contract_manifest_t;

/**
 * @brief Contract NEF (Neo Executable Format)
 * 
 * Represents the executable code of a Neo smart contract.
 */
typedef struct neoc_contract_nef {
    neoc_serializable_t base;       /**< Base serializable interface */
    
    uint32_t magic;                 /**< NEF magic number (0x3346454E) */
    char *compiler;                 /**< Compiler identification string */
    char *source;                   /**< Source code reference */
    uint8_t *tokens;                /**< Method tokens */
    size_t token_count;             /**< Number of tokens */
    uint8_t *script;                /**< Contract bytecode */
    size_t script_length;           /**< Length of bytecode */
    uint32_t checksum;              /**< Script checksum */
} neoc_contract_nef_t;

/**
 * @brief Contract State
 * 
 * Represents the complete state of a deployed contract.
 */
typedef struct neoc_contract_state {
    neoc_serializable_t base;           /**< Base serializable interface */
    
    int32_t id;                         /**< Contract ID */
    int32_t update_counter;             /**< Number of times contract was updated */
    neoc_hash160_t hash;                /**< Contract hash */
    neoc_contract_nef_t nef;            /**< Contract NEF */
    neoc_contract_manifest_t manifest;  /**< Contract manifest */
} neoc_contract_state_t;

/**
 * @brief Contract Method Token
 * 
 * Represents a method token used for contract invocations.
 */
typedef struct neoc_contract_method_token {
    neoc_hash160_t hash;        /**< Contract hash */
    char *method;               /**< Method name */
    uint16_t params_count;      /**< Parameter count */
    bool has_return_value;      /**< Whether method returns a value */
    uint8_t call_flags;         /**< Call flags */
} neoc_contract_method_token_t;

/**
 * @brief Contract Storage Entry
 * 
 * Represents a key-value pair in contract storage.
 */
typedef struct neoc_contract_storage_entry {
    uint8_t *key;               /**< Storage key */
    size_t key_length;          /**< Key length */
    uint8_t *value;             /**< Storage value */
    size_t value_length;        /**< Value length */
} neoc_contract_storage_entry_t;

/**
 * @brief Native Contract State
 * 
 * Represents the state of a native (built-in) contract.
 */
typedef struct neoc_native_contract_state {
    int32_t id;                         /**< Native contract ID */
    neoc_hash160_t hash;                /**< Contract hash */
    neoc_contract_manifest_t *manifest; /**< Contract manifest (owned) */
    neoc_contract_nef_t *nef;           /**< Contract NEF (owned) */
    int *update_history;                /**< Historical update heights */
    size_t update_history_count;        /**< Number of updates recorded */
    char **active_in;                   /**< Active network versions (optional) */
    size_t active_in_count;             /**< Number of active version entries */
} neoc_native_contract_state_t;

// Creation functions

/**
 * @brief Create a contract manifest
 * 
 * @param manifest Output manifest (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_create(neoc_contract_manifest_t **manifest);

/**
 * @brief Create a contract NEF
 * 
 * @param nef Output NEF (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_nef_create(neoc_contract_nef_t **nef);

/**
 * @brief Create a contract state
 * 
 * @param state Output state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_state_create(neoc_contract_state_t **state);

/**
 * @brief Create a contract method token
 * 
 * @param token Output token (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_method_token_create(neoc_contract_method_token_t **token);

/**
 * @brief Create a contract storage entry
 * 
 * @param entry Output entry (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_storage_entry_create(neoc_contract_storage_entry_t **entry);

// Manipulation functions for Contract Manifest

/**
 * @brief Set manifest name
 * 
 * @param manifest Manifest to modify
 * @param name New name (copied)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_set_name(neoc_contract_manifest_t *manifest, const char *name);

/**
 * @brief Add group to manifest
 * 
 * @param manifest Manifest to modify
 * @param pub_key Public key in hex format (33 bytes compressed)
 * @param signature Base64-encoded signature
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_add_group(neoc_contract_manifest_t *manifest, 
                                               const char *pub_key, 
                                               const char *signature);

/**
 * @brief Add supported standard to manifest
 * 
 * @param manifest Manifest to modify
 * @param standard Standard name (e.g., "NEP-17")
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_add_standard(neoc_contract_manifest_t *manifest, 
                                                  const char *standard);

/**
 * @brief Add method to manifest ABI
 * 
 * @param manifest Manifest to modify
 * @param name Method name
 * @param parameters Method parameters (copied)
 * @param parameter_count Number of parameters
 * @param offset Method offset
 * @param return_type Return type
 * @param safe Whether method is safe
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_add_method(neoc_contract_manifest_t *manifest,
                                                const char *name,
                                                const neoc_contract_method_parameter_t *parameters,
                                                size_t parameter_count,
                                                int32_t offset,
                                                uint8_t return_type,
                                                bool safe);

/**
 * @brief Add permission to manifest
 * 
 * @param manifest Manifest to modify
 * @param contract Contract hash or "*" for wildcard
 * @param methods Allowed methods (NULL for all)
 * @param method_count Number of methods
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_add_permission(neoc_contract_manifest_t *manifest,
                                                    const char *contract,
                                                    const char **methods,
                                                    size_t method_count);

/**
 * @brief Add trusted contract to manifest
 * 
 * @param manifest Manifest to modify
 * @param contract Contract hash or "*" for wildcard
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_add_trust(neoc_contract_manifest_t *manifest, 
                                               const char *contract);

// Manipulation functions for Contract NEF

/**
 * @brief Set NEF compiler
 * 
 * @param nef NEF to modify
 * @param compiler Compiler string
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_nef_set_compiler(neoc_contract_nef_t *nef, const char *compiler);

/**
 * @brief Set NEF script
 * 
 * @param nef NEF to modify
 * @param script Bytecode script (copied)
 * @param length Script length
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_nef_set_script(neoc_contract_nef_t *nef, 
                                           const uint8_t *script, 
                                           size_t length);

// Utility functions

/**
 * @brief Get contract manifest from stack item
 * 
 * @param stack_item Stack item containing manifest data
 * @param manifest Output manifest (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_from_stack_item(const stack_item_t *stack_item,
                                                     neoc_contract_manifest_t **manifest);

/**
 * @brief Get contract state from stack item
 * 
 * @param stack_item Stack item containing state data
 * @param state Output state (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_state_from_stack_item(const stack_item_t *stack_item,
                                                  neoc_contract_state_t **state);

/**
 * @brief Compare two contract manifests for equality
 * 
 * @param manifest1 First manifest
 * @param manifest2 Second manifest
 * @param equal Output equality result
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_equals(const neoc_contract_manifest_t *manifest1,
                                            const neoc_contract_manifest_t *manifest2,
                                            bool *equal);

/**
 * @brief Compare two contract states for equality
 * 
 * @param state1 First state
 * @param state2 Second state
 * @param equal Output equality result
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_state_equals(const neoc_contract_state_t *state1,
                                         const neoc_contract_state_t *state2,
                                         bool *equal);

// Free functions

/**
 * @brief Free a contract group
 * 
 * @param group Group to free
 */
void neoc_contract_group_free(neoc_contract_group_t *group);

/**
 * @brief Free a contract method
 * 
 * @param method Method to free
 */
void neoc_contract_method_free(neoc_contract_method_t *method);

/**
 * @brief Free a contract event
 * 
 * @param event Event to free
 */
void neoc_contract_event_free(neoc_contract_event_t *event);

/**
 * @brief Free a contract ABI
 * 
 * @param abi ABI to free
 */
void neoc_contract_abi_free(neoc_contract_abi_t *abi);

/**
 * @brief Free a contract permission
 * 
 * @param permission Permission to free
 */
void neoc_contract_permission_free(neoc_contract_permission_t *permission);

/**
 * @brief Free a contract manifest
 * 
 * @param manifest Manifest to free
 */
void neoc_contract_manifest_free(neoc_contract_manifest_t *manifest);

/**
 * @brief Free a contract NEF
 * 
 * @param nef NEF to free
 */
void neoc_contract_nef_free(neoc_contract_nef_t *nef);

/**
 * @brief Free a contract state
 * 
 * @param state State to free
 */
void neoc_contract_state_free(neoc_contract_state_t *state);

/**
 * @brief Free a contract method token
 * 
 * @param token Token to free
 */
void neoc_contract_method_token_free(neoc_contract_method_token_t *token);

/**
 * @brief Free a contract storage entry
 * 
 * @param entry Entry to free
 */
void neoc_contract_storage_entry_free(neoc_contract_storage_entry_t *entry);

/**
 * @brief Free a native contract state
 * 
 * @param state Native state to free
 */
void neoc_native_contract_state_free(neoc_native_contract_state_t *state);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_RESPONSE_TYPES_H
