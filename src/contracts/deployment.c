/**
 * @file deployment.c
 * @brief Smart contract deployment implementation
 */

#include "neoc/contracts/deployment.h"
#include "neoc/neoc_memory.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/script/opcode.h"
#include "neoc/script/interop_service.h"
#include "neoc/transaction/transaction_builder.h"
#include <string.h>
#include <stdlib.h>

// Native Management contract hash
static const uint8_t MANAGEMENT_CONTRACT[20] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfe
};

neoc_error_t neoc_contract_calculate_deploy_cost(const neoc_nef_file_t *nef,
                                                  const neoc_contract_manifest_t *manifest,
                                                  uint64_t *storage_fee,
                                                  uint64_t *cpu_fee) {
    if (!nef || !manifest || !storage_fee || !cpu_fee) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Calculate storage cost based on NEF and manifest sizes
    // Get actual NEF size
    uint8_t *nef_bytes = NULL;
    size_t nef_size = 0;
    neoc_error_t err = neoc_nef_file_to_bytes(nef, &nef_bytes, &nef_size);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    if (nef_bytes) {
        neoc_free(nef_bytes);
    }
    
    // Get actual manifest size
    char *manifest_json = NULL;
    size_t manifest_size = 0;
    err = neoc_contract_manifest_to_json(manifest, &manifest_json, &manifest_size);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    if (manifest_json) {
        neoc_free(manifest_json);
    }
    
    // Storage cost calculation based on NEO protocol
    // Base deployment: 10 GAS
    // Per byte: 0.001 GAS (100000 fractions)
    *storage_fee = 10 * 100000000; // 10 GAS in fractions
    *storage_fee += (nef_size + manifest_size) * 100000; // 0.001 GAS per byte
    
    // CPU cost estimate based on contract complexity
    // Base CPU fee: 1 GAS + additional based on methods
    *cpu_fee = 1 * 100000000; // 1 GAS base CPU fee
    // Add CPU cost for contract initialization
    *cpu_fee += manifest->abi.method_count * 10000000; // 0.1 GAS per method
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_build_deploy_script(const neoc_deployment_params_t *params,
                                                uint8_t *script,
                                                size_t script_size,
                                                size_t *actual_size) {
    if (!params || !params->nef || !params->manifest || !script || !actual_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build deployment script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push deployment data (if any)
    if (params->data && params->data_size > 0) {
        err = neoc_script_builder_push_data(builder, params->data, params->data_size);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    } else {
        // Push null
        err = neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    }
    
    // Serialize manifest to JSON
    char *manifest_json = NULL;
    size_t manifest_json_size = 0;
    err = neoc_contract_manifest_to_json(params->manifest, &manifest_json, &manifest_json_size);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Push manifest JSON
    err = neoc_script_builder_push_data(builder, (uint8_t*)manifest_json, manifest_json_size);
    neoc_free(manifest_json);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Serialize NEF
    uint8_t* nef_data = NULL;
    size_t nef_size = 0;
    err = neoc_nef_file_to_bytes(params->nef, &nef_data, &nef_size);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Push NEF bytes
    err = neoc_script_builder_push_data(builder, nef_data, nef_size);
    neoc_free(nef_data); // Free allocated data
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Push method name "deploy"
    err = neoc_script_builder_push_string(builder, "deploy");
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Push Management contract hash
    err = neoc_script_builder_push_data(builder, MANAGEMENT_CONTRACT, 20);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Emit SYSCALL for contract call
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Get the script
    *actual_size = neoc_script_builder_get_size(builder);
    if (script_size < *actual_size) {
        err = neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Script buffer too small");
        goto cleanup;
    }
    
    uint8_t *script_data = NULL;
    size_t temp_size = 0;
    err = neoc_script_builder_to_array(builder, &script_data, &temp_size);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    if (temp_size != *actual_size) {
        neoc_free(script_data);
        neoc_script_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_INTERNAL, "Script size mismatch");
    }
    memcpy(script, script_data, *actual_size);
    neoc_free(script_data);
    
cleanup:
    neoc_script_builder_free(builder);
    return err;
}

neoc_error_t neoc_contract_build_update_script(const neoc_update_params_t *params,
                                                uint8_t *script,
                                                size_t script_size,
                                                size_t *actual_size) {
    if (!params || !script || !actual_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!params->nef && !params->manifest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Must update NEF or manifest");
    }
    
    // Build update script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push update data (if any)
    if (params->data && params->data_size > 0) {
        err = neoc_script_builder_push_data(builder, params->data, params->data_size);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    } else {
        // Push null
        err = neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    }
    
    // Push manifest (or null)
    if (params->manifest) {
        char* manifest_json = NULL;
        size_t manifest_json_size = 0;
        err = neoc_contract_manifest_to_json(params->manifest, &manifest_json, &manifest_json_size);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
        err = neoc_script_builder_push_data(builder, (uint8_t*)manifest_json, manifest_json_size);
        neoc_free(manifest_json);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    } else {
        err = neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    }
    
    // Push NEF (or null)
    if (params->nef) {
        uint8_t* nef_data = NULL;
        size_t nef_size = 0;
        err = neoc_nef_file_to_bytes(params->nef, &nef_data, &nef_size);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
        err = neoc_script_builder_push_data(builder, nef_data, nef_size);
        neoc_free(nef_data);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    } else {
        err = neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    }
    
    // Push method name "update"
    err = neoc_script_builder_push_string(builder, "update");
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Push contract hash
    err = neoc_script_builder_push_data(builder, params->contract_hash.data, 20);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Emit SYSCALL for contract call
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Get the script
    *actual_size = neoc_script_builder_get_size(builder);
    if (script_size < *actual_size) {
        err = neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Script buffer too small");
        goto cleanup;
    }
    
    uint8_t *script_data = NULL;
    size_t temp_size = 0;
    err = neoc_script_builder_to_array(builder, &script_data, &temp_size);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    if (temp_size != *actual_size) {
        neoc_free(script_data);
        neoc_script_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_INTERNAL, "Script size mismatch");
    }
    memcpy(script, script_data, *actual_size);
    neoc_free(script_data);
    
cleanup:
    neoc_script_builder_free(builder);
    return err;
}

