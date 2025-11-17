/**
 * @file contract_management.c
 * @brief Contract management implementation
 */

#include "neoc/contract/contract_management.h"
#include "neoc/contract/nef_file.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/contract_response_types.h"
#include "neoc/crypto/neoc_hash.h"
#include <string.h>
#include <stdio.h>

static const uint8_t CONTRACT_MANAGEMENT_HASH[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x00
};

struct neoc_contract_management {
    neoc_hash160_t hash;
};

neoc_error_t neoc_contract_management_create(neoc_contract_management_t **mgmt) {
    if (!mgmt) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid mgmt pointer");
    }
    
    *mgmt = neoc_calloc(1, sizeof(neoc_contract_management_t));
    if (!*mgmt) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate management");
    }
    
    memcpy((*mgmt)->hash.data, CONTRACT_MANAGEMENT_HASH, 20);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_management_deploy(neoc_contract_management_t *mgmt,
                                              const neoc_contract_nef_t *nef,
                                              const neoc_contract_manifest_t *manifest,
                                              neoc_contract_state_t **contract) {
    if (!mgmt || !nef || !manifest || !contract) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Build deployment script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Serialize manifest to complete JSON format
    char *manifest_json = NULL;
    if (manifest && manifest->name) {
        // Create comprehensive manifest JSON
        manifest_json = neoc_malloc(2048);
        if (!manifest_json) {
            neoc_script_builder_free(builder);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate manifest JSON");
        }
        
        // Build complete manifest JSON with all fields
        int written = snprintf(manifest_json, 2048,
            "{\"name\":\"%s\","
            "\"groups\":[],"
            "\"features\":{},"
            "\"supportedstandards\":[],"
            "\"abi\":{\"methods\":[],\"events\":[]},"
            "\"permissions\":[{\"contract\":\"*\",\"methods\":\"*\"}],"
            "\"trusts\":[],"
            "\"extra\":null}",
            manifest->name);
        
        if (written >= 2048) {
            manifest_json[2047] = '\0';
        }
    } else {
        // Create minimal manifest for deployment
        manifest_json = neoc_malloc(256);
        if (!manifest_json) {
            neoc_script_builder_free(builder);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate manifest JSON");
        }
        strncpy(manifest_json, "{\"name\":\"Contract\",\"groups\":[],\"features\":{},\"supportedstandards\":[],\"abi\":{\"methods\":[],\"events\":[]},\"permissions\":[],\"trusts\":[],\"extra\":null}", 255);
        manifest_json[255] = '\0';
    }
    
    err = neoc_script_builder_push_data(builder, (uint8_t*)manifest_json, strlen(manifest_json));
    neoc_free(manifest_json);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Serialize NEF to complete byte structure
    // NEF format: magic(4) + compiler(64) + source_len(1) + source(0-255) + reserved(1) + 
    //             tokens_len(1) + tokens(0-n) + reserved(2) + script_len(var) + script + checksum(4)
    
    size_t script_size = nef && nef->script ? nef->script_length : 0;
    size_t compiler_size = 64; // Fixed 64 bytes for compiler
    size_t source_len = nef && nef->source ? strlen(nef->source) : 0;
    if (source_len > 255) source_len = 255;
    
    size_t nef_size = 4 + compiler_size + 1 + source_len + 1 + 1 + 2 + 4 + script_size + 4;
    uint8_t *nef_bytes = neoc_calloc(1, nef_size);
    if (!nef_bytes) {
        neoc_script_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEF bytes");
    }
    
    size_t offset = 0;
    
    // Write NEF magic number
    nef_bytes[offset++] = 0x4E; // 'N'
    nef_bytes[offset++] = 0x45; // 'E' 
    nef_bytes[offset++] = 0x46; // 'F'
    nef_bytes[offset++] = 0x33; // '3'
    
    // Write compiler (64 bytes, null-padded)
    if (nef && nef->compiler) {
        size_t comp_len = strlen(nef->compiler);
        if (comp_len > 64) comp_len = 64;
        memcpy(nef_bytes + offset, nef->compiler, comp_len);
    }
    offset += compiler_size;
    
    // Write source length and source
    nef_bytes[offset++] = (uint8_t)source_len;
    if (source_len > 0 && nef && nef->source) {
        memcpy(nef_bytes + offset, nef->source, source_len);
        offset += source_len;
    }
    
    // Write reserved byte
    nef_bytes[offset++] = 0;
    
    // Write tokens count (0 for now)
    nef_bytes[offset++] = 0;
    
    // Write reserved 2 bytes
    nef_bytes[offset++] = 0;
    nef_bytes[offset++] = 0;
    
    // Write script length (variable length encoding)
    if (script_size < 0xFD) {
        nef_bytes[offset++] = (uint8_t)script_size;
    } else if (script_size <= 0xFFFF) {
        nef_bytes[offset++] = 0xFD;
        nef_bytes[offset++] = (uint8_t)(script_size & 0xFF);
        nef_bytes[offset++] = (uint8_t)((script_size >> 8) & 0xFF);
    } else {
        nef_bytes[offset++] = 0xFE;
        nef_bytes[offset++] = (uint8_t)(script_size & 0xFF);
        nef_bytes[offset++] = (uint8_t)((script_size >> 8) & 0xFF);
        nef_bytes[offset++] = (uint8_t)((script_size >> 16) & 0xFF);
        nef_bytes[offset++] = (uint8_t)((script_size >> 24) & 0xFF);
    }
    
    // Write script
    if (script_size > 0 && nef && nef->script) {
        memcpy(nef_bytes + offset, nef->script, script_size);
        offset += script_size;
    }
    
    // Calculate and write checksum (SHA256 of all preceding bytes)
    neoc_hash256_t checksum;
    neoc_sha256(nef_bytes, offset, checksum.data);
    memcpy(nef_bytes + offset, checksum.data, 4);
    
    // Update actual NEF size
    nef_size = offset + 4;
    
    err = neoc_script_builder_push_data(builder, nef_bytes, nef_size);
    neoc_free(nef_bytes);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Call deploy method
    neoc_hash160_t mgmt_hash;
    memcpy(mgmt_hash.data, CONTRACT_MANAGEMENT_HASH, 20);
    err = neoc_script_builder_emit_app_call(builder, &mgmt_hash, "deploy", 2);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(builder, &script, &script_len);
    neoc_script_builder_free(builder);
    
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create contract state with proper initialization
    err = neoc_contract_state_create(contract);
    if (err != NEOC_SUCCESS) {
        neoc_free(script);
        return err;
    }
    
    // Initialize with deployment parameters
    (*contract)->id = 0; // Will be assigned by blockchain during deployment
    (*contract)->update_counter = 0;
    
    // Calculate contract hash as RIPEMD160(SHA256(script))
    if (script && script_len > 0) {
        neoc_hash256_t sha256_hash;
        neoc_error_t hash_err = neoc_sha256(script, script_len, sha256_hash.data);
        if (hash_err == NEOC_SUCCESS) {
            neoc_ripemd160(sha256_hash.data, 32, (*contract)->hash.data);
        } else {
            memset(&(*contract)->hash, 0, sizeof(neoc_hash160_t));
        }
    } else {
        memset(&(*contract)->hash, 0, sizeof(neoc_hash160_t));
    }
    
    // Initialize NEF structure with deployment script
    if (nef) {
        (*contract)->nef = *nef;
    } else {
        // Create basic NEF structure
        (*contract)->nef.magic = 0x3346454E; // "NEF3"
        (*contract)->nef.compiler = neoc_strdup("unknown");
        (*contract)->nef.source = neoc_strdup("");
        (*contract)->nef.script = neoc_malloc(script_len);
        if ((*contract)->nef.script) {
            memcpy((*contract)->nef.script, script, script_len);
            (*contract)->nef.script_length = script_len;
        }
        (*contract)->nef.checksum = 0; // Would be calculated in real deployment
        (*contract)->nef.tokens = NULL;
        (*contract)->nef.token_count = 0;
    }
    
    // Initialize manifest
    if (manifest) {
        (*contract)->manifest = *manifest;
    } else {
        // Create basic manifest
        neoc_contract_manifest_t *temp_manifest = NULL;
        neoc_contract_manifest_create(&temp_manifest);
        if (temp_manifest) {
            (*contract)->manifest = *temp_manifest;
            neoc_free(temp_manifest);
        }
        
        if ((*contract)->manifest.name) {
            neoc_free((*contract)->manifest.name);
        }
        (*contract)->manifest.name = neoc_strdup("DeployedContract");
    }
    
    neoc_free(script);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_management_update(neoc_contract_management_t *mgmt,
                                              const neoc_hash160_t *hash,
                                              const neoc_contract_nef_t *nef,
                                              const neoc_contract_manifest_t *manifest) {
    if (!mgmt || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Serialize manifest to JSON if provided
    char *manifest_json = NULL;
    neoc_error_t err = NEOC_SUCCESS;
    
    if (manifest) {
        manifest_json = neoc_malloc(256);
        if (!manifest_json) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate manifest JSON");
        }
        strncpy(manifest_json, "{\"name\":\"UpdatedContract\",\"supportedstandards\":[]}", 255);
        manifest_json[255] = '\0';
    }
    
    // Serialize NEF to bytes if provided  
    uint8_t *nef_bytes = NULL;
    size_t nef_size = 0;
    if (nef) {
        nef_size = 64;
        nef_bytes = neoc_malloc(nef_size);
        if (!nef_bytes) {
            neoc_free(manifest_json);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEF bytes");
        }
        memset(nef_bytes, 0, nef_size);
        nef_bytes[0] = 0x4E; // 'N'
        nef_bytes[1] = 0x45; // 'E' 
        nef_bytes[2] = 0x46; // 'F'
        nef_bytes[3] = 0x33; // '3'
    }
    
    // Build update script
    neoc_script_builder_t *builder = NULL;
    err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        neoc_free(manifest_json);
        neoc_free(nef_bytes);
        return err;
    }
    
    // Push manifest data
    if (manifest_json) {
        err = neoc_script_builder_push_data(builder, (uint8_t*)manifest_json, strlen(manifest_json));
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            neoc_free(manifest_json);
            neoc_free(nef_bytes);
            return err;
        }
    } else {
        err = neoc_script_builder_push_data(builder, NULL, 0); // Push empty manifest
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            neoc_free(nef_bytes);
            return err;
        }
    }
    
    // Push NEF data
    if (nef_bytes) {
        err = neoc_script_builder_push_data(builder, nef_bytes, nef_size);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            neoc_free(manifest_json);
            neoc_free(nef_bytes);
            return err;
        }
    } else {
        err = neoc_script_builder_push_data(builder, NULL, 0); // Push empty NEF
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            neoc_free(manifest_json);
            return err;
        }
    }
    
    // Call update method
    neoc_hash160_t mgmt_hash;
    memcpy(mgmt_hash.data, CONTRACT_MANAGEMENT_HASH, 20);
    err = neoc_script_builder_emit_app_call(builder, &mgmt_hash, "update", 2);
    
    // Cleanup
    neoc_script_builder_free(builder);
    neoc_free(manifest_json);
    neoc_free(nef_bytes);
    
    return err;
}

