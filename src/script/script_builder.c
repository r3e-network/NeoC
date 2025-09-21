#include "neoc/script/script_builder.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/script/opcode.h"
#include "neoc/script/interop_service.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/neoc_memory.h"
#include <stdlib.h>
#include <string.h>

// Script builder structure
struct neoc_script_builder_t {
    neoc_binary_writer_t *writer;
};

// Helper function to write var int
// Note: Enable when variable length encoding is needed
#if 0
static neoc_error_t write_var_int(neoc_binary_writer_t *writer, uint64_t value) {
    if (value < 0xFD) {
        return neoc_binary_writer_write_byte(writer, (uint8_t)value);
    } else if (value <= 0xFFFF) {
        neoc_error_t err = neoc_binary_writer_write_byte(writer, 0xFD);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_uint16(writer, (uint16_t)value);
    } else if (value <= 0xFFFFFFFF) {
        neoc_error_t err = neoc_binary_writer_write_byte(writer, 0xFE);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_uint32(writer, (uint32_t)value);
    } else {
        neoc_error_t err = neoc_binary_writer_write_byte(writer, 0xFF);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_uint64(writer, value);
    }
}
#endif

neoc_error_t neoc_script_builder_build_verification_script(const uint8_t *public_key,
                                                            size_t public_key_len,
                                                            uint8_t **script,
                                                            size_t *script_len) {
    if (!public_key || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (public_key_len != 33 && public_key_len != 65) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid public key length");
    }
    
    // Calculate script length
    // Format: PUSHDATA1 <len> <pubkey> SYSCALL <CheckSig>
    // 1 byte for PUSHDATA1, 1 byte for length, pubkey bytes, 1 byte for SYSCALL, 4 bytes for interop hash
    *script_len = 1 + 1 + public_key_len + 1 + 4;
    
    *script = malloc(*script_len);
    if (!*script) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate script");
    }
    
    uint8_t *ptr = *script;
    
    // PUSHDATA1 opcode
    *ptr++ = NEOC_OP_PUSHDATA1;
    
    // Length of public key
    *ptr++ = (uint8_t)public_key_len;
    
    // Public key bytes
    memcpy(ptr, public_key, public_key_len);
    ptr += public_key_len;
    
    // SYSCALL opcode
    *ptr++ = NEOC_OP_SYSCALL;
    
    // CheckSig interop service hash (little-endian)
    uint32_t checksig = neoc_interop_get_hash(NEOC_INTEROP_SYSTEM_CRYPTO_CHECKSIG);
    memcpy(ptr, &checksig, 4);
    ptr += 4;
    
    // Verify script length
    if ((size_t)(ptr - *script) != *script_len) {
        *script_len = ptr - *script;
    }
    
    return NEOC_SUCCESS;
}

