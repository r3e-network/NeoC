/**
 * @file contract_manifest.h
 * @brief Neo smart contract manifest support
 */

#ifndef NEOC_CONTRACT_MANIFEST_H
#define NEOC_CONTRACT_MANIFEST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/wallet/nep6.h"
#include "neoc/protocol/contract_response_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Types are defined in protocol/contract_response_types.h (included above)

/**
 * @brief Contract features flags
 */
typedef enum {
    CONTRACT_FEATURE_NONE = 0,
    CONTRACT_FEATURE_HAS_STORAGE = 1,
    CONTRACT_FEATURE_PAYABLE = 2,
    CONTRACT_FEATURE_HAS_STORAGE_PAYABLE = 3
} neoc_contract_features_t;

/**
 * @brief Method safe flag
 */
typedef enum {
    METHOD_SAFE_NONE = 0,
    METHOD_SAFE_READ_ONLY = 1
} neoc_method_safe_t;

// Note: Most functions are in protocol/contract_response_types.h
//       This header provides additional convenience functions

/**
 * @brief Load manifest from JSON string
 * 
 * @param json JSON string
 * @param manifest Output manifest handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_from_json(const char *json,
                                              neoc_contract_manifest_t **manifest);

neoc_contract_manifest_t* neoc_contract_manifest_clone(const neoc_contract_manifest_t *manifest);

/**
 * @brief Release allocated members of an embedded manifest without freeing the struct itself.
 *
 * Use this when the manifest is part of another object.
 *
 * @param manifest Manifest to dispose
 */
void neoc_contract_manifest_dispose(neoc_contract_manifest_t *manifest);

/**
 * @brief Serialize manifest to JSON string
 * 
 * @param manifest Manifest handle
 * @param json Output JSON string (caller must free)
 * @param json_len Output JSON length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_to_json(const neoc_contract_manifest_t *manifest,
                                             char **json,
                                             size_t *json_len);

/**
 * @brief Set manifest supported standards
 * 
 * @param manifest Manifest handle
 * @param standards Array of standard names (e.g., "NEP-17")
 * @param count Number of standards
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_set_standards(neoc_contract_manifest_t *manifest,
                                                   const char **standards,
                                                   size_t count);

// Note: neoc_contract_manifest_add_method() and neoc_contract_manifest_add_permission()
//       are already defined in protocol/contract_response_types.h

/**
 * @brief Add an event to the manifest
 * 
 * @param manifest Manifest handle
 * @param name Event name
 * @param parameters Array of parameter types
 * @param param_count Number of parameters
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_add_event(neoc_contract_manifest_t *manifest,
                                               const char *name,
                                               const neoc_nep6_param_type_t *parameters,
                                               size_t param_count);

// Note: neoc_contract_manifest_add_permission() is already defined in protocol/contract_response_types.h

/**
 * @brief Set contract features
 * 
 * @param manifest Manifest handle
 * @param features Feature flags
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_set_features(neoc_contract_manifest_t *manifest,
                                                  neoc_contract_features_t features);

/**
 * @brief Set contract extra data
 * 
 * @param manifest Manifest handle
 * @param extra Extra JSON data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_set_extra(neoc_contract_manifest_t *manifest,
                                               const char *extra);

/**
 * @brief Get contract name
 * 
 * @param manifest Manifest handle
 * @return Contract name (do not free)
 */
const char* neoc_contract_manifest_get_name(const neoc_contract_manifest_t *manifest);

/**
 * @brief Get supported standards
 * 
 * @param manifest Manifest handle
 * @param standards Output array of standard names
 * @param count Output number of standards
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_get_standards(const neoc_contract_manifest_t *manifest,
                                                   const char ***standards,
                                                   size_t *count);

/**
 * @brief Get number of methods
 * 
 * @param manifest Manifest handle
 * @return Number of methods
 */
size_t neoc_contract_manifest_get_method_count(const neoc_contract_manifest_t *manifest);

/**
 * @brief Get method by index
 * 
 * @param manifest Manifest handle
 * @param index Method index
 * @param method Output method handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_get_method(const neoc_contract_manifest_t *manifest,
                                                size_t index,
                                                const neoc_contract_method_t **method);

/**
 * @brief Get method by name
 * 
 * @param manifest Manifest handle
 * @param name Method name
 * @param method Output method handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_contract_manifest_get_method_by_name(const neoc_contract_manifest_t *manifest,
                                                        const char *name,
                                                        const neoc_contract_method_t **method);

/**
 * @brief Get method name
 * 
 * @param method Method handle
 * @return Method name (do not free)
 */
const char* neoc_contract_method_get_name(const neoc_contract_method_t *method);

/**
 * @brief Get method offset
 * 
 * @param method Method handle
 * @return Method offset in script
 */
uint32_t neoc_contract_method_get_offset(const neoc_contract_method_t *method);

/**
 * @brief Check if method is safe
 * 
 * @param method Method handle
 * @return true if method is safe (read-only)
 */
bool neoc_contract_method_is_safe(const neoc_contract_method_t *method);

/**
 * @brief Check if contract has storage feature
 * 
 * @param manifest Manifest handle
 * @return true if contract has storage
 */
bool neoc_contract_manifest_has_storage_feature(const neoc_contract_manifest_t *manifest);

/**
 * @brief Check if contract is payable
 * 
 * @param manifest Manifest handle
 * @return true if contract is payable
 */
bool neoc_contract_manifest_has_payable_feature(const neoc_contract_manifest_t *manifest);

/**
 * @brief Free contract manifest
 * 
 * @param manifest Manifest handle
 */
void neoc_contract_manifest_free(neoc_contract_manifest_t *manifest);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CONTRACT_MANIFEST_H
