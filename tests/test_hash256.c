/**
 * @file test_hash256.c
 * @brief Hash256 tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/types/neoc_hash256.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== HASH256 TESTS ===== */

void test_from_valid_hash(void) {
    neoc_hash256_t hash1, hash2;
    
    // Test with 0x prefix
    neoc_error_t err = neoc_hash256_from_hex(&hash1, "0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test without 0x prefix
    err = neoc_hash256_from_hex(&hash2, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Both should be equal
    TEST_ASSERT_TRUE(neoc_hash256_equal(&hash1, &hash2));
    
    // Convert back to hex and verify
    char hex[65];
    err = neoc_hash256_to_hex(&hash1, hex, sizeof(hex), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", hex);
}

void test_creation_throws(void) {
    neoc_hash256_t hash;
    
    // Test with invalid hex (odd length)
    neoc_error_t err = neoc_hash256_from_hex(&hash, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21ae");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test with invalid character
    err = neoc_hash256_from_hex(&hash, "g804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test with wrong length (31 bytes / 62 chars)
    err = neoc_hash256_from_hex(&hash, "0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a2");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test with wrong length (33 bytes / 66 chars)
    err = neoc_hash256_from_hex(&hash, "0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a12");
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_from_bytes(void) {
    const char* hex_str = "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a";
    
    // Decode hex to bytes
    uint8_t bytes[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(hex_str, bytes, sizeof(bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    // Create hash from bytes
    neoc_hash256_t hash;
    err = neoc_hash256_from_bytes(&hash, bytes);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert back to hex and verify
    char hex[65];
    err = neoc_hash256_to_hex(&hash, hex, sizeof(hex), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING(hex_str, hex);
}

void test_to_array(void) {
    neoc_hash256_t hash;
    neoc_error_t err = neoc_hash256_from_hex(&hash, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get little-endian bytes
    uint8_t le_bytes[32];
    err = neoc_hash256_to_little_endian_bytes(&hash, le_bytes, sizeof(le_bytes));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should be reversed from original
    uint8_t expected[] = {
        0x1a, 0xa2, 0x74, 0x39, 0x1a, 0xb7, 0x12, 0x7c,
        0xa6, 0xd6, 0xb9, 0x17, 0xd4, 0x13, 0x91, 0x90,
        0x00, 0xeb, 0xee, 0x2b, 0x14, 0x97, 0x4e, 0x67,
        0xb4, 0x9a, 0xc6, 0x20, 0x82, 0xa9, 0x04, 0xb8
    };
    TEST_ASSERT_EQUAL_MEMORY(expected, le_bytes, 32);
}

void test_serialize_and_deserialize(void) {
    const char* hex_str = "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a";
    
    // Create hash from hex
    neoc_hash256_t hash;
    neoc_error_t err = neoc_hash256_from_hex(&hash, hex_str);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Serialize to little-endian bytes
    uint8_t serialized[32];
    err = neoc_hash256_to_little_endian_bytes(&hash, serialized, sizeof(serialized));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Deserialize from little-endian bytes (reverse back to big-endian)
    neoc_hash256_t hash2;
    uint8_t reversed[32];
    for (int i = 0; i < 32; i++) {
        reversed[i] = serialized[31 - i];
    }
    err = neoc_hash256_from_bytes(&hash2, reversed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should be equal
    TEST_ASSERT_TRUE(neoc_hash256_equal(&hash, &hash2));
    
    // Convert back to hex and verify
    char hex[65];
    err = neoc_hash256_to_hex(&hash2, hex, sizeof(hex), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING(hex_str, hex);
}

void test_equals(void) {
    // Create hashes from different formats
    neoc_hash256_t hash1, hash2, hash3;
    
    // Create from little-endian bytes (reversed)
    const char* hex1 = "1aa274391ab7127ca6d6b917d413919000ebee2b14974e67b49ac62082a904b8";
    uint8_t bytes1[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(hex1, bytes1, sizeof(bytes1), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Reverse bytes for little-endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes1[i];
        bytes1[i] = bytes1[31 - i];
        bytes1[31 - i] = temp;
    }
    
    err = neoc_hash256_from_bytes(&hash1, bytes1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create second hash
    const char* hex2 = "b43034ab680d646f8b6ca71647aa6ba167b2eb0b3757e545f6c2715787b13272";
    uint8_t bytes2[32];
    err = neoc_hex_decode(hex2, bytes2, sizeof(bytes2), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Reverse bytes for little-endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes2[i];
        bytes2[i] = bytes2[31 - i];
        bytes2[31 - i] = temp;
    }
    
    err = neoc_hash256_from_bytes(&hash2, bytes2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create third hash from hex with 0x prefix
    err = neoc_hash256_from_hex(&hash3, "0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test equality
    TEST_ASSERT_FALSE(neoc_hash256_equal(&hash1, &hash2));
    TEST_ASSERT_TRUE(neoc_hash256_equal(&hash1, &hash1));
    TEST_ASSERT_TRUE(neoc_hash256_equal(&hash1, &hash3));
}

void test_compare_to(void) {
    neoc_hash256_t hash1, hash2, hash3;
    
    // Create from little-endian bytes (reversed)
    const char* hex1 = "1aa274391ab7127ca6d6b917d413919000ebee2b14974e67b49ac62082a904b8";
    uint8_t bytes1[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(hex1, bytes1, sizeof(bytes1), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Reverse bytes for little-endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes1[i];
        bytes1[i] = bytes1[31 - i];
        bytes1[31 - i] = temp;
    }
    
    err = neoc_hash256_from_bytes(&hash1, bytes1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create second hash
    const char* hex2 = "b43034ab680d646f8b6ca71647aa6ba167b2eb0b3757e545f6c2715787b13272";
    uint8_t bytes2[32];
    err = neoc_hex_decode(hex2, bytes2, sizeof(bytes2), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Reverse bytes for little-endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes2[i];
        bytes2[i] = bytes2[31 - i];
        bytes2[31 - i] = temp;
    }
    
    err = neoc_hash256_from_bytes(&hash2, bytes2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create third hash from hex
    err = neoc_hash256_from_hex(&hash3, "0xf4609b99e171190c22adcf70c88a7a14b5b530914d2398287bd8bb7ad95a661c");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Compare hashes
    int cmp12 = neoc_hash256_compare(&hash1, &hash2);
    int cmp13 = neoc_hash256_compare(&hash1, &hash3);
    int cmp23 = neoc_hash256_compare(&hash2, &hash3);
    
    // Based on test expectations
    TEST_ASSERT_TRUE(cmp12 > 0);  // hash1 > hash2
    TEST_ASSERT_TRUE(cmp13 < 0);  // hash3 > hash1
    TEST_ASSERT_TRUE(cmp23 < 0);  // hash3 > hash2
}

void test_size(void) {
    neoc_hash256_t hash;
    neoc_error_t err = neoc_hash256_from_hex(&hash, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Hash256 should be 32 bytes (size is a constant)
    TEST_ASSERT_EQUAL_INT(32, NEOC_HASH256_SIZE);
}

void test_zero_hash(void) {
    neoc_hash256_t hash;
    
    // Initialize to zero
    neoc_error_t err = neoc_hash256_init_zero(&hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Check if zero
    TEST_ASSERT_TRUE(neoc_hash256_is_zero(&hash));
    
    // Create non-zero hash
    err = neoc_hash256_from_hex(&hash, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should not be zero
    TEST_ASSERT_FALSE(neoc_hash256_is_zero(&hash));
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== HASH256 TESTS ===\n");
    
    RUN_TEST(test_from_valid_hash);
    RUN_TEST(test_creation_throws);
    RUN_TEST(test_from_bytes);
    RUN_TEST(test_to_array);
    RUN_TEST(test_serialize_and_deserialize);
    RUN_TEST(test_equals);
    RUN_TEST(test_compare_to);
    RUN_TEST(test_size);
    RUN_TEST(test_zero_hash);
    
    UNITY_END();
    return 0;
}
