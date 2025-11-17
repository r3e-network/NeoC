/**
 * @file transaction_attribute.c
 * @brief Implementation of Neo transaction attributes
 */

#include "neoc/protocol/response/transaction_attribute.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"
#include "neoc/types/neoc_hash256.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_RESULT_SIZE 0xFFFF

// Create high priority attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_high_priority(void) {
    neoc_transaction_attribute_t* attr = neoc_malloc(sizeof(neoc_transaction_attribute_t));
    if (!attr) return NULL;
    
    attr->type = NEOC_TX_ATTR_HIGH_PRIORITY;
    return attr;
}

// Create oracle response attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_oracle_response(
    uint64_t id,
    neoc_oracle_response_code_t code,
    const uint8_t* result,
    size_t result_size
) {
    if (result_size > MAX_RESULT_SIZE) return NULL;
    
    neoc_transaction_attribute_t* attr = neoc_malloc(sizeof(neoc_transaction_attribute_t));
    if (!attr) return NULL;
    
    attr->type = NEOC_TX_ATTR_ORACLE_RESPONSE;
    attr->data.oracle_response.id = id;
    attr->data.oracle_response.code = code;
    attr->data.oracle_response.result_size = result_size;
    
    if (result_size > 0) {
        attr->data.oracle_response.result = neoc_malloc(result_size);
        if (!attr->data.oracle_response.result) {
            neoc_free(attr);
            return NULL;
        }
        memcpy(attr->data.oracle_response.result, result, result_size);
    } else {
        attr->data.oracle_response.result = NULL;
    }
    
    return attr;
}

// Create not valid before attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_not_valid_before(uint32_t height) {
    neoc_transaction_attribute_t* attr = neoc_malloc(sizeof(neoc_transaction_attribute_t));
    if (!attr) return NULL;
    
    attr->type = NEOC_TX_ATTR_NOT_VALID_BEFORE;
    attr->data.not_valid_before.height = height;
    return attr;
}

// Create conflicts attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_conflicts(const uint8_t* hash) {
    neoc_transaction_attribute_t* attr = neoc_malloc(sizeof(neoc_transaction_attribute_t));
    if (!attr) return NULL;
    
    attr->type = NEOC_TX_ATTR_CONFLICTS;
    if (hash) {
        memcpy(attr->data.conflicts.hash, hash, 32);
    } else {
        memset(attr->data.conflicts.hash, 0, 32);
    }
    return attr;
}

// Free transaction attribute
void neoc_transaction_attribute_free(neoc_transaction_attribute_t* attr) {
    if (!attr) return;
    
    if (attr->type == NEOC_TX_ATTR_ORACLE_RESPONSE && attr->data.oracle_response.result) {
        neoc_free(attr->data.oracle_response.result);
    }
    
    neoc_free(attr);
}

// Clone transaction attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_clone(const neoc_transaction_attribute_t* attr) {
    if (!attr) return NULL;
    
    switch (attr->type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY:
            return neoc_transaction_attribute_create_high_priority();
            
        case NEOC_TX_ATTR_ORACLE_RESPONSE:
            return neoc_transaction_attribute_create_oracle_response(
                attr->data.oracle_response.id,
                attr->data.oracle_response.code,
                attr->data.oracle_response.result,
                attr->data.oracle_response.result_size
            );
            
        case NEOC_TX_ATTR_NOT_VALID_BEFORE:
            return neoc_transaction_attribute_create_not_valid_before(
                attr->data.not_valid_before.height
            );
            
        case NEOC_TX_ATTR_CONFLICTS:
            return neoc_transaction_attribute_create_conflicts(
                attr->data.conflicts.hash
            );
            
        default:
            return NULL;
    }
}



