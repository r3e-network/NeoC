#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif
#include "../../../include/neoc/protocol/response/contract_nef.h"
#include "../../../include/neoc/protocol/contract_response_types.h"
#include "../../../include/neoc/neoc_memory.h"
#include "../../../include/neoc/crypto/hash.h"
#include "../../../include/neoc/crypto/sha256.h"
#include "../../../include/neoc/utils/neoc_base64.h"

// Create an empty contract NEF (matches contract_response_types.h)
neoc_error_t neoc_contract_nef_create(neoc_contract_nef_t **nef) {
    if (!nef) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *nef = neoc_malloc(sizeof(neoc_contract_nef_t));
    if (!*nef) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(*nef, 0, sizeof(neoc_contract_nef_t));
    (*nef)->magic = 0x3346454E; // "NEF3" in hex
    
    return NEOC_SUCCESS;
}


// Free contract NEF
void neoc_contract_nef_free(neoc_contract_nef_t* nef) {
    if (!nef) {
        return;
    }
    
    if (nef->compiler) {
        neoc_free(nef->compiler);
    }
    
    if (nef->source) {
        neoc_free(nef->source);
    }
    
    if (nef->script) {
        neoc_free(nef->script);
    }
    
    if (nef->tokens) {
        neoc_free(nef->tokens);
    }
    
    neoc_free(nef);
}

// Clone contract NEF
neoc_contract_nef_t* neoc_contract_nef_clone(const neoc_contract_nef_t* nef) {
    if (!nef) {
        return NULL;
    }
    
    neoc_contract_nef_t* clone = neoc_malloc(sizeof(neoc_contract_nef_t));
    if (!clone) {
        return NULL;
    }
    
    // Copy simple fields
    clone->magic = nef->magic;
    clone->script_length = nef->script_length;
    clone->token_count = nef->token_count;
    clone->checksum = nef->checksum;
    
    // Deep copy compiler
    if (nef->compiler) {
        clone->compiler = neoc_strdup(nef->compiler);
        if (!clone->compiler) {
            neoc_free(clone);
            return NULL;
        }
    } else {
        clone->compiler = NULL;
    }
    
    // Deep copy source
    if (nef->source) {
        clone->source = neoc_strdup(nef->source);
        if (!clone->source) {
            if (clone->compiler) neoc_free(clone->compiler);
            neoc_free(clone);
            return NULL;
        }
    } else {
        clone->source = NULL;
    }
    
    // Deep copy script
    if (nef->script && nef->script_length > 0) {
        clone->script = neoc_malloc(nef->script_length);
        if (!clone->script) {
            if (clone->compiler) neoc_free(clone->compiler);
            if (clone->source) neoc_free(clone->source);
            neoc_free(clone);
            return NULL;
        }
        memcpy(clone->script, nef->script, nef->script_length);
    } else {
        clone->script = NULL;
    }
    
    // Deep copy tokens
    if (nef->tokens && nef->token_count > 0) {
        clone->tokens = neoc_malloc(nef->token_count);
        if (!clone->tokens) {
            if (clone->script) neoc_free(clone->script);
            if (clone->compiler) neoc_free(clone->compiler);
            if (clone->source) neoc_free(clone->source);
            neoc_free(clone);
            return NULL;
        }
        memcpy(clone->tokens, nef->tokens, nef->token_count);
    } else {
        clone->tokens = NULL;
    }
    
    return clone;
}