// Script builder implementation
neoc_error_t neoc_script_builder_create(neoc_script_builder_t **builder) {
    if (!builder) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    *builder = neoc_calloc(1, sizeof(neoc_script_builder_t));
    if (!*builder) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    neoc_error_t err = neoc_binary_writer_create(256, true, &(*builder)->writer);
    if (err != NEOC_SUCCESS) {
        neoc_free(*builder);
        *builder = NULL;
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_builder_emit(neoc_script_builder_t *builder, neoc_opcode_t opcode) {
    if (!builder || !builder->writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    return neoc_binary_writer_write_byte(builder->writer, (uint8_t)opcode);
}

neoc_error_t neoc_script_builder_emit_with_data(neoc_script_builder_t *builder,
                                                 neoc_opcode_t opcode,
                                                 const uint8_t *data,
                                                 size_t data_len) {
    if (!builder || !builder->writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_error_t err = neoc_binary_writer_write_byte(builder->writer, (uint8_t)opcode);
    if (err != NEOC_SUCCESS) return err;
    
    if (data && data_len > 0) {
        return neoc_binary_writer_write_bytes(builder->writer, data, data_len);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_builder_emit_syscall(neoc_script_builder_t *builder,
                                               neoc_interop_service_t service) {
    if (!builder || !builder->writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_error_t err = neoc_binary_writer_write_byte(builder->writer, NEOC_OP_SYSCALL);
    if (err != NEOC_SUCCESS) return err;
    
    uint32_t hash = neoc_interop_get_hash(service);
    return neoc_binary_writer_write_uint32(builder->writer, hash);
}

neoc_error_t neoc_script_builder_push_integer(neoc_script_builder_t *builder, int64_t value) {
    if (!builder || !builder->writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (value == -1) {
        return neoc_script_builder_emit(builder, NEOC_OP_PUSHM1);
    } else if (value >= 0 && value <= 16) {
        return neoc_script_builder_emit(builder, NEOC_OP_PUSH0 + (uint8_t)value);
    } else if (value >= -128 && value <= 127) {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHINT8);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_byte(builder->writer, (uint8_t)value);
    } else if (value >= -32768 && value <= 32767) {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHINT16);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_int16(builder->writer, (int16_t)value);
    } else if (value >= -2147483648LL && value <= 2147483647LL) {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHINT32);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_int32(builder->writer, (int32_t)value);
    } else {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHINT64);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_int64(builder->writer, value);
    }
}

neoc_error_t neoc_script_builder_push_bool(neoc_script_builder_t *builder, bool value) {
    if (!builder) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    return neoc_script_builder_emit(builder, value ? NEOC_OP_PUSH1 : NEOC_OP_PUSH0);
}

neoc_error_t neoc_script_builder_push_data(neoc_script_builder_t *builder,
                                            const uint8_t *data,
                                            size_t data_len) {
    if (!builder || !builder->writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!data && data_len > 0) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (data_len == 0) {
        return neoc_script_builder_emit(builder, NEOC_OP_PUSH0);
    } else if (data_len < 0x4C) {
        // Direct push
        neoc_error_t err = neoc_binary_writer_write_byte(builder->writer, (uint8_t)data_len);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_bytes(builder->writer, data, data_len);
    } else if (data_len <= 0xFF) {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHDATA1);
        if (err != NEOC_SUCCESS) return err;
        err = neoc_binary_writer_write_byte(builder->writer, (uint8_t)data_len);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_bytes(builder->writer, data, data_len);
    } else if (data_len <= 0xFFFF) {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHDATA2);
        if (err != NEOC_SUCCESS) return err;
        err = neoc_binary_writer_write_uint16(builder->writer, (uint16_t)data_len);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_bytes(builder->writer, data, data_len);
    } else {
        neoc_error_t err = neoc_script_builder_emit(builder, NEOC_OP_PUSHDATA4);
        if (err != NEOC_SUCCESS) return err;
        err = neoc_binary_writer_write_uint32(builder->writer, (uint32_t)data_len);
        if (err != NEOC_SUCCESS) return err;
        return neoc_binary_writer_write_bytes(builder->writer, data, data_len);
    }
}

neoc_error_t neoc_script_builder_push_string(neoc_script_builder_t *builder, const char *str) {
    if (!builder || !str) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    return neoc_script_builder_push_data(builder, (const uint8_t*)str, strlen(str));
}

neoc_error_t neoc_script_builder_contract_call(neoc_script_builder_t *builder,
                                               const neoc_hash160_t *script_hash,
                                               const char *method,
                                               const neoc_contract_parameter_t **params,
                                               size_t param_count,
                                               neoc_call_flags_t call_flags) {
    (void)params; // Note: implement parameter pushing
    if (!builder || !script_hash || !method) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_error_t err;
    
    // Push parameters in reverse order
    if (param_count == 0) {
        err = neoc_script_builder_emit(builder, NEOC_OP_NEWARRAY0);
        if (err != NEOC_SUCCESS) return err;
    } else {
        // Push parameters onto stack before packing
        // Parameters should already be pushed onto the stack by the caller
        // Pack them into an array for the contract call
        err = neoc_script_builder_push_integer(builder, (int64_t)param_count);
        if (err != NEOC_SUCCESS) return err;
        err = neoc_script_builder_emit(builder, NEOC_OP_PACK);
        if (err != NEOC_SUCCESS) return err;
    }
    
    // Push call flags
    err = neoc_script_builder_push_integer(builder, (int64_t)call_flags);
    if (err != NEOC_SUCCESS) return err;
    
    // Push method name
    err = neoc_script_builder_push_string(builder, method);
    if (err != NEOC_SUCCESS) return err;
    
    // Push script hash (little endian)
    uint8_t hash_bytes[20];
    err = neoc_hash160_to_bytes(script_hash, hash_bytes, sizeof(hash_bytes));
    if (err != NEOC_SUCCESS) return err;
    err = neoc_script_builder_push_data(builder, hash_bytes, 20);
    if (err != NEOC_SUCCESS) return err;
    
    // Emit syscall
    return neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
}

neoc_error_t neoc_script_builder_to_array(const neoc_script_builder_t *builder,
                                          uint8_t **script,
                                          size_t *script_len) {
    if (!builder || !builder->writer || !script || !script_len) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    return neoc_binary_writer_to_array(builder->writer, script, script_len);
}

size_t neoc_script_builder_get_size(const neoc_script_builder_t *builder) {
    if (!builder || !builder->writer) {
        return 0;
    }
    
    return neoc_binary_writer_get_position(builder->writer);
}

void neoc_script_builder_reset(neoc_script_builder_t *builder) {
    if (builder && builder->writer) {
        neoc_binary_writer_reset(builder->writer);
    }
}

void neoc_script_builder_free(neoc_script_builder_t *builder) {
    if (builder) {
        if (builder->writer) {
            neoc_binary_writer_free(builder->writer);
        }
        neoc_free(builder);
    }
}
