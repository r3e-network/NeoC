#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif
#include "../../../include/neoc/protocol/response/contract_state.h"
#include "../../../include/neoc/protocol/contract_response_types.h"
#include "../../../include/neoc/protocol/response/contract_nef.h"
#include "../../../include/neoc/contract/contract_manifest.h"
#include "../../../include/neoc/neoc_memory.h"
#include "../../../include/neoc/utils/neoc_hex.h"
#include "../../../include/neoc/utils/neoc_bytes_utils.h"

// Create contract state
neoc_error_t neoc_contract_state_create(neoc_contract_state_t **state) {
    if (!state) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *state = neoc_malloc(sizeof(neoc_contract_state_t));
    if (!*state) {
        return NEOC_ERROR_MEMORY;
    }
    
    // Initialize fields to zero
    memset(*state, 0, sizeof(neoc_contract_state_t));
    
    return NEOC_SUCCESS;
}

// Free contract state
void neoc_contract_state_free(neoc_contract_state_t* state) {
    if (!state) {
        return;
    }
    
    neoc_contract_nef_dispose(&state->nef);
    neoc_contract_manifest_dispose(&state->manifest);
    
    neoc_free(state);
}

// Clone contract state
neoc_contract_state_t* neoc_contract_state_clone(const neoc_contract_state_t* state) {
    if (!state) {
        return NULL;
    }
    
    neoc_contract_state_t* clone = NULL;
    neoc_error_t err = neoc_contract_state_create(&clone);
    if (err != NEOC_SUCCESS) {
        return NULL;
    }
    
    // Copy all fields
    clone->id = state->id;
    clone->update_counter = state->update_counter;
    memcpy(&clone->hash, &state->hash, sizeof(neoc_hash160_t));
    
    // Clone NEF
    neoc_contract_nef_t* nef_clone = neoc_contract_nef_clone(&state->nef);
    if (nef_clone) {
        clone->nef = *nef_clone;
        neoc_free(nef_clone); // Free the wrapper
    }
    
    // Clone manifest - simplified copy for now
    clone->manifest = state->manifest;
    
    return clone;
}

// Compare contract states
neoc_error_t neoc_contract_state_equals(
    const neoc_contract_state_t* a,
    const neoc_contract_state_t* b,
    bool* equal) {
    
    if (!a || !b || !equal) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *equal = false;
    
    // Compare hash
    if (memcmp(&a->hash, &b->hash, sizeof(neoc_hash160_t)) != 0) {
        return NEOC_SUCCESS;
    }
    
    // Compare other fields
    if (a->id != b->id || a->update_counter != b->update_counter) {
        return NEOC_SUCCESS;
    }
    
    // Compare NEF
    if (!neoc_contract_nef_equals(&a->nef, &b->nef)) {
        return NEOC_SUCCESS;
    }
    
    // Compare manifest - for production implementation, this would:
    // 1. Compare manifest names, standards, and methods
    // 2. Validate ABI compatibility
    // 3. Check permission and trust settings
    // Currently simplified to always consider manifests equal
    
    *equal = true;
    return NEOC_SUCCESS;
}

// Parse from JSON
neoc_contract_state_t* neoc_contract_state_from_json(const char* json_str) {
    if (!json_str) {
        return NULL;
    }
    
#ifdef HAVE_CJSON
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return NULL;
    }
    
    neoc_contract_state_t* state = NULL;
    neoc_error_t err = neoc_contract_state_create(&state);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return NULL;
    }
    
    // Parse ID
    cJSON *id = cJSON_GetObjectItem(root, "id");
    if (id && cJSON_IsNumber(id)) {
        state->id = (uint32_t)id->valueint;
    }
    
    // Parse update counter
    cJSON *update_counter = cJSON_GetObjectItem(root, "updatecounter");
    if (update_counter && cJSON_IsNumber(update_counter)) {
        state->update_counter = (uint16_t)update_counter->valueint;
    }
    
    // Parse hash
    cJSON *hash = cJSON_GetObjectItem(root, "hash");
    if (hash && cJSON_IsString(hash)) {
        neoc_hash160_from_string(hash->valuestring, &state->hash);
    }
    
    // Parse NEF
    cJSON *nef = cJSON_GetObjectItem(root, "nef");
    if (nef && cJSON_IsObject(nef)) {
        char* nef_str = cJSON_Print(nef);
        if (nef_str) {
            neoc_contract_nef_t* parsed_nef = neoc_contract_nef_from_json(nef_str);
            if (parsed_nef) {
                state->nef = *parsed_nef;
                neoc_free(parsed_nef); // Free the wrapper
            }
            neoc_free(nef_str);
        }
    }
    
    // Parse manifest
    cJSON *manifest = cJSON_GetObjectItem(root, "manifest");
    if (manifest && cJSON_IsObject(manifest)) {
        char* manifest_str = cJSON_Print(manifest);
        if (manifest_str) {
            neoc_contract_manifest_t* parsed_manifest = NULL;
            if (neoc_contract_manifest_from_json(manifest_str, &parsed_manifest) == NEOC_SUCCESS) {
                state->manifest = *parsed_manifest;
                neoc_free(parsed_manifest); // Free the wrapper
            }
            neoc_free(manifest_str);
        }
    }
    
    cJSON_Delete(root);
    return state;
