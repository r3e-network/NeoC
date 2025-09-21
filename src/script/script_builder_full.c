/**
 * @file script_builder_full.c
 * @brief Complete script builder implementation
 */

#include "neoc/script/script_builder_full.h"
#include "neoc/script/script_builder.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdlib.h>

struct neoc_script_builder_t {
    uint8_t *buffer;
    size_t size;
    size_t capacity;
};

neoc_error_t neoc_script_builder_create(neoc_script_builder_t **builder) {
    if (!builder) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *builder = neoc_calloc(1, sizeof(neoc_script_builder_t));
    if (!*builder) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    (*builder)->capacity = 256;
    (*builder)->buffer = neoc_malloc((*builder)->capacity);
    if (!(*builder)->buffer) {
        neoc_free(*builder);
        *builder = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    return NEOC_SUCCESS;
}

void neoc_script_builder_free(neoc_script_builder_t *builder) {
    if (builder) {
        if (builder->buffer) {
            neoc_free(builder->buffer);
        }
        neoc_free(builder);
    }
}

neoc_error_t neoc_script_builder_push_data(neoc_script_builder_t *builder,
                                            const uint8_t *data, size_t data_len) {
    if (!builder || !data) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Calculate required size
    size_t required_size = data_len;
    if (data_len == 0) {
        required_size = 1; // PUSH0
    } else if (data_len <= 75) {
        required_size = data_len + 1; // Direct push
    } else if (data_len <= 0xFF) {
        required_size = data_len + 2; // PUSHDATA1
    } else if (data_len <= 0xFFFF) {
        required_size = data_len + 3; // PUSHDATA2
    } else {
        required_size = data_len + 5; // PUSHDATA4
    }
    
    // Ensure capacity
    if (builder->size + required_size > builder->capacity) {
        size_t new_capacity = (builder->size + required_size) * 2;
        uint8_t *new_buffer = neoc_realloc(builder->buffer, new_capacity);
        if (!new_buffer) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        builder->buffer = new_buffer;
        builder->capacity = new_capacity;
    }
    
    // Add appropriate PUSHDATA opcode
    if (data_len == 0) {
        builder->buffer[builder->size++] = 0x10; // PUSH0
    } else if (data_len == 1 && data[0] >= 1 && data[0] <= 16) {
        // PUSH1-PUSH16 for small integers
        builder->buffer[builder->size++] = 0x10 + data[0];
    } else if (data_len <= 75) {
        builder->buffer[builder->size++] = (uint8_t)data_len;
    } else if (data_len <= 0xFF) {
        builder->buffer[builder->size++] = 0x0C; // PUSHDATA1
        builder->buffer[builder->size++] = (uint8_t)data_len;
    } else if (data_len <= 0xFFFF) {
        builder->buffer[builder->size++] = 0x0D; // PUSHDATA2
        builder->buffer[builder->size++] = (uint8_t)(data_len & 0xFF);
        builder->buffer[builder->size++] = (uint8_t)((data_len >> 8) & 0xFF);
    } else {
        builder->buffer[builder->size++] = 0x0E; // PUSHDATA4
        builder->buffer[builder->size++] = (uint8_t)(data_len & 0xFF);
        builder->buffer[builder->size++] = (uint8_t)((data_len >> 8) & 0xFF);
        builder->buffer[builder->size++] = (uint8_t)((data_len >> 16) & 0xFF);
        builder->buffer[builder->size++] = (uint8_t)((data_len >> 24) & 0xFF);
    }
    
    // Copy data if not a special case
    if (data_len > 0 && !(data_len == 1 && data[0] >= 1 && data[0] <= 16)) {
        memcpy(builder->buffer + builder->size, data, data_len);
        builder->size += data_len;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_builder_push_bool(neoc_script_builder_t *builder, bool value) {
    if (!builder) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Push boolean as PUSH0 (false) or PUSH1 (true)
    uint8_t opcode = value ? 0x51 : 0x00; // PUSH1 or PUSH0
    return neoc_script_builder_push_data(builder, &opcode, 1);
}

neoc_error_t neoc_script_builder_push_null(neoc_script_builder_t *builder) {
    if (!builder) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // PUSH0 opcode
    uint8_t opcode = 0x00;
    return neoc_script_builder_push_data(builder, &opcode, 1);
}

neoc_error_t neoc_script_builder_push_integer(neoc_script_builder_t *builder, int64_t value) {
    if (!builder) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Handle special cases for small integers
    if (value == -1) {
        return neoc_script_builder_emit_opcode(builder, 0x0F); // PUSHM1
    } else if (value >= 0 && value <= 16) {
        return neoc_script_builder_emit_opcode(builder, 0x10 + (uint8_t)value); // PUSH0-PUSH16
    }
    
    // Convert to little-endian bytes
    uint8_t bytes[8];
    int len = 0;
    int64_t temp = value;
    bool negative = value < 0;
    
    if (negative) {
        temp = -temp;
    }
    
    // Encode as variable length integer
    while (temp > 0 || len == 0) {
        bytes[len++] = (uint8_t)(temp & 0xFF);
        temp >>= 8;
        if (len >= 8) break;
    }
    
    // Add sign bit if necessary
    if (negative) {
        bytes[len - 1] |= 0x80;
    }
    
    return neoc_script_builder_push_data(builder, bytes, len);
}

neoc_error_t neoc_script_builder_push_hash160(neoc_script_builder_t *builder,
                                               const neoc_hash160_t *hash) {
    if (!builder || !hash) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return neoc_script_builder_push_data(builder, hash->data, 20);
}

neoc_error_t neoc_script_builder_emit_app_call(neoc_script_builder_t *builder,
                                                const neoc_hash160_t *script_hash,
                                                const char *method,
                                                uint8_t param_count) {
    if (!builder || !script_hash || !method) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Push method name
    size_t method_len = strlen(method);
    neoc_error_t err = neoc_script_builder_push_data(builder, (const uint8_t*)method, method_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push param count
    err = neoc_script_builder_push_integer(builder, param_count);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push script hash
    err = neoc_script_builder_push_data(builder, script_hash->data, 20);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Add SYSCALL opcode for System.Contract.Call
    // The interop service hash for System.Contract.Call is 0x627d5b52
    uint8_t syscall_bytes[5];
    syscall_bytes[0] = 0x41; // SYSCALL opcode
    syscall_bytes[1] = 0x52; // Hash bytes (little-endian)
    syscall_bytes[2] = 0x5b;
    syscall_bytes[3] = 0x7d;
    syscall_bytes[4] = 0x62;
    
    // Ensure capacity and append
    if (builder->size + 5 > builder->capacity) {
        size_t new_capacity = (builder->size + 5) * 2;
        uint8_t *new_buffer = neoc_realloc(builder->buffer, new_capacity);
        if (!new_buffer) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        builder->buffer = new_buffer;
        builder->capacity = new_capacity;
    }
    
    memcpy(builder->buffer + builder->size, syscall_bytes, 5);
    builder->size += 5;
    
    return NEOC_SUCCESS;
}


neoc_error_t neoc_script_builder_emit_push_int(neoc_script_builder_t *builder, int64_t value) {
    return neoc_script_builder_push_integer(builder, value);
}

neoc_error_t neoc_script_builder_emit_push_string(neoc_script_builder_t *builder, const char *str) {
    if (!builder || !str) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    return neoc_script_builder_push_data(builder, (const uint8_t*)str, strlen(str));
}

neoc_error_t neoc_script_builder_push_string(neoc_script_builder_t *builder, const char *str) {
    return neoc_script_builder_emit_push_string(builder, str);
}



neoc_error_t neoc_script_builder_push_param(neoc_script_builder_t *builder,
                                            const neoc_contract_parameter_t *param) {
    if (!builder || !param) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Push parameter based on type
    switch (param->type) {
        case NEOC_CONTRACT_PARAM_BOOLEAN: {
            return neoc_script_builder_push_bool(builder, param->value.boolean_value);
        }
        case NEOC_CONTRACT_PARAM_INTEGER: {
            return neoc_script_builder_push_integer(builder, param->value.integer_value);
        }
        case NEOC_CONTRACT_PARAM_BYTE_ARRAY: {
            if (param->value.byte_array.data) {
                return neoc_script_builder_push_data(builder, param->value.byte_array.data, param->value.byte_array.len);
            }
            return neoc_script_builder_push_data(builder, NULL, 0);
        }
        case NEOC_CONTRACT_PARAM_STRING: {
            if (param->value.string_value) {
                return neoc_script_builder_push_string(builder, param->value.string_value);
            }
            return neoc_script_builder_push_string(builder, "");
        }
        case NEOC_CONTRACT_PARAM_HASH160: {
            return neoc_script_builder_push_data(builder, param->value.hash160.data, 20);
        }
        case NEOC_CONTRACT_PARAM_HASH256: {
            return neoc_script_builder_push_data(builder, param->value.hash256.data, 32);
        }
        default:
            return NEOC_ERROR_INVALID_ARGUMENT;
    }
}

neoc_error_t neoc_script_builder_to_array(const neoc_script_builder_t *builder,
                                          uint8_t **script,
                                          size_t *script_len) {
    if (!builder || !script || !script_len) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *script = neoc_malloc(builder->size);
    if (!*script) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(*script, builder->buffer, builder->size);
    *script_len = builder->size;
    
    return NEOC_SUCCESS;
}

