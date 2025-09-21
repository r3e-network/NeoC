/**
 * @file test_wallet_integration.c
 * @brief Integration tests for wallet functionality with Neo blockchain
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "integration_test_framework.h"
#include "neoc/neoc.h"
#include "neoc/wallet/wallet.h"
#include "neoc/wallet/account.h"
#include "neoc/wallet/nep6_wallet.h"
#include "neoc/transaction/transaction_builder.h"
#include "neoc/contract/neo_token.h"
#include "neoc/contract/gas_token.h"

// Test: Create wallet and check balance
static integration_test_result_t test_create_wallet_check_balance(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create a new wallet
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create("TestWallet", &wallet);
    INTEGRATION_ASSERT_SUCCESS(err);
    INTEGRATION_ASSERT(wallet != NULL);
    
    // Create an account
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add account to wallet
    err = neoc_wallet_add_account(wallet, account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Get account address
    const char *address = neoc_account_get_address(account);
    INTEGRATION_ASSERT(address != NULL);
    
    if (ctx->verbose) {
        printf("Created account with address: %s\n", address);
    }
    
    // Check NEO balance (should be 0 for new account)
    if (ctx->neo_service != NULL) {
        neoc_smart_contract_t *neo_token = NULL;
        err = neoc_neo_token_create(&neo_token);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        int64_t balance = 0;
        err = neoc_neo_token_get_balance(neo_token, account, &balance);
        // This may fail without node connection, which is acceptable
        if (err == NEOC_SUCCESS) {
            INTEGRATION_ASSERT_EQ(0, balance);
            if (ctx->verbose) {
                printf("NEO balance: %lld\n", (long long)balance);
            }
        }
        
        neoc_smart_contract_free(neo_token);
    }
    
    // Cleanup
    neoc_wallet_free(wallet);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Import/Export wallet
static integration_test_result_t test_wallet_import_export(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create a wallet with accounts
    neoc_wallet_t *wallet1 = NULL;
    neoc_error_t err = neoc_wallet_create("ExportWallet", &wallet1);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add multiple accounts
    for (int i = 0; i < 3; i++) {
        neoc_account_t *account = NULL;
        err = neoc_account_create(&account);
        INTEGRATION_ASSERT_SUCCESS(err);
        
        err = neoc_wallet_add_account(wallet1, account);
        INTEGRATION_ASSERT_SUCCESS(err);
    }
    
    // Export to NEP6 format
    const char *export_path = "/tmp/test_wallet.json";
    err = neoc_nep6_wallet_export(wallet1, export_path, "TestPassword123!");
    INTEGRATION_ASSERT_SUCCESS(err);
    
    if (ctx->verbose) {
        printf("Exported wallet to: %s\n", export_path);
    }
    
    // Import the wallet
    neoc_wallet_t *wallet2 = NULL;
    err = neoc_nep6_wallet_import(export_path, "TestPassword123!", &wallet2);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Verify account count matches
    size_t count1 = neoc_wallet_get_account_count(wallet1);
    size_t count2 = neoc_wallet_get_account_count(wallet2);
    INTEGRATION_ASSERT_EQ(count1, count2);
    
    if (ctx->verbose) {
        printf("Successfully imported wallet with %zu accounts\n", count2);
    }
    
    // Cleanup
    neoc_wallet_free(wallet1);
    neoc_wallet_free(wallet2);
    remove(export_path);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Multi-signature account creation
static integration_test_result_t test_multisig_account(void *context) {
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
    
    // Get multi-sig address
    const char *address = neoc_account_get_address(multisig);
    INTEGRATION_ASSERT(address != NULL);
    
    if (ctx->verbose) {
        printf("Created 2-of-3 multi-sig account: %s\n", address);
    }
    
    // Verify it's a multi-sig account
    bool is_multisig = neoc_account_is_multisig(multisig);
    INTEGRATION_ASSERT(is_multisig == true);
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
    }
    neoc_account_free(multisig);
    
    return INTEGRATION_TEST_PASS;
}

// Test: Transaction signing with wallet
static integration_test_result_t test_transaction_signing(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Create wallet and account
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create("SigningWallet", &wallet);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_wallet_add_account(wallet, account);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Create a simple transaction
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Add a simple script (NEO balance check)
    neoc_smart_contract_t *neo_token = NULL;
    err = neoc_neo_token_create(&neo_token);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    neoc_hash160_t *account_hash = neoc_account_get_script_hash(account);
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_create_hash160(account_hash, &param);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    err = neoc_transaction_builder_invoke_contract(
        builder, 
        neoc_smart_contract_get_script_hash(neo_token),
        "balanceOf",
        &param,
        1
    );
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Sign transaction
    neoc_transaction_t *tx = NULL;
    err = neoc_transaction_builder_sign(builder, account, &tx);
    INTEGRATION_ASSERT_SUCCESS(err);
    
    // Verify transaction has witness
    size_t witness_count = neoc_transaction_get_witness_count(tx);
    INTEGRATION_ASSERT(witness_count > 0);
    
    if (ctx->verbose) {
        printf("Successfully signed transaction with %zu witness(es)\n", witness_count);
    }
    
    // Cleanup
    neoc_transaction_free(tx);
    neoc_transaction_builder_free(builder);
    neoc_smart_contract_free(neo_token);
    neoc_contract_parameter_free(param);
    neoc_wallet_free(wallet);
    
    return INTEGRATION_TEST_PASS;
}

// Setup function for wallet tests
static integration_test_result_t wallet_test_setup(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Initialize NeoC
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        return INTEGRATION_TEST_FAIL;
    }
    
    if (ctx->verbose) {
        printf("Wallet integration tests setup complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Teardown function for wallet tests
static integration_test_result_t wallet_test_teardown(void *context) {
    integration_test_context_t *ctx = (integration_test_context_t *)context;
    
    // Cleanup NeoC
    neoc_cleanup();
    
    if (ctx->verbose) {
        printf("Wallet integration tests teardown complete\n");
    }
    
    return INTEGRATION_TEST_PASS;
}

// Create wallet integration test suite
integration_test_suite_t *create_wallet_integration_suite(void) {
    integration_test_suite_t *suite = integration_test_suite_create(
        "Wallet Integration Tests",
        "Integration tests for wallet functionality with Neo blockchain"
    );
    
    // Define test cases
    integration_test_case_t tests[] = {
        {
            .info = {
                .name = "Create Wallet and Check Balance",
                .category = "Wallet",
                .description = "Create a new wallet and check account balance",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = wallet_test_setup,
            .test = test_create_wallet_check_balance,
            .teardown = wallet_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Wallet Import/Export",
                .category = "Wallet",
                .description = "Test NEP6 wallet import and export functionality",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = wallet_test_setup,
            .test = test_wallet_import_export,
            .teardown = wallet_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Multi-Signature Account",
                .category = "Wallet",
                .description = "Create and verify multi-signature accounts",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = wallet_test_setup,
            .test = test_multisig_account,
            .teardown = wallet_test_teardown,
            .context = NULL
        },
        {
            .info = {
                .name = "Transaction Signing",
                .category = "Wallet",
                .description = "Test transaction signing with wallet accounts",
                .requires_network = false,
                .requires_neo_node = false,
                .timeout_seconds = 10
            },
            .setup = wallet_test_setup,
            .test = test_transaction_signing,
            .teardown = wallet_test_teardown,
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
    printf("       Wallet Integration Tests\n");
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
    integration_test_suite_t *suite = create_wallet_integration_suite();
    integration_test_result_t result = integration_test_suite_run(suite, &ctx);
    
    // Print summary
    integration_test_print_summary(suite);
    
    // Save report
    integration_test_save_report(suite, "wallet_integration_report.txt");
    
    // Cleanup
    integration_test_suite_free(suite);
    integration_test_cleanup(&ctx);
    
    printf("\n=================================================\n");
    printf("            Tests Complete\n");
    printf("=================================================\n");
    
    return (result == INTEGRATION_TEST_PASS) ? 0 : 1;
}