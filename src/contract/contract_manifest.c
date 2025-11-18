/**
 * @file contract_manifest.c
 * @brief Neo smart contract manifest implementation
 */

#include "neoc/contract/contract_manifest.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>
#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

// Contract method structure
struct neoc_contract_method_t {
    char *name;
    neoc_nep6_param_type_t *parameters;
    size_t parameter_count;
    neoc_nep6_param_type_t return_type;
    uint32_t offset;
    bool safe;
};

// Contract event structure
struct neoc_contract_event_t {
    char *name;
    neoc_nep6_param_type_t *parameters;
    size_t parameter_count;
};

// Contract permission structure
struct neoc_contract_permission_t {
    neoc_hash160_t *contract;  // NULL for wildcard
    char **methods;             // NULL for wildcard
    size_t method_count;
};

// Contract ABI structure
struct neoc_contract_abi_t {
    neoc_contract_method_t *methods;
    size_t method_count;
    size_t method_capacity;
    neoc_contract_event_t *events;
    size_t event_count;
    size_t event_capacity;
};

// Contract manifest structure
struct neoc_contract_manifest_t {
    char *name;
    char **supported_standards;
    size_t supported_standards_count;
    size_t standard_capacity;
    neoc_contract_abi_t *abi;
    neoc_contract_permission_t *permissions;
    size_t permission_count;
    size_t permission_capacity;
    neoc_contract_features_t features;
    char *extra;
};

