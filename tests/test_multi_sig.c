/**
 * @file test_multi_sig.c
 * @brief Tests for multi-signature account functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unity.h"
#include "neoc/neoc.h"
#include "neoc/wallet/multi_sig.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/script/opcode.h"

void setUp(void) {
    neoc_error_t result = neoc_init();
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
}

void tearDown(void) {
    neoc_cleanup();
}

void test_multi_sig_create_2_of_3(void) {
    // Create 3 key pairs
    neoc_ec_key_pair_t *key1 = NULL, *key2 = NULL, *key3 = NULL;
    neoc_error_t err;
    
    err = neoc_ec_key_pair_create_random(&key1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key1);
    
    err = neoc_ec_key_pair_create_random(&key2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key2);
    
    err = neoc_ec_key_pair_create_random(&key3);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key3);
    
    // Get public keys (directly access structure members)
    neoc_ec_public_key_t *pub_keys[3] = {
        key1->public_key,
        key2->public_key,
        key3->public_key
    };
    
    // Create 2-of-3 multi-sig account
    neoc_multi_sig_account_t *account = NULL;
    err = neoc_multi_sig_create(2, pub_keys, 3, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    
    // Verify account properties
    TEST_ASSERT_EQUAL_UINT8(2, account->threshold);
    TEST_ASSERT_EQUAL_UINT8(3, account->public_key_count);
    TEST_ASSERT_NOT_NULL(account->verification_script);
    TEST_ASSERT_TRUE(account->script_size > 0);
    
    // Clean up
    neoc_multi_sig_free(account);
    neoc_ec_key_pair_free(key1);
    neoc_ec_key_pair_free(key2);
    neoc_ec_key_pair_free(key3);
}

void test_multi_sig_verification_script(void) {
    // Create 2 key pairs
    neoc_ec_key_pair_t *key1 = NULL, *key2 = NULL;
    neoc_error_t err;
    
    err = neoc_ec_key_pair_create_random(&key1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_ec_key_pair_create_random(&key2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get public keys
    neoc_ec_public_key_t *pub_keys[2] = {
        key1->public_key,
        key2->public_key
    };
    
    // Create 1-of-2 multi-sig account
    neoc_multi_sig_account_t *account = NULL;
    err = neoc_multi_sig_create(1, pub_keys, 2, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    
    // Get verification script
    uint8_t script[256];
    size_t actual_size;
    err = neoc_multi_sig_get_verification_script(account, script, sizeof(script), &actual_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(actual_size > 0);
    TEST_ASSERT_TRUE(actual_size <= sizeof(script));
    
    // Verify script starts with PUSH1 (threshold = 1)
    TEST_ASSERT_EQUAL_HEX8(NEOC_OP_PUSH1, script[0]);
    
    // Verify script contains SYSCALL near the end
    TEST_ASSERT_TRUE(actual_size >= 5);
    TEST_ASSERT_EQUAL_HEX8(NEOC_OP_SYSCALL, script[actual_size - 5]);
    
    // Clean up
    neoc_multi_sig_free(account);
    neoc_ec_key_pair_free(key1);
    neoc_ec_key_pair_free(key2);
}

void test_multi_sig_script_hash(void) {
    // Create a key pair
    neoc_ec_key_pair_t *key = NULL;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_public_key_t *pub_key = key->public_key;
    
    // Create 1-of-1 multi-sig account (equivalent to single sig)
    neoc_multi_sig_account_t *account = NULL;
    err = neoc_multi_sig_create(1, &pub_key, 1, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(account);
    
    // Get script hash
    neoc_hash160_t hash;
    err = neoc_multi_sig_get_script_hash(account, &hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify hash is not zero
    neoc_hash160_t zero_hash;
    neoc_hash160_init_zero(&zero_hash);
    TEST_ASSERT_FALSE(neoc_hash160_equal(&hash, &zero_hash));
    
    // Clean up
    neoc_multi_sig_free(account);
    neoc_ec_key_pair_free(key);
}

void test_multi_sig_contains_key(void) {
    // Create 3 key pairs
    neoc_ec_key_pair_t *key1 = NULL, *key2 = NULL, *key3 = NULL;
    neoc_error_t err;
    
    err = neoc_ec_key_pair_create_random(&key1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_ec_key_pair_create_random(&key2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_ec_key_pair_create_random(&key3);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_public_key_t *pub_keys[2] = {
        key1->public_key,
        key2->public_key
    };
    
    // Create multi-sig with only key1 and key2
    neoc_multi_sig_account_t *account = NULL;
    err = neoc_multi_sig_create(2, pub_keys, 2, &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test contains
    TEST_ASSERT_TRUE(neoc_multi_sig_contains_key(account, pub_keys[0]));
    TEST_ASSERT_TRUE(neoc_multi_sig_contains_key(account, pub_keys[1]));
    TEST_ASSERT_FALSE(neoc_multi_sig_contains_key(account, key3->public_key));
    
    // Clean up
    neoc_multi_sig_free(account);
    neoc_ec_key_pair_free(key1);
    neoc_ec_key_pair_free(key2);
    neoc_ec_key_pair_free(key3);
}

void test_multi_sig_invalid_threshold(void) {
    neoc_ec_key_pair_t *key = NULL;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_public_key_t *pub_key = key->public_key;
    neoc_multi_sig_account_t *account = NULL;
    
    // Test threshold = 0
    err = neoc_multi_sig_create(0, &pub_key, 1, &account);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    TEST_ASSERT_NULL(account);
    
    // Test threshold > key count
    err = neoc_multi_sig_create(2, &pub_key, 1, &account);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    TEST_ASSERT_NULL(account);
    
    neoc_ec_key_pair_free(key);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_multi_sig_create_2_of_3);
    RUN_TEST(test_multi_sig_verification_script);
    RUN_TEST(test_multi_sig_script_hash);
    RUN_TEST(test_multi_sig_contains_key);
    RUN_TEST(test_multi_sig_invalid_threshold);
    
    UNITY_END();
}
