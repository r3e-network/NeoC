/**
 * @file test_signer.c
 * @brief Transaction signer tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/transaction/signer.h>
#include <neoc/types/neoc_hash160.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

// Test constants
// static const char* test_account_wif = "Kzt94tAAiZSgH7Yt4i25DW6jJFprZFPSqTgLr5dWmWgKDKCjXMfZ";
// static const char* contract1_script = "d802a401";
// static const char* contract2_script = "c503b112";
static const char* group_pubkey1_hex = "0306d3e7f18e6dd477d34ce3cfeca172a877f3c907cc6c2b66c295d1fcc76ff8f7";
static const char* group_pubkey2_hex = "02958ab88e4cea7ae1848047daeb8883daf5fdf5c1301dbbfe973f0a29fe75de60";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== SIGNER CREATION TESTS ===== */

void test_create_signer_with_called_by_entry_scope(void) {
    // Create a test script hash
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create signer with CALLED_BY_ENTRY scope
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_called_by_entry(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(signer);
    
    // Verify signer properties
    TEST_ASSERT_TRUE(neoc_signer_has_called_by_entry_scope(signer));
    TEST_ASSERT_FALSE(neoc_signer_has_global_scope(signer));
    TEST_ASSERT_FALSE(neoc_signer_has_custom_contracts_scope(signer));
    TEST_ASSERT_FALSE(neoc_signer_has_custom_groups_scope(signer));
    
    neoc_signer_free(signer);
}

void test_create_signer_with_global_scope(void) {
    // Create a test script hash
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create signer with GLOBAL scope
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_global(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(signer);
    
    // Verify scope
    TEST_ASSERT_TRUE(neoc_signer_has_global_scope(signer));
    TEST_ASSERT_FALSE(neoc_signer_has_called_by_entry_scope(signer));
    
    neoc_signer_free(signer);
}

void test_create_signer_with_none_scope(void) {
    // Create a test script hash
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create signer with NONE scope
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create(&script_hash, NEOC_WITNESS_SCOPE_NONE, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(signer);
    
    // Verify scope
    TEST_ASSERT_FALSE(neoc_signer_has_global_scope(signer));
    TEST_ASSERT_FALSE(neoc_signer_has_called_by_entry_scope(signer));
    TEST_ASSERT_FALSE(neoc_signer_has_custom_contracts_scope(signer));
    
    neoc_signer_free(signer);
}

/* ===== ALLOWED CONTRACTS TESTS ===== */

void test_add_allowed_contracts(void) {
    // Create contract hashes
    neoc_hash160_t contract1, contract2;
    
    // For testing, just create dummy hashes
    neoc_error_t err = neoc_hash160_from_hex(&contract1, "abcdefabcdefabcdefabcdefabcdefabcdefabcd");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_hash160_from_hex(&contract2, "1234567890123456789012345678901234567890");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create signer
    neoc_hash160_t script_hash;
    err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_called_by_entry(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add allowed contracts
    err = neoc_signer_add_allowed_contract(signer, &contract1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_signer_add_allowed_contract(signer, &contract2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify scope updated to include CUSTOM_CONTRACTS
    TEST_ASSERT_TRUE(neoc_signer_has_custom_contracts_scope(signer));
    
    // Verify we still have called by entry scope
    TEST_ASSERT_TRUE(neoc_signer_has_called_by_entry_scope(signer));
    
    neoc_signer_free(signer);
}

void test_fail_adding_contracts_with_global_scope(void) {
    // Create signer with GLOBAL scope
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_global(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to add allowed contract - should fail
    neoc_hash160_t contract;
    err = neoc_hash160_from_hex(&contract, "abcdefabcdefabcdefabcdefabcdefabcdefabcd");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_signer_add_allowed_contract(signer, &contract);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_signer_free(signer);
}

/* ===== ALLOWED GROUPS TESTS ===== */

void test_add_allowed_groups(void) {
    // Create public keys for groups
    // Group public keys are 33 bytes (compressed EC public keys)
    uint8_t group1[33], group2[33];
    size_t decoded_len;
    
    neoc_error_t err = neoc_hex_decode(group_pubkey1_hex, group1, sizeof(group1), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, decoded_len);
    
    err = neoc_hex_decode(group_pubkey2_hex, group2, sizeof(group2), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, decoded_len);
    
    // Create signer
    neoc_hash160_t script_hash;
    err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create(&script_hash, NEOC_WITNESS_SCOPE_NONE, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add allowed groups
    err = neoc_signer_add_allowed_group(signer, group1, 33);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_signer_add_allowed_group(signer, group2, 33);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify scope updated to include CUSTOM_GROUPS
    TEST_ASSERT_TRUE(neoc_signer_has_custom_groups_scope(signer));
    
    neoc_signer_free(signer);
}

void test_fail_adding_groups_with_global_scope(void) {
    // Create signer with GLOBAL scope
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_global(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to add allowed group - should fail
    uint8_t group[33];
    size_t decoded_len;
    err = neoc_hex_decode(group_pubkey1_hex, group, sizeof(group), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_signer_add_allowed_group(signer, group, 33);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_signer_free(signer);
}

/* ===== LIMIT TESTS ===== */

void test_fail_adding_too_many_contracts(void) {
    // Create signer
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_called_by_entry(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to add 17 contracts (more than MAX_SIGNER_SUBITEMS which is 16)
    neoc_hash160_t contracts[17];
    for (int i = 0; i < 17; i++) {
        char hex[45];
        sprintf(hex, "0x%040d", i);
        err = neoc_hash160_from_hex(&contracts[i], hex);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        err = neoc_signer_add_allowed_contract(signer, &contracts[i]);
        if (i < 16) {
            // First 16 should succeed
            TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        } else {
            // 17th should fail
            TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        }
    }
    
    neoc_signer_free(signer);
}

void test_fail_adding_too_many_groups(void) {
    // Create signer
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_called_by_entry(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to add 17 groups (more than MAX_SIGNER_SUBITEMS)
    uint8_t group[33];
    size_t decoded_len;
    err = neoc_hex_decode(group_pubkey1_hex, group, sizeof(group), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    for (int i = 0; i < 17; i++) {
        err = neoc_signer_add_allowed_group(signer, group, 33);
        if (i < 16) {
            // First 16 should succeed
            TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        } else {
            // 17th should fail
            TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        }
    }
    
    neoc_signer_free(signer);
}

/* ===== SIZE TEST ===== */

void test_signer_size(void) {
    // Create a signer
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_called_by_entry(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get size
    size_t size = neoc_signer_get_size(signer);
    TEST_ASSERT_TRUE(size > 0);
    
    neoc_signer_free(signer);
}

/* ===== COPY TEST ===== */

void test_signer_copy(void) {
    // Create a signer with some properties
    neoc_hash160_t script_hash;
    neoc_error_t err = neoc_hash160_from_hex(&script_hash, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_t* signer = NULL;
    err = neoc_signer_create_called_by_entry(&script_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add a contract
    neoc_hash160_t contract;
    err = neoc_hash160_from_hex(&contract, "abcdefabcdefabcdefabcdefabcdefabcdefabcd");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    err = neoc_signer_add_allowed_contract(signer, &contract);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Copy the signer
    neoc_signer_t* copy = NULL;
    err = neoc_signer_copy(signer, &copy);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(copy);
    
    // Verify copy has same properties
    TEST_ASSERT_TRUE(neoc_signer_has_called_by_entry_scope(copy));
    TEST_ASSERT_TRUE(neoc_signer_has_custom_contracts_scope(copy));
    
    neoc_signer_free(signer);
    neoc_signer_free(copy);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== SIGNER TESTS ===\n");
    
    RUN_TEST(test_create_signer_with_called_by_entry_scope);
    RUN_TEST(test_create_signer_with_global_scope);
    RUN_TEST(test_create_signer_with_none_scope);
    RUN_TEST(test_add_allowed_contracts);
    RUN_TEST(test_fail_adding_contracts_with_global_scope);
    RUN_TEST(test_add_allowed_groups);
    RUN_TEST(test_fail_adding_groups_with_global_scope);
    RUN_TEST(test_fail_adding_too_many_contracts);
    RUN_TEST(test_fail_adding_too_many_groups);
    RUN_TEST(test_signer_size);
    RUN_TEST(test_signer_copy);
    
    UNITY_END();
    return 0;
}
