/**
 * @file test_transaction_integration.c
 * @brief Integration tests for transaction creation and management
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "integration_test_framework.h"
#include "neoc/neoc.h"
#include "neoc/transaction/transaction.h"
#include "neoc/transaction/transaction_builder.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness.h"
#include "neoc/wallet/account.h"
#include "neoc/contract/neo_token.h"
#include "neoc/script/script_builder.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/types/hash256.h"

// Test: Basic transaction creation
static integration_test_result_t test_basic_transaction_creation(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create transaction builder
    neoc_transaction_builder_t *builder = NULL;
    neoc_error_t err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(builder != NULL);
    
    // Set transaction attributes
    err = neoc_transaction_builder_set_version(builder, 0);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_nonce(builder, (uint32_t)time(NULL));
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_valid_until_block(builder, TX_DEFAULT_VALID_UNTIL_BLOCK);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_system_fee(builder, 100000000); // 1 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_network_fee(builder, 1000000); // 0.01 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add a simple script
    neoc_script_builder_t *script_builder = NULL;
    err = neoc_script_builder_create(&script_builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_push_integer(script_builder, 42);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(script_builder, &script, &script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_script(builder, script, script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create account for signing
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add signer
    neoc_signer_t *signer = NULL;
    neoc_hash160_t *account_hash = neoc_account_get_script_hash(account);
    err = neoc_signer_create(account_hash, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, &signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_signer(builder, signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build and sign transaction
    neoc_transaction_t *tx = NULL;
    err = neoc_transaction_builder_sign(builder, account, &tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(tx != NULL);
    
    // Verify transaction properties
    uint8_t version = neoc_transaction_get_version(tx);
    INTEGRATION_ASSERT_EQ(0, version);
    
    size_t witness_count = neoc_transaction_get_witness_count(tx);
    INTEGRATION_ASSERT(witness_count > 0);
    
    size_t signer_count = neoc_transaction_get_signer_count(tx);
    INTEGRATION_ASSERT_EQ(1, signer_count);
    
    if (ctx->verbose) {
        printf("Created transaction with %zu signer(s) and %zu witness(es)\n", 
               signer_count, witness_count);
        
        // Get transaction hash
        neoc_hash256_t *tx_hash = neoc_transaction_get_hash(tx);
        if (tx_hash) {
            char hash_str[65];
            err = neoc_hash256_to_string(tx_hash, hash_str, sizeof(hash_str));
            if (err == NEOC_SUCCESS) {
                printf("Transaction hash: %s\n", hash_str);
            }
            neoc_hash256_free(tx_hash);
        }
    }
    
    // Cleanup
    neoc_transaction_free(tx);
    neoc_signer_free(signer);
    neoc_account_free(account);
    free(script);
    neoc_script_builder_free(script_builder);
    neoc_transaction_builder_free(builder);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Multi-signature transaction
static integration_test_result_t test_multisig_transaction(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create multiple accounts for multi-sig
    neoc_account_t *accounts[3];
    neoc_ec_public_key_t *pub_keys[3];
    
    for (int i = 0; i < 3; i++) {
        neoc_error_t err = neoc_account_create(&accounts[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        pub_keys[i] = neoc_account_get_public_key(accounts[i]);
        INTEGRATION_ASSERT(pub_keys[i] != NULL);
    }
    
    // Create 2-of-3 multi-sig account
    neoc_account_t *multisig = NULL;
    neoc_error_t err = neoc_account_create_multisig(2, pub_keys, 3, &multisig);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create transaction builder
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Set basic transaction properties
    err = neoc_transaction_builder_set_nonce(builder, 12345);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_valid_until_block(builder, TX_DEFAULT_VALID_UNTIL_BLOCK + 1000);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add a script
    neoc_smart_contract_t *neo_token = NULL;
    err = neoc_neo_token_create(&neo_token);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_hash160_t *multisig_hash = neoc_account_get_script_hash(multisig);
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_create_hash160(multisig_hash, &param);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_invoke_contract(
        builder,
        neoc_smart_contract_get_script_hash(neo_token),
        "balanceOf",
        &param,
        1
    );
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add multi-sig signer
    neoc_signer_t *signer = NULL;
    err = neoc_signer_create(multisig_hash, NEOC_WITNESS_SCOPE_GLOBAL, &signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_signer(builder, signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build transaction (not signed yet)
    neoc_transaction_t *tx = NULL;
    err = neoc_transaction_builder_build(builder, &tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Sign with first account
    neoc_witness_t *witness1 = NULL;
    err = neoc_transaction_sign_with_account(tx, accounts[0], &witness1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Sign with second account (reaching threshold)
    neoc_witness_t *witness2 = NULL;
    err = neoc_transaction_sign_with_account(tx, accounts[1], &witness2);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Combine signatures into multi-sig witness
    neoc_witness_t *multisig_witness = NULL;
    neoc_witness_t *witnesses[] = {witness1, witness2};
    err = neoc_witness_create_multisig(witnesses, 2, multisig, &multisig_witness);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add witness to transaction
    err = neoc_transaction_add_witness(tx, multisig_witness);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Verify transaction has witness
    size_t witness_count = neoc_transaction_get_witness_count(tx);
    INTEGRATION_ASSERT(witness_count > 0);
    
    if (ctx->verbose) {
        printf("Created multi-sig transaction with %zu witness(es)\n", witness_count);
        printf("Multi-sig: 2-of-3 threshold signature\n");
    }
    
    // Cleanup
    neoc_witness_free(witness1);
    neoc_witness_free(witness2);
    neoc_witness_free(multisig_witness);
    neoc_transaction_free(tx);
    neoc_signer_free(signer);
    neoc_contract_parameter_free(param);
    neoc_smart_contract_free(neo_token);
    neoc_transaction_builder_free(builder);
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
    }
    neoc_account_free(multisig);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Transaction serialization/deserialization
static integration_test_result_t test_transaction_serialization(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create and build a transaction
    neoc_transaction_builder_t *builder = NULL;
    neoc_error_t err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Set properties
    err = neoc_transaction_builder_set_version(builder, 0);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint32_t nonce = 0xDEADBEEF;
    err = neoc_transaction_builder_set_nonce(builder, nonce);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint32_t valid_until = 3000000;
    err = neoc_transaction_builder_set_valid_until_block(builder, valid_until);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    int64_t sys_fee = 200000000; // 2 GAS
    err = neoc_transaction_builder_set_system_fee(builder, sys_fee);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    int64_t net_fee = 2000000; // 0.02 GAS
    err = neoc_transaction_builder_set_network_fee(builder, net_fee);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add script
    uint8_t script[] = {0x51, 0x52, 0x93}; // PUSH1 PUSH2 ADD
    err = neoc_transaction_builder_set_script(builder, script, sizeof(script));
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create account and add signer
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_signer_t *signer = NULL;
    neoc_hash160_t *account_hash = neoc_account_get_script_hash(account);
    err = neoc_signer_create(account_hash, NEOC_WITNESS_SCOPE_NONE, &signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_signer(builder, signer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build and sign transaction
    neoc_transaction_t *tx1 = NULL;
    err = neoc_transaction_builder_sign(builder, account, &tx1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Serialize transaction
    neoc_binary_writer_t *writer = NULL;
    err = neoc_binary_writer_create(&writer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_serialize(tx1, writer);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint8_t *serialized = NULL;
    size_t serialized_len = 0;
    err = neoc_binary_writer_to_array(writer, &serialized, &serialized_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(serialized_len > 0);
    
    if (ctx->verbose) {
        printf("Serialized transaction: %zu bytes\n", serialized_len);
    }
    
    // Deserialize transaction
    neoc_binary_reader_t *reader = NULL;
    err = neoc_binary_reader_create(serialized, serialized_len, &reader);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_transaction_t *tx2 = NULL;
    err = neoc_transaction_deserialize(reader, &tx2);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Verify deserialized transaction matches original
    INTEGRATION_ASSERT_EQ(neoc_transaction_get_version(tx1), neoc_transaction_get_version(tx2));
    INTEGRATION_ASSERT_EQ(neoc_transaction_get_nonce(tx1), neoc_transaction_get_nonce(tx2));
    INTEGRATION_ASSERT_EQ(neoc_transaction_get_valid_until_block(tx1), neoc_transaction_get_valid_until_block(tx2));
    INTEGRATION_ASSERT_EQ(neoc_transaction_get_system_fee(tx1), neoc_transaction_get_system_fee(tx2));
    INTEGRATION_ASSERT_EQ(neoc_transaction_get_network_fee(tx1), neoc_transaction_get_network_fee(tx2));
    
    // Verify hashes match
    neoc_hash256_t *hash1 = neoc_transaction_get_hash(tx1);
    neoc_hash256_t *hash2 = neoc_transaction_get_hash(tx2);
    INTEGRATION_ASSERT(neoc_hash256_equals(hash1, hash2));
    
    if (ctx->verbose) {
        printf("Transaction serialization/deserialization successful\n");
        printf("Hashes match: verified\n");
    }
    
    // Cleanup
    neoc_hash256_free(hash1);
    neoc_hash256_free(hash2);
    neoc_transaction_free(tx1);
    neoc_transaction_free(tx2);
    neoc_binary_reader_free(reader);
    free(serialized);
    neoc_binary_writer_free(writer);
    neoc_signer_free(signer);
    neoc_account_free(account);
    neoc_transaction_builder_free(builder);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Complex transaction with attributes
static integration_test_result_t test_complex_transaction(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create transaction builder
    neoc_transaction_builder_t *builder = NULL;
    neoc_error_t err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Set all transaction properties
    err = neoc_transaction_builder_set_version(builder, 0);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_nonce(builder, 999999);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_valid_until_block(builder, TX_DEFAULT_VALID_UNTIL_BLOCK + 4000);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_system_fee(builder, 500000000); // 5 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_network_fee(builder, 5000000); // 0.05 GAS
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add transaction attributes (high priority, oracle response, etc.)
    neoc_transaction_attribute_t *attr1 = NULL;
    err = neoc_transaction_attribute_create_high_priority(&attr1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_add_attribute(builder, attr1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create multiple accounts
    neoc_account_t *accounts[3];
    for (int i = 0; i < 3; i++) {
        err = neoc_account_create(&accounts[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
    }
    
    // Add multiple signers with different scopes
    neoc_witness_scope_t scopes[] = {
        NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY,
        NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS,
        NEOC_WITNESS_SCOPE_GLOBAL
    };
    
    for (int i = 0; i < 3; i++) {
        neoc_signer_t *signer = NULL;
        neoc_hash160_t *account_hash = neoc_account_get_script_hash(accounts[i]);
        err = neoc_signer_create(account_hash, scopes[i], &signer);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        // Add allowed contracts for CUSTOM_CONTRACTS scope
        if (scopes[i] == NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) {
            neoc_hash160_t *contract_hash = NULL;
            err = neoc_hash160_from_string("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5", &contract_hash);
            INTEGRATION_ASSERT_SUCCESS(err);
            
            err = neoc_signer_add_allowed_contract(signer, contract_hash);
            INTEGRATION_ASSERT_SUCCESS(err);
            
            neoc_hash160_free(contract_hash);
        }
        
        err = neoc_transaction_builder_add_signer(builder, signer);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        neoc_signer_free(signer);
    }
    
    // Build complex invocation script
    neoc_script_builder_t *script_builder = NULL;
    err = neoc_script_builder_create(&script_builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add multiple operations
    err = neoc_script_builder_push_integer(script_builder, 100);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_push_integer(script_builder, 200);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_emit_op_code(script_builder, 0x93); // ADD
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_push_integer(script_builder, 300);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_script_builder_emit_op_code(script_builder, 0x9E); // MUL
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_to_array(script_builder, &script, &script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_set_script(builder, script, script_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Build transaction
    neoc_transaction_t *tx = NULL;
    err = neoc_transaction_builder_build(builder, &tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Sign with all accounts
    for (int i = 0; i < 3; i++) {
        neoc_witness_t *witness = NULL;
        err = neoc_transaction_sign_with_account(tx, accounts[i], &witness);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        err = neoc_transaction_add_witness(tx, witness);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        neoc_witness_free(witness);
    }
    
    // Verify complex transaction
    size_t signer_count = neoc_transaction_get_signer_count(tx);
    INTEGRATION_ASSERT_EQ(3, signer_count);
    
    size_t witness_count = neoc_transaction_get_witness_count(tx);
    INTEGRATION_ASSERT_EQ(3, witness_count);
    
    size_t attr_count = neoc_transaction_get_attribute_count(tx);
    INTEGRATION_ASSERT(attr_count > 0);
    
    if (ctx->verbose) {
        printf("Created complex transaction:\n");
        printf("  - %zu signers with different scopes\n", signer_count);
        printf("  - %zu witnesses\n", witness_count);
        printf("  - %zu attributes\n", attr_count);
        printf("  - Script size: %zu bytes\n", script_len);
        
        // Calculate transaction size
        neoc_binary_writer_t *writer = NULL;
        neoc_binary_writer_create(&writer);
        neoc_transaction_serialize(tx, writer);
        uint8_t *tx_bytes = NULL;
        size_t tx_size = 0;
        neoc_binary_writer_to_array(writer, &tx_bytes, &tx_size);
        printf("  - Total size: %zu bytes\n", tx_size);
        free(tx_bytes);
        neoc_binary_writer_free(writer);
    }
    
    // Cleanup
    neoc_transaction_free(tx);
    neoc_transaction_attribute_free(attr1);
    free(script);
    neoc_script_builder_free(script_builder);
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
    }
    neoc_transaction_builder_free(builder);
    
    return INTEGRATION_TEST_PASS;
}

// Setup function for transaction tests
static integration_test_result_t transaction_test_setup(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Initialize NeoC
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        return INTEGRATION_TEST_FAIL;
    }
    
    if (ctx->verbose) {
        printf("Transaction integration tests setup complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Teardown function for transaction tests
static integration_test_result_t transaction_test_teardown(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Cleanup NeoC
    neoc_cleanup();
    
    if (ctx->verbose) {
        printf("Transaction integration tests teardown complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Create transaction integration test suite
integration_test_suite_t *create_transaction_integration_suite(void) {
    integration_test_suite_t *suite = integration_test_suite_create(
        "Transaction Integration Tests",
        "Integration tests for transaction creation and management"
    );
    
    // Define test cases
    integration_test_case_t tests[] = {
        {
            .info = {
                .name = "Basic Transaction Creation",
                .category = "Transaction",
                .description = "Test basic transaction creation and signing",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = transaction_test_setup,
            .test = test_basic_transaction_creation,
            .teardown = transaction_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Multi-Signature Transaction",
                .category = "Transaction",
                .description = "Test multi-signature transaction creation",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = transaction_test_setup,
            .test = test_multisig_transaction,
            .teardown = transaction_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Transaction Serialization",
                .category = "Transaction",
                .description = "Test transaction serialization and deserialization",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = transaction_test_setup,
            .test = test_transaction_serialization,
            .teardown = transaction_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Complex Transaction",
                .category = "Transaction",
                .description = "Test complex transaction with multiple signers and attributes",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = transaction_test_setup,
            .test = test_complex_transaction,
            .teardown = transaction_test_teardown,
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
    printf("      Transaction Integration Tests\n");
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
    integration_test_suite_t *suite = create_transaction_integration_suite();
    integration_test_result_t result = integration_test_suite_run(suite, &ctx);
    
    // Print summary
    integration_test_print_summary(suite);
    
    // Save report
    integration_test_save_report(suite, "transaction_integration_report.txt");
    
    // Cleanup
    integration_test_suite_free(suite);
    integration_test_cleanup(&ctx);
    
    printf("\n=================================================\n");
    printf("            Tests Complete\n");
    printf("=================================================\n");
    
    return (result == INTEGRATION_TEST_PASS) ? 0 : 1;
}
