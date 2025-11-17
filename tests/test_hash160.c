/**
 * @file test_hash160.c
 * @brief Hash160 tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/types/neoc_hash160.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== HASH160 TESTS ===== */

void test_from_valid_hash(void) {
    neoc_hash160_t hash1, hash2;
    
    // Test with 0x prefix
    neoc_error_t err = neoc_hash160_from_hex(&hash1, "0x23ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test without 0x prefix
    err = neoc_hash160_from_hex(&hash2, "23ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Both should be equal
    TEST_ASSERT_TRUE(neoc_hash160_equal(&hash1, &hash2));
    
    // Convert back to hex and verify
    char hex[41];
    err = neoc_hash160_to_hex(&hash1, hex, sizeof(hex), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("23ba2703c53263e8d6e522dc32203339dcd8eee9", hex);
}

void test_creation_throws(void) {
    neoc_hash160_t hash;
    
    // Test with odd length hex - should succeed with padding
    neoc_error_t err = neoc_hash160_from_hex(&hash, "0x23ba2703c53263e8d6e522dc32203339dcd8eee");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test with invalid character
    err = neoc_hash160_from_hex(&hash, "g3ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test with wrong length (19 bytes / 38 chars)
    err = neoc_hash160_from_hex(&hash, "23ba2703c53263e8d6e522dc32203339dcd8ee");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test with wrong length (32 bytes / 64 chars)
    err = neoc_hash160_from_hex(&hash, "c56f33fc6ecfcd0c225c4ab356fee59390af8560be0e930faebe74a6daff7c9b");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_to_array(void) {
    neoc_hash160_t hash;
    neoc_error_t err = neoc_hash160_from_hex(&hash, "23ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get little-endian bytes
    uint8_t le_bytes[20];
    err = neoc_hash160_to_little_endian_bytes(&hash, le_bytes, sizeof(le_bytes));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should be reversed from original
    uint8_t expected[] = {
        0xe9, 0xee, 0xd8, 0xdc, 0x39, 0x33, 0x20, 0x32,
        0xdc, 0x22, 0xe5, 0xd6, 0xe8, 0x63, 0x32, 0xc5,
        0x03, 0x27, 0xba, 0x23
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, le_bytes, 20);
}

void test_equals(void) {
    neoc_hash160_t hash1, hash2, hash3;
    
    // Create two different hashes from scripts
    uint8_t script1[] = {0x01, 0xa4, 0x02, 0xd8};
    uint8_t script2[] = {0xd8, 0x02, 0xa4, 0x01};
    
    neoc_error_t err = neoc_hash160_from_script(&hash1, script1, sizeof(script1));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_hash160_from_script(&hash2, script2, sizeof(script2));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Copy hash1 to hash3
    err = neoc_hash160_copy(&hash3, &hash1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test equality
    TEST_ASSERT_FALSE(neoc_hash160_equal(&hash1, &hash2));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&hash1, &hash3));
}

void test_from_valid_address(void) {
    neoc_hash160_t hash;
    neoc_error_t err = neoc_hash160_from_address(&hash, "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get little-endian bytes
    uint8_t le_bytes[20];
    err = neoc_hash160_to_little_endian_bytes(&hash, le_bytes, sizeof(le_bytes));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Expected hash in little-endian
    const char* expected_hex = "09a55874c2da4b86e5d49ff530a1b153eb12c7d6";
    uint8_t expected[20];
    size_t decoded_len;
    err = neoc_hex_decode(expected_hex, expected, sizeof(expected), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(20, decoded_len);
    
    TEST_ASSERT_EQUAL_MEMORY(expected, le_bytes, 20);
}

void test_from_invalid_address(void) {
    neoc_hash160_t hash;
    
    // Invalid address (extra characters)
    neoc_error_t err = neoc_hash160_from_address(&hash, "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8keas");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Empty address
    err = neoc_hash160_from_address(&hash, "");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Invalid characters
    err = neoc_hash160_from_address(&hash, "InvalidNeoAddress");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_from_public_key_bytes(void) {
    const char* key_hex = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";
    
    // Decode public key
    uint8_t pubkey[33];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(key_hex, pubkey, sizeof(pubkey), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, decoded_len);
    
    // Create hash from public key
    neoc_hash160_t hash;
    err = neoc_hash160_from_public_key(&hash, pubkey);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify hash is not zero
    TEST_ASSERT_FALSE(neoc_hash160_is_zero(&hash));
}

void test_from_contract_script(void) {
    const char* script_hex = "110c21026aa8fe6b4360a67a530e23c08c6a72525afde34719c5436f9d3ced759f939a3d110b41138defaf";
    
    // Decode script
    size_t hex_len = strlen(script_hex);
    size_t script_len = hex_len / 2;
    uint8_t* script = neoc_malloc(script_len);
    TEST_ASSERT_NOT_NULL(script);
    
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(script_hex, script, script_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create hash from script
    neoc_hash160_t hash;
    err = neoc_hash160_from_script(&hash, script, decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert to hex and verify
    char hex[41];
    err = neoc_hash160_to_hex(&hash, hex, sizeof(hex), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("afaed076854454449770763a628f379721ea9808", hex);
    
    neoc_free(script);
}

void test_to_address(void) {
    // Create hash from a known public key
    const char* pubkey_hex = "0265bf906bf385fbf3f777832e55a87991bcfbe19b097fb7c5ca2e4025a4d5e5d6";
    uint8_t pubkey[33];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(pubkey_hex, pubkey, sizeof(pubkey), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_hash160_t hash;
    err = neoc_hash160_from_public_key(&hash, pubkey);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert to address
    char address[64];
    err = neoc_hash160_to_address(&hash, address, sizeof(address));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify it's a valid Neo address (starts with 'N')
    TEST_ASSERT_EQUAL_INT('N', address[0]);
}

void test_compare_to(void) {
    neoc_hash160_t hash1, hash2, hash3;
    
    // Create three different hashes from scripts
    uint8_t script1[] = {0x01, 0xa4, 0x02, 0xd8};
    uint8_t script2[] = {0xd8, 0x02, 0xa4, 0x01};
    uint8_t script3[] = {0xa7, 0xb3, 0xa1, 0x91};
    
    neoc_error_t err = neoc_hash160_from_script(&hash1, script1, sizeof(script1));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_hash160_from_script(&hash2, script2, sizeof(script2));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_hash160_from_script(&hash3, script3, sizeof(script3));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Compare hashes
    int cmp12 = neoc_hash160_compare(&hash1, &hash2);
    int cmp13 = neoc_hash160_compare(&hash1, &hash3);
    int cmp23 = neoc_hash160_compare(&hash2, &hash3);
    
    // We don't know exact ordering, but they should be different
    TEST_ASSERT_TRUE(cmp12 != 0);
    TEST_ASSERT_TRUE(cmp13 != 0);
    TEST_ASSERT_TRUE(cmp23 != 0);
}

void test_zero_hash(void) {
    neoc_hash160_t hash;
    
    // Initialize to zero
    neoc_error_t err = neoc_hash160_init_zero(&hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Check if zero
    TEST_ASSERT_TRUE(neoc_hash160_is_zero(&hash));
    
    // Create non-zero hash
    err = neoc_hash160_from_hex(&hash, "23ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should not be zero
    TEST_ASSERT_FALSE(neoc_hash160_is_zero(&hash));
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== HASH160 TESTS ===\n");
    
    RUN_TEST(test_from_valid_hash);
    RUN_TEST(test_creation_throws);
    RUN_TEST(test_to_array);
    RUN_TEST(test_equals);
    RUN_TEST(test_from_valid_address);
    RUN_TEST(test_from_invalid_address);
    RUN_TEST(test_from_public_key_bytes);
    RUN_TEST(test_from_contract_script);
    RUN_TEST(test_to_address);
    RUN_TEST(test_compare_to);
    RUN_TEST(test_zero_hash);
    
    UNITY_END();
    return 0;
}