#else
    return NULL; // cJSON not available
#endif
}

// Convert to JSON
char* neoc_contract_state_to_json(const neoc_contract_state_t* state) {
    if (!state) {
        return NULL;
    }
    
    // Get base class JSON components
    char hash_str[41];
    neoc_hash160_to_string(&state->hash, hash_str, sizeof(hash_str));
    
    // Simplified manifest JSON for now
    const char* manifest_json = "{\"name\":\"Contract\"}";
    
    char* nef_json = neoc_contract_nef_to_json(&state->nef);
    if (!nef_json) {
        return NULL;
    }
    
    // Calculate required buffer size
    size_t json_size = strlen("{\"id\":,\"updatecounter\":,\"hash\":\"\",\"nef\":,\"manifest\":}") + 
                       20 + 20 + strlen(hash_str) + 
                       strlen(nef_json) + strlen(manifest_json) + 1;
    
    char* json = neoc_malloc(json_size);
    if (!json) {
        neoc_free(nef_json);
        return NULL;
    }
    
    snprintf(json, json_size, 
             "{\"id\":%d,\"updatecounter\":%d,\"hash\":\"%s\",\"nef\":%s,\"manifest\":%s}", 
             state->id, state->update_counter, hash_str, nef_json, manifest_json);
    
    neoc_free(nef_json);
    return json;
}

// Contract identifiers from stack item
neoc_contract_identifiers_t* neoc_contract_identifiers_from_stack_item(
    const neoc_stack_item_t* stack_item) {
    
    if (!stack_item) {
        return NULL;
    }
    
    neoc_contract_identifiers_t* identifiers = neoc_malloc(sizeof(neoc_contract_identifiers_t));
    if (!identifiers) {
        return NULL;
    }
    
    // Initialize with default values
    identifiers->id = 0;
    memset(&identifiers->hash, 0, sizeof(neoc_hash160_t));
    
    // Parse stack item if it's an array with at least 2 elements
    // Format expected: [id, hash_bytes]
    if (stack_item_get_type(stack_item) == STACK_ITEM_TYPE_ARRAY) {
        size_t array_length = stack_item_array_count(stack_item);
        
        if (array_length >= 2) {
            // Parse ID from first element
            stack_item_t* id_item = stack_item_array_get(stack_item, 0);
            if (id_item && stack_item_get_type(id_item) == STACK_ITEM_TYPE_INTEGER) {
                int64_t id_value;
                if (stack_item_to_integer(id_item, &id_value) == NEOC_SUCCESS) {
                    identifiers->id = (uint32_t)id_value;
                }
            }
            
            // Parse hash from second element
            stack_item_t* hash_item = stack_item_array_get(stack_item, 1);
            if (hash_item && stack_item_get_type(hash_item) == STACK_ITEM_TYPE_BYTE_STRING) {
                uint8_t hash_bytes[20];
                size_t hash_length = 20;
                if (stack_item_to_byte_array(hash_item, hash_bytes, &hash_length) == NEOC_SUCCESS && hash_length == 20) {
                    memcpy(identifiers->hash.data, hash_bytes, 20);
                }
            }
        }
    }
    
    return identifiers;
}

// Free contract identifiers
void neoc_contract_identifiers_free(neoc_contract_identifiers_t* identifiers) {
    if (identifiers) {
        neoc_free(identifiers);
    }
}

// Compare contract identifiers
bool neoc_contract_identifiers_equals(
    const neoc_contract_identifiers_t* a,
    const neoc_contract_identifiers_t* b) {
    
    if (!a && !b) return true;
    if (!a || !b) return false;
    
    return a->id == b->id && 
           memcmp(&a->hash, &b->hash, sizeof(neoc_hash160_t)) == 0;
}
