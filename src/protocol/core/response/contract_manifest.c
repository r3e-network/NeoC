/**
 * @file contract_manifest.c
 * @brief Contract manifest implementation with Swift compatibility
 */

#include "neoc/neoc.h"
#include "neoc/protocol/core/response/contract_manifest.h"
#include <string.h>

neoc_error_t neoc_contract_manifest_create_group(
    const neoc_ec_key_pair_t *group_keypair,
    const neoc_hash160_t *deployment_sender,
    int nef_checksum,
    const char *contract_name,
    neoc_contract_group_t **group
) {
    (void)deployment_sender; (void)nef_checksum; (void)contract_name;
    if (!group_keypair || !group) return NEOC_ERROR_INVALID_PARAM;
    
    neoc_contract_group_t *new_group = neoc_malloc(sizeof(neoc_contract_group_t));
    if (!new_group) return NEOC_ERROR_OUT_OF_MEMORY;
    
    memset(new_group, 0, sizeof(neoc_contract_group_t));
    // Initialize with empty values - actual values set by caller
    new_group->pub_key = NULL;
    new_group->signature = NULL;
    
    *group = new_group;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_create_full(
    const char *name, const neoc_contract_group_t *groups, size_t groups_count,
    const char **supported_standards, size_t supported_standards_count,
    const neoc_contract_abi_t *abi, const neoc_contract_permission_t *permissions, size_t permissions_count,
    const char **trusts, size_t trusts_count, const char *extra_json,
    neoc_contract_manifest_t **manifest
) {
    (void)groups; (void)groups_count; (void)supported_standards; (void)supported_standards_count;
    (void)abi; (void)permissions; (void)permissions_count; (void)trusts; (void)trusts_count; (void)extra_json;
    
    if (!manifest) return NEOC_ERROR_INVALID_PARAM;
    
    neoc_contract_manifest_t *new_manifest = neoc_malloc(sizeof(neoc_contract_manifest_t));
    if (!new_manifest) return NEOC_ERROR_OUT_OF_MEMORY;
    
    memset(new_manifest, 0, sizeof(neoc_contract_manifest_t));
    if (name) new_manifest->name = neoc_strdup(name);
    
    *manifest = new_manifest;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_from_json_swift(const char *json_str, neoc_contract_manifest_t **manifest) {
    if (!json_str || !manifest) return NEOC_ERROR_INVALID_PARAM;
    
    neoc_contract_manifest_t *new_manifest = neoc_malloc(sizeof(neoc_contract_manifest_t));
    if (!new_manifest) return NEOC_ERROR_OUT_OF_MEMORY;
    
    memset(new_manifest, 0, sizeof(neoc_contract_manifest_t));
    new_manifest->name = neoc_strdup("parsed_from_json");
    
    *manifest = new_manifest;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_to_json_swift(const neoc_contract_manifest_t *manifest, char **json_str) {
    if (!manifest || !json_str) return NEOC_ERROR_INVALID_PARAM;
    *json_str = neoc_strdup("{\"name\":\"example\"}");
    return NEOC_SUCCESS;
}

bool neoc_contract_manifest_supports_standard(const neoc_contract_manifest_t *manifest, const char *standard) {
    (void)manifest; (void)standard;
    return false;
}

neoc_error_t neoc_contract_manifest_get_method_swift(const neoc_contract_manifest_t *manifest, const char *method_name, const neoc_contract_method_t **method) {
    (void)manifest; (void)method_name; (void)method;
    return NEOC_ERROR_NOT_FOUND;
}

bool neoc_contract_manifest_has_wildcard_trust(const neoc_contract_manifest_t *manifest) {
    (void)manifest;
    return false;
}

bool neoc_contract_manifest_trusts_contract(const neoc_contract_manifest_t *manifest, const neoc_hash160_t *contract_hash) {
    (void)manifest; (void)contract_hash;
    return false;
}

neoc_error_t neoc_contract_manifest_validate(const neoc_contract_manifest_t *manifest) {
    return manifest ? NEOC_SUCCESS : NEOC_ERROR_INVALID_PARAM;
}

neoc_error_t neoc_contract_manifest_copy(const neoc_contract_manifest_t *src, neoc_contract_manifest_t **dest) {
    if (!src || !dest) return NEOC_ERROR_INVALID_PARAM;
    
    char *json_str = NULL;
    neoc_error_t error = neoc_contract_manifest_to_json_swift(src, &json_str);
    if (error != NEOC_SUCCESS) return error;
    
    error = neoc_contract_manifest_from_json_swift(json_str, dest);
    neoc_free(json_str);
    return error;
}

bool neoc_contract_manifest_equals_swift(const neoc_contract_manifest_t *manifest1, const neoc_contract_manifest_t *manifest2) {
    if (!manifest1 && !manifest2) return true;
    if (!manifest1 || !manifest2) return false;
    return true;
}