// Compare contract NEFs
bool neoc_contract_nef_equals(
    const neoc_contract_nef_t* a,
    const neoc_contract_nef_t* b) {
    
    if (!a && !b) return true;
    if (!a || !b) return false;
    
    // Compare magic
    if (a->magic != b->magic) {
        return false;
    }
    
    // Compare compiler
    if (a->compiler && b->compiler) {
        if (strcmp(a->compiler, b->compiler) != 0) {
            return false;
        }
    } else if (a->compiler != b->compiler) {
        return false;
    }
    
    // Compare source
    if (a->source && b->source) {
        if (strcmp(a->source, b->source) != 0) {
            return false;
        }
    } else if (a->source != b->source) {
        return false;
    }
    
    // Compare script
    if (a->script_length != b->script_length) {
        return false;
    }
    
    if (a->script_length > 0 && memcmp(a->script, b->script, a->script_length) != 0) {
        return false;
    }
    
    // Compare checksum
    if (a->checksum != b->checksum) {
        return false;
    }
    
    // Compare tokens
    if (a->token_count != b->token_count) {
        return false;
    }
    
    if (a->token_count > 0 && memcmp(a->tokens, b->tokens, a->token_count) != 0) {
        return false;
    }
    
    return true;
}

// Parse from JSON
neoc_contract_nef_t* neoc_contract_nef_from_json(const char* json_str) {
    if (!json_str) {
        return NULL;
    }
    
#ifdef HAVE_CJSON
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return NULL;
    }
    
    neoc_contract_nef_t* nef = neoc_malloc(sizeof(neoc_contract_nef_t));
    if (!nef) {
        cJSON_Delete(root);
        return NULL;
    }
    
    // Initialize with defaults
    memset(nef, 0, sizeof(neoc_contract_nef_t));
    
    // Parse magic
    cJSON *magic = cJSON_GetObjectItem(root, "magic");
    if (magic && cJSON_IsString(magic)) {
        sscanf(magic->valuestring, "%x", &nef->magic);
    } else {
        nef->magic = 0x3346454E; // Default NEF magic
    }
    
    // Parse compiler
    cJSON *compiler = cJSON_GetObjectItem(root, "compiler");
    if (compiler && cJSON_IsString(compiler)) {
        nef->compiler = neoc_strdup(compiler->valuestring);
    } else {
        nef->compiler = neoc_strdup("");
    }
    
    // Parse source
    cJSON *source = cJSON_GetObjectItem(root, "source");
    if (source && cJSON_IsString(source)) {
        nef->source = neoc_strdup(source->valuestring);
    } else {
        nef->source = neoc_strdup("");
    }
    
    // Parse script (base64 encoded)
    cJSON *script = cJSON_GetObjectItem(root, "script");
    if (script && cJSON_IsString(script)) {
        size_t decoded_length;
        nef->script = neoc_base64_decode_alloc(script->valuestring, &decoded_length);
        nef->script_length = decoded_length;
    } else {
        nef->script = neoc_malloc(1);
        nef->script_length = 0;
    }
    
    // Parse checksum
    cJSON *checksum = cJSON_GetObjectItem(root, "checksum");
    if (checksum && cJSON_IsString(checksum)) {
        sscanf(checksum->valuestring, "%x", &nef->checksum);
    }
    
    // Initialize tokens (empty for now)
    nef->tokens = NULL;
    nef->token_count = 0;
    
    cJSON_Delete(root);
    
    // Validate that we have required fields
    if (!nef->compiler || !nef->source || !nef->script) {
        neoc_contract_nef_free(nef);
        return NULL;
    }
    
    return nef;
#else
    return NULL; // cJSON not available
#endif
}

// Convert to JSON
char* neoc_contract_nef_to_json(const neoc_contract_nef_t* nef) {
    if (!nef) {
        return NULL;
    }
    
    // Convert script to base64
    char* script_base64 = neoc_base64_encode_alloc(nef->script, nef->script_length);
    if (!script_base64) {
        return NULL;
    }
    
    // Convert checksum to hex string
    char checksum_str[16];
    snprintf(checksum_str, sizeof(checksum_str), "%08x", nef->checksum);
    
    // Calculate required buffer size
    size_t json_size = strlen("{\"magic\":\"\",\"compiler\":\"\",\"source\":\"\",\"script\":\"\",\"checksum\":\"\"}") +
                       4 + strlen(nef->compiler) + strlen(nef->source) +
                       strlen(script_base64) + strlen(checksum_str) + 100;
    
    char* json = neoc_malloc(json_size);
    if (!json) {
        neoc_free(script_base64);
        return NULL;
    }
    
    snprintf(json, json_size,
             "{\"magic\":\"%08x\",\"compiler\":\"%s\",\"source\":\"%s\",\"script\":\"%s\",\"checksum\":\"%s\"}",
             nef->magic, nef->compiler, nef->source, script_base64, checksum_str);
    
    neoc_free(script_base64);
    return json;
}