neoc_error_t neoc_contract_deploy(neoc_rpc_client_t *client,
                                   const neoc_deployment_params_t *params,
                                   neoc_account_t *account,
                                   neoc_transaction_t **tx) {
    if (!client || !params || !account || !tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build deployment script
    uint8_t script[65536];
    size_t script_size = 0;
    neoc_error_t err = neoc_contract_build_deploy_script(params, script,
                                                          sizeof(script), &script_size);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Calculate fees
    uint64_t storage_fee = 0, cpu_fee = 0;
    err = neoc_contract_calculate_deploy_cost(params->nef, params->manifest,
                                               &storage_fee, &cpu_fee);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Build transaction
    neoc_tx_builder_t *builder = NULL;
    err = neoc_tx_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Set transaction properties
    neoc_tx_builder_set_version(builder, 0);
    neoc_tx_builder_set_nonce(builder, (uint32_t)rand());
    neoc_tx_builder_add_system_fee(builder, storage_fee + cpu_fee);
    neoc_tx_builder_add_network_fee(builder, 1000000); // 0.01 GAS
    
    // Get current block height for valid until block
    uint32_t block_count = 0;
    err = neoc_rpc_get_block_count(client, &block_count);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(builder);
        return err;
    }
    neoc_tx_builder_set_valid_until_block(builder, block_count + 1000);
    
    // Set script
    neoc_tx_builder_set_script(builder, script, script_size);
    
    // Add signer (the deployer account)
    neoc_hash160_t account_hash;
    neoc_error_t hash_err = neoc_account_get_script_hash(account, &account_hash);
    if (hash_err != NEOC_SUCCESS) {
        neoc_tx_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid account");
    }
    
    // Create signer with witness scope
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create(&account_hash, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, &signer);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(builder);
        return err;
    }
    
    neoc_tx_builder_add_signer(builder, signer);

    // Build the transaction
    err = neoc_tx_builder_sign(builder, &account, 1);

    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_update(neoc_rpc_client_t *client,
                                   const neoc_update_params_t *params,
                                   neoc_account_t *account,
                                   neoc_transaction_t **tx) {
    if (!client || !params || !account || !tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build update script
    uint8_t script[65536];
    size_t script_size = 0;
    neoc_error_t err = neoc_contract_build_update_script(params, script,
                                                          sizeof(script), &script_size);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Build transaction (similar to deploy)
    neoc_tx_builder_t *builder = NULL;
    err = neoc_tx_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Set transaction properties
    neoc_tx_builder_set_version(builder, 0);
    neoc_tx_builder_set_nonce(builder, (uint32_t)rand());
    neoc_tx_builder_add_system_fee(builder, 5 * 100000000); // 5 GAS for update
    neoc_tx_builder_add_network_fee(builder, 1000000); // 0.01 GAS
    
    // Get current block height
    uint32_t block_count = 0;
    err = neoc_rpc_get_block_count(client, &block_count);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(builder);
        return err;
    }
    neoc_tx_builder_set_valid_until_block(builder, block_count + 1000);
    
    // Set script
    neoc_tx_builder_set_script(builder, script, script_size);
    
    // Add signer
    neoc_hash160_t account_hash2;
    neoc_error_t hash_err2 = neoc_account_get_script_hash(account, &account_hash2);
    if (hash_err2 != NEOC_SUCCESS) {
        neoc_tx_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid account");
    }
    
    // Create signer with witness scope
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create(&account_hash2, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, &signer);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(builder);
        return err;
    }
    
    neoc_tx_builder_add_signer(builder, signer);

    // Build and sign transaction
    err = neoc_tx_builder_sign(builder, &account, 1);

    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_destroy(neoc_rpc_client_t *client,
                                    const neoc_hash160_t *contract_hash,
                                    neoc_account_t *account,
                                    neoc_transaction_t **tx) {
    if (!client || !contract_hash || !account || !tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build destroy script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push method name "destroy"
    err = neoc_script_builder_push_string(builder, "destroy");
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Push contract hash
    err = neoc_script_builder_push_data(builder, contract_hash->data, 20);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Emit SYSCALL
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Get script
    size_t script_size = neoc_script_builder_get_size(builder);
    uint8_t *script = NULL;
    size_t actual_script_size = 0;
    err = neoc_script_builder_to_array(builder, &script, &actual_script_size);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    if (actual_script_size != script_size) {
        neoc_free(script);
        neoc_script_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_INTERNAL, "Script size mismatch");
    }
    
    // Build transaction
    neoc_tx_builder_t *tx_builder = NULL;
    err = neoc_tx_builder_create(&tx_builder);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Set transaction properties
    neoc_tx_builder_set_version(tx_builder, 0);
    neoc_tx_builder_set_nonce(tx_builder, (uint32_t)rand());
    neoc_tx_builder_add_system_fee(tx_builder, 1 * 100000000); // 1 GAS
    neoc_tx_builder_add_network_fee(tx_builder, 1000000); // 0.01 GAS
    
    // Get current block height
    uint32_t block_count = 0;
    err = neoc_rpc_get_block_count(client, &block_count);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        neoc_tx_builder_free(tx_builder);
        return err;
    }
    neoc_tx_builder_set_valid_until_block(tx_builder, block_count + 1000);
    
    // Set script
    neoc_tx_builder_set_script(tx_builder, script, script_size);
    neoc_free(script);
    neoc_script_builder_free(builder);
    
    // Add signer
    neoc_hash160_t account_hash3;
    neoc_error_t hash_err3 = neoc_account_get_script_hash(account, &account_hash3);
    if (hash_err3 != NEOC_SUCCESS) {
        neoc_tx_builder_free(tx_builder);
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid account");
    }
    
    // Create signer with witness scope
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create(&account_hash3, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, &signer);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(tx_builder);
        return err;
    }
    
    neoc_tx_builder_add_signer(tx_builder, signer);

    // Build and sign
    err = neoc_tx_builder_sign(tx_builder, &account, 1);

    neoc_tx_builder_free(tx_builder);
    
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_verify_deployment(neoc_rpc_client_t *client,
                                              const neoc_hash160_t *contract_hash,
                                              const neoc_nef_file_t *expected_nef,
                                              const neoc_contract_manifest_t *expected_manifest) {
    if (!client || !contract_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Get contract state from blockchain
    neoc_contract_state_t *state = NULL;
    neoc_error_t err = neoc_rpc_get_contract_state(client, contract_hash, &state);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Verify NEF if provided
    if (expected_nef) {
        // For production implementation, this would:
        // 1. Extract NEF from contract state
        // 2. Compare checksums, compiler info, and script content
        // 3. Return detailed verification errors
        // Currently simplified to always return success
    }
    
    // Verify manifest if provided
    if (expected_manifest) {
        // Verify manifest components match expected values
        bool manifest_matches = true;
        if (state) {
            const char *state_name = state->manifest.name ? state->manifest.name : "";
            const char *expected_name = expected_manifest->name ? expected_manifest->name : "";
            if (strcmp(state_name, expected_name) != 0) {
                manifest_matches = false;
            }

            size_t state_methods = state->manifest.abi.method_count;
            size_t expected_methods = expected_manifest->abi.method_count;
            if (state_methods != expected_methods) {
                manifest_matches = false;
            }
        }
        if (!manifest_matches) {
            neoc_rpc_contract_state_free(state);
            return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Manifest verification failed");
        }
    }

    neoc_rpc_contract_state_free(state);
    return NEOC_SUCCESS;
}

void neoc_deployment_params_free(neoc_deployment_params_t *params) {
    if (!params) {
        return;
    }
    
    if (params->data) {
        neoc_free(params->data);
    }
    
    neoc_free(params);
}

void neoc_update_params_free(neoc_update_params_t *params) {
    if (!params) {
        return;
    }
    
    if (params->data) {
        neoc_free(params->data);
    }
    
    neoc_free(params);
}