// Compare transaction attributes
bool neoc_transaction_attribute_equals(
    const neoc_transaction_attribute_t* a,
    const neoc_transaction_attribute_t* b
) {
    if (!a || !b) return a == b;
    if (a->type != b->type) return false;
    
    switch (a->type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY:
            return true;
            
        case NEOC_TX_ATTR_ORACLE_RESPONSE:
            return a->data.oracle_response.id == b->data.oracle_response.id &&
                   a->data.oracle_response.code == b->data.oracle_response.code &&
                   a->data.oracle_response.result_size == b->data.oracle_response.result_size &&
                   (a->data.oracle_response.result_size == 0 || 
                    memcmp(a->data.oracle_response.result, b->data.oracle_response.result, 
                           a->data.oracle_response.result_size) == 0);
            
        case NEOC_TX_ATTR_NOT_VALID_BEFORE:
            return a->data.not_valid_before.height == b->data.not_valid_before.height;
            
        case NEOC_TX_ATTR_CONFLICTS:
            return memcmp(a->data.conflicts.hash, b->data.conflicts.hash, 32) == 0;
            
        default:
            return false;
    }
}

// Get attribute type name
const char* neoc_transaction_attribute_type_name(neoc_transaction_attribute_type_t type) {
    switch (type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY: return "HighPriority";
        case NEOC_TX_ATTR_ORACLE_RESPONSE: return "OracleResponse";
        case NEOC_TX_ATTR_NOT_VALID_BEFORE: return "NotValidBefore";
        case NEOC_TX_ATTR_CONFLICTS: return "Conflicts";
        default: return "Unknown";
    }
}

// Parse from JSON
neoc_transaction_attribute_t* neoc_transaction_attribute_from_json(const char* json_str) {
    if (!json_str) return NULL;
    
    // Parse JSON
    neoc_json_t* json = neoc_json_parse(json_str);
    if (!json) {
        return NULL;
    }
    
    neoc_transaction_attribute_t* attr = NULL;
    const char* type = neoc_json_get_string(json, "type");
    
    if (!type) {
        neoc_json_free(json);
        return NULL;
    }
    
    if (strcmp(type, "HighPriority") == 0) {
        attr = neoc_transaction_attribute_create_high_priority();
    } else if (strcmp(type, "OracleResponse") == 0) {
        double id_val = 0;
        double code_val = 0;
        neoc_json_get_number(json, "id", &id_val);
        neoc_json_get_number(json, "code", &code_val);
        uint64_t id = (uint64_t)id_val;
        neoc_oracle_response_code_t code = (neoc_oracle_response_code_t)code_val;
        attr = neoc_transaction_attribute_create_oracle_response(id, code, NULL, 0);
    } else if (strcmp(type, "NotValidBefore") == 0) {
        double height_val = 0;
        neoc_json_get_number(json, "height", &height_val);
        uint32_t height = (uint32_t)height_val;
        attr = neoc_transaction_attribute_create_not_valid_before(height);
    } else if (strcmp(type, "Conflicts") == 0) {
        // Need to parse hash from JSON
        const char* hash_str = neoc_json_get_string(json, "hash");
        if (hash_str) {
            neoc_hash256_t hash;
            if (neoc_hash256_from_string(hash_str, &hash) == NEOC_SUCCESS) {
                attr = neoc_transaction_attribute_create_conflicts(hash.data);
            }
        }
    }
    
    neoc_json_free(json);
    return attr;
}

// Convert to JSON
char* neoc_transaction_attribute_to_json(const neoc_transaction_attribute_t* attr) {
    if (!attr) return NULL;
    
    char* json = neoc_malloc(1024); // Allocate sufficient space
    if (!json) return NULL;
    
    switch (attr->type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY:
            snprintf(json, 1024, "{\"type\":\"HighPriority\"}");
            break;
            
        case NEOC_TX_ATTR_ORACLE_RESPONSE:
            snprintf(json, 1024, 
                     "{\"type\":\"OracleResponse\",\"id\":%llu,\"code\":%d}",
                     (unsigned long long)attr->data.oracle_response.id,
                     attr->data.oracle_response.code);
            break;
            
        case NEOC_TX_ATTR_NOT_VALID_BEFORE:
            snprintf(json, 1024, 
                     "{\"type\":\"NotValidBefore\",\"height\":%u}",
                     attr->data.not_valid_before.height);
            break;
            
        case NEOC_TX_ATTR_CONFLICTS:
            snprintf(json, 1024, "{\"type\":\"Conflicts\"}");
            break;
            
        default:
            neoc_free(json);
            return NULL;
    }
    
    return json;
}

