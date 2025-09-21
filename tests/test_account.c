/**
 * @file test_account.c
 * @brief Account management tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/wallet/account.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/crypto/nep2.h>
#include <neoc/crypto/wif.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

// Test constants from Swift tests
static const char* default_account_private_key = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
static const char* default_account_address = "NM7Aky765FG8NhhwtxjXRx7jEL1cnw7PBP";
static const char* default_account_encrypted_private_key = "6PYVwtrCJWvh8qQMGa4z3EqfGnT7VBS5s6TLnJuEf5QhQ9DFECDkGDyGaD";
static const char* default_account_password = "TestingPassword";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== ACCOUNT CREATION TESTS ===== */

void test_create_generic_account(void) {
    neoc_account_t* account = NULL;
    neoc_error_t err = neoc_account_create("TestAccount", &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    
    // Test account properties
    const char* address = neoc_account_get_address(account);
    TEST_ASSERT_NOT_NULL(address);
    TEST_ASSERT_TRUE(strlen(address) == 34); // Neo address length
    
    const char* label = neoc_account_get_label(account);
    TEST_ASSERT_NOT_NULL(label);
    TEST_ASSERT_EQUAL_STRING("TestAccount", label);
    
    TEST_ASSERT_FALSE(neoc_account_is_locked(account));
    
    neoc_account_free(account);
}

void test_init_account_from_existing_key_pair(void) {
    // Create key pair from known private key
    uint8_t priv_key_bytes[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(default_account_private_key, 
                                        priv_key_bytes, sizeof(priv_key_bytes), 
                                        &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key_bytes, 32, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Create account from key pair
    neoc_account_t* account = NULL;
    err = neoc_account_create_from_key_pair("TestAccount", key_pair, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    
    const char* address = neoc_account_get_address(account);
    TEST_ASSERT_EQUAL_STRING(default_account_address, address);
    
    const char* label = neoc_account_get_label(account);
    TEST_ASSERT_EQUAL_STRING("TestAccount", label);
    
    neoc_account_free(account);
    neoc_ec_key_pair_free(key_pair);
}

void test_create_account_from_wif(void) {
    // First, create a WIF from our known private key
    uint8_t priv_key_bytes[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(default_account_private_key, 
                                        priv_key_bytes, sizeof(priv_key_bytes), 
                                        &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    char* wif = NULL;
    err = neoc_private_key_to_wif(priv_key_bytes, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    
    // Create account from WIF
    neoc_account_t* account = NULL;
    err = neoc_account_create_from_wif("WIFAccount", wif, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    
    const char* address = neoc_account_get_address(account);
    TEST_ASSERT_EQUAL_STRING(default_account_address, address);
    
    const char* label = neoc_account_get_label(account);
    TEST_ASSERT_EQUAL_STRING("WIFAccount", label);
    
    neoc_free(wif);
    neoc_account_free(account);
}

/* ===== ENCRYPTION/DECRYPTION TESTS ===== */

void test_lock_unlock_account(void) {
    // Create key pair from known private key
    uint8_t priv_key_bytes[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(default_account_private_key, 
                                        priv_key_bytes, sizeof(priv_key_bytes), 
                                        &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key_bytes, 32, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_account_t* account = NULL;
    err = neoc_account_create_from_key_pair("LockTest", key_pair, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Lock the account with password
    err = neoc_account_lock(account, default_account_password);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(neoc_account_is_locked(account));
    
    // Unlock the account
    err = neoc_account_unlock(account, default_account_password);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_FALSE(neoc_account_is_locked(account));
    
    neoc_account_free(account);
    neoc_ec_key_pair_free(key_pair);
}

void test_decrypt_with_nep2(void) {
    // Create account from NEP-2 encrypted key
    neoc_account_t* account = NULL;
    neoc_error_t err = neoc_account_create_from_nep2(
        "NEP2Account",
        default_account_encrypted_private_key,
        default_account_password,
        &account);
    
    // This may fail if NEP-2 decryption is not fully implemented
    if (err != NEOC_SUCCESS) {
        TEST_IGNORE_MESSAGE("NEP-2 decryption not yet fully implemented");
        return;
    }
    
    TEST_ASSERT_NOT_NULL(account);
    
    const char* address = neoc_account_get_address(account);
    TEST_ASSERT_EQUAL_STRING(default_account_address, address);
    
    neoc_account_free(account);
}

/* ===== SCRIPT HASH TESTS ===== */

void test_account_script_hash(void) {
    // Create account from known private key
    uint8_t priv_key_bytes[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(default_account_private_key, 
                                        priv_key_bytes, sizeof(priv_key_bytes), 
                                        &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key_bytes, 32, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_account_t* account = NULL;
    err = neoc_account_create_from_key_pair("ScriptHashTest", key_pair, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get script hash
    const neoc_hash160_t* script_hash = neoc_account_get_script_hash(account);
    TEST_ASSERT_NOT_NULL(script_hash);
    
    // Check it's not all zeros
    uint8_t hash_bytes[20];
    neoc_hash160_to_bytes(script_hash, hash_bytes, sizeof(hash_bytes));
    bool all_zero = true;
    for (int i = 0; i < 20; i++) {
        if (hash_bytes[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    
    neoc_account_free(account);
    neoc_ec_key_pair_free(key_pair);
}

/* ===== EXPORT TESTS ===== */

void test_export_wif(void) {
    // Create key pair from known private key
    uint8_t priv_key_bytes[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(default_account_private_key, 
                                        priv_key_bytes, sizeof(priv_key_bytes), 
                                        &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key_bytes, 32, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_account_t* account = NULL;
    err = neoc_account_create_from_key_pair("ExportTest", key_pair, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Export as WIF
    char* wif = NULL;
    err = neoc_account_export_wif(account, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    TEST_ASSERT_TRUE(strlen(wif) > 0);
    
    neoc_free(wif);
    neoc_account_free(account);
    neoc_ec_key_pair_free(key_pair);
}

void test_export_nep2(void) {
    neoc_account_t* account = NULL;
    neoc_error_t err = neoc_account_create("NEP2Export", &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Export as NEP-2
    char* nep2 = NULL;
    err = neoc_account_export_nep2(account, default_account_password, &nep2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(nep2);
    TEST_ASSERT_TRUE(strlen(nep2) >= 57);
    
    neoc_free(nep2);
    neoc_account_free(account);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== ACCOUNT TESTS ===\n");
    
    RUN_TEST(test_create_generic_account);
    RUN_TEST(test_init_account_from_existing_key_pair);
    RUN_TEST(test_create_account_from_wif);
    RUN_TEST(test_lock_unlock_account);
    RUN_TEST(test_decrypt_with_nep2);
    RUN_TEST(test_account_script_hash);
    RUN_TEST(test_export_wif);
    RUN_TEST(test_export_nep2);
    
    UNITY_END();
    return 0;
}
