/**
 * @file test_witness.c
 * @brief Transaction witness tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/transaction/witness.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== WITNESS CREATION TESTS ===== */

void test_create_witness(void) {
    // Create invocation and verification scripts
    uint8_t invocation_script[10];
    uint8_t verification_script[10];
    memset(invocation_script, 0xAA, sizeof(invocation_script));
    memset(verification_script, 0xBB, sizeof(verification_script));
    
    // Create witness
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create(
        invocation_script, sizeof(invocation_script),
        verification_script, sizeof(verification_script),
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    
    // Verify witness has scripts
    TEST_ASSERT_NOT_NULL(witness->invocation_script);
    TEST_ASSERT_NOT_NULL(witness->verification_script);
    TEST_ASSERT_EQUAL_INT(10, witness->invocation_script_len);
    TEST_ASSERT_EQUAL_INT(10, witness->verification_script_len);
    
    neoc_witness_free(witness);
}

void test_serialize_witness(void) {
    // Create invocation and verification scripts
    uint8_t invocation_script[20];
    uint8_t verification_script[30];
    memset(invocation_script, 0xAA, sizeof(invocation_script));
    memset(verification_script, 0xBB, sizeof(verification_script));
    
    // Create witness
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create(
        invocation_script, sizeof(invocation_script),
        verification_script, sizeof(verification_script),
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Serialize witness
    uint8_t* buffer = NULL;
    size_t serialized_size;
    err = neoc_witness_serialize(witness, &buffer, &serialized_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(serialized_size > 0);
    
    // Cleanup
    neoc_free(buffer);
    neoc_witness_free(witness);
}

void test_create_witness_from_signature(void) {
    // Create a test signature (64 bytes for ECDSA)
    uint8_t signature[64];
    for (int i = 0; i < 64; i++) {
        signature[i] = (uint8_t)(i + 1);
    }
    
    // Create a test public key (33 bytes compressed)
    uint8_t public_key[33];
    public_key[0] = 0x02; // Compressed key prefix
    for (int i = 1; i < 33; i++) {
        public_key[i] = (uint8_t)(0x10 + i);
    }
    
    // Create witness from signature
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create_from_signature(
        signature, sizeof(signature),
        public_key, sizeof(public_key),
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    
    // Verify witness has scripts
    TEST_ASSERT_NOT_NULL(witness->invocation_script);
    TEST_ASSERT_NOT_NULL(witness->verification_script);
    TEST_ASSERT_TRUE(witness->invocation_script_len > 0);
    TEST_ASSERT_TRUE(witness->verification_script_len > 0);
    
    neoc_witness_free(witness);
}

void test_witness_size(void) {
    // Create custom scripts
    uint8_t invocation_script[10];
    uint8_t verification_script[10];
    memset(invocation_script, 1, sizeof(invocation_script));
    memset(verification_script, 2, sizeof(verification_script));
    
    // Create witness
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create(
        invocation_script, sizeof(invocation_script),
        verification_script, sizeof(verification_script),
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    
    // Get size
    size_t size = neoc_witness_get_size(witness);
    TEST_ASSERT_TRUE(size > 0);
    // Should be at least: 1 byte length + 10 bytes invocation + 1 byte length + 10 bytes verification = 22
    TEST_ASSERT_TRUE(size >= 22);
    
    neoc_witness_free(witness);
}

void test_deserialize_witness(void) {
    // Create a witness first
    uint8_t invocation_script[15];
    uint8_t verification_script[20];
    memset(invocation_script, 0xCC, sizeof(invocation_script));
    memset(verification_script, 0xDD, sizeof(verification_script));
    
    neoc_witness_t* original = NULL;
    neoc_error_t err = neoc_witness_create(
        invocation_script, sizeof(invocation_script),
        verification_script, sizeof(verification_script),
        &original
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Serialize it
    uint8_t* buffer = NULL;
    size_t serialized_size;
    err = neoc_witness_serialize(original, &buffer, &serialized_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Deserialize it
    neoc_witness_t* deserialized = NULL;
    err = neoc_witness_deserialize(buffer, serialized_size, &deserialized);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(deserialized);
    
    // Verify both have same script lengths
    TEST_ASSERT_EQUAL_INT(original->invocation_script_len, deserialized->invocation_script_len);
    TEST_ASSERT_EQUAL_INT(original->verification_script_len, deserialized->verification_script_len);
    
    // Verify script contents match
    TEST_ASSERT_EQUAL_MEMORY(original->invocation_script, deserialized->invocation_script, original->invocation_script_len);
    TEST_ASSERT_EQUAL_MEMORY(original->verification_script, deserialized->verification_script, original->verification_script_len);
    
    neoc_free(buffer);
    neoc_witness_free(original);
    neoc_witness_free(deserialized);
}

void test_witness_empty_scripts(void) {
    // Test with empty invocation script
    uint8_t verification_script[10];
    memset(verification_script, 0xEE, sizeof(verification_script));
    
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create(
        NULL, 0,
        verification_script, sizeof(verification_script),
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    TEST_ASSERT_NULL(witness->invocation_script);
    TEST_ASSERT_EQUAL_INT(0, witness->invocation_script_len);
    TEST_ASSERT_NOT_NULL(witness->verification_script);
    TEST_ASSERT_EQUAL_INT(10, witness->verification_script_len);
    
    neoc_witness_free(witness);
    
    // Test with empty verification script
    uint8_t invocation_script[10];
    memset(invocation_script, 0xFF, sizeof(invocation_script));
    
    witness = NULL;
    err = neoc_witness_create(
        invocation_script, sizeof(invocation_script),
        NULL, 0,
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    TEST_ASSERT_NOT_NULL(witness->invocation_script);
    TEST_ASSERT_EQUAL_INT(10, witness->invocation_script_len);
    TEST_ASSERT_NULL(witness->verification_script);
    TEST_ASSERT_EQUAL_INT(0, witness->verification_script_len);
    
    neoc_witness_free(witness);
}

void test_witness_both_empty(void) {
    // Create witness with both scripts empty
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create(NULL, 0, NULL, 0, &witness);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    
    // Should have empty scripts
    TEST_ASSERT_NULL(witness->invocation_script);
    TEST_ASSERT_NULL(witness->verification_script);
    TEST_ASSERT_EQUAL_INT(0, witness->invocation_script_len);
    TEST_ASSERT_EQUAL_INT(0, witness->verification_script_len);
    
    neoc_witness_free(witness);
}

void test_witness_large_scripts(void) {
    // Create large scripts
    uint8_t invocation_script[256];
    uint8_t verification_script[512];
    for (int i = 0; i < 256; i++) {
        invocation_script[i] = (uint8_t)(i & 0xFF);
    }
    for (int i = 0; i < 512; i++) {
        verification_script[i] = (uint8_t)((i * 2) & 0xFF);
    }
    
    // Create witness
    neoc_witness_t* witness = NULL;
    neoc_error_t err = neoc_witness_create(
        invocation_script, sizeof(invocation_script),
        verification_script, sizeof(verification_script),
        &witness
    );
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(witness);
    
    // Verify sizes
    TEST_ASSERT_EQUAL_INT(256, witness->invocation_script_len);
    TEST_ASSERT_EQUAL_INT(512, witness->verification_script_len);
    
    // Serialize and verify we can handle large scripts
    uint8_t* buffer = NULL;
    size_t serialized_size;
    err = neoc_witness_serialize(witness, &buffer, &serialized_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(serialized_size > 768); // At least the script sizes
    
    neoc_free(buffer);
    neoc_witness_free(witness);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== WITNESS TESTS ===\n");
    
    RUN_TEST(test_create_witness);
    RUN_TEST(test_serialize_witness);
    RUN_TEST(test_create_witness_from_signature);
    RUN_TEST(test_witness_size);
    RUN_TEST(test_deserialize_witness);
    RUN_TEST(test_witness_empty_scripts);
    RUN_TEST(test_witness_both_empty);
    RUN_TEST(test_witness_large_scripts);
    
    UNITY_END();
    return 0;
}