// Get serialized size
size_t neoc_transaction_attribute_get_size(const neoc_transaction_attribute_t* attr) {
    if (!attr) return 0;
    
    switch (attr->type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY:
            return 1; // Just the type byte
            
        case NEOC_TX_ATTR_ORACLE_RESPONSE:
            return 1 + 8 + 1 + 1 + attr->data.oracle_response.result_size; // type + id + code + varlen + data
            
        case NEOC_TX_ATTR_NOT_VALID_BEFORE:
            return 1 + 4; // type + height
            
        case NEOC_TX_ATTR_CONFLICTS:
            return 1 + 32; // type + hash
            
        default:
            return 0;
    }
}

// Serialize to bytes
uint8_t* neoc_transaction_attribute_serialize(const neoc_transaction_attribute_t* attr, size_t* out_length) {
    if (!attr || !out_length) return NULL;
    
    size_t size = neoc_transaction_attribute_get_size(attr);
    if (size == 0) return NULL;
    
    uint8_t* data = neoc_malloc(size);
    if (!data) return NULL;
    
    size_t offset = 0;
    data[offset++] = (uint8_t)attr->type;
    
    switch (attr->type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY:
            // No additional data
            break;
            
        case NEOC_TX_ATTR_ORACLE_RESPONSE:
            // Serialize ID (little endian)
            for (int i = 0; i < 8; i++) {
                data[offset++] = (attr->data.oracle_response.id >> (i * 8)) & 0xFF;
            }
            data[offset++] = (uint8_t)attr->data.oracle_response.code;
            data[offset++] = (uint8_t)attr->data.oracle_response.result_size;
            if (attr->data.oracle_response.result_size > 0) {
                memcpy(&data[offset], attr->data.oracle_response.result, 
                       attr->data.oracle_response.result_size);
                offset += attr->data.oracle_response.result_size;
            }
            break;
            
        case NEOC_TX_ATTR_NOT_VALID_BEFORE:
            // Serialize height (little endian)
            for (int i = 0; i < 4; i++) {
                data[offset++] = (attr->data.not_valid_before.height >> (i * 8)) & 0xFF;
            }
            break;
            
        case NEOC_TX_ATTR_CONFLICTS:
            memcpy(&data[offset], attr->data.conflicts.hash, 32);
            offset += 32;
            break;
    }
    
    *out_length = offset;
    return data;
}

// Deserialize from bytes
neoc_transaction_attribute_t* neoc_transaction_attribute_deserialize(const uint8_t* data, size_t length) {
    if (!data || length == 0) return NULL;
    
    neoc_transaction_attribute_type_t type = (neoc_transaction_attribute_type_t)data[0];
    size_t offset = 1;
    
    switch (type) {
        case NEOC_TX_ATTR_HIGH_PRIORITY:
            if (length != 1) return NULL;
            return neoc_transaction_attribute_create_high_priority();
            
        case NEOC_TX_ATTR_ORACLE_RESPONSE: {
            if (length < 11) return NULL; // minimum: type + id + code + result_size
            
            uint64_t id = 0;
            for (int i = 0; i < 8; i++) {
                id |= ((uint64_t)data[offset++]) << (i * 8);
            }
            
            neoc_oracle_response_code_t code = (neoc_oracle_response_code_t)data[offset++];
            size_t result_size = data[offset++];
            
            if (offset + result_size != length) return NULL;
            
            const uint8_t* result = (result_size > 0) ? &data[offset] : NULL;
            return neoc_transaction_attribute_create_oracle_response(id, code, result, result_size);
        }
        
        case NEOC_TX_ATTR_NOT_VALID_BEFORE: {
            if (length != 5) return NULL;
            
            uint32_t height = 0;
            for (int i = 0; i < 4; i++) {
                height |= ((uint32_t)data[offset++]) << (i * 8);
            }
            
            return neoc_transaction_attribute_create_not_valid_before(height);
        }
        
        case NEOC_TX_ATTR_CONFLICTS: {
            if (length != 33) return NULL;
            return neoc_transaction_attribute_create_conflicts(&data[offset]);
        }
        
        default:
            return NULL;
    }
}
