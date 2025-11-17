/**
 * @file test_bip39_account.c
 * @brief BIP39 Account tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/wallet/bip39_account.h>
#include <neoc/crypto/bip39.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== BIP39 ACCOUNT TESTS ===== */

void test_generate_and_recover_bip39_account(void) {
    const char* password = "Insecure Pa55w0rd";
    neoc_bip39_account_t* account1 = NULL;
    neoc_bip39_account_t* account2 = NULL;
    
    // Generate a new BIP39 account
    neoc_error_t err = neoc_bip39_account_create(password, NEOC_BIP39_WORD_COUNT_12, &account1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account1);
    
    const char* mnemonic = neoc_bip39_account_get_mnemonic(account1);
    TEST_ASSERT_NOT_NULL(mnemonic);
    TEST_ASSERT_TRUE(strlen(mnemonic) > 0);
    
    // Get address from first account
    char *address1 = NULL;
    err = neoc_bip39_account_get_address(account1, &address1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(address1);
    
    // Recover account from mnemonic
    err = neoc_bip39_account_from_mnemonic(password, mnemonic, &account2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account2);
    
    // Get address from second account
    char *address2 = NULL;
    err = neoc_bip39_account_get_address(account2, &address2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(address2);
    
    // Addresses should match
    TEST_ASSERT_EQUAL_STRING(address1, address2);
    
    // Get private keys and compare
    uint8_t private_key1[32];
    uint8_t private_key2[32];
    size_t key_len1 = sizeof(private_key1);
    size_t key_len2 = sizeof(private_key2);
    
    err = neoc_bip39_account_get_private_key(account1, private_key1, &key_len1);
    if (err == NEOC_SUCCESS) {
        err = neoc_bip39_account_get_private_key(account2, private_key2, &key_len2);
        if (err == NEOC_SUCCESS) {
            TEST_ASSERT_EQUAL_UINT32(key_len1, key_len2);
            TEST_ASSERT_EQUAL_MEMORY(private_key1, private_key2, 32);
        }
    }
    
    // Clean up
    neoc_free(address1);
    neoc_free(address2);
    neoc_bip39_account_free(account1);
    neoc_bip39_account_free(account2);
}

void test_bip39_account_mnemonic_validation(void) {
    const char* valid_mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    const char* invalid_mnemonic = "invalid words that are not in the wordlist";
    const char* password = "test_password";
    neoc_bip39_account_t* account = NULL;
    
    // Test with valid mnemonic
    neoc_error_t err = neoc_bip39_account_from_mnemonic(password, valid_mnemonic, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    neoc_bip39_account_free(account);
    account = NULL;
    
    // Test with invalid mnemonic - should fail
    err = neoc_bip39_account_from_mnemonic(password, invalid_mnemonic, &account);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    TEST_ASSERT_NULL(account);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== BIP39 ACCOUNT TESTS ===\n");
    
    RUN_TEST(test_generate_and_recover_bip39_account);
    RUN_TEST(test_bip39_account_mnemonic_validation);
    
    UNITY_END();
    return 0;
}
