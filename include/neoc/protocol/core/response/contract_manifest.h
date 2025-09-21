/**
 * @file contract_manifest.h
 * @brief Contract manifest structure and related types
 * 
 * Converted from Swift source: protocol/core/response/ContractManifest.swift
 * This header provides access to contract manifest types defined in contract_response_types.h
 * and adds Swift-compatible functions for contract manifest operations.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_MANIFEST_H
#define NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_MANIFEST_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/protocol/contract_response_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Note: Core types are defined in contract_response_types.h:
 * - neoc_contract_group_t
 * - neoc_contract_event_t
 * - neoc_contract_method_t
 * - neoc_contract_abi_t
 * - neoc_contract_permission_t
 * - neoc_contract_manifest_t
 */

/* ========== Swift-compatible Contract Group Functions ========== */

/**
 * @brief Create a contract group with keypair and deployment info (Swift-compatible)
 * 
 * Equivalent to Swift ContractManifest.createGroup() method.
 * 
 * @param group_keypair EC key pair for signing
 * @param deployment_sender Hash160 of deployment sender
 * @param nef_checksum NEF checksum
 * @param contract_name Contract name (can be empty string)
 * @param group Output pointer for the created group (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_create_group(
    const neoc_ec_key_pair_t *group_keypair,
    const neoc_hash160_t *deployment_sender,
    int nef_checksum,
    const char *contract_name,
    neoc_contract_group_t **group
);

/* ========== Swift-compatible Contract Manifest Functions ========== */

/**
 * @brief Create a complete contract manifest (Swift-compatible)
 * 
 * Extended version with all Swift fields supported.
 * 
 * @param name Contract name (can be NULL)
 * @param groups Array of contract groups (can be NULL)
 * @param groups_count Number of groups
 * @param supported_standards Array of supported standards (can be NULL)
 * @param supported_standards_count Number of supported standards
 * @param abi Contract ABI (can be NULL)
 * @param permissions Array of permissions (can be NULL)
 * @param permissions_count Number of permissions
 * @param trusts Array of trusted contracts (can be NULL)
 * @param trusts_count Number of trusted contracts
 * @param extra_json Extra JSON metadata string (can be NULL)
 * @param manifest Output pointer for the created manifest (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_create_full(
    const char *name,
    const neoc_contract_group_t *groups,
    size_t groups_count,
    const char **supported_standards,
    size_t supported_standards_count,
    const neoc_contract_abi_t *abi,
    const neoc_contract_permission_t *permissions,
    size_t permissions_count,
    const char **trusts,
    size_t trusts_count,
    const char *extra_json,
    neoc_contract_manifest_t **manifest
);

/**
 * @brief Parse contract manifest from JSON string (Swift-compatible)
 * 
 * @param json_str JSON string containing manifest data
 * @param manifest Output pointer for the parsed manifest (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_from_json_swift(
    const char *json_str,
    neoc_contract_manifest_t **manifest
);

/**
 * @brief Convert contract manifest to JSON string (Swift-compatible)
 * 
 * @param manifest Manifest to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_to_json_swift(
    const neoc_contract_manifest_t *manifest,
    char **json_str
);

/**
 * @brief Check if manifest supports a specific NEP standard
 * 
 * @param manifest Contract manifest
 * @param standard Standard name (e.g., "NEP-17", "NEP-11")
 * @return true if supported, false otherwise
 */
bool neoc_contract_manifest_supports_standard(
    const neoc_contract_manifest_t *manifest,
    const char *standard
);

/**
 * @brief Get method by name from manifest ABI (Swift-compatible)
 * 
 * @param manifest Contract manifest
 * @param method_name Method name to find
 * @param method Output pointer to method (not owned by caller)
 * @return NEOC_SUCCESS if found, error code otherwise
 */
neoc_error_t neoc_contract_manifest_get_method_swift(
    const neoc_contract_manifest_t *manifest,
    const char *method_name,
    const neoc_contract_method_t **method
);

/**
 * @brief Check if manifest has wildcard trust
 * 
 * @param manifest Contract manifest
 * @return true if trusts wildcard ("*"), false otherwise
 */
bool neoc_contract_manifest_has_wildcard_trust(
    const neoc_contract_manifest_t *manifest
);

/**
 * @brief Check if manifest trusts a specific contract
 * 
 * @param manifest Contract manifest
 * @param contract_hash Contract hash to check
 * @return true if trusted, false otherwise
 */
bool neoc_contract_manifest_trusts_contract(
    const neoc_contract_manifest_t *manifest,
    const neoc_hash160_t *contract_hash
);

/**
 * @brief Validate contract manifest structure
 * 
 * @param manifest Contract manifest to validate
 * @return NEOC_SUCCESS if valid, error code with details otherwise
 */
neoc_error_t neoc_contract_manifest_validate(
    const neoc_contract_manifest_t *manifest
);

/**
 * @brief Create a deep copy of contract manifest
 * 
 * @param src Source manifest to copy
 * @param dest Output pointer for copied manifest (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_contract_manifest_copy(
    const neoc_contract_manifest_t *src,
    neoc_contract_manifest_t **dest
);

/**
 * @brief Compare two contract manifests for equality (Swift-compatible)
 * 
 * @param manifest1 First manifest
 * @param manifest2 Second manifest
 * @return true if equal, false otherwise
 */
bool neoc_contract_manifest_equals_swift(
    const neoc_contract_manifest_t *manifest1,
    const neoc_contract_manifest_t *manifest2
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_CONTRACT_MANIFEST_H */
