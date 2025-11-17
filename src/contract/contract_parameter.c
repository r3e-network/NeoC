#include "neoc/contract/contract_parameter.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <stdlib.h>
#include <string.h>

neoc_error_t neoc_contract_param_create_any(void *value,
                                             neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_ANY;
    (*param)->value.interop_interface = value;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_boolean(bool value,
                                                 neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_BOOLEAN;
    (*param)->value.boolean_value = value;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_integer(int64_t value,
                                                 neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_INTEGER;
    (*param)->value.integer_value = value;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_byte_array(const uint8_t *data, size_t len,
                                                    neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_BYTE_ARRAY;
    
    if (data && len > 0) {
        (*param)->value.byte_array.data = malloc(len);
        if (!(*param)->value.byte_array.data) {
            free(*param);
            *param = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate byte array");
        }
        memcpy((*param)->value.byte_array.data, data, len);
        (*param)->value.byte_array.len = len;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_string(const char *value,
                                                neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_STRING;
    
    if (value) {
        (*param)->value.string_value = neoc_strdup(value);
        if (!(*param)->value.string_value) {
            free(*param);
            *param = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate string");
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_hash160(const neoc_hash160_t *hash,
                                                 neoc_contract_parameter_t **param) {
    if (!hash || !param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_HASH160;
    memcpy(&(*param)->value.hash160, hash, sizeof(neoc_hash160_t));
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_hash256(const neoc_hash256_t *hash,
                                                 neoc_contract_parameter_t **param) {
    if (!hash || !param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_HASH256;
    memcpy(&(*param)->value.hash256, hash, sizeof(neoc_hash256_t));
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_public_key(const uint8_t public_key[33],
                                                    neoc_contract_parameter_t **param) {
    if (!public_key || !param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_PUBLIC_KEY;
    memcpy((*param)->value.public_key.data, public_key, 33);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_signature(const uint8_t signature[64],
                                                   neoc_contract_parameter_t **param) {
    if (!signature || !param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_SIGNATURE;
    memcpy((*param)->value.signature.data, signature, 64);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_array(neoc_contract_parameter_t **items,
                                               size_t count,
                                               neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_ARRAY;
    
    if (items && count > 0) {
        (*param)->value.array.items = malloc(count * sizeof(neoc_contract_parameter_t*));
        if (!(*param)->value.array.items) {
            free(*param);
            *param = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate array items");
        }
        memcpy((*param)->value.array.items, items, count * sizeof(neoc_contract_parameter_t*));
        (*param)->value.array.count = count;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_create_map(neoc_contract_parameter_t **keys,
                                             neoc_contract_parameter_t **values,
                                             size_t count,
                                             neoc_contract_parameter_t **param) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter pointer");
    }
    
    if ((keys && !values) || (!keys && values)) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Keys and values must both be provided or both be NULL");
    }
    
    *param = calloc(1, sizeof(neoc_contract_parameter_t));
    if (!*param) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameter");
    }
    
    (*param)->type = NEOC_CONTRACT_PARAM_MAP;
    
    if (keys && values && count > 0) {
        (*param)->value.map.keys = malloc(count * sizeof(neoc_contract_parameter_t*));
        (*param)->value.map.values = malloc(count * sizeof(neoc_contract_parameter_t*));
        
        if (!(*param)->value.map.keys || !(*param)->value.map.values) {
            if ((*param)->value.map.keys) free((*param)->value.map.keys);
            if ((*param)->value.map.values) free((*param)->value.map.values);
            free(*param);
            *param = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate map arrays");
        }
        
        memcpy((*param)->value.map.keys, keys, count * sizeof(neoc_contract_parameter_t*));
        memcpy((*param)->value.map.values, values, count * sizeof(neoc_contract_parameter_t*));
        (*param)->value.map.count = count;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_param_set_name(neoc_contract_parameter_t *param,
                                           const char *name) {
    if (!param) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter");
    }
    
    // Free old name if exists
    if (param->name) {
        free(param->name);
        param->name = NULL;
    }
    
    // Set new name if provided
    if (name) {
        param->name = neoc_strdup(name);
        if (!param->name) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate name");
        }
    }
    
    return NEOC_SUCCESS;
}

const char* neoc_contract_param_type_to_string(neoc_contract_param_type_t type) {
    switch (type) {
        case NEOC_CONTRACT_PARAM_ANY: return "Any";
        case NEOC_CONTRACT_PARAM_BOOLEAN: return "Boolean";
        case NEOC_CONTRACT_PARAM_INTEGER: return "Integer";
        case NEOC_CONTRACT_PARAM_BYTE_ARRAY: return "ByteArray";
        case NEOC_CONTRACT_PARAM_STRING: return "String";
        case NEOC_CONTRACT_PARAM_HASH160: return "Hash160";
        case NEOC_CONTRACT_PARAM_HASH256: return "Hash256";
        case NEOC_CONTRACT_PARAM_PUBLIC_KEY: return "PublicKey";
        case NEOC_CONTRACT_PARAM_SIGNATURE: return "Signature";
        case NEOC_CONTRACT_PARAM_ARRAY: return "Array";
        case NEOC_CONTRACT_PARAM_MAP: return "Map";
        case NEOC_CONTRACT_PARAM_INTEROP_INTERFACE: return "InteropInterface";
        case NEOC_CONTRACT_PARAM_VOID: return "Void";
        default: return "Unknown";
    }
}

neoc_error_t neoc_contract_param_serialize(const neoc_contract_parameter_t *param,
                                            uint8_t **bytes,
                                            size_t *bytes_len) {
    if (!param || !bytes || !bytes_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create binary writer with initial capacity and auto-grow
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(256, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Serialize based on parameter type
    switch (param->type) {
        case NEOC_CONTRACT_PARAM_BOOLEAN:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_BOOLEAN);
            neoc_binary_writer_write_byte(writer, param->value.boolean_value ? 1 : 0);
            break;
            
        case NEOC_CONTRACT_PARAM_INTEGER:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_INTEGER);
            neoc_binary_writer_write_uint64(writer, param->value.integer_value);
            break;
            
        case NEOC_CONTRACT_PARAM_BYTE_ARRAY:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_BYTE_ARRAY);
            neoc_binary_writer_write_var_bytes(writer, param->value.byte_array.data,
                                              param->value.byte_array.len);
            break;
            
        case NEOC_CONTRACT_PARAM_STRING:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_STRING);
            neoc_binary_writer_write_var_string(writer, param->value.string_value);
            break;
            
        case NEOC_CONTRACT_PARAM_HASH160:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_HASH160);
            neoc_binary_writer_write_bytes(writer, param->value.hash160.data, 20);
            break;
            
        case NEOC_CONTRACT_PARAM_HASH256:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_HASH256);
            neoc_binary_writer_write_bytes(writer, param->value.hash256.data, 32);
            break;
            
        case NEOC_CONTRACT_PARAM_PUBLIC_KEY:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_PUBLIC_KEY);
            neoc_binary_writer_write_bytes(writer, param->value.public_key.data, 33);
            break;
            
        case NEOC_CONTRACT_PARAM_SIGNATURE:
            neoc_binary_writer_write_byte(writer, NEOC_CONTRACT_PARAM_SIGNATURE);
            neoc_binary_writer_write_bytes(writer, param->value.signature.data, 64);
            break;
            
        default:
            neoc_binary_writer_free(writer);
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unknown parameter type");
    }
    
    // Get the serialized bytes
    err = neoc_binary_writer_to_array(writer, bytes, bytes_len);
    neoc_binary_writer_free(writer);
    
    return err;
}

void neoc_contract_param_free(neoc_contract_parameter_t *param) {
    if (!param) return;
    
    // Free name if allocated
    if (param->name) {
        free(param->name);
    }
    
    // Free value based on type
    switch (param->type) {
        case NEOC_CONTRACT_PARAM_BYTE_ARRAY:
            if (param->value.byte_array.data) {
                free(param->value.byte_array.data);
            }
            break;
            
        case NEOC_CONTRACT_PARAM_STRING:
            if (param->value.string_value) {
                free(param->value.string_value);
            }
            break;
            
        case NEOC_CONTRACT_PARAM_ARRAY:
            if (param->value.array.items) {
                // Note: We don't free the individual items here
                // The caller is responsible for freeing them
                free(param->value.array.items);
            }
            break;
            
        case NEOC_CONTRACT_PARAM_MAP:
            if (param->value.map.keys) {
                free(param->value.map.keys);
            }
            if (param->value.map.values) {
                free(param->value.map.values);
            }
            break;
            
        default:
            // Other types don't need special cleanup
            break;
    }
    
    free(param);
}