// Serialize to bytes
uint8_t* neoc_contract_nef_serialize(const neoc_contract_nef_t* nef, size_t* out_length) {
    if (!nef || !out_length) {
        return NULL;
    }
    
    // Calculate total size
    size_t total_size = 4 + 64 + 256 + 4 + nef->script_length + 4 + 2 + 4 + nef->token_count;
    
    uint8_t* data = neoc_malloc(total_size);
    if (!data) {
        return NULL;
    }
    
    size_t offset = 0;
    
    // Write magic (convert uint32_t to bytes)
    uint32_t magic_le = nef->magic;
    memcpy(data + offset, &magic_le, 4);
    offset += 4;
    
    // Write compiler
    memcpy(data + offset, nef->compiler, 64);
    offset += 64;
    
    // Write source
    memcpy(data + offset, nef->source, 256);
    offset += 256;
    
    // Write script length
    uint32_t script_length = (uint32_t)nef->script_length;
    memcpy(data + offset, &script_length, 4);
    offset += 4;
    
    // Write script
    memcpy(data + offset, nef->script, nef->script_length);
    offset += nef->script_length;
    
    // Write checksum (4 bytes for uint32_t)
    memcpy(data + offset, &nef->checksum, 4);
    offset += 4;
    
    // Write reserved (skip - not in our struct)
    uint16_t reserved = 0;
    memcpy(data + offset, &reserved, 2);
    offset += 2;
    
    // Write tokens length
    uint32_t token_count = (uint32_t)nef->token_count;
    memcpy(data + offset, &token_count, 4);
    offset += 4;
    
    // Write tokens
    if (nef->token_count > 0) {
        memcpy(data + offset, nef->tokens, nef->token_count);
        offset += nef->token_count;
    }
    
    *out_length = offset;
    return data;
}

// Deserialize from bytes
neoc_contract_nef_t* neoc_contract_nef_deserialize(const uint8_t* data, size_t length) {
    if (!data || length < 4 + 64 + 256 + 4 + 32 + 2 + 4) {
        return NULL;
    }
    
    neoc_contract_nef_t* nef = neoc_malloc(sizeof(neoc_contract_nef_t));
    if (!nef) {
        return NULL;
    }
    
    size_t offset = 0;
    
    // Read magic (as uint32_t)
    memcpy(&nef->magic, data + offset, 4);
    offset += 4;
    
    // Validate magic (convert to bytes for comparison)
    uint32_t expected_magic = 0x3346454E;
    if (nef->magic != expected_magic) {
        neoc_free(nef);
        return NULL;
    }
    
    // Read compiler (allocate and copy)
    char compiler_buf[65] = {0};
    memcpy(compiler_buf, data + offset, 64);
    nef->compiler = neoc_strdup(compiler_buf);
    if (!nef->compiler) {
        neoc_free(nef);
        return NULL;
    }
    offset += 64;
    
    // Read source (allocate and copy)
    char source_buf[257] = {0};
    memcpy(source_buf, data + offset, 256);
    nef->source = neoc_strdup(source_buf);
    if (!nef->source) {
        neoc_free(nef->compiler);
        neoc_free(nef);
        return NULL;
    }
    offset += 256;
    
    // Read script length
    uint32_t script_length;
    memcpy(&script_length, data + offset, 4);
    offset += 4;
    
    if (offset + script_length + 4 + 2 + 4 > length) {
        neoc_free(nef->compiler);
        neoc_free(nef->source);
        neoc_free(nef);
        return NULL;
    }
    
    // Read script
    nef->script_length = script_length;
    nef->script = neoc_malloc(script_length);
    if (!nef->script) {
        neoc_free(nef->compiler);
        neoc_free(nef->source);
        neoc_free(nef);
        return NULL;
    }
    memcpy(nef->script, data + offset, script_length);
    offset += script_length;
    
    // Read checksum (4 bytes for uint32_t)
    memcpy(&nef->checksum, data + offset, 4);
    offset += 4;
    
    // Read reserved (skip)
    offset += 2;
    
    // Read tokens length
    uint32_t token_count;
    memcpy(&token_count, data + offset, 4);
    offset += 4;
    
    // Read tokens
    nef->token_count = token_count;
    if (token_count > 0) {
        if (offset + token_count > length) {
            neoc_free(nef->script);
            neoc_free(nef->compiler);
            neoc_free(nef->source);
            neoc_free(nef);
            return NULL;
        }
        
        nef->tokens = neoc_malloc(token_count);
        if (!nef->tokens) {
            neoc_free(nef->script);
            neoc_free(nef->compiler);
            neoc_free(nef->source);
            neoc_free(nef);
            return NULL;
        }
        memcpy(nef->tokens, data + offset, token_count);
    } else {
        nef->tokens = NULL;
    }
    
    return nef;
}

