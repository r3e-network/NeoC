/**
 * @file test_hash256.c
 * @brief Unit tests converted from Hash256Tests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/types/hash256.h"
#include "neoc/utils/hex.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test create from valid hash
static void test_from_valid_hash(void) {
    printf("Testing create from valid hash...\n");
    
    // Test with 0x prefix
    neoc_hash256_t *hash1 = NULL;
    neoc_error_t err = neoc_hash256_from_string("0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", &hash1);
    assert(err == NEOC_SUCCESS);
    assert(hash1 != NULL);
    
    const char *str1 = neoc_hash256_to_string(hash1);
    assert(strcmp(str1, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a") == 0);
    
    // Test without 0x prefix
    neoc_hash256_t *hash2 = NULL;
    err = neoc_hash256_from_string("b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", &hash2);
    assert(err == NEOC_SUCCESS);
    assert(hash2 != NULL);
    
    const char *str2 = neoc_hash256_to_string(hash2);
    assert(strcmp(str2, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a") == 0);
    
    neoc_hash256_free(hash1);
    neoc_hash256_free(hash2);
    printf("  ✅ Create from valid hash test passed\n");
}

// Test creation errors
static void test_creation_throws(void) {
    printf("Testing creation error handling...\n");
    
    neoc_hash256_t *hash = NULL;
    neoc_error_t err;
    
    // Test invalid hex (odd length)
    err = neoc_hash256_from_string("b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21ae", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    // Test invalid hex character
    err = neoc_hash256_from_string("g804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    // Test too short (31 bytes)
    err = neoc_hash256_from_string("0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a2", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    // Test too long (33 bytes)
    err = neoc_hash256_from_string("0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a12", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    printf("  ✅ Creation error handling test passed\n");
}

// Test from bytes
static void test_from_bytes(void) {
    printf("Testing from bytes...\n");
    
    const char *hash_hex = "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a";
    uint8_t bytes[32];
    size_t bytes_len = 0;
    neoc_error_t err = neoc_hex_decode(hash_hex, bytes, sizeof(bytes), &bytes_len);
    assert(err == NEOC_SUCCESS);
    assert(bytes_len == 32);
    
    neoc_hash256_t *hash = NULL;
    err = neoc_hash256_from_bytes(bytes, bytes_len, &hash);
    assert(err == NEOC_SUCCESS);
    assert(hash != NULL);
    
    const char *hash_str = neoc_hash256_to_string(hash);
    assert(strcmp(hash_str, hash_hex) == 0);
    
    neoc_hash256_free(hash);
    printf("  ✅ From bytes test passed\n");
}

// Test to little endian array
static void test_to_array(void) {
    printf("Testing to little endian array...\n");
    
    neoc_hash256_t *hash = NULL;
    neoc_error_t err = neoc_hash256_from_string("b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", &hash);
    assert(err == NEOC_SUCCESS);
    
    size_t array_len = 0;
    const uint8_t *array = neoc_hash256_to_little_endian_array(hash, &array_len);
    assert(array != NULL);
    assert(array_len == 32);
    
    // Verify it's reversed
    uint8_t expected[32];
    size_t expected_len = 0;
    err = neoc_hex_decode("b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", expected, sizeof(expected), &expected_len);
    assert(err == NEOC_SUCCESS);
    
    // Check reversed order
    for (size_t i = 0; i < 32; i++) {
        assert(array[i] == expected[31 - i]);
    }
    
    neoc_hash256_free(hash);
    printf("  ✅ To little endian array test passed\n");
}

// Test serialize and deserialize
static void test_serialize_and_deserialize(void) {
    printf("Testing serialize and deserialize...\n");
    
    const char *hash_str = "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a";
    
    neoc_hash256_t *hash = NULL;
    neoc_error_t err = neoc_hash256_from_string(hash_str, &hash);
    assert(err == NEOC_SUCCESS);
    
    // Serialize
    neoc_binary_writer_t *writer = NULL;
    err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_hash256_serialize(hash, writer);
    assert(err == NEOC_SUCCESS);
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_len);
    assert(err == NEOC_SUCCESS);
    assert(data_len == 32);
    
    // Verify it's reversed (little endian)
    uint8_t expected[32];
    size_t expected_len = 0;
    err = neoc_hex_decode(hash_str, expected, sizeof(expected), &expected_len);
    assert(err == NEOC_SUCCESS);
    
    for (size_t i = 0; i < 32; i++) {
        assert(data[i] == expected[31 - i]);
    }
    
    // Deserialize
    neoc_binary_reader_t *reader = NULL;
    err = neoc_binary_reader_create(data, data_len, &reader);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash256_t *deserialized = NULL;
    err = neoc_hash256_deserialize(reader, &deserialized);
    assert(err == NEOC_SUCCESS);
    
    // Verify
    const char *deserialized_str = neoc_hash256_to_string(deserialized);
    assert(strcmp(deserialized_str, hash_str) == 0);
    
    free(data);
    neoc_binary_writer_free(writer);
    neoc_binary_reader_free(reader);
    neoc_hash256_free(hash);
    neoc_hash256_free(deserialized);
    
    printf("  ✅ Serialize and deserialize test passed\n");
}

// Test equals
static void test_equals(void) {
    printf("Testing equals...\n");
    
    // Create first hash from reversed bytes
    const char *hex1 = "1aa274391ab7127ca6d6b917d413919000ebee2b14974e67b49ac62082a904b8";
    uint8_t bytes1[32];
    size_t bytes1_len = 0;
    neoc_error_t err = neoc_hex_decode(hex1, bytes1, sizeof(bytes1), &bytes1_len);
    assert(err == NEOC_SUCCESS);
    
    // Reverse for little endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes1[i];
        bytes1[i] = bytes1[31 - i];
        bytes1[31 - i] = temp;
    }
    
    neoc_hash256_t *hash1 = NULL;
    err = neoc_hash256_from_bytes(bytes1, bytes1_len, &hash1);
    assert(err == NEOC_SUCCESS);
    
    // Create second hash from different reversed bytes
    const char *hex2 = "b43034ab680d646f8b6ca71647aa6ba167b2eb0b3757e545f6c2715787b13272";
    uint8_t bytes2[32];
    size_t bytes2_len = 0;
    err = neoc_hex_decode(hex2, bytes2, sizeof(bytes2), &bytes2_len);
    assert(err == NEOC_SUCCESS);
    
    // Reverse for little endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes2[i];
        bytes2[i] = bytes2[31 - i];
        bytes2[31 - i] = temp;
    }
    
    neoc_hash256_t *hash2 = NULL;
    err = neoc_hash256_from_bytes(bytes2, bytes2_len, &hash2);
    assert(err == NEOC_SUCCESS);
    
    // Create third hash that should equal hash1
    neoc_hash256_t *hash3 = NULL;
    err = neoc_hash256_from_string("0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", &hash3);
    assert(err == NEOC_SUCCESS);
    
    // Test equality
    assert(neoc_hash256_equals(hash1, hash2) == false);
    assert(neoc_hash256_equals(hash1, hash1) == true);
    assert(neoc_hash256_equals(hash1, hash3) == true);
    
    neoc_hash256_free(hash1);
    neoc_hash256_free(hash2);
    neoc_hash256_free(hash3);
    
    printf("  ✅ Equals test passed\n");
}

// Test compare
static void test_compare_to(void) {
    printf("Testing compare to...\n");
    
    // Create first hash from reversed bytes
    const char *hex1 = "1aa274391ab7127ca6d6b917d413919000ebee2b14974e67b49ac62082a904b8";
    uint8_t bytes1[32];
    size_t bytes1_len = 0;
    neoc_error_t err = neoc_hex_decode(hex1, bytes1, sizeof(bytes1), &bytes1_len);
    assert(err == NEOC_SUCCESS);
    
    // Reverse for little endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes1[i];
        bytes1[i] = bytes1[31 - i];
        bytes1[31 - i] = temp;
    }
    
    neoc_hash256_t *hash1 = NULL;
    err = neoc_hash256_from_bytes(bytes1, bytes1_len, &hash1);
    assert(err == NEOC_SUCCESS);
    
    // Create second hash from different reversed bytes
    const char *hex2 = "b43034ab680d646f8b6ca71647aa6ba167b2eb0b3757e545f6c2715787b13272";
    uint8_t bytes2[32];
    size_t bytes2_len = 0;
    err = neoc_hex_decode(hex2, bytes2, sizeof(bytes2), &bytes2_len);
    assert(err == NEOC_SUCCESS);
    
    // Reverse for little endian
    for (int i = 0; i < 16; i++) {
        uint8_t temp = bytes2[i];
        bytes2[i] = bytes2[31 - i];
        bytes2[31 - i] = temp;
    }
    
    neoc_hash256_t *hash2 = NULL;
    err = neoc_hash256_from_bytes(bytes2, bytes2_len, &hash2);
    assert(err == NEOC_SUCCESS);
    
    // Create third hash
    neoc_hash256_t *hash3 = NULL;
    err = neoc_hash256_from_string("0xf4609b99e171190c22adcf70c88a7a14b5b530914d2398287bd8bb7ad95a661c", &hash3);
    assert(err == NEOC_SUCCESS);
    
    // Test comparisons
    assert(neoc_hash256_compare(hash1, hash2) > 0);
    assert(neoc_hash256_compare(hash3, hash1) > 0);
    assert(neoc_hash256_compare(hash3, hash2) > 0);
    assert(neoc_hash256_compare(hash2, hash1) < 0);
    assert(neoc_hash256_compare(hash1, hash1) == 0);
    
    neoc_hash256_free(hash1);
    neoc_hash256_free(hash2);
    neoc_hash256_free(hash3);
    
    printf("  ✅ Compare to test passed\n");
}

// Test size
static void test_size(void) {
    printf("Testing size...\n");
    
    neoc_hash256_t *hash = NULL;
    neoc_error_t err = neoc_hash256_from_string("b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a", &hash);
    assert(err == NEOC_SUCCESS);
    
    size_t size = neoc_hash256_get_size(hash);
    assert(size == 32);
    
    neoc_hash256_free(hash);
    printf("  ✅ Size test passed\n");
}

int main(void) {
    printf("\n=== Hash256Tests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_from_valid_hash();
    test_creation_throws();
    test_from_bytes();
    test_to_array();
    test_serialize_and_deserialize();
    test_equals();
    test_compare_to();
    test_size();
    
    tearDown();
    
    printf("\n✅ All Hash256Tests tests passed!\n\n");
    return 0;
}