/**
 * @file script_helper.c
 * @brief Helper functions for creating verification and invocation scripts
 */

#include "neoc/script/script_helper.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/script/opcode.h"
#include "neoc/script/interop_service.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/crypto/base58.h"
#include "neoc/neoc_memory.h"
#include "neoc/neo_constants.h"
#include <string.h>

neoc_error_t neoc_script_create_single_sig_verification(const uint8_t *public_key,
                                                         size_t public_key_len,
                                                         uint8_t **script,
                                                         size_t *script_len) {
    if (!public_key || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (public_key_len != 33 && public_key_len != 65) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Invalid public key size");
    }
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push public key
    err = neoc_script_builder_push_data(builder, public_key, public_key_len);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Emit CHECKSIG
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CRYPTO_CHECKSIG);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);
    
    return err;
}

neoc_error_t neoc_script_create_multi_sig_verification(const uint8_t **public_keys,
                                                        size_t *key_lengths,
                                                        size_t key_count,
                                                        size_t min_signatures,
                                                        uint8_t **script,
                                                        size_t *script_len) {
    if (!public_keys || !key_lengths || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (key_count == 0 || key_count > 1024) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Invalid key count");
    }
    
    if (min_signatures == 0 || min_signatures > key_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid minimum signatures");
    }
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push minimum signatures required
    err = neoc_script_builder_push_integer(builder, (int64_t)min_signatures);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push all public keys
    for (size_t i = 0; i < key_count; i++) {
        if (key_lengths[i] != 33 && key_lengths[i] != 65) {
            neoc_script_builder_free(builder);
            return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Invalid public key size");
        }
        
        err = neoc_script_builder_push_data(builder, public_keys[i], key_lengths[i]);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            return err;
        }
    }
    
    // Push key count
    err = neoc_script_builder_push_integer(builder, (int64_t)key_count);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Emit CHECKMULTISIG
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);
    
    return err;
}

neoc_error_t neoc_script_create_contract_call(const neoc_hash160_t *contract_hash,
                                              const char *method,
                                              const neoc_contract_parameter_t *params,
                                              size_t param_count,
                                              uint8_t **script,
                                              size_t *script_len) {
    if (!contract_hash || !method || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push parameters in reverse order
    if (params && param_count > 0) {
        for (int i = (int)param_count - 1; i >= 0; i--) {
            err = neoc_script_builder_push_param(builder, &params[i]);
            if (err != NEOC_SUCCESS) {
                neoc_script_builder_free(builder);
                return err;
            }
        }
    }
    
    // Push parameter count
    err = neoc_script_builder_push_integer(builder, (int64_t)param_count);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push method name
    err = neoc_script_builder_push_string(builder, method);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push contract hash
    err = neoc_script_builder_push_data(builder, contract_hash->data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Emit SYSCALL for contract call
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);
    
    return err;
}

neoc_error_t neoc_script_create_nep17_transfer(const neoc_hash160_t *token_hash,
                                               const neoc_hash160_t *from,
                                               const neoc_hash160_t *to,
                                               uint64_t amount,
                                               const uint8_t *data,
                                               size_t data_len,
                                               uint8_t **script,
                                               size_t *script_len) {
    if (!token_hash || !from || !to || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push data parameter (or null)
    if (data && data_len > 0) {
        err = neoc_script_builder_push_data(builder, data, data_len);
    } else {
        err = neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
    }
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push amount
    err = neoc_script_builder_push_integer(builder, (int64_t)amount);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push to address
    err = neoc_script_builder_push_data(builder, to->data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push from address
    err = neoc_script_builder_push_data(builder, from->data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push parameter count (4)
    err = neoc_script_builder_push_integer(builder, 4);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push method name "transfer"
    err = neoc_script_builder_push_string(builder, "transfer");
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push token contract hash
    err = neoc_script_builder_push_data(builder, token_hash->data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Emit SYSCALL for contract call
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Add ASSERT to ensure transfer succeeded
    err = neoc_script_builder_emit(builder, NEOC_OP_ASSERT);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);
    
    return err;
}

neoc_error_t neoc_script_create_witness_invocation(const uint8_t *signature,
                                                   size_t signature_len,
                                                   uint8_t **script,
                                                   size_t *script_len) {
    if (!signature || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (signature_len != 64 && signature_len != 65) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Invalid signature size");
    }
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push signature
    err = neoc_script_builder_push_data(builder, signature, signature_len);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);
    
    return err;
}

neoc_error_t neoc_script_create_witness_multi_invocation(const uint8_t **signatures,
                                                         size_t *signature_lengths,
                                                         size_t signature_count,
                                                         uint8_t **script,
                                                         size_t *script_len) {
    if (!signatures || !signature_lengths || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (signature_count == 0 || signature_count > 1024) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Invalid signature count");
    }
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push all signatures
    for (size_t i = 0; i < signature_count; i++) {
        if (signature_lengths[i] != 64 && signature_lengths[i] != 65) {
            neoc_script_builder_free(builder);
            return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Invalid signature size");
        }
        
        err = neoc_script_builder_push_data(builder, signatures[i], signature_lengths[i]);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            return err;
        }
    }
    
    // Get script
    err = neoc_script_builder_to_array(builder, script, script_len);
    neoc_script_builder_free(builder);
    
    return err;
}

neoc_error_t neoc_script_hash_to_address(const neoc_hash160_t *script_hash,
                                         uint8_t version,
                                         char **address) {
    if (!script_hash || !address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Use version parameter for address encoding (currently using NEO N3 format)
    uint8_t address_version = version ? version : NEOC_ADDRESS_VERSION;
    
    // Allocate buffer for address and versioned data
    *address = neoc_malloc(NEOC_MAX_ADDRESS_LENGTH);
    if (!*address) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Create versioned data (version + script hash)
    uint8_t versioned[21];
    versioned[0] = address_version;
    memcpy(versioned + 1, script_hash->data, 20);
    
    // Encode to Base58Check
    neoc_error_t err = neoc_base58_check_encode(versioned, 21, *address, NEOC_MAX_ADDRESS_LENGTH);
    if (err != NEOC_SUCCESS) {
        neoc_free(*address);
        *address = NULL;
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_address_to_hash(const char *address,
                                         neoc_hash160_t *script_hash) {
    if (!address || !script_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Parse address to script hash
    return neoc_hash160_from_address(script_hash, address);
}
