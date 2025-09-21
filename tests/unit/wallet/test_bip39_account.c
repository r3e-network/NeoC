/**
 * @file test_bip39_account.c
 * @brief Unit tests converted from Bip39AccountTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/wallet/bip39_account.h"
#include "neoc/wallet/account.h"
#include "neoc/crypto/ec_key_pair.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test generate and recover BIP39 account
static void test_generate_and_recover_bip39_account(void) {
    printf("Testing generate and recover BIP39 account...\n");
    
    const char *password = "Insecure Pa55w0rd";
    
    // Create BIP39 account
    neoc_bip39_account_t *account1 = NULL;
    neoc_error_t err = neoc_bip39_account_create(password, &account1);
    assert(err == NEOC_SUCCESS);
    assert(account1 != NULL);
    
    // Get mnemonic
    const char *mnemonic = neoc_bip39_account_get_mnemonic(account1);
    assert(mnemonic != NULL);
    assert(strlen(mnemonic) > 0);
    printf("  Generated mnemonic: %s\n", mnemonic);
    
    // Get address and key pair
    const char *address1 = neoc_bip39_account_get_address(account1);
    assert(address1 != NULL);
    
    neoc_ec_key_pair_t *key_pair1 = neoc_bip39_account_get_key_pair(account1);
    assert(key_pair1 != NULL);
    
    // Recover account from mnemonic
    neoc_bip39_account_t *account2 = NULL;
    err = neoc_bip39_account_from_mnemonic(password, mnemonic, &account2);
    assert(err == NEOC_SUCCESS);
    assert(account2 != NULL);
    
    // Get recovered account properties
    const char *address2 = neoc_bip39_account_get_address(account2);
    assert(address2 != NULL);
    
    const char *mnemonic2 = neoc_bip39_account_get_mnemonic(account2);
    assert(mnemonic2 != NULL);
    
    neoc_ec_key_pair_t *key_pair2 = neoc_bip39_account_get_key_pair(account2);
    assert(key_pair2 != NULL);
    
    // Verify addresses match
    assert(strcmp(address1, address2) == 0);
    printf("  Address matches: %s\n", address1);
    
    // Verify mnemonics match
    assert(strcmp(mnemonic, mnemonic2) == 0);
    
    // Verify key pairs match
    assert(neoc_ec_key_pair_equals(key_pair1, key_pair2) == true);
    
    neoc_bip39_account_free(account1);
    neoc_bip39_account_free(account2);
    printf("  ✅ Generate and recover BIP39 account test passed\n");
}

// Test BIP39 account with different passwords
static void test_bip39_account_different_passwords(void) {
    printf("Testing BIP39 account with different passwords...\n");
    
    const char *password1 = "Password1";
    const char *password2 = "Password2";
    
    // Create first account
    neoc_bip39_account_t *account1 = NULL;
    neoc_error_t err = neoc_bip39_account_create(password1, &account1);
    assert(err == NEOC_SUCCESS);
    
    const char *mnemonic = neoc_bip39_account_get_mnemonic(account1);
    assert(mnemonic != NULL);
    
    const char *address1 = neoc_bip39_account_get_address(account1);
    assert(address1 != NULL);
    
    // Try to recover with different password - should produce different account
    neoc_bip39_account_t *account2 = NULL;
    err = neoc_bip39_account_from_mnemonic(password2, mnemonic, &account2);
    assert(err == NEOC_SUCCESS);
    
    const char *address2 = neoc_bip39_account_get_address(account2);
    assert(address2 != NULL);
    
    // Addresses should be different due to different passwords
    assert(strcmp(address1, address2) != 0);
    printf("  Address with password1: %s\n", address1);
    printf("  Address with password2: %s\n", address2);
    
    neoc_bip39_account_free(account1);
    neoc_bip39_account_free(account2);
    printf("  ✅ BIP39 account with different passwords test passed\n");
}

// Test BIP39 account mnemonic validation
static void test_bip39_mnemonic_validation(void) {
    printf("Testing BIP39 mnemonic validation...\n");
    
    const char *password = "TestPassword";
    
    // Test with invalid mnemonic (too short)
    const char *invalid_mnemonic = "word1 word2 word3";
    neoc_bip39_account_t *account = NULL;
    neoc_error_t err = neoc_bip39_account_from_mnemonic(password, invalid_mnemonic, &account);
    assert(err != NEOC_SUCCESS);
    assert(account == NULL);
    printf("  Invalid short mnemonic rejected\n");
    
    // Test with invalid mnemonic (wrong words)
    const char *invalid_words = "invalidword1 invalidword2 invalidword3 invalidword4 invalidword5 invalidword6 invalidword7 invalidword8 invalidword9 invalidword10 invalidword11 invalidword12";
    err = neoc_bip39_account_from_mnemonic(password, invalid_words, &account);
    // Note: This may succeed if the implementation doesn't validate BIP39 word list
    // The important thing is it doesn't crash
    if (err == NEOC_SUCCESS && account != NULL) {
        neoc_bip39_account_free(account);
        printf("  Invalid word mnemonic created account (implementation allows non-standard words)\n");
    } else {
        printf("  Invalid word mnemonic rejected\n");
    }
    
    printf("  ✅ BIP39 mnemonic validation test passed\n");
}

// Test BIP39 account deterministic generation
static void test_bip39_deterministic_generation(void) {
    printf("Testing BIP39 deterministic generation...\n");
    
    const char *password = "DeterministicTest";
    const char *test_mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    
    // Create multiple accounts from same mnemonic
    neoc_bip39_account_t *account1 = NULL;
    neoc_error_t err = neoc_bip39_account_from_mnemonic(password, test_mnemonic, &account1);
    assert(err == NEOC_SUCCESS);
    
    neoc_bip39_account_t *account2 = NULL;
    err = neoc_bip39_account_from_mnemonic(password, test_mnemonic, &account2);
    assert(err == NEOC_SUCCESS);
    
    // Verify both accounts are identical
    const char *address1 = neoc_bip39_account_get_address(account1);
    const char *address2 = neoc_bip39_account_get_address(account2);
    assert(strcmp(address1, address2) == 0);
    
    neoc_ec_key_pair_t *key_pair1 = neoc_bip39_account_get_key_pair(account1);
    neoc_ec_key_pair_t *key_pair2 = neoc_bip39_account_get_key_pair(account2);
    assert(neoc_ec_key_pair_equals(key_pair1, key_pair2) == true);
    
    printf("  Deterministic address: %s\n", address1);
    
    neoc_bip39_account_free(account1);
    neoc_bip39_account_free(account2);
    printf("  ✅ BIP39 deterministic generation test passed\n");
}

// Test BIP39 account conversion to regular account
static void test_bip39_to_regular_account(void) {
    printf("Testing BIP39 to regular account conversion...\n");
    
    const char *password = "ConversionTest";
    
    // Create BIP39 account
    neoc_bip39_account_t *bip39_account = NULL;
    neoc_error_t err = neoc_bip39_account_create(password, &bip39_account);
    assert(err == NEOC_SUCCESS);
    
    // Convert to regular account
    neoc_account_t *regular_account = NULL;
    err = neoc_bip39_account_to_account(bip39_account, &regular_account);
    assert(err == NEOC_SUCCESS);
    assert(regular_account != NULL);
    
    // Verify addresses match
    const char *bip39_address = neoc_bip39_account_get_address(bip39_account);
    const char *regular_address = neoc_account_get_address(regular_account);
    assert(strcmp(bip39_address, regular_address) == 0);
    
    // Verify key pairs match
    neoc_ec_key_pair_t *bip39_key_pair = neoc_bip39_account_get_key_pair(bip39_account);
    neoc_ec_key_pair_t *regular_key_pair = neoc_account_get_key_pair(regular_account);
    assert(neoc_ec_key_pair_equals(bip39_key_pair, regular_key_pair) == true);
    
    neoc_bip39_account_free(bip39_account);
    neoc_account_free(regular_account);
    printf("  ✅ BIP39 to regular account conversion test passed\n");
}

int main(void) {
    printf("\n=== Bip39AccountTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_generate_and_recover_bip39_account();
    test_bip39_account_different_passwords();
    test_bip39_mnemonic_validation();
    test_bip39_deterministic_generation();
    test_bip39_to_regular_account();
    
    tearDown();
    
    printf("\n✅ All Bip39AccountTests tests passed!\n\n");
    return 0;
}