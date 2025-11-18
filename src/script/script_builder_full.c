#include "neoc/script/script_builder_full.h"
#include "neoc/contract/contract_parameter.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/script/interop_service.h"
#include "neoc/neoc_memory.h"
#include <string.h>

static neoc_error_t neoc_script_builder_push_array_internal(neoc_script_builder_t *builder,
                                                             neoc_contract_parameter_t *const *items,
                                                             size_t count);

static neoc_error_t neoc_script_builder_push_map_internal(neoc_script_builder_t *builder,
                                                           neoc_contract_parameter_t *const *keys,
                                                           neoc_contract_parameter_t *const *values,
                                                           size_t count);

neoc_error_t neoc_script_builder_push_null(neoc_script_builder_t *builder) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Script builder is NULL");
    }
    return neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
}

neoc_error_t neoc_script_builder_push_hash160(neoc_script_builder_t *builder,
                                               const neoc_hash160_t *hash) {
    if (!builder || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid hash160 arguments");
    }

    uint8_t buffer[NEOC_HASH160_SIZE];
    neoc_error_t err = neoc_hash160_to_little_endian_bytes(hash, buffer, sizeof(buffer));
    if (err != NEOC_SUCCESS) {
        return err;
    }
    return neoc_script_builder_push_data(builder, buffer, sizeof(buffer));
}

neoc_error_t neoc_script_builder_emit_push_int(neoc_script_builder_t *builder, int64_t value) {
    return neoc_script_builder_push_integer(builder, value);
}

static neoc_error_t push_byte_array(neoc_script_builder_t *builder,
                                     const uint8_t *data,
                                     size_t length) {
    if (!data && length > 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Byte array pointer is NULL");
    }
    if (length == 0) {
        return neoc_script_builder_emit(builder, NEOC_OP_PUSH0);
    }
    return neoc_script_builder_push_data(builder, data, length);
}

static neoc_error_t push_hash256(neoc_script_builder_t *builder,
                                  const neoc_hash256_t *hash) {
    if (!hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "hash256 pointer is NULL");
    }
    uint8_t buffer[NEOC_HASH256_SIZE];
    neoc_error_t err = neoc_hash256_to_little_endian_bytes(hash, buffer, sizeof(buffer));
    if (err != NEOC_SUCCESS) {
        return err;
    }
    return neoc_script_builder_push_data(builder, buffer, sizeof(buffer));
}

neoc_error_t neoc_script_builder_push_param(neoc_script_builder_t *builder,
                                            const neoc_contract_parameter_t *param) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Builder is NULL");
    }

    if (!param) {
        return neoc_script_builder_push_null(builder);
    }

    switch (param->type) {
        case NEOC_CONTRACT_PARAM_ANY:
        case NEOC_CONTRACT_PARAM_VOID:
            return NEOC_SUCCESS;
        case NEOC_CONTRACT_PARAM_BOOLEAN:
            return neoc_script_builder_push_bool(builder, param->value.boolean_value);
        case NEOC_CONTRACT_PARAM_INTEGER:
            return neoc_script_builder_push_integer(builder, param->value.integer_value);
        case NEOC_CONTRACT_PARAM_BYTE_ARRAY:
        case NEOC_CONTRACT_PARAM_SIGNATURE:
            return push_byte_array(builder,
                                   param->value.byte_array.data,
                                   param->value.byte_array.len);
        case NEOC_CONTRACT_PARAM_PUBLIC_KEY:
            return neoc_script_builder_push_data(builder,
                                                 param->value.public_key.data,
                                                 sizeof(param->value.public_key.data));
        case NEOC_CONTRACT_PARAM_STRING:
            return neoc_script_builder_push_string(builder, param->value.string_value ? param->value.string_value : "");
        case NEOC_CONTRACT_PARAM_HASH160:
            return neoc_script_builder_push_hash160(builder, &param->value.hash160);
        case NEOC_CONTRACT_PARAM_HASH256:
            return push_hash256(builder, &param->value.hash256);
        case NEOC_CONTRACT_PARAM_ARRAY:
            return neoc_script_builder_push_array_internal(builder,
                                                            param->value.array.items,
                                                            param->value.array.count);
        case NEOC_CONTRACT_PARAM_MAP:
            return neoc_script_builder_push_map_internal(builder,
                                                          param->value.map.keys,
                                                          param->value.map.values,
                                                          param->value.map.count);
        case NEOC_CONTRACT_PARAM_INTEROP_INTERFACE:
            return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "Interop interface parameters are not supported");
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unsupported contract parameter type");
    }
}