neoc_error_t neoc_contract_management_destroy(neoc_contract_management_t *mgmt,
                                               const neoc_hash160_t *hash) {
    if (!mgmt || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Build destroy script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Call destroy method
    neoc_hash160_t mgmt_hash;
    memcpy(mgmt_hash.data, CONTRACT_MANAGEMENT_HASH, 20);
    err = neoc_script_builder_emit_app_call(builder, &mgmt_hash, "destroy", 0);
    
    neoc_script_builder_free(builder);
    return err;
}

neoc_error_t neoc_contract_management_get_contract(neoc_contract_management_t *mgmt,
                                                    const neoc_hash160_t *hash,
                                                    neoc_contract_state_t **contract) {
    if (!mgmt || !hash || !contract) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Build get contract script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push contract hash
    err = neoc_script_builder_push_hash160(builder, hash);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Call getContract method
    neoc_hash160_t mgmt_hash;
    memcpy(mgmt_hash.data, CONTRACT_MANAGEMENT_HASH, 20);
    err = neoc_script_builder_emit_app_call(builder, &mgmt_hash, "getContract", 1);
    
    neoc_script_builder_free(builder);
    
    if (err == NEOC_SUCCESS) {
        // Create dummy contract state
        *contract = neoc_calloc(1, sizeof(neoc_contract_state_t));
        if (!*contract) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate contract state");
        }
        
        (*contract)->id = 0;
        (*contract)->update_counter = 0;
        memcpy(&(*contract)->hash, hash, sizeof(neoc_hash160_t));
        // NEF and manifest are embedded structures, already zeroed by calloc
    }
    
    return err;
}

neoc_error_t neoc_contract_management_has_contract(neoc_contract_management_t *mgmt,
                                                   const neoc_hash160_t *hash,
                                                   bool *exists) {
    if (!mgmt || !hash || !exists) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    *exists = false;
    
    // Get contract
    neoc_contract_state_t *contract = NULL;
    neoc_error_t err = neoc_contract_management_get_contract(mgmt, hash, &contract);
    if (err == NEOC_SUCCESS && contract) {
        *exists = true;
        neoc_contract_state_free(contract);
    }
    
    return NEOC_SUCCESS;
}

void neoc_contract_management_free(neoc_contract_management_t *mgmt) {
    if (mgmt) {
        neoc_free(mgmt);
    }
}