// Helper to duplicate string
static char* str_dup(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = neoc_malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

neoc_error_t neoc_contract_manifest_create(neoc_contract_manifest_t **manifest) {
    if (!manifest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid manifest pointer");
    }
    
    *manifest = neoc_calloc(1, sizeof(neoc_contract_manifest_t));
    if (!*manifest) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate manifest");
    }
    
    neoc_contract_manifest_t* m = *manifest;
    
    // Set default name
    m->name = str_dup("Contract");
    if (!m->name) {
        neoc_free(*manifest);
        *manifest = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate name");
    }
    
    // Initialize ABI as embedded structure (not pointer)
    m->abi.methods = NULL;
    m->abi.method_count = 0;
    m->abi.events = NULL;
    m->abi.event_count = 0;
    
    // Initialize other fields
    m->groups = NULL;
    m->group_count = 0;
    m->supported_standards = NULL;
    m->supported_standards_count = 0;
    m->permissions = NULL;
    m->permission_count = 0;
    m->trusts = NULL;
    m->trust_count = 0;
    m->trust_wildcard = false;
    m->extra = NULL;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_set_standards(neoc_contract_manifest_t *manifest,
                                                   const char **standards,
                                                   size_t count) {
    if (!manifest || (!standards && count > 0)) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Free existing standards  
    for (size_t i = 0; i < manifest->supported_standards_count; i++) {
        if (manifest->supported_standards && manifest->supported_standards[i]) {
            neoc_free(manifest->supported_standards[i]);
        }
    }
    manifest->supported_standards_count = 0;
    
    // Free existing standards
    if (manifest->supported_standards) {
        for (size_t i = 0; i < manifest->supported_standards_count; i++) {
            if (manifest->supported_standards[i]) {
                neoc_free(manifest->supported_standards[i]);
            }
        }
        neoc_free(manifest->supported_standards);
    }
    
    // Allocate new array
    manifest->supported_standards = neoc_calloc(count, sizeof(char*));
    if (!manifest->supported_standards) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate standards");
    }
    
    // Copy standards
    for (size_t i = 0; i < count; i++) {
        manifest->supported_standards[i] = str_dup(standards[i]);
        if (!manifest->supported_standards[i]) {
            // Clean up on error
            for (size_t j = 0; j < i; j++) {
                neoc_free(manifest->supported_standards[j]);
            }
            manifest->supported_standards_count = 0;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy standard");
        }
    }
    
    manifest->supported_standards_count = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_add_method(neoc_contract_manifest_t *manifest,
                                                const char *name,
                                                const neoc_contract_method_parameter_t *parameters,
                                                size_t parameter_count,
                                                int32_t offset,
                                                uint8_t return_type,
                                                bool safe) {
    if (!manifest || !name) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Resize array if needed
    size_t new_count = manifest->abi.method_count + 1;
    neoc_contract_method_t *new_methods = neoc_realloc(manifest->abi.methods,
                                                         new_count * sizeof(neoc_contract_method_t));
    if (!new_methods) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize methods");
    }
    manifest->abi.methods = new_methods;
    
    neoc_contract_method_t *method = &manifest->abi.methods[manifest->abi.method_count];
    
    method->name = str_dup(name);
    if (!method->name) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy name");
    }
    
    if (parameter_count > 0 && parameters) {
        method->parameters = neoc_malloc(parameter_count * sizeof(neoc_contract_method_parameter_t));
        if (!method->parameters) {
            neoc_free(method->name);
            method->name = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameters");
        }
        // Copy parameter structures
        memcpy(method->parameters, parameters, parameter_count * sizeof(neoc_contract_method_parameter_t));
    }
    
    method->parameter_count = parameter_count;
    method->return_type = return_type;
    method->offset = offset;
    method->safe = safe;
    
    manifest->abi.method_count++;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_add_event(neoc_contract_manifest_t *manifest,
                                               const char *name,
                                               const neoc_nep6_param_type_t *parameters,
                                               size_t param_count) {
    if (!manifest || !name) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Resize array if needed
    if (manifest->abi.event_count == 0 || manifest->abi.event_count % 10 == 0) {
        size_t new_capacity = manifest->abi.event_count + 10;
        neoc_contract_event_t *new_events = neoc_realloc(manifest->abi.events,
                                                          new_capacity * sizeof(neoc_contract_event_t));
        if (!new_events) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize events");
        }
        manifest->abi.events = new_events;
    }
    
    neoc_contract_event_t *event = &manifest->abi.events[manifest->abi.event_count];
    
    event->name = str_dup(name);
    if (!event->name) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy name");
    }
    
    if (param_count > 0 && parameters) {
        event->parameters = neoc_malloc(param_count * sizeof(neoc_nep6_param_type_t));
        if (!event->parameters) {
            neoc_free(event->name);
            event->name = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameters");
        }
        memcpy(event->parameters, parameters, param_count * sizeof(neoc_nep6_param_type_t));
    }
    
    event->parameter_count = param_count;
    
    manifest->abi.event_count++;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_add_permission(neoc_contract_manifest_t *manifest,
                                                    const char *contract,
                                                    const char **methods,
                                                    size_t method_count) {
    if (!manifest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid manifest");
    }
    
    // Resize array if needed
    if (manifest->permission_count == 0 || manifest->permission_count % 10 == 0) {
        size_t new_capacity = manifest->permission_count + 10;
        neoc_contract_permission_t *new_perms = neoc_realloc(manifest->permissions,
                                                              new_capacity * sizeof(neoc_contract_permission_t));
        if (!new_perms) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize permissions");
        }
        manifest->permissions = new_perms;
    }
    
    neoc_contract_permission_t *perm = &manifest->permissions[manifest->permission_count];
    
    // Copy contract string if provided
    if (contract) {
        perm->contract = str_dup(contract);
        if (!perm->contract) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate contract string");
        }
    } else {
        perm->contract = NULL;  // Wildcard
    }
    
    // Copy methods if provided
    if (method_count > 0 && methods) {
        perm->methods = neoc_calloc(method_count, sizeof(char*));
        if (!perm->methods) {
            neoc_free(perm->contract);
            perm->contract = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate methods");
        }
        
        for (size_t i = 0; i < method_count; i++) {
            perm->methods[i] = str_dup(methods[i]);
            if (!perm->methods[i]) {
                // Clean up on error
                for (size_t j = 0; j < i; j++) {
                    neoc_free(perm->methods[j]);
                }
                neoc_free(perm->methods);
                neoc_free(perm->contract);
                perm->methods = NULL;
                perm->contract = NULL;
                return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy method");
            }
        }
        perm->method_count = method_count;
    } else {
        perm->methods = NULL;  // Wildcard
        perm->method_count = 0;
    }
    
    manifest->permission_count++;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_set_features(neoc_contract_manifest_t *manifest,
                                                  neoc_contract_features_t features) {
    if (!manifest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid manifest");
    }
    
    // Features are stored as a JSON object in the manifest
    // Serialize features dictionary to JSON string
    if (features != CONTRACT_FEATURE_NONE) {
        // Store features as JSON string in extra field
        // Format: {"storage":true,"payable":false,...}
        char feature_json[1024] = {0};
        int offset = 0;
        offset = snprintf(feature_json, sizeof(feature_json), "{");
        
        bool first = true;
        
        // Check each feature flag
        if (features & CONTRACT_FEATURE_HAS_STORAGE) {
            if (!first) {
                offset += snprintf(feature_json + offset, 
                                 sizeof(feature_json) - offset, ",");
            }
            offset += snprintf(feature_json + offset, 
                             sizeof(feature_json) - offset,
                             "\"storage\":true");
            first = false;
        }
        
        if (features & CONTRACT_FEATURE_PAYABLE) {
            if (!first) {
                offset += snprintf(feature_json + offset, 
                                 sizeof(feature_json) - offset, ",");
            }
            offset += snprintf(feature_json + offset, 
                             sizeof(feature_json) - offset,
                             "\"payable\":true");
            first = false;
        }
        
        offset += snprintf(feature_json + offset, sizeof(feature_json) - offset, "}");
        
        // Store in manifest's extra field
        if (!manifest->extra) {
            manifest->extra = str_dup(feature_json);
        } else {
            // Merge with existing extra data
            size_t extra_len = strlen(manifest->extra) + strlen(feature_json) + 10;
            char *new_extra = neoc_malloc(extra_len);
            if (new_extra) {
                snprintf(new_extra, extra_len, "%s,\"features\":%s", manifest->extra, feature_json);
                neoc_free(manifest->extra);
                manifest->extra = new_extra;
            }
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_set_extra(neoc_contract_manifest_t *manifest,
                                               const char *extra) {
    if (!manifest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid manifest");
    }
    
    neoc_free(manifest->extra);
    manifest->extra = extra ? str_dup(extra) : NULL;
    
    return NEOC_SUCCESS;
}

const char* neoc_contract_manifest_get_name(const neoc_contract_manifest_t *manifest) {
    return manifest ? manifest->name : NULL;
}

neoc_error_t neoc_contract_manifest_get_standards(const neoc_contract_manifest_t *manifest,
                                                   const char ***standards,
                                                   size_t *count) {
    if (!manifest || !standards || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *standards = (const char**)manifest->supported_standards;
    *count = manifest->supported_standards_count;
    
    return NEOC_SUCCESS;
}

size_t neoc_contract_manifest_get_method_count(const neoc_contract_manifest_t *manifest) {
    return manifest ? manifest->abi.method_count : 0;
}

neoc_error_t neoc_contract_manifest_get_method(const neoc_contract_manifest_t *manifest,
                                                size_t index,
                                                const neoc_contract_method_t **method) {
    if (!manifest || !method) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (index >= manifest->abi.method_count) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_BOUNDS, "Index out of bounds");
    }
    
    *method = &manifest->abi.methods[index];
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_get_method_by_name(const neoc_contract_manifest_t *manifest,
                                                        const char *name,
                                                        const neoc_contract_method_t **method) {
    if (!manifest || !name || !method) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    for (size_t i = 0; i < manifest->abi.method_count; i++) {
        if (strcmp(manifest->abi.methods[i].name, name) == 0) {
            *method = &manifest->abi.methods[i];
            return NEOC_SUCCESS;
        }
    }
    
    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Method not found");
}

const char* neoc_contract_method_get_name(const neoc_contract_method_t *method) {
    return method ? method->name : NULL;
}

uint32_t neoc_contract_method_get_offset(const neoc_contract_method_t *method) {
    return method ? method->offset : 0;
}

bool neoc_contract_method_is_safe(const neoc_contract_method_t *method) {
    return method ? method->safe : false;
}

bool neoc_contract_manifest_has_storage_feature(const neoc_contract_manifest_t *manifest) {
    if (!manifest || !manifest->extra) {
        return false;
    }
    
#ifdef HAVE_CJSON
    cJSON *root = cJSON_Parse(manifest->extra);
    if (!root) {
        return false;
    }
    
    cJSON *features = cJSON_GetObjectItem(root, "features");
    if (features) {
        cJSON *storage = cJSON_GetObjectItem(features, "storage");
        if (storage && cJSON_IsBool(storage)) {
            bool has_storage = cJSON_IsTrue(storage);
            cJSON_Delete(root);
            return has_storage;
        }
    }
    
    cJSON_Delete(root);
#endif
    return false;
}

bool neoc_contract_manifest_has_payable_feature(const neoc_contract_manifest_t *manifest) {
    if (!manifest || !manifest->extra) {
        return false;
    }
    
#ifdef HAVE_CJSON
    cJSON *root = cJSON_Parse(manifest->extra);
    if (!root) {
        return false;
    }
    
    cJSON *features = cJSON_GetObjectItem(root, "features");
    if (features) {
        cJSON *payable = cJSON_GetObjectItem(features, "payable");
        if (payable && cJSON_IsBool(payable)) {
            bool is_payable = cJSON_IsTrue(payable);
            cJSON_Delete(root);
            return is_payable;
        }
    }
    
    cJSON_Delete(root);
#endif
    return false;
}

// Simplified JSON serialization
neoc_error_t neoc_contract_manifest_to_json(const neoc_contract_manifest_t *manifest,
                                             char **json,
                                             size_t *json_len) {
    if (!manifest || !json || !json_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Estimate size
    size_t estimated_size = 8192;
    char *buffer = neoc_malloc(estimated_size);
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    int offset = snprintf(buffer, estimated_size,
        "{\n"
        "  \"name\": \"%s\",\n"
        "  \"groups\": [],\n"
        "  \"features\": {\n"
        "    \"storage\": %s,\n"
        "    \"payable\": %s\n"
        "  },\n"
        "  \"supportedstandards\": [",
        manifest->name,
        neoc_contract_manifest_has_storage_feature(manifest) ? "true" : "false",
        neoc_contract_manifest_has_payable_feature(manifest) ? "true" : "false"
    );
    
    // Add standards
    for (size_t i = 0; i < manifest->supported_standards_count; i++) {
        offset += snprintf(buffer + offset, estimated_size - offset,
            "%s\"%s\"", i > 0 ? ", " : "", manifest->supported_standards[i]);
    }
    
    offset += snprintf(buffer + offset, estimated_size - offset,
        "],\n"
        "  \"abi\": {\n"
        "    \"methods\": [\n"
    );
    
    // Add methods
    for (size_t i = 0; i < manifest->abi.method_count; i++) {
        neoc_contract_method_t *method = &manifest->abi.methods[i];
        offset += snprintf(buffer + offset, estimated_size - offset,
            "      {\n"
            "        \"name\": \"%s\",\n"
            "        \"parameters\": [],\n"
            "        \"returntype\": \"Void\",\n"
            "        \"offset\": %u,\n"
            "        \"safe\": %s\n"
            "      }%s\n",
            method->name,
            method->offset,
            method->safe ? "true" : "false",
            i < manifest->abi.method_count - 1 ? "," : ""
        );
    }
    
    offset += snprintf(buffer + offset, estimated_size - offset,
        "    ],\n"
        "    \"events\": [\n"
    );
    
    // Add events
    for (size_t i = 0; i < manifest->abi.event_count; i++) {
        neoc_contract_event_t *event = &manifest->abi.events[i];
        offset += snprintf(buffer + offset, estimated_size - offset,
            "      {\n"
            "        \"name\": \"%s\",\n"
            "        \"parameters\": []\n"
            "      }%s\n",
            event->name,
            i < manifest->abi.event_count - 1 ? "," : ""
        );
    }
    
    offset += snprintf(buffer + offset, estimated_size - offset,
        "    ]\n"
        "  },\n"
        "  \"permissions\": [],\n"
        "  \"trusts\": [],\n"
        "  \"extra\": %s\n"
        "}\n",
        manifest->extra ? manifest->extra : "null"
    );
    
    *json = buffer;
    *json_len = offset;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_manifest_from_json(const char *json,
                                               neoc_contract_manifest_t **manifest) {
    if (!json || !manifest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
#ifndef HAVE_CJSON
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON not available");
#else
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON");
    }
    
    neoc_error_t err = neoc_contract_manifest_create(manifest);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return err;
    }
    
    // Parse name
    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (name && cJSON_IsString(name)) {
        neoc_free((*manifest)->name);
        (*manifest)->name = str_dup(name->valuestring);
    }
    
    // Parse supported standards
    cJSON *standards = cJSON_GetObjectItem(root, "supportedstandards");
    if (standards && cJSON_IsArray(standards)) {
        size_t count = cJSON_GetArraySize(standards);
        if (count > 0) {
            const char **standard_names = neoc_malloc(count * sizeof(char*));
            if (standard_names) {
                for (size_t i = 0; i < count; i++) {
                    cJSON *item = cJSON_GetArrayItem(standards, i);
                    if (item && cJSON_IsString(item)) {
                        standard_names[i] = item->valuestring;
                    }
                }
                neoc_contract_manifest_set_standards(*manifest, standard_names, count);
                neoc_free(standard_names);
            }
        }
    }
    
    // Parse ABI methods
    cJSON *abi = cJSON_GetObjectItem(root, "abi");
    if (abi) {
        cJSON *methods = cJSON_GetObjectItem(abi, "methods");
        if (methods && cJSON_IsArray(methods)) {
            size_t method_count = cJSON_GetArraySize(methods);
            for (size_t i = 0; i < method_count; i++) {
                cJSON *method = cJSON_GetArrayItem(methods, i);
                if (method) {
                    cJSON *method_name = cJSON_GetObjectItem(method, "name");
                    cJSON *offset = cJSON_GetObjectItem(method, "offset");
                    cJSON *safe = cJSON_GetObjectItem(method, "safe");
                    
                    if (method_name && cJSON_IsString(method_name)) {
                        neoc_contract_manifest_add_method(*manifest,
                                                         method_name->valuestring,
                                                         NULL, 0,
                                                         offset ? offset->valueint : 0,
                                                         0, // return type
                                                         safe ? cJSON_IsTrue(safe) : false);
                    }
                }
            }
        }
    }
    
    cJSON_Delete(root);
    return NEOC_SUCCESS;
#endif
}

void neoc_contract_manifest_dispose(neoc_contract_manifest_t *manifest) {
    if (!manifest) return;
    
    neoc_free(manifest->name);
    manifest->name = NULL;
    
    // Free groups
    for (size_t i = 0; i < manifest->group_count; i++) {
        neoc_free(manifest->groups[i].pub_key);
        neoc_free(manifest->groups[i].signature);
    }
    neoc_free(manifest->groups);
    manifest->groups = NULL;
    manifest->group_count = 0;
    
    // Free standards
    for (size_t i = 0; i < manifest->supported_standards_count; i++) {
        neoc_free(manifest->supported_standards[i]);
    }
    neoc_free(manifest->supported_standards);
    manifest->supported_standards = NULL;
    manifest->supported_standards_count = 0;
    
    // Free ABI
    // Free methods
    for (size_t i = 0; i < manifest->abi.method_count; i++) {
        neoc_free(manifest->abi.methods[i].name);
        neoc_free(manifest->abi.methods[i].parameters);
    }
    neoc_free(manifest->abi.methods);
    manifest->abi.methods = NULL;
    manifest->abi.method_count = 0;
    
    // Free events
    for (size_t i = 0; i < manifest->abi.event_count; i++) {
        neoc_free(manifest->abi.events[i].name);
        neoc_free(manifest->abi.events[i].parameters);
    }
    neoc_free(manifest->abi.events);
    manifest->abi.events = NULL;
    manifest->abi.event_count = 0;
    
    // Free permissions
    for (size_t i = 0; i < manifest->permission_count; i++) {
        neoc_free(manifest->permissions[i].contract);
        if (manifest->permissions[i].methods) {
            for (size_t j = 0; j < manifest->permissions[i].method_count; j++) {
                neoc_free(manifest->permissions[i].methods[j]);
            }
            neoc_free(manifest->permissions[i].methods);
        }
    }
    neoc_free(manifest->permissions);
    manifest->permissions = NULL;
    manifest->permission_count = 0;
    
    // Free trusts
    for (size_t i = 0; i < manifest->trust_count; i++) {
        neoc_free(manifest->trusts[i]);
    }
    neoc_free(manifest->trusts);
    manifest->trusts = NULL;
    manifest->trust_count = 0;
    
    neoc_free(manifest->extra);
    manifest->extra = NULL;
    manifest->trust_count = 0;
    manifest->trust_wildcard = false;
}

void neoc_contract_manifest_free(neoc_contract_manifest_t *manifest) {
    if (!manifest) return;
    
    neoc_contract_manifest_dispose(manifest);
    neoc_free(manifest);
}

neoc_contract_manifest_t* neoc_contract_manifest_clone(const neoc_contract_manifest_t *manifest) {
    if (!manifest) {
        return NULL;
    }

    char *json = NULL;
    size_t json_len = 0;
    neoc_error_t err = neoc_contract_manifest_to_json(manifest, &json, &json_len);
    if (err != NEOC_SUCCESS || !json) {
        if (json) neoc_free(json);
        return NULL;
    }

    neoc_contract_manifest_t *clone = NULL;
    err = neoc_contract_manifest_from_json(json, &clone);
    neoc_free(json);
    if (err != NEOC_SUCCESS) {
        if (clone) {
            neoc_contract_manifest_free(clone);
        }
        return NULL;
    }

    return clone;
}