neoc_error_t neoc_script_builder_push_params(neoc_script_builder_t *builder,
                                             const neoc_contract_parameter_t **params,
                                             size_t count) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Builder is NULL");
    }
    if (count > 0 && !params) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Parameter array is NULL");
    }

    if (count == 0) {
        return neoc_script_builder_emit(builder, NEOC_OP_NEWARRAY0);
    }

    for (size_t i = 0; i < count; i++) {
        neoc_error_t err = neoc_script_builder_push_param(builder, params[i]);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }

    neoc_error_t err = neoc_script_builder_push_integer(builder, (int64_t)count);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    return neoc_script_builder_emit(builder, NEOC_OP_PACK);
}

neoc_error_t neoc_script_builder_build_multisig_script(int minimum_signatures,
                                                       const uint8_t **public_keys,
                                                       const size_t *public_key_lens,
                                                       size_t public_key_count,
                                                       uint8_t **script,
                                                       size_t *script_len) {
    if (!public_keys || !public_key_lens || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid multisig arguments");
    }
    if (public_key_count == 0 || minimum_signatures <= 0 ||
        (size_t)minimum_signatures > public_key_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid multisig signature counts");
    }

    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_script_builder_push_integer(builder, (int64_t)minimum_signatures);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }

    for (size_t i = 0; i < public_key_count; i++) {
        if (!public_keys[i] || public_key_lens[i] == 0) {
            neoc_script_builder_free(builder);
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid public key");
        }
        err = neoc_script_builder_push_data(builder, public_keys[i], public_key_lens[i]);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            return err;
        }
    }

    err = neoc_script_builder_push_integer(builder, (int64_t)public_key_count);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }

    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }

    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);

    if (err != NEOC_SUCCESS && script) {
        if (*script) {
            neoc_free(*script);
            *script = NULL;
        }
        if (script_len) {
            *script_len = 0;
        }
    }

    return err;
}

static neoc_error_t neoc_script_builder_push_array_internal(neoc_script_builder_t *builder,
                                                             neoc_contract_parameter_t *const *items,
                                                             size_t count) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Builder is NULL");
    }

    if (count == 0) {
        return neoc_script_builder_emit(builder, NEOC_OP_NEWARRAY0);
    }

    for (size_t i = 0; i < count; i++) {
        neoc_error_t err = neoc_script_builder_push_param(builder, items[i]);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }

    neoc_error_t err = neoc_script_builder_push_integer(builder, (int64_t)count);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_script_builder_emit(builder, NEOC_OP_PACK);
}

static neoc_error_t neoc_script_builder_push_map_internal(neoc_script_builder_t *builder,
                                                           neoc_contract_parameter_t *const *keys,
                                                           neoc_contract_parameter_t *const *values,
                                                           size_t count) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Builder is NULL");
    }

    if (count == 0) {
        return neoc_script_builder_emit(builder, NEOC_OP_NEWMAP);
    }

    if (!keys || !values) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Map keys or values array is NULL");
    }

    for (size_t i = 0; i < count; i++) {
        neoc_error_t err = neoc_script_builder_push_param(builder, values[i]);
        if (err != NEOC_SUCCESS) {
            return err;
        }
        err = neoc_script_builder_push_param(builder, keys[i]);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }

    neoc_error_t err = neoc_script_builder_push_integer(builder, (int64_t)count);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    return neoc_script_builder_emit(builder, NEOC_OP_PACKMAP);
}

neoc_error_t neoc_script_builder_emit_app_call(neoc_script_builder_t *builder,
                                               const neoc_hash160_t *script_hash,
                                               const char *method,
                                               uint8_t param_count) {
    if (!builder || !script_hash || !method) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid app call arguments");
    }

    neoc_error_t err;
    if (param_count == 0) {
        err = neoc_script_builder_emit(builder, NEOC_OP_NEWARRAY0);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    } else {
        err = neoc_script_builder_push_integer(builder, param_count);
        if (err != NEOC_SUCCESS) {
            return err;
        }
        err = neoc_script_builder_emit(builder, NEOC_OP_PACK);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }

    err = neoc_script_builder_push_integer(builder, (int64_t)NEOC_CALL_FLAGS_ALL);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_script_builder_push_string(builder, method);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint8_t hash_le[NEOC_HASH160_SIZE];
    err = neoc_hash160_to_little_endian_bytes(script_hash, hash_le, sizeof(hash_le));
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_script_builder_push_data(builder, hash_le, sizeof(hash_le));
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
}