// Calculate checksum
uint32_t neoc_contract_nef_calculate_checksum(const neoc_contract_nef_t* nef) {
    if (!nef) {
        return 0;
    }
    
    // Calculate size of data to hash
    size_t data_size = 4 + 64 + 256 + 4 + nef->script_length + 2 + 4 + nef->token_count;
    uint8_t* data = neoc_malloc(data_size);
    if (!data) {
        return 0;
    }
    
    size_t offset = 0;
    
    // Add magic
    memcpy(data + offset, &nef->magic, 4);
    offset += 4;
    
    // Add compiler (as fixed-size buffer)
    char compiler_buf[64] = {0};
    if (nef->compiler) {
        strncpy(compiler_buf, nef->compiler, 63);
    }
    memcpy(data + offset, compiler_buf, 64);
    offset += 64;
    
    // Add source (as fixed-size buffer)
    char source_buf[256] = {0};
    if (nef->source) {
        strncpy(source_buf, nef->source, 255);
    }
    memcpy(data + offset, source_buf, 256);
    offset += 256;
    
    // Add script length
    uint32_t script_length = (uint32_t)nef->script_length;
    memcpy(data + offset, &script_length, 4);
    offset += 4;
    
    // Add script
    memcpy(data + offset, nef->script, nef->script_length);
    offset += nef->script_length;
    
    // Add reserved field (currently unused by protocol)
    uint16_t reserved = 0;
    memcpy(data + offset, &reserved, 2);
    offset += 2;
    
    // Add tokens length
    uint32_t token_count = (uint32_t)nef->token_count;
    memcpy(data + offset, &token_count, 4);
    offset += 4;
    
    // Add tokens
    if (nef->token_count > 0) {
        memcpy(data + offset, nef->tokens, nef->token_count);
    }
    
    // Calculate checksum using NEF standard algorithm
    // Uses double SHA256 and takes first 4 bytes
    uint8_t hash1[32], hash2[32];
    if (neoc_sha256(data, data_size, hash1) != NEOC_SUCCESS ||
        neoc_sha256(hash1, 32, hash2) != NEOC_SUCCESS) {
        neoc_free(data);
        return 0;
    }
    uint32_t checksum;
    memcpy(&checksum, hash2, 4);
    
    neoc_free(data);
    return checksum;
}

// Validate NEF
bool neoc_contract_nef_validate(const neoc_contract_nef_t* nef) {
    if (!nef) {
        return false;
    }
    
    // Check magic number
    uint32_t expected_magic = 0x3346454E;
    if (nef->magic != expected_magic) {
        return false;
    }
    
    // Check script exists
    if (!nef->script || nef->script_length == 0) {
        return false;
    }
    
    // Verify checksum
    uint32_t calculated = neoc_contract_nef_calculate_checksum(nef);
    return calculated == nef->checksum;
}
