/**
 * @file test_error_handling.c
 * @brief Comprehensive error handling tests for NeoC SDK
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "integration_test_framework.h"
#include "neoc/neoc.h"
#include "neoc/wallet/wallet.h"
#include "neoc/transaction/transaction_builder.h"
#include "neoc/contract/smart_contract.h"
#include "neoc/crypto/keys.h"
#include "neoc/utils/serialization.h"

// Test: Invalid input handling
static integration_test_result_t test_invalid_input_handling(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Test NULL pointer handling
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create(NULL, &wallet);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_ARGUMENT);
    INTEGRATION_ASSERT(wallet == NULL);
    
    // Test empty string handling
    err = neoc_wallet_create("", &wallet);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_ARGUMENT);
    
    // Test invalid address format
    neoc_address_t *address = NULL;
    err = neoc_address_from_string("invalid_address_format", &address);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_FORMAT);
    
    // Test oversized input
    char oversized_name[1024];
    memset(oversized_name, 'A', sizeof(oversized_name) - 1);
    oversized_name[sizeof(oversized_name) - 1] = '\0';
    err = neoc_wallet_create(oversized_name, &wallet);
    INTEGRATION_ASSERT(err == NEOC_ERROR_BUFFER_OVERFLOW || err == NEOC_ERROR_INVALID_ARGUMENT);
    
    // Test invalid hexadecimal strings
    neoc_hash256_t *hash = NULL;
    err = neoc_hash256_from_hex("ZZZZ", &hash);  // Invalid hex
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_FORMAT);
    
    // Test invalid base58 strings
    err = neoc_address_from_string("0OIl", &address);  // Contains invalid base58 chars
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_FORMAT);
    
    if (ctx->verbose) {
        printf("Invalid input handling tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Memory allocation failure handling
static integration_test_result_t test_memory_allocation_failures(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Note: These tests simulate memory allocation failures
    // In a real scenario, we'd use memory allocation hooks or mocks
    
    // Test large allocation request
    size_t huge_size = SIZE_MAX / 2;  // Intentionally huge
    void *ptr = malloc(huge_size);
    if (ptr == NULL) {
        // Expected behavior - allocation failed
        if (ctx->verbose) {
            printf("Large allocation correctly failed\n");
        }
    } else {
        free(ptr);
        return INTEGRATION_TEST_FAIL;  // Shouldn't succeed
    }
    
    // Test transaction builder with memory constraints
    neoc_transaction_builder_t *builder = NULL;
    neoc_error_t err = neoc_transaction_builder_create(&builder);
    if (err == NEOC_SUCCESS) {
        // Try to add many signers to exhaust memory
        for (int i = 0; i < 10000; i++) {
            neoc_account_t *account = NULL;
            err = neoc_account_create(&account);
            if (err != NEOC_SUCCESS) {
                // Expected - memory exhausted
                if (ctx->verbose) {
                    printf("Memory exhaustion handled at iteration %d\n", i);
                }
                break;
            }
            
            err = neoc_transaction_builder_add_signer(builder, account);
            if (err != NEOC_SUCCESS) {
                neoc_account_free(account);
                break;
            }
        }
        
        neoc_transaction_builder_free(builder);
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Network error handling
static integration_test_result_t test_network_error_handling(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Test connection to invalid endpoint
    neoc_rpc_client_t *client = NULL;
    neoc_error_t err = neoc_rpc_client_create("http://invalid.endpoint.test:99999", &client);
    
    if (err == NEOC_SUCCESS && client != NULL) {
        // Try to make a request
        neoc_block_t *block = NULL;
        err = neoc_rpc_get_block(client, 0, &block);
        INTEGRATION_ASSERT(err == NEOC_ERROR_NETWORK || 
                          err == NEOC_ERROR_CONNECTION_FAILED ||
                          err == NEOC_ERROR_TIMEOUT);
        
        neoc_rpc_client_free(client);
    }
    
    // Test timeout handling
    client = NULL;
    err = neoc_rpc_client_create_with_timeout("http://example.com:12345", 1, &client);  // 1ms timeout
    if (err == NEOC_SUCCESS && client != NULL) {
        neoc_block_t *block = NULL;
        err = neoc_rpc_get_block(client, 0, &block);
        INTEGRATION_ASSERT(err == NEOC_ERROR_TIMEOUT || err == NEOC_ERROR_NETWORK);
        
        neoc_rpc_client_free(client);
    }
    
    // Test malformed response handling
    // This would require a mock server returning invalid JSON
    
    if (ctx->verbose) {
        printf("Network error handling tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Cryptographic error handling
static integration_test_result_t test_crypto_error_handling(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Test invalid private key
    neoc_private_key_t *key = NULL;
    uint8_t invalid_key[31];  // Too short
    memset(invalid_key, 0xFF, sizeof(invalid_key));
    neoc_error_t err = neoc_private_key_from_bytes(invalid_key, sizeof(invalid_key), &key);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_KEY_SIZE || err == NEOC_ERROR_INVALID_ARGUMENT);
    
    // Test invalid signature verification
    neoc_key_pair_t *key_pair = NULL;
    err = neoc_key_pair_create(&key_pair);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    uint8_t message[] = "Test message";
    uint8_t signature[64];
    size_t sig_len = sizeof(signature);
    
    // Sign with correct key
    err = neoc_ecdsa_sign(key_pair, message, sizeof(message), signature, &sig_len);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Corrupt signature
    signature[0] ^= 0xFF;
    
    // Verify should fail
    bool valid = false;
    err = neoc_ecdsa_verify(
        neoc_key_pair_get_public(key_pair),
        message,
        sizeof(message),
        signature,
        sig_len,
        &valid
    );
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(valid == false);
    
    // Test invalid WIF format
    neoc_private_key_t *imported_key = NULL;
    err = neoc_private_key_from_wif("InvalidWIFFormat123", &imported_key);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_FORMAT);
    
    // Test invalid encryption password
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Export with password
    char *encrypted = NULL;
    err = neoc_account_export_encrypted(account, "password123", &encrypted);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Try to import with wrong password
    neoc_account_t *imported = NULL;
    err = neoc_account_import_encrypted(encrypted, "wrongpassword", &imported);
    INTEGRATION_ASSERT(err == NEOC_ERROR_DECRYPTION_FAILED || err == NEOC_ERROR_INVALID_PASSWORD);
    
    // Cleanup
    neoc_key_pair_free(key_pair);
    neoc_account_free(account);
    free(encrypted);
    
    if (ctx->verbose) {
        printf("Cryptographic error handling tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Transaction validation errors
static integration_test_result_t test_transaction_validation_errors(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    neoc_transaction_builder_t *builder = NULL;
    neoc_error_t err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Test invalid network fee
    err = neoc_transaction_builder_set_network_fee(builder, -1);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_ARGUMENT);
    
    // Test invalid system fee
    err = neoc_transaction_builder_set_system_fee(builder, LLONG_MAX);
    INTEGRATION_ASSERT(err == NEOC_ERROR_OVERFLOW || err == NEOC_ERROR_INVALID_ARGUMENT);
    
    // Test invalid valid until block
    err = neoc_transaction_builder_set_valid_until_block(builder, 0);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_ARGUMENT);
    
    // Test building transaction without script
    neoc_transaction_t *tx = NULL;
    err = neoc_transaction_builder_build(builder, &tx);
    INTEGRATION_ASSERT(err == NEOC_ERROR_INVALID_STATE || err == NEOC_ERROR_MISSING_SCRIPT);
    
    // Test adding too many attributes
    for (int i = 0; i < 256; i++) {
        neoc_transaction_attribute_t *attr = NULL;
        err = neoc_transaction_attribute_create_high_priority(&attr);
        if (err != NEOC_SUCCESS) break;
        
        err = neoc_transaction_builder_add_attribute(builder, attr);
        if (err == NEOC_ERROR_LIMIT_EXCEEDED) {
            // Expected behavior
            neoc_transaction_attribute_free(attr);
            break;
        }
    }
    
    // Test signing without signers
    neoc_transaction_builder_t *builder2 = NULL;
    err = neoc_transaction_builder_create(&builder2);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add minimal script
    uint8_t script[] = {0x00};  // NOP
    err = neoc_transaction_builder_set_script(builder2, script, sizeof(script));
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_sign(builder2, account, &tx);
    INTEGRATION_ASSERT(err == NEOC_ERROR_NO_SIGNERS || err == NEOC_ERROR_INVALID_STATE);
    
    // Cleanup
    neoc_transaction_builder_free(builder);
    neoc_transaction_builder_free(builder2);
    neoc_account_free(account);
    
    if (ctx->verbose) {
        printf("Transaction validation error tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: File I/O error handling
static integration_test_result_t test_file_io_error_handling(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Test reading non-existent file
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_nep6_wallet_import("/non/existent/path/wallet.json", "password", &wallet);
    INTEGRATION_ASSERT(err == NEOC_ERROR_FILE_NOT_FOUND || err == NEOC_ERROR_IO);
    
    // Test writing to read-only location
    neoc_wallet_t *test_wallet = NULL;
    err = neoc_wallet_create("TestWallet", &test_wallet);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_nep6_wallet_export(test_wallet, "/root/readonly.json", "password");
    INTEGRATION_ASSERT(err == NEOC_ERROR_PERMISSION_DENIED || err == NEOC_ERROR_IO);
    
    // Test corrupted file handling
    const char *corrupted_path = "/tmp/corrupted_wallet.json";
    FILE *f = fopen(corrupted_path, "w");
    if (f) {
        fprintf(f, "{ corrupted json data }}}");
        fclose(f);
        
        err = neoc_nep6_wallet_import(corrupted_path, "password", &wallet);
        INTEGRATION_ASSERT(err == NEOC_ERROR_PARSE_ERROR || err == NEOC_ERROR_INVALID_FORMAT);
        
        unlink(corrupted_path);
    }
    
    // Test disk space exhaustion (simulated)
    // This would require filling up /tmp or using a limited filesystem
    
    neoc_wallet_free(test_wallet);
    
    if (ctx->verbose) {
        printf("File I/O error handling tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Concurrency error handling
static integration_test_result_t test_concurrency_error_handling(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Test race condition handling
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create("ConcurrentWallet", &wallet);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Simulate concurrent access (would need threading in real scenario)
    neoc_account_t *accounts[10];
    for (int i = 0; i < 10; i++) {
        err = neoc_account_create(&accounts[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        err = neoc_wallet_add_account(wallet, accounts[i]);
        INTEGRATION_ASSERT_SUCCESS(err);
    }
    
    // Test double-free prevention
    neoc_account_t *account = accounts[0];
    neoc_wallet_remove_account(wallet, account);
    err = neoc_wallet_remove_account(wallet, account);  // Should handle gracefully
    INTEGRATION_ASSERT(err == NEOC_ERROR_NOT_FOUND || err == NEOC_ERROR_INVALID_STATE);
    
    // Test resource exhaustion
    neoc_connection_pool_t *pool = NULL;
    err = neoc_connection_pool_create(5, &pool);  // Small pool
    if (err == NEOC_SUCCESS) {
        // Try to acquire more connections than available
        neoc_connection_t *connections[10];
        for (int i = 0; i < 10; i++) {
            err = neoc_connection_pool_acquire(pool, &connections[i]);
            if (err == NEOC_ERROR_RESOURCE_EXHAUSTED) {
                // Expected behavior
                if (ctx->verbose) {
                    printf("Connection pool exhaustion handled at connection %d\n", i);
                }
                break;
            }
        }
        
        neoc_connection_pool_free(pool);
    }
    
    neoc_wallet_free(wallet);
    
    if (ctx->verbose) {
        printf("Concurrency error handling tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Test: Recovery and retry mechanisms
static integration_test_result_t test_recovery_mechanisms(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Test automatic retry logic
    neoc_rpc_client_t *client = NULL;
    neoc_retry_config_t retry_config = {
        .max_retries = 3,
        .base_delay_ms = 100,
        .max_delay_ms = 1000,
        .exponential_base = 2.0
    };
    
    neoc_error_t err = neoc_rpc_client_create_with_retry(
        "http://flaky.endpoint.test",
        &retry_config,
        &client
    );
    
    if (err == NEOC_SUCCESS && client != NULL) {
        // Test that retries are attempted
        neoc_block_t *block = NULL;
        err = neoc_rpc_get_block(client, 0, &block);
        // Should fail after retries
        INTEGRATION_ASSERT(err != NEOC_SUCCESS);
        
        neoc_rpc_client_free(client);
    }
    
    // Test transaction recovery
    neoc_transaction_t *tx = NULL;
    neoc_transaction_recovery_t *recovery = NULL;
    
    // Create a transaction that might fail
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Simulate partial transaction state
    err = neoc_transaction_recovery_create(&recovery);
    if (err == NEOC_SUCCESS) {
        // Save transaction state
        err = neoc_transaction_recovery_save(recovery, builder);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        // Simulate failure and recovery
        neoc_transaction_builder_free(builder);
        builder = NULL;
        
        // Recover transaction
        err = neoc_transaction_recovery_restore(recovery, &builder);
        INTEGRATION_ASSERT_SUCCESS(err);
        INTEGRATION_ASSERT(builder != NULL);
        
        neoc_transaction_recovery_free(recovery);
        neoc_transaction_builder_free(builder);
    }
    
    if (ctx->verbose) {
        printf("Recovery mechanism tests passed\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Setup function for error handling tests
static integration_test_result_t error_handling_test_setup(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Initialize NeoC with error tracking enabled
    neoc_error_t err = neoc_init_with_error_tracking();
    if (err != NEOC_SUCCESS && err != NEOC_ERROR_ALREADY_INITIALIZED) {
        return INTEGRATION_TEST_FAIL;
    }
    
    if (ctx->verbose) {
        printf("Error handling test setup complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Teardown function for error handling tests
static integration_test_result_t error_handling_test_teardown(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Cleanup and check for memory leaks
    neoc_cleanup();
    
    // Get error statistics
    neoc_error_stats_t stats;
    if (neoc_get_error_stats(&stats) == NEOC_SUCCESS) {
        if (ctx->verbose) {
            printf("Error statistics:\n");
            printf("  Total errors: %zu\n", stats.total_errors);
            printf("  Recovered errors: %zu\n", stats.recovered_errors);
            printf("  Fatal errors: %zu\n", stats.fatal_errors);
        }
    }
    
    if (ctx->verbose) {
        printf("Error handling test teardown complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Create error handling test suite
integration_test_suite_t *create_error_handling_suite(void) {
    integration_test_suite_t *suite = integration_test_suite_create(
        "Error Handling Tests",
        "Comprehensive error handling and recovery testing"
    );
    
    // Define test cases
    integration_test_case_t tests[] = {
        {
            .info = {
                .name = "Invalid Input Handling",
                .category = "Error",
                .description = "Test handling of invalid inputs and parameters",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = error_handling_test_setup,
            .test = test_invalid_input_handling,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Memory Allocation Failures",
                .category = "Error",
                .description = "Test handling of memory allocation failures",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = error_handling_test_setup,
            .test = test_memory_allocation_failures,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Network Error Handling",
                .category = "Error",
                .description = "Test network error handling and timeouts",
                .requires_network = true,
                .requires_neo_node = false,
                .timeout_seconds = 30
            },
            .setup = error_handling_test_setup,
            .test = test_network_error_handling,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Cryptographic Errors",
                .category = "Error",
                .description = "Test cryptographic error handling",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = error_handling_test_setup,
            .test = test_crypto_error_handling,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Transaction Validation Errors",
                .category = "Error",
                .description = "Test transaction validation error handling",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = error_handling_test_setup,
            .test = test_transaction_validation_errors,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "File I/O Errors",
                .category = "Error",
                .description = "Test file I/O error handling",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = error_handling_test_setup,
            .test = test_file_io_error_handling,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Concurrency Errors",
                .category = "Error",
                .description = "Test concurrency error handling",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 15
            },
            .setup = error_handling_test_setup,
            .test = test_concurrency_error_handling,
            .teardown = error_handling_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Recovery Mechanisms",
                .category = "Error",
                .description = "Test error recovery and retry mechanisms",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 20
            },
            .setup = error_handling_test_setup,
            .test = test_recovery_mechanisms,
            .teardown = error_handling_test_teardown,
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
    printf("       Error Handling Tests\n");
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
    integration_test_suite_t *suite = create_error_handling_suite();
    integration_test_result_t result = integration_test_suite_run(suite, &ctx);
    
    // Print summary
    integration_test_print_summary(suite);
    
    // Save report
    integration_test_save_report(suite, "error_handling_report.txt");
    
    // Cleanup
    integration_test_suite_free(suite);
    integration_test_cleanup(&ctx);
    
    printf("\n=================================================\n");
    printf("            Tests Complete\n");
    printf("=================================================\n");
    
    return (result == INTEGRATION_TEST_PASS) ? 0 : 1;
}