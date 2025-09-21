/**
 * @file test_wallet.c
 * @brief Unit tests converted from WalletTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "neoc/neoc.h"
#include "neoc/wallet/wallet.h"
#include "neoc/wallet/account.h"
#include "neoc/wallet/nep6_wallet.h"
#include "neoc/crypto/scrypt_params.h"
#include "neoc/types/hash160.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test create default wallet
static void test_create_default_wallet(void) {
    printf("Testing create default wallet...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    assert(wallet != NULL);
    
    // Check wallet properties
    assert(strcmp(neoc_wallet_get_name(wallet), "NeoSwiftWallet") == 0);
    assert(strcmp(neoc_wallet_get_version(wallet), NEOC_WALLET_CURRENT_VERSION) == 0);
    
    // Check that wallet has accounts
    size_t account_count = neoc_wallet_get_account_count(wallet);
    assert(account_count > 0);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Create default wallet test passed\n");
}

// Test create wallet with accounts
static void test_create_wallet_with_accounts(void) {
    printf("Testing create wallet with accounts...\n");
    
    // Create two accounts
    neoc_account_t *account1 = NULL;
    neoc_error_t err = neoc_account_create(&account1);
    assert(err == NEOC_SUCCESS);
    assert(account1 != NULL);
    
    neoc_account_t *account2 = NULL;
    err = neoc_account_create(&account2);
    assert(err == NEOC_SUCCESS);
    assert(account2 != NULL);
    
    // Create wallet with accounts
    neoc_account_t *accounts[] = {account1, account2};
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create_with_accounts(accounts, 2, &wallet);
    assert(err == NEOC_SUCCESS);
    assert(wallet != NULL);
    
    // Verify wallet has both accounts
    assert(neoc_wallet_get_account_count(wallet) == 2);
    
    // Verify first account is default
    neoc_account_t *default_account = neoc_wallet_get_default_account(wallet);
    assert(default_account == account1);
    
    // Verify wallet contains both accounts
    assert(neoc_wallet_contains_account(wallet, account1) == true);
    assert(neoc_wallet_contains_account(wallet, account2) == true);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Create wallet with accounts test passed\n");
}

// Test create wallet with no accounts fails
static void test_create_wallet_no_accounts(void) {
    printf("Testing create wallet with no accounts...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create_with_accounts(NULL, 0, &wallet);
    assert(err != NEOC_SUCCESS);
    assert(wallet == NULL);
    
    printf("  ✅ Create wallet with no accounts test passed\n");
}

// Test is default account
static void test_is_default_account(void) {
    printf("Testing is default account...\n");
    
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *accounts[] = {account};
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create_with_accounts(accounts, 1, &wallet);
    assert(err == NEOC_SUCCESS);
    
    assert(neoc_wallet_is_default_account(wallet, account) == true);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Is default account test passed\n");
}

// Test holds account
static void test_holds_account(void) {
    printf("Testing holds account...\n");
    
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    // Add account to wallet
    err = neoc_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    
    // Get script hash
    neoc_hash160_t *script_hash = neoc_account_get_script_hash(account);
    assert(script_hash != NULL);
    
    // Check wallet holds the account
    assert(neoc_wallet_holds_account_by_hash(wallet, script_hash) == true);
    
    // Remove account
    err = neoc_wallet_remove_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    
    // Check wallet no longer holds the account
    assert(neoc_wallet_holds_account_by_hash(wallet, script_hash) == false);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Holds account test passed\n");
}

// Test add account
static void test_add_account(void) {
    printf("Testing add account...\n");
    
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    size_t initial_count = neoc_wallet_get_account_count(wallet);
    
    err = neoc_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    
    assert(neoc_wallet_get_account_count(wallet) == initial_count + 1);
    
    // Verify account is in wallet
    neoc_hash160_t *script_hash = neoc_account_get_script_hash(account);
    neoc_account_t *retrieved = neoc_wallet_get_account_by_hash(wallet, script_hash);
    assert(retrieved == account);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Add account test passed\n");
}

// Test add same account twice
static void test_add_same_account(void) {
    printf("Testing add same account twice...\n");
    
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    size_t initial_count = neoc_wallet_get_account_count(wallet);
    
    // Add account first time
    err = neoc_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    
    // Add same account again - should be idempotent
    err = neoc_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS || err == NEOC_ERROR_DUPLICATE);
    
    // Count should only increase by 1
    assert(neoc_wallet_get_account_count(wallet) == initial_count + 1);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Add same account test passed\n");
}

// Test remove account
static void test_remove_account(void) {
    printf("Testing remove account...\n");
    
    neoc_account_t *account1 = NULL;
    neoc_error_t err = neoc_account_create(&account1);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *account2 = NULL;
    err = neoc_account_create(&account2);
    assert(err == NEOC_SUCCESS);
    
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_wallet_add_account(wallet, account1);
    assert(err == NEOC_SUCCESS);
    err = neoc_wallet_add_account(wallet, account2);
    assert(err == NEOC_SUCCESS);
    
    size_t count_before = neoc_wallet_get_account_count(wallet);
    
    // Remove account1
    err = neoc_wallet_remove_account(wallet, account1);
    assert(err == NEOC_SUCCESS);
    assert(neoc_wallet_get_account_count(wallet) == count_before - 1);
    
    // Remove account2 by script hash
    neoc_hash160_t *hash2 = neoc_account_get_script_hash(account2);
    err = neoc_wallet_remove_account_by_hash(wallet, hash2);
    assert(err == NEOC_SUCCESS);
    assert(neoc_wallet_get_account_count(wallet) == count_before - 2);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Remove account test passed\n");
}

// Test remove default account
static void test_remove_default_account(void) {
    printf("Testing remove default account...\n");
    
    neoc_account_t *account1 = NULL;
    neoc_error_t err = neoc_account_create(&account1);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *account2 = NULL;
    err = neoc_account_create(&account2);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *accounts[] = {account1, account2};
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create_with_accounts(accounts, 2, &wallet);
    assert(err == NEOC_SUCCESS);
    
    assert(neoc_wallet_get_account_count(wallet) == 2);
    assert(neoc_wallet_get_default_account(wallet) == account1);
    
    // Remove default account
    err = neoc_wallet_remove_account(wallet, account1);
    assert(err == NEOC_SUCCESS);
    assert(neoc_wallet_get_account_count(wallet) == 1);
    
    // Second account should now be default
    assert(neoc_wallet_get_default_account(wallet) == account2);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Remove default account test passed\n");
}

// Test cannot remove last account
static void test_cannot_remove_last_account(void) {
    printf("Testing cannot remove last account...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    assert(neoc_wallet_get_account_count(wallet) == 1);
    neoc_account_t *last_account = neoc_wallet_get_account(wallet, 0);
    assert(last_account != NULL);
    
    // Try to remove last account - should fail
    err = neoc_wallet_remove_account(wallet, last_account);
    assert(err != NEOC_SUCCESS);
    assert(neoc_wallet_get_account_count(wallet) == 1);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Cannot remove last account test passed\n");
}

// Test wallet to NEP6 conversion
static void test_wallet_to_nep6(void) {
    printf("Testing wallet to NEP6 conversion...\n");
    
    const char *wallet_name = "TestWallet";
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *accounts[] = {account};
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create_with_accounts(accounts, 1, &wallet);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_wallet_set_name(wallet, wallet_name);
    assert(err == NEOC_SUCCESS);
    
    // Encrypt account
    err = neoc_wallet_encrypt_all_accounts(wallet, "12345678");
    assert(err == NEOC_SUCCESS);
    
    // Convert to NEP6
    neoc_nep6_wallet_t *nep6 = NULL;
    err = neoc_wallet_to_nep6(wallet, &nep6);
    assert(err == NEOC_SUCCESS);
    assert(nep6 != NULL);
    
    // Verify NEP6 properties
    assert(strcmp(neoc_nep6_wallet_get_name(nep6), wallet_name) == 0);
    assert(strcmp(neoc_nep6_wallet_get_version(nep6), NEOC_WALLET_CURRENT_VERSION) == 0);
    
    neoc_nep6_wallet_free(nep6);
    neoc_wallet_free(wallet);
    printf("  ✅ Wallet to NEP6 conversion test passed\n");
}

// Test create wallet with password
static void test_create_wallet_with_password(void) {
    printf("Testing create wallet with password...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create_with_password("12345678", &wallet);
    assert(err == NEOC_SUCCESS);
    assert(wallet != NULL);
    
    assert(strcmp(neoc_wallet_get_name(wallet), "NeoSwiftWallet") == 0);
    assert(strcmp(neoc_wallet_get_version(wallet), NEOC_WALLET_CURRENT_VERSION) == 0);
    assert(neoc_wallet_get_account_count(wallet) == 1);
    
    // Account should be encrypted
    neoc_account_t *account = neoc_wallet_get_account(wallet, 0);
    assert(account != NULL);
    assert(neoc_account_get_encrypted_private_key(account) != NULL);
    assert(neoc_account_get_key_pair(account) == NULL);  // Key pair should be null when encrypted
    
    // Decrypt account
    err = neoc_wallet_decrypt_all_accounts(wallet, "12345678");
    assert(err == NEOC_SUCCESS);
    assert(neoc_account_get_key_pair(account) != NULL);  // Key pair should be available after decryption
    assert(neoc_account_get_encrypted_private_key(account) != NULL);  // Encrypted key still available
    
    neoc_wallet_free(wallet);
    printf("  ✅ Create wallet with password test passed\n");
}

// Test set and get default account
static void test_set_get_default_account(void) {
    printf("Testing set and get default account...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    assert(neoc_wallet_get_default_account(wallet) != NULL);
    
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_wallet_set_default_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    assert(neoc_wallet_get_default_account(wallet) == account);
    
    neoc_wallet_free(wallet);
    printf("  ✅ Set and get default account test passed\n");
}

// Test fail setting default account not in wallet
static void test_fail_set_default_account_not_in_wallet(void) {
    printf("Testing fail setting default account not in wallet...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    
    // Try to set account as default without adding it to wallet
    err = neoc_wallet_set_default_account(wallet, account);
    assert(err != NEOC_SUCCESS);
    
    neoc_account_free(account);
    neoc_wallet_free(wallet);
    printf("  ✅ Fail setting default account not in wallet test passed\n");
}

// Test encrypt wallet
static void test_encrypt_wallet(void) {
    printf("Testing encrypt wallet...\n");
    
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create(&wallet);
    assert(err == NEOC_SUCCESS);
    
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    err = neoc_wallet_add_account(wallet, account);
    assert(err == NEOC_SUCCESS);
    
    // Verify accounts have key pairs before encryption
    for (size_t i = 0; i < neoc_wallet_get_account_count(wallet); i++) {
        neoc_account_t *acc = neoc_wallet_get_account(wallet, i);
        assert(neoc_account_get_key_pair(acc) != NULL);
    }
    
    // Encrypt all accounts
    err = neoc_wallet_encrypt_all_accounts(wallet, "pw");
    assert(err == NEOC_SUCCESS);
    
    // Verify accounts no longer have key pairs after encryption
    for (size_t i = 0; i < neoc_wallet_get_account_count(wallet); i++) {
        neoc_account_t *acc = neoc_wallet_get_account(wallet, i);
        assert(neoc_account_get_key_pair(acc) == NULL);
        assert(neoc_account_get_encrypted_private_key(acc) != NULL);
    }
    
    neoc_wallet_free(wallet);
    printf("  ✅ Encrypt wallet test passed\n");
}

int main(void) {
    printf("\n=== WalletTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_create_default_wallet();
    test_create_wallet_with_accounts();
    test_create_wallet_no_accounts();
    test_is_default_account();
    test_holds_account();
    test_add_account();
    test_add_same_account();
    test_remove_account();
    test_remove_default_account();
    test_cannot_remove_last_account();
    test_wallet_to_nep6();
    test_create_wallet_with_password();
    test_set_get_default_account();
    test_fail_set_default_account_not_in_wallet();
    test_encrypt_wallet();
    
    tearDown();
    
    printf("\n✅ All WalletTests tests passed!\n\n");
    return 0;
}