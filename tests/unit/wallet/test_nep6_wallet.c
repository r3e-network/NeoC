/**
 * @file test_nep6_wallet.c
 * @brief Unit tests converted from NEP6WalletTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/wallet/nep6_wallet.h"
#include "neoc/wallet/nep6_account.h"
#include "neoc/wallet/nep6_contract.h"
#include "neoc/crypto/scrypt_params.h"
#include "neoc/utils/json.h"

// Test wallet JSON data (simplified version)
static const char *TEST_WALLET_JSON = 
    "{"
    "\"name\":\"Wallet\","
    "\"version\":\"1.0\","
    "\"scrypt\":{\"n\":16384,\"r\":8,\"p\":1},"
    "\"accounts\":["
    "{"
    "\"address\":\"NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke\","
    "\"label\":\"Account1\","
    "\"isDefault\":true,"
    "\"lock\":false,"
    "\"key\":\"6PYVEi6ZGdsLoCYbbGWqoYef7VWMbKwcew86m5fpxnZRUD8tEjainBgQW1\","
    "\"contract\":{"
    "\"script\":\"DCECJJQloGtaH45hM/x5r6LCuEML+TJyl/F2dh33no2JKcULQZVEDXg=\","
    "\"deployed\":false,"
    "\"parameters\":[{\"name\":\"signature\",\"type\":\"Signature\"}]"
    "}"
    "},"
    "{"
    "\"address\":\"NWcx4EfYdfqn5jNjDz8AHE6hWtWdUGDdmy\","
    "\"label\":\"Account2\","
    "\"isDefault\":false,"
    "\"lock\":false,"
    "\"key\":\"6PYSQWBqZE5oEFdMGCJ3xR7bz6ezz814oKE7GqwB9i5uhtUzkshe9B6YGB\","
    "\"contract\":{"
    "\"script\":\"DCEDHMqqRt98SU9EJpjIwXwJMR42FcLcBCy9Ov6rpg+kB0ALQZVEDXg=\","
    "\"deployed\":false,"
    "\"parameters\":[{\"name\":\"signature\",\"type\":\"Signature\"}]"
    "}"
    "}"
    "]"
    "}";

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test read wallet from JSON
static void test_read_wallet(void) {
    printf("Testing read NEP6 wallet from JSON...\n");
    
    // Parse wallet from JSON
    neoc_nep6_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_nep6_wallet_from_json(TEST_WALLET_JSON, &wallet);
    assert(err == NEOC_SUCCESS);
    assert(wallet != NULL);
    
    // Verify wallet properties
    assert(strcmp(neoc_nep6_wallet_get_name(wallet), "Wallet") == 0);
    assert(strcmp(neoc_nep6_wallet_get_version(wallet), "1.0") == 0);
    
    // Verify scrypt params
    neoc_scrypt_params_t *scrypt = neoc_nep6_wallet_get_scrypt(wallet);
    assert(scrypt != NULL);
    assert(neoc_scrypt_params_get_n(scrypt) == 16384);
    assert(neoc_scrypt_params_get_r(scrypt) == 8);
    assert(neoc_scrypt_params_get_p(scrypt) == 1);
    
    // Verify accounts
    size_t account_count = neoc_nep6_wallet_get_account_count(wallet);
    assert(account_count == 2);
    
    // Check first account
    neoc_nep6_account_t *account1 = neoc_nep6_wallet_get_account(wallet, 0);
    assert(account1 != NULL);
    assert(strcmp(neoc_nep6_account_get_address(account1), "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke") == 0);
    assert(strcmp(neoc_nep6_account_get_label(account1), "Account1") == 0);
    assert(neoc_nep6_account_is_default(account1) == true);
    assert(neoc_nep6_account_is_locked(account1) == false);
    assert(strcmp(neoc_nep6_account_get_key(account1), "6PYVEi6ZGdsLoCYbbGWqoYef7VWMbKwcew86m5fpxnZRUD8tEjainBgQW1") == 0);
    
    // Check first account contract
    neoc_nep6_contract_t *contract1 = neoc_nep6_account_get_contract(account1);
    assert(contract1 != NULL);
    assert(strcmp(neoc_nep6_contract_get_script(contract1), "DCECJJQloGtaH45hM/x5r6LCuEML+TJyl/F2dh33no2JKcULQZVEDXg=") == 0);
    assert(neoc_nep6_contract_is_deployed(contract1) == false);
    
    // Check first parameter
    size_t param_count = neoc_nep6_contract_get_parameter_count(contract1);
    assert(param_count == 1);
    neoc_nep6_parameter_t *param1 = neoc_nep6_contract_get_parameter(contract1, 0);
    assert(param1 != NULL);
    assert(strcmp(neoc_nep6_parameter_get_name(param1), "signature") == 0);
    assert(neoc_nep6_parameter_get_type(param1) == NEOC_CONTRACT_PARAM_TYPE_SIGNATURE);
    
    // Check second account
    neoc_nep6_account_t *account2 = neoc_nep6_wallet_get_account(wallet, 1);
    assert(account2 != NULL);
    assert(strcmp(neoc_nep6_account_get_address(account2), "NWcx4EfYdfqn5jNjDz8AHE6hWtWdUGDdmy") == 0);
    assert(strcmp(neoc_nep6_account_get_label(account2), "Account2") == 0);
    assert(neoc_nep6_account_is_default(account2) == false);
    assert(neoc_nep6_account_is_locked(account2) == false);
    assert(strcmp(neoc_nep6_account_get_key(account2), "6PYSQWBqZE5oEFdMGCJ3xR7bz6ezz814oKE7GqwB9i5uhtUzkshe9B6YGB") == 0);
    
    // Check second account contract
    neoc_nep6_contract_t *contract2 = neoc_nep6_account_get_contract(account2);
    assert(contract2 != NULL);
    assert(strcmp(neoc_nep6_contract_get_script(contract2), "DCEDHMqqRt98SU9EJpjIwXwJMR42FcLcBCy9Ov6rpg+kB0ALQZVEDXg=") == 0);
    assert(neoc_nep6_contract_is_deployed(contract2) == false);
    
    // Check second parameter
    param_count = neoc_nep6_contract_get_parameter_count(contract2);
    assert(param_count == 1);
    neoc_nep6_parameter_t *param2 = neoc_nep6_contract_get_parameter(contract2, 0);
    assert(param2 != NULL);
    assert(strcmp(neoc_nep6_parameter_get_name(param2), "signature") == 0);
    assert(neoc_nep6_parameter_get_type(param2) == NEOC_CONTRACT_PARAM_TYPE_SIGNATURE);
    
    neoc_nep6_wallet_free(wallet);
    printf("  ✅ Read NEP6 wallet test passed\n");
}

// Test create NEP6 wallet
static void test_create_nep6_wallet(void) {
    printf("Testing create NEP6 wallet...\n");
    
    // Create wallet
    neoc_nep6_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_nep6_wallet_create("TestWallet", "1.0", &wallet);
    assert(err == NEOC_SUCCESS);
    assert(wallet != NULL);
    
    // Verify properties
    assert(strcmp(neoc_nep6_wallet_get_name(wallet), "TestWallet") == 0);
    assert(strcmp(neoc_nep6_wallet_get_version(wallet), "1.0") == 0);
    
    // Set scrypt params
    neoc_scrypt_params_t *scrypt = NULL;
    err = neoc_scrypt_params_create_default(&scrypt);
    assert(err == NEOC_SUCCESS);
    err = neoc_nep6_wallet_set_scrypt(wallet, scrypt);
    assert(err == NEOC_SUCCESS);
    
    // Create account
    neoc_nep6_account_t *account = NULL;
    err = neoc_nep6_account_create("NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke", "TestAccount", &account);
    assert(err == NEOC_SUCCESS);
    err = neoc_nep6_account_set_default(account, true);
    assert(err == NEOC_SUCCESS);
    err = neoc_nep6_account_set_key(account, "6PYVEi6ZGdsLoCYbbGWqoYef7VWMbKwcew86m5fpxnZRUD8tEjainBgQW1");
    assert(err == NEOC_SUCCESS);
    
    // Add account to wallet
    err = neoc_nep6_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    assert(neoc_nep6_wallet_get_account_count(wallet) == 1);
    
    neoc_scrypt_params_free(scrypt);
    neoc_nep6_wallet_free(wallet);
    printf("  ✅ Create NEP6 wallet test passed\n");
}

// Test NEP6 wallet serialization
static void test_wallet_serialization(void) {
    printf("Testing NEP6 wallet serialization...\n");
    
    // Create wallet
    neoc_nep6_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_nep6_wallet_create("TestWallet", "1.0", &wallet);
    assert(err == NEOC_SUCCESS);
    
    // Set default scrypt params
    neoc_scrypt_params_t *scrypt = NULL;
    err = neoc_scrypt_params_create_default(&scrypt);
    assert(err == NEOC_SUCCESS);
    err = neoc_nep6_wallet_set_scrypt(wallet, scrypt);
    assert(err == NEOC_SUCCESS);
    
    // Serialize to JSON
    char *json = NULL;
    err = neoc_nep6_wallet_to_json(wallet, &json);
    assert(err == NEOC_SUCCESS);
    assert(json != NULL);
    
    // Parse back from JSON
    neoc_nep6_wallet_t *wallet2 = NULL;
    err = neoc_nep6_wallet_from_json(json, &wallet2);
    assert(err == NEOC_SUCCESS);
    assert(wallet2 != NULL);
    
    // Verify properties match
    assert(strcmp(neoc_nep6_wallet_get_name(wallet), neoc_nep6_wallet_get_name(wallet2)) == 0);
    assert(strcmp(neoc_nep6_wallet_get_version(wallet), neoc_nep6_wallet_get_version(wallet2)) == 0);
    
    free(json);
    neoc_scrypt_params_free(scrypt);
    neoc_nep6_wallet_free(wallet);
    neoc_nep6_wallet_free(wallet2);
    printf("  ✅ NEP6 wallet serialization test passed\n");
}

// Test wallet with no default account
static void test_wallet_no_default_account(void) {
    printf("Testing wallet with no default account...\n");
    
    const char *json_no_default = 
        "{"
        "\"name\":\"Wallet\","
        "\"version\":\"1.0\","
        "\"accounts\":["
        "{"
        "\"address\":\"NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke\","
        "\"label\":\"Account1\","
        "\"isDefault\":false,"
        "\"lock\":false,"
        "\"key\":\"6PYVEi6ZGdsLoCYbbGWqoYef7VWMbKwcew86m5fpxnZRUD8tEjainBgQW1\""
        "}"
        "]"
        "}";
    
    // Parse wallet
    neoc_nep6_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_nep6_wallet_from_json(json_no_default, &wallet);
    assert(err == NEOC_SUCCESS);
    assert(wallet != NULL);
    
    // Check that no account is marked as default
    size_t count = neoc_nep6_wallet_get_account_count(wallet);
    assert(count == 1);
    
    neoc_nep6_account_t *account = neoc_nep6_wallet_get_account(wallet, 0);
    assert(neoc_nep6_account_is_default(account) == false);
    
    // Get default account should return NULL
    neoc_nep6_account_t *default_account = neoc_nep6_wallet_get_default_account(wallet);
    assert(default_account == NULL);
    
    neoc_nep6_wallet_free(wallet);
    printf("  ✅ Wallet with no default account test passed\n");
}

// Test find account by address
static void test_find_account_by_address(void) {
    printf("Testing find account by address...\n");
    
    // Parse test wallet
    neoc_nep6_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_nep6_wallet_from_json(TEST_WALLET_JSON, &wallet);
    assert(err == NEOC_SUCCESS);
    
    // Find first account by address
    neoc_nep6_account_t *account1 = neoc_nep6_wallet_find_account_by_address(wallet, "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke");
    assert(account1 != NULL);
    assert(strcmp(neoc_nep6_account_get_label(account1), "Account1") == 0);
    
    // Find second account by address
    neoc_nep6_account_t *account2 = neoc_nep6_wallet_find_account_by_address(wallet, "NWcx4EfYdfqn5jNjDz8AHE6hWtWdUGDdmy");
    assert(account2 != NULL);
    assert(strcmp(neoc_nep6_account_get_label(account2), "Account2") == 0);
    
    // Try to find non-existent account
    neoc_nep6_account_t *account3 = neoc_nep6_wallet_find_account_by_address(wallet, "InvalidAddress");
    assert(account3 == NULL);
    
    neoc_nep6_wallet_free(wallet);
    printf("  ✅ Find account by address test passed\n");
}

int main(void) {
    printf("\n=== NEP6WalletTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_read_wallet();
    test_create_nep6_wallet();
    test_wallet_serialization();
    test_wallet_no_default_account();
    test_find_account_by_address();
    
    tearDown();
    
    printf("\n✅ All NEP6WalletTests tests passed!\n\n");
    return 0;
}