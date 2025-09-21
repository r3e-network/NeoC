/**
 * @file test_e2e_scenarios.c
 * @brief End-to-end test scenarios simulating real-world Neo blockchain interactions
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "integration_test_framework.h"
#include "neoc/neoc.h"
#include "neoc/wallet/wallet.h"
#include "neoc/wallet/account.h"
#include "neoc/wallet/nep6_wallet.h"
#include "neoc/transaction/transaction_builder.h"
#include "neoc/contract/neo_token.h"
#include "neoc/contract/gas_token.h"
#include "neoc/contract/smart_contract.h"
#include "neoc/script/script_builder.h"
#include "neoc/types/contract_parameter.h"

// Test: Complete wallet lifecycle
static integration_test_result_t test_wallet_lifecycle(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    if (ctx->verbose) {
        printf("\n=== Wallet Lifecycle E2E Test ===\n");
        printf("Simulating complete wallet creation, usage, and management\n\n");
    }
    
    // Step 1: Create a new wallet
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create("E2E_TestWallet", &wallet);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 1: Created wallet 'E2E_TestWallet'\n");
    }
    
    // Step 2: Generate multiple accounts
    neoc_account_t *accounts[5];
    const char *addresses[5];
    
    for (int i = 0; i < 5; i++) {
        err = neoc_account_create(&accounts[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        err = neoc_wallet_add_account(wallet, accounts[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        addresses[i] = neoc_account_get_address(accounts[i]);
        INTEGRATION_ASSERT(addresses[i] != NULL);
        
        if (ctx->verbose) {
            printf("Step 2.%d: Added account %d: %s\n", i+1, i+1, addresses[i]);
        }
    }
    
    // Step 3: Set default account
    err = neoc_wallet_set_default_account(wallet, accounts[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 3: Set account 1 as default\n");
    }
    
    // Step 4: Export wallet to NEP6 format
    const char *export_path = "/tmp/e2e_wallet.json";
    const char *password = "SecurePassword123!@#";
    
    err = neoc_nep6_wallet_export(wallet, export_path, password);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 4: Exported wallet to %s\n", export_path);
    }
    
    // Step 5: Lock wallet (encrypt private keys)
    err = neoc_wallet_lock(wallet, password);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    bool is_locked = neoc_wallet_is_locked(wallet);
    INTEGRATION_ASSERT(is_locked == true);
    
    if (ctx->verbose) {
        printf("Step 5: Wallet locked with encryption\n");
    }
    
    // Step 6: Unlock wallet for operations
    err = neoc_wallet_unlock(wallet, password);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    is_locked = neoc_wallet_is_locked(wallet);
    INTEGRATION_ASSERT(is_locked == false);
    
    if (ctx->verbose) {
        printf("Step 6: Wallet unlocked for operations\n");
    }
    
    // Step 7: Create multi-sig account from wallet accounts
    neoc_ec_public_key_t *pub_keys[3];
    for (int i = 0; i < 3; i++) {
        pub_keys[i] = neoc_account_get_public_key(accounts[i]);
        INTEGRATION_ASSERT(pub_keys[i] != NULL);
    }
    
    neoc_account_t *multisig = NULL;
    err = neoc_account_create_multisig(2, pub_keys, 3, &multisig);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_wallet_add_account(wallet, multisig);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        const char *multisig_addr = neoc_account_get_address(multisig);
        printf("Step 7: Created 2-of-3 multi-sig account: %s\n", multisig_addr);
    }
    
    // Step 8: Remove an account
    err = neoc_wallet_remove_account(wallet, accounts[4]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    size_t account_count = neoc_wallet_get_account_count(wallet);
    INTEGRATION_ASSERT_EQ(5, account_count); // 4 regular + 1 multisig
    
    if (ctx->verbose) {
        printf("Step 8: Removed account 5, current count: %zu\n", account_count);
    }
    
    // Step 9: Import wallet from NEP6
    neoc_wallet_free(wallet);
    wallet = NULL;
    
    err = neoc_nep6_wallet_import(export_path, password, &wallet);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    account_count = neoc_wallet_get_account_count(wallet);
    if (ctx->verbose) {
        printf("Step 9: Re-imported wallet, account count: %zu\n", account_count);
    }
    
    // Step 10: Clean up
    remove(export_path);
    neoc_wallet_free(wallet);
    
    if (ctx->verbose) {
        printf("\n✅ Wallet lifecycle test completed successfully!\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Complete token transfer scenario
static integration_test_result_t test_token_transfer_scenario(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    if (ctx->verbose) {
        printf("\n=== Token Transfer E2E Test ===\n");
        printf("Simulating NEO and GAS token transfer scenario\n\n");
    }
    
    // Step 1: Create sender and receiver accounts
    neoc_account_t *sender = NULL;
    neoc_account_t *receiver = NULL;
    
    err = neoc_account_create(&sender);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_account_create(&receiver);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    const char *sender_addr = neoc_account_get_address(sender);
    const char *receiver_addr = neoc_account_get_address(receiver);
    
    if (ctx->verbose) {
        printf("Step 1: Created accounts\n");
        printf("  Sender:   %s\n", sender_addr);
        printf("  Receiver: %s\n", receiver_addr);
    }
    
    // Step 2: Create NEO token contract
    neoc_smart_contract_t *neo_token = NULL;
    err = neoc_neo_token_create(&neo_token);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Step 3: Create GAS token contract
    neoc_smart_contract_t *gas_token = NULL;
    err = neoc_gas_token_create(&gas_token);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 2-3: Created NEO and GAS token contracts\n");
    }
    
    // Step 4: Build NEO transfer transaction
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Set transaction properties
    err = neoc_transaction_builder_set_nonce(builder, (uint32_t)time(NULL));
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_valid_until_block(builder, 1000000);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_system_fee(builder, 100000000); // 1 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_network_fee(builder, 1000000); // 0.01 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create transfer parameters
    neoc_contract_parameter_t *neo_params[4];
    neoc_hash160_t *sender_hash = neoc_account_get_script_hash(sender);
    neoc_hash160_t *receiver_hash = neoc_account_get_script_hash(receiver);
    
    err = neoc_contract_parameter_create_hash160(sender_hash, &neo_params[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_hash160(receiver_hash, &neo_params[1]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_integer(10, &neo_params[2]); // 10 NEO
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_any(&neo_params[3]); // data
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add NEO transfer to transaction
    err = neoc_transaction_builder_invoke_contract(
        builder,
        neoc_smart_contract_get_script_hash(neo_token),
        "transfer",
        neo_params,
        4
    );
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 4: Built NEO transfer (10 NEO)\n");
    }
    
    // Step 5: Add GAS transfer to same transaction
    neoc_contract_parameter_t *gas_params[4];
    
    err = neoc_contract_parameter_create_hash160(sender_hash, &gas_params[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_hash160(receiver_hash, &gas_params[1]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_integer(500000000, &gas_params[2]); // 5 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_any(&gas_params[3]); // data
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add GAS transfer
    err = neoc_transaction_builder_invoke_contract(
        builder,
        neoc_smart_contract_get_script_hash(gas_token),
        "transfer",
        gas_params,
        4
    );
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 5: Added GAS transfer (5 GAS) to same transaction\n");
    }
    
    // Step 6: Add signer
    neoc_signer_t *signer = NULL;
    err = neoc_signer_create(sender_hash, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, &signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_signer(builder, signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Step 7: Sign transaction
    neoc_transaction_t *tx = NULL;
    err = neoc_transaction_builder_sign(builder, sender, &tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 6-7: Added signer and signed transaction\n");
        
        // Get transaction details
        neoc_hash256_t *tx_hash = neoc_transaction_get_hash(tx);
        if (tx_hash) {
            char hash_str[65];
            err = neoc_hash256_to_string(tx_hash, hash_str, sizeof(hash_str));
            if (err == NEOC_SUCCESS) {
                printf("  Transaction hash: %s\n", hash_str);
            }
            neoc_hash256_free(tx_hash);
        }
        
        // Calculate transaction size
        neoc_binary_writer_t *writer = NULL;
        neoc_binary_writer_create(&writer);
        neoc_transaction_serialize(tx, writer);
        uint8_t *tx_bytes = NULL;
        size_t tx_size = 0;
        neoc_binary_writer_to_array(writer, &tx_bytes, &tx_size);
        printf("  Transaction size: %zu bytes\n", tx_size);
        printf("  Network fee: 0.01 GAS\n");
        printf("  System fee: 1 GAS\n");
        free(tx_bytes);
        neoc_binary_writer_free(writer);
    }
    
    // Step 8: Verify transaction structure
    size_t witness_count = neoc_transaction_get_witness_count(tx);
    INTEGRATION_ASSERT(witness_count > 0);
    
    size_t signer_count = neoc_transaction_get_signer_count(tx);
    INTEGRATION_ASSERT_EQ(1, signer_count);
    
    if (ctx->verbose) {
        printf("Step 8: Transaction validation passed\n");
        printf("  Witnesses: %zu\n", witness_count);
        printf("  Signers: %zu\n", signer_count);
    }
    
    // Step 9: Simulate broadcasting (would send to Neo node in real scenario)
    if (ctx->neo_service != NULL) {
        // Would call neoc_service_send_transaction(ctx->neo_service, tx);
        if (ctx->verbose) {
            printf("Step 9: Transaction ready for broadcast\n");
        }
    } else {
        if (ctx->verbose) {
            printf("Step 9: Skipping broadcast (no Neo node connected)\n");
        }
    }
    
    // Cleanup
    neoc_transaction_free(tx);
    neoc_signer_free(signer);
    neoc_transaction_builder_free(builder);
    for (int i = 0; i < 4; i++) {
        neoc_contract_parameter_free(neo_params[i]);
        neoc_contract_parameter_free(gas_params[i]);
    }
    neoc_smart_contract_free(neo_token);
    neoc_smart_contract_free(gas_token);
    neoc_account_free(sender);
    neoc_account_free(receiver);
    
    if (ctx->verbose) {
        printf("\n✅ Token transfer scenario completed successfully!\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Smart contract deployment and invocation
static integration_test_result_t test_contract_deployment_scenario(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    if (ctx->verbose) {
        printf("\n=== Smart Contract Deployment E2E Test ===\n");
        printf("Simulating contract deployment and invocation\n\n");
    }
    
    // Step 1: Create deployer account
    neoc_account_t *deployer = NULL;
    neoc_error_t err = neoc_account_create(&deployer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    const char *deployer_addr = neoc_account_get_address(deployer);
    
    if (ctx->verbose) {
        printf("Step 1: Created deployer account: %s\n", deployer_addr);
    }
    
    // Step 2: Create contract manifest
    neoc_contract_manifest_t *manifest = NULL;
    err = neoc_contract_manifest_create(&manifest);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_set_name(manifest, "E2ETestContract");
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_add_group(manifest, "TestGroup");
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_add_feature(manifest, "Storage", true);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_add_feature(manifest, "Payable", true);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add contract methods
    neoc_contract_abi_method_t methods[] = {
        {
            .name = "init",
            .safe = false,
            .return_type = NEOC_CONTRACT_PARAMETER_TYPE_VOID,
            .parameter_count = 0
        },
        {
            .name = "getValue",
            .safe = true,
            .return_type = NEOC_CONTRACT_PARAMETER_TYPE_INTEGER,
            .parameter_count = 1
        },
        {
            .name = "setValue",
            .safe = false,
            .return_type = NEOC_CONTRACT_PARAMETER_TYPE_BOOLEAN,
            .parameter_count = 2
        }
    };
    
    for (int i = 0; i < 3; i++) {
        err = neoc_contract_manifest_add_method(manifest, &methods[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
    }
    
    // Add contract events
    neoc_contract_abi_event_t event = {
        .name = "ValueChanged",
        .parameter_count = 2
    };
    err = neoc_contract_manifest_add_event(manifest, &event);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 2: Created contract manifest\n");
        printf("  Name: E2ETestContract\n");
        printf("  Methods: init, getValue, setValue\n");
        printf("  Events: ValueChanged\n");
    }
    
    // Step 3: Create contract bytecode (simplified example)
    uint8_t contract_bytecode[] = {
        0x56, 0x00, // Version
        0x51,       // PUSH1
        0x52,       // PUSH2
        0x93,       // ADD
        0xC5, 0x6E, 0x65, 0x6F, // NEO identifier
        0x41, 0x16, 0x61, 0x68, 0x16, 0x4E, // System calls
        0x40        // RET
    };
    size_t bytecode_len = sizeof(contract_bytecode);
    
    // Step 4: Build deployment transaction
    neoc_transaction_builder_t *deploy_builder = NULL;
    err = neoc_transaction_builder_create(&deploy_builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_nonce(deploy_builder, (uint32_t)time(NULL));
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_valid_until_block(deploy_builder, 2000000);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Deployment requires more GAS
    err = neoc_transaction_builder_set_system_fee(deploy_builder, 1000000000); // 10 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_network_fee(deploy_builder, 10000000); // 0.1 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create deployment script
    neoc_script_builder_t *deploy_script = NULL;
    err = neoc_script_builder_create(&deploy_script);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Push contract bytecode and manifest
    err = neoc_script_builder_push_bytes(deploy_script, contract_bytecode, bytecode_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    char *manifest_json = NULL;
    size_t manifest_len = 0;
    err = neoc_contract_manifest_to_json(manifest, &manifest_json, &manifest_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_push_string(deploy_script, manifest_json);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add deploy system call
    err = neoc_script_builder_emit_sys_call(deploy_script, "System.Contract.Create");
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint8_t *deploy_script_bytes = NULL;
    size_t deploy_script_len = 0;
    err = neoc_script_builder_to_array(deploy_script, &deploy_script_bytes, &deploy_script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_script(deploy_builder, deploy_script_bytes, deploy_script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 3-4: Built contract deployment transaction\n");
        printf("  Bytecode size: %zu bytes\n", bytecode_len);
        printf("  Manifest size: %zu bytes\n", manifest_len);
        printf("  Deployment cost: 10.1 GAS\n");
    }
    
    // Step 5: Add deployer as signer
    neoc_signer_t *deploy_signer = NULL;
    neoc_hash160_t *deployer_hash = neoc_account_get_script_hash(deployer);
    err = neoc_signer_create(deployer_hash, NEOC_WITNESS_SCOPE_GLOBAL, &deploy_signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_signer(deploy_builder, deploy_signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Step 6: Sign deployment transaction
    neoc_transaction_t *deploy_tx = NULL;
    err = neoc_transaction_builder_sign(deploy_builder, deployer, &deploy_tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 5-6: Signed deployment transaction\n");
        
        neoc_hash256_t *deploy_hash = neoc_transaction_get_hash(deploy_tx);
        if (deploy_hash) {
            char hash_str[65];
            err = neoc_hash256_to_string(deploy_hash, hash_str, sizeof(hash_str));
            if (err == NEOC_SUCCESS) {
                printf("  Deployment TX hash: %s\n", hash_str);
            }
            neoc_hash256_free(deploy_hash);
        }
    }
    
    // Step 7: Calculate deployed contract hash
    // (In real scenario, this would be obtained from deployment result)
    neoc_hash160_t *contract_hash = NULL;
    err = neoc_hash160_from_string("0x1234567890123456789012345678901234567890", &contract_hash);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        char hash_str[42];
        neoc_hash160_to_string(contract_hash, hash_str, sizeof(hash_str));
        printf("Step 7: Contract would be deployed at: %s\n", hash_str);
    }
    
    // Step 8: Create invocation transaction
    neoc_transaction_builder_t *invoke_builder = NULL;
    err = neoc_transaction_builder_create(&invoke_builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_nonce(invoke_builder, (uint32_t)time(NULL) + 1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_valid_until_block(invoke_builder, 2000000);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_system_fee(invoke_builder, 50000000); // 0.5 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_network_fee(invoke_builder, 1000000); // 0.01 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Invoke setValue method
    neoc_contract_parameter_t *invoke_params[2];
    err = neoc_contract_parameter_create_string("testKey", &invoke_params[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_integer(42, &invoke_params[1]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_smart_contract_t *deployed_contract = NULL;
    err = neoc_smart_contract_create(contract_hash, &deployed_contract);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_invoke_contract(
        invoke_builder,
        contract_hash,
        "setValue",
        invoke_params,
        2
    );
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 8: Built contract invocation transaction\n");
        printf("  Method: setValue(\"testKey\", 42)\n");
        printf("  Cost: 0.51 GAS\n");
    }
    
    // Step 9: Sign invocation transaction
    neoc_signer_t *invoke_signer = NULL;
    err = neoc_signer_create(deployer_hash, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, &invoke_signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_signer(invoke_builder, invoke_signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_transaction_t *invoke_tx = NULL;
    err = neoc_transaction_builder_sign(invoke_builder, deployer, &invoke_tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Step 9: Signed invocation transaction\n");
        
        neoc_hash256_t *invoke_hash = neoc_transaction_get_hash(invoke_tx);
        if (invoke_hash) {
            char hash_str[65];
            err = neoc_hash256_to_string(invoke_hash, hash_str, sizeof(hash_str));
            if (err == NEOC_SUCCESS) {
                printf("  Invocation TX hash: %s\n", hash_str);
            }
            neoc_hash256_free(invoke_hash);
        }
    }
    
    // Cleanup
    neoc_transaction_free(invoke_tx);
    neoc_signer_free(invoke_signer);
    neoc_transaction_builder_free(invoke_builder);
    neoc_contract_parameter_free(invoke_params[0]);
    neoc_contract_parameter_free(invoke_params[1]);
    neoc_smart_contract_free(deployed_contract);
    neoc_transaction_free(deploy_tx);
    neoc_signer_free(deploy_signer);
    free(deploy_script_bytes);
    neoc_script_builder_free(deploy_script);
    free(manifest_json);
    neoc_transaction_builder_free(deploy_builder);
    neoc_contract_manifest_free(manifest);
    neoc_hash160_free(contract_hash);
    neoc_account_free(deployer);
    
    if (ctx->verbose) {
        printf("\n✅ Contract deployment scenario completed successfully!\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Setup function for E2E tests
static integration_test_result_t e2e_test_setup(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Initialize NeoC
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        return INTEGRATION_TEST_FAIL;
    }
    
    if (ctx->verbose) {
        printf("E2E test setup complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Teardown function for E2E tests
static integration_test_result_t e2e_test_teardown(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Cleanup NeoC
    neoc_cleanup();
    
    if (ctx->verbose) {
        printf("E2E test teardown complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Create E2E test suite
integration_test_suite_t *create_e2e_test_suite(void) {
    integration_test_suite_t *suite = integration_test_suite_create(
        "End-to-End Test Scenarios",
        "Complete workflows simulating real-world Neo blockchain interactions"
    );
    
    // Define test cases
    integration_test_case_t tests[] = {
        {
            .info = {
                .name = "Wallet Lifecycle",
                .category = "E2E",
                .description = "Complete wallet creation, management, and usage workflow",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 20
            },
            .setup = e2e_test_setup,
            .test = test_wallet_lifecycle,
            .teardown = e2e_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Token Transfer Scenario",
                .category = "E2E",
                .description = "Complete NEO and GAS token transfer workflow",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 20
            },
            .setup = e2e_test_setup,
            .test = test_token_transfer_scenario,
            .teardown = e2e_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Contract Deployment Scenario",
                .category = "E2E",
                .description = "Complete smart contract deployment and invocation workflow",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 20
            },
            .setup = e2e_test_setup,
            .test = test_contract_deployment_scenario,
            .teardown = e2e_test_teardown,
            .context = NULL
        }
    };
    
    // Add tests to suite
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        integration_test_suite_add_test(suite, &tests[i]);
    }
    
    return suite;
}

// Main function for standalone execution
int main(void) {
    printf("=================================================\n");
    printf("        End-to-End Test Scenarios\n");
    printf("=================================================\n\n");
    
    // Create test context
    integration_test_context_t ctx = {
        .node_config = {
            .rpc_url = "http://localhost:10332",
            .network = "testnet",
            .network_magic = 0x12345678,
            .use_testnet = true,
            .use_local = true
        },
        .neo_service = NULL,
        .test_data = NULL,
        .error_message = NULL,
        .verbose = true
    };
    
    // Initialize framework
    neoc_error_t err = integration_test_init(&ctx);
    if (err != NEOC_SUCCESS) {
        printf("Failed to initialize integration test framework\n");
        return 1;
    }
    
    // Create and run test suite
    integration_test_suite_t *suite = create_e2e_test_suite();
    integration_test_result_t result = integration_test_suite_run(suite, &ctx);
    
    // Print summary
    integration_test_print_summary(suite);
    
    // Save report
    integration_test_save_report(suite, "e2e_test_report.txt");
    
    // Cleanup
    integration_test_suite_free(suite);
    integration_test_cleanup(&ctx);
    
    printf("\n=================================================\n");
    printf("            Tests Complete\n");
    printf("=================================================\n");
    
    return (result == INTEGRATION_TEST_PASS) ? 0 : 1;
}