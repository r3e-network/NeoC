/**
 * @file test_smart_contract_integration.c
 * @brief Integration tests for smart contract interactions with Neo blockchain
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "integration_test_framework.h"
#include "neoc/neoc.h"
#include "neoc/contract/smart_contract.h"
#include "neoc/contract/neo_token.h"
#include "neoc/contract/gas_token.h"
#include "neoc/contract/policy_contract.h"
#include "neoc/script/script_builder.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/wallet/account.h"
#include "neoc/transaction/transaction_builder.h"

// Test: NEO token operations
static integration_test_result_t test_neo_token_operations(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create NEO token contract
    neoc_smart_contract_t *neo_token = NULL;
    neoc_error_t err = neoc_neo_token_create(&neo_token);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(neo_token != NULL);
    
    // Verify NEO token script hash
    neoc_hash160_t *script_hash = neoc_smart_contract_get_script_hash(neo_token);
    INTEGRATION_ASSERT(script_hash != NULL);
    
    // Create a test account
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build script to check NEO balance
    neoc_script_builder_t *builder = NULL;
    err = neoc_script_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_hash160_t *account_hash = neoc_account_get_script_hash(account);
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_create_hash160(account_hash, &param);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_contract_call(builder, script_hash, "balanceOf", &param, 1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Get script bytes
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(builder, &script, &script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(script_len > 0);
    
    if (ctx->verbose) {
        printf("Built NEO balance check script: %zu bytes\n", script_len);
    }
    
    // Test NEO token methods (offline)
    const char *symbol = neoc_neo_token_get_symbol();
    INTEGRATION_ASSERT(strcmp(symbol, "NEO") == 0);
    
    uint8_t decimals = neoc_neo_token_get_decimals();
    INTEGRATION_ASSERT_EQ(0, decimals);
    
    if (ctx->verbose) {
        printf("NEO Token - Symbol: %s, Decimals: %d\n", symbol, decimals);
    }
    
    // If connected to node, try actual operations
    if (ctx->neo_service != NULL) {
        // Get total supply
        int64_t total_supply = 0;
        err = neoc_neo_token_get_total_supply(neo_token, &total_supply);
        if (err == NEOC_SUCCESS) {
            INTEGRATION_ASSERT(total_supply > 0);
            if (ctx->verbose) {
                printf("NEO Total Supply: %lld\n", (long long)total_supply);
            }
        }
        
        // Check account balance
        int64_t balance = 0;
        err = neoc_neo_token_get_balance(neo_token, account, &balance);
        if (err == NEOC_SUCCESS) {
            // New account should have 0 balance
            INTEGRATION_ASSERT_EQ(0, balance);
        }
    }
    
    // Cleanup
    free(script);
    neoc_contract_parameter_free(param);
    neoc_script_builder_free(builder);
    neoc_account_free(account);
    neoc_smart_contract_free(neo_token);
    
    return INTEGRATION_TEST_PASS;
}

// Test: GAS token operations
static integration_test_result_t test_gas_token_operations(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create GAS token contract
    neoc_smart_contract_t *gas_token = NULL;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(gas_token != NULL);
    
    // Verify GAS token properties
    const char *symbol = neoc_gas_token_get_symbol();
    INTEGRATION_ASSERT(strcmp(symbol, "GAS") == 0);
    
    uint8_t decimals = neoc_gas_token_get_decimals();
    INTEGRATION_ASSERT_EQ(8, decimals);
    
    if (ctx->verbose) {
        printf("GAS Token - Symbol: %s, Decimals: %d\n", symbol, decimals);
    }
    
    // Create accounts for transfer test
    neoc_account_t *sender = NULL;
    err = neoc_account_create(&sender);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_account_t *receiver = NULL;
    err = neoc_account_create(&receiver);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build GAS transfer script (even though it won't execute without funds)
    neoc_script_builder_t *builder = NULL;
    err = neoc_script_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create transfer parameters
    neoc_contract_parameter_t *params[4];
    err = neoc_contract_parameter_create_hash160(neoc_account_get_script_hash(sender), &params[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_hash160(neoc_account_get_script_hash(receiver), &params[1]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_integer(100000000, &params[2]); // 1 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_parameter_create_any(&params[3]); // data parameter
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_hash160_t *gas_script_hash = neoc_smart_contract_get_script_hash(gas_token);
    err = neoc_script_builder_contract_call(builder, gas_script_hash, "transfer", params, 4);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Get script bytes
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(builder, &script, &script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(script_len > 0);
    
    if (ctx->verbose) {
        printf("Built GAS transfer script: %zu bytes\n", script_len);
    }
    
    // Cleanup
    free(script);
    for (int i = 0; i < 4; i++) {
        neoc_contract_parameter_free(params[i]);
    }
    neoc_script_builder_free(builder);
    neoc_account_free(sender);
    neoc_account_free(receiver);
    neoc_smart_contract_free(gas_token);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Policy contract operations
static integration_test_result_t test_policy_contract_operations(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create policy contract
    neoc_policy_contract_t *policy = NULL;
    neoc_error_t err = neoc_policy_contract_create(&policy);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(policy != NULL);
    
    // Verify policy contract script hash
    neoc_hash160_t *script_hash = neoc_policy_contract_get_script_hash(policy);
    INTEGRATION_ASSERT(script_hash != NULL);
    
    if (ctx->verbose) {
        char hash_str[42];
        err = neoc_hash160_to_string(script_hash, hash_str, sizeof(hash_str));
        if (err == NEOC_SUCCESS) {
            printf("Policy contract hash: %s\n", hash_str);
        }
    }
    
    // If connected to node, get policy values
    if (ctx->neo_service != NULL) {
        // Get fee per byte
        int64_t fee_per_byte = 0;
        err = neoc_policy_contract_get_fee_per_byte(policy, &fee_per_byte);
        if (err == NEOC_SUCCESS) {
            INTEGRATION_ASSERT(fee_per_byte >= 0);
            if (ctx->verbose) {
                printf("Fee per byte: %lld\n", (long long)fee_per_byte);
            }
        }
        
        // Get exec fee factor
        int32_t exec_fee_factor = 0;
        err = neoc_policy_contract_get_exec_fee_factor(policy, &exec_fee_factor);
        if (err == NEOC_SUCCESS) {
            INTEGRATION_ASSERT(exec_fee_factor > 0);
            if (ctx->verbose) {
                printf("Exec fee factor: %d\n", exec_fee_factor);
            }
        }
        
        // Get storage price
        int64_t storage_price = 0;
        err = neoc_policy_contract_get_storage_price(policy, &storage_price);
        if (err == NEOC_SUCCESS) {
            INTEGRATION_ASSERT(storage_price >= 0);
            if (ctx->verbose) {
                printf("Storage price: %lld\n", (long long)storage_price);
            }
        }
    }
    
    // Test building policy modification script (requires committee signature in real scenario)
    neoc_script_builder_t *builder = NULL;
    err = neoc_script_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_create_integer(1000, &param); // New fee value
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_contract_call(builder, script_hash, "setFeePerByte", &param, 1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(builder, &script, &script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(script_len > 0);
    
    if (ctx->verbose) {
        printf("Built policy modification script: %zu bytes\n", script_len);
    }
    
    // Cleanup
    free(script);
    neoc_contract_parameter_free(param);
    neoc_script_builder_free(builder);
    neoc_policy_contract_free(policy);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Contract invocation script building
static integration_test_result_t test_contract_invocation_building(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create a custom contract hash for testing
    neoc_hash160_t *contract_hash = NULL;
    neoc_error_t err = neoc_hash160_from_string("0x1234567890123456789012345678901234567890", &contract_hash);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create smart contract instance
    neoc_smart_contract_t *contract = NULL;
    err = neoc_smart_contract_create(contract_hash, &contract);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Test various parameter types in invocation
    neoc_contract_parameter_t *params[6];
    
    // Integer parameter
    err = neoc_contract_parameter_create_integer(42, &params[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Boolean parameter
    err = neoc_contract_parameter_create_boolean(true, &params[1]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // String parameter
    err = neoc_contract_parameter_create_string("Hello, Neo!", &params[2]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Byte array parameter
    uint8_t bytes[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    err = neoc_contract_parameter_create_byte_array(bytes, sizeof(bytes), &params[3]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Hash160 parameter
    neoc_hash160_t *hash = NULL;
    err = neoc_hash160_from_string("0xabcdefabcdefabcdefabcdefabcdefabcdefabcd", &hash);
    INTEGRATION_ASSERT_SUCCESS(err);
    err = neoc_contract_parameter_create_hash160(hash, &params[4]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Array parameter
    neoc_contract_parameter_t *array_items[2];
    err = neoc_contract_parameter_create_integer(100, &array_items[0]);
    INTEGRATION_ASSERT_SUCCESS(err);
    err = neoc_contract_parameter_create_integer(200, &array_items[1]);
    INTEGRATION_ASSERT_SUCCESS(err);
    err = neoc_contract_parameter_create_array(array_items, 2, &params[5]);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build invocation script
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_smart_contract_build_invoke_script(contract, "testMethod", params, 6, &script, &script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(script_len > 0);
    
    if (ctx->verbose) {
        printf("Built complex invocation script: %zu bytes with 6 parameters\n", script_len);
    }
    
    // Test script verification
    INTEGRATION_ASSERT(script[script_len - 1] == 0x41); // SYSCALL opcode
    
    // Cleanup
    free(script);
    for (int i = 0; i < 6; i++) {
        neoc_contract_parameter_free(params[i]);
    }
    neoc_contract_parameter_free(array_items[0]);
    neoc_contract_parameter_free(array_items[1]);
    neoc_hash160_free(hash);
    neoc_hash160_free(contract_hash);
    neoc_smart_contract_free(contract);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Contract manifest handling
static integration_test_result_t test_contract_manifest(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create a contract manifest
    neoc_contract_manifest_t *manifest = NULL;
    neoc_error_t err = neoc_contract_manifest_create(&manifest);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Set manifest properties
    err = neoc_contract_manifest_set_name(manifest, "TestContract");
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_add_group(manifest, "TestGroup");
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_add_feature(manifest, "Storage", true);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_contract_manifest_add_feature(manifest, "Payable", false);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add ABI method
    neoc_contract_abi_method_t method = {
        .name = "transfer",
        .safe = false,
        .return_type = NEOC_CONTRACT_PARAMETER_TYPE_BOOLEAN,
        .parameter_count = 3
    };
    err = neoc_contract_manifest_add_method(manifest, &method);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add ABI event
    neoc_contract_abi_event_t event = {
        .name = "Transfer",
        .parameter_count = 3
    };
    err = neoc_contract_manifest_add_event(manifest, &event);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Serialize manifest to JSON
    char *json = NULL;
    size_t json_len = 0;
    err = neoc_contract_manifest_to_json(manifest, &json, &json_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(json != NULL);
    INTEGRATION_ASSERT(json_len > 0);
    
    if (ctx->verbose) {
        printf("Contract manifest JSON: %zu bytes\n", json_len);
        if (json_len < 500) {  // Only print if reasonably sized
            printf("%s\n", json);
        }
    }
    
    // Verify JSON contains expected elements
    INTEGRATION_ASSERT(strstr(json, "TestContract") != NULL);
    INTEGRATION_ASSERT(strstr(json, "transfer") != NULL);
    INTEGRATION_ASSERT(strstr(json, "Transfer") != NULL);
    
    // Cleanup
    free(json);
    neoc_contract_manifest_free(manifest);
    
    return INTEGRATION_TEST_PASS;
}

// Setup function for smart contract tests
static integration_test_result_t contract_test_setup(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Initialize NeoC
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        return INTEGRATION_TEST_FAIL;
    }
    
    if (ctx->verbose) {
        printf("Smart contract integration tests setup complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Teardown function for smart contract tests
static integration_test_result_t contract_test_teardown(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Cleanup NeoC
    neoc_cleanup();
    
    if (ctx->verbose) {
        printf("Smart contract integration tests teardown complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Create smart contract integration test suite
integration_test_suite_t *create_smart_contract_integration_suite(void) {
    integration_test_suite_t *suite = integration_test_suite_create(
        "Smart Contract Integration Tests",
        "Integration tests for smart contract functionality with Neo blockchain"
    );
    
    // Define test cases
    integration_test_case_t tests[] = {
        {
            .info = {
                .name = "NEO Token Operations",
                .category = "Contract",
                .description = "Test NEO token contract operations",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = contract_test_setup,
            .test = test_neo_token_operations,
            .teardown = contract_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "GAS Token Operations",
                .category = "Contract",
                .description = "Test GAS token contract operations",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = contract_test_setup,
            .test = test_gas_token_operations,
            .teardown = contract_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Policy Contract Operations",
                .category = "Contract",
                .description = "Test policy contract operations",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = contract_test_setup,
            .test = test_policy_contract_operations,
            .teardown = contract_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Contract Invocation Building",
                .category = "Contract",
                .description = "Test building contract invocation scripts",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = contract_test_setup,
            .test = test_contract_invocation_building,
            .teardown = contract_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Contract Manifest Handling",
                .category = "Contract",
                .description = "Test contract manifest creation and serialization",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = contract_test_setup,
            .test = test_contract_manifest,
            .teardown = contract_test_teardown,
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
    printf("     Smart Contract Integration Tests\n");
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
    integration_test_suite_t *suite = create_smart_contract_integration_suite();
    integration_test_result_t result = integration_test_suite_run(suite, &ctx);
    
    // Print summary
    integration_test_print_summary(suite);
    
    // Save report
    integration_test_save_report(suite, "smart_contract_integration_report.txt");
    
    // Cleanup
    integration_test_suite_free(suite);
    integration_test_cleanup(&ctx);
    
    printf("\n=================================================\n");
    printf("            Tests Complete\n");
    printf("=================================================\n");
    
    return (result == INTEGRATION_TEST_PASS) ? 0 : 1;
}