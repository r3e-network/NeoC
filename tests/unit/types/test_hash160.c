/**
 * @file test_hash160.c
 * @brief Unit tests converted from Hash160Tests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/types/hash160.h"
#include "neoc/crypto/ec_public_key.h"
#include "neoc/script/op_code.h"
#include "neoc/script/interop_service.h"
#include "neoc/utils/hex.h"
#include "neoc/utils/address.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"

// Test data from TestProperties.swift
static const char *DEFAULT_ACCOUNT_PUBLIC_KEY = "033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b";
static const char *DEFAULT_ACCOUNT_ADDRESS = "NM7Aky765FG8NhhwtxjXRx7jEL1cnw7PBP";
static const char *COMMITTEE_ACCOUNT_SCRIPT_HASH = "05859de95ccbbd5668e0f055b208273634d4657f";

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
    neoc_hash160_t *hash1 = NULL;
    neoc_error_t err = neoc_hash160_from_string("0x23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash1);
    assert(err == NEOC_SUCCESS);
    assert(hash1 != NULL);
    
    const char *str1 = neoc_hash160_to_string(hash1);
    assert(strcmp(str1, "23ba2703c53263e8d6e522dc32203339dcd8eee9") == 0);
    
    // Test without 0x prefix
    neoc_hash160_t *hash2 = NULL;
    err = neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash2);
    assert(err == NEOC_SUCCESS);
    assert(hash2 != NULL);
    
    const char *str2 = neoc_hash160_to_string(hash2);
    assert(strcmp(str2, "23ba2703c53263e8d6e522dc32203339dcd8eee9") == 0);
    
    neoc_hash160_free(hash1);
    neoc_hash160_free(hash2);
    printf("  ✅ Create from valid hash test passed\n");
}

// Test creation errors
static void test_creation_throws(void) {
    printf("Testing creation error handling...\n");
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err;
    
    // Test invalid hex with 0x prefix
    err = neoc_hash160_from_string("0x23ba2703c53263e8d6e522dc32203339dcd8eee", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    // Test invalid hex character
    err = neoc_hash160_from_string("g3ba2703c53263e8d6e522dc32203339dcd8eee9", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    // Test too short (19 bytes)
    err = neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8ee", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    // Test too long (32 bytes)
    err = neoc_hash160_from_string("c56f33fc6ecfcd0c225c4ab356fee59390af8560be0e930faebe74a6daff7c9b", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    printf("  ✅ Creation error handling test passed\n");
}

// Test to little endian array
static void test_to_array(void) {
    printf("Testing to little endian array...\n");
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err = neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash);
    assert(err == NEOC_SUCCESS);
    
    size_t array_len = 0;
    const uint8_t *array = neoc_hash160_to_little_endian_array(hash, &array_len);
    assert(array != NULL);
    assert(array_len == 20);
    
    // Verify it's reversed
    uint8_t expected[20];
    size_t expected_len = 0;
    err = neoc_hex_decode("23ba2703c53263e8d6e522dc32203339dcd8eee9", expected, sizeof(expected), &expected_len);
    assert(err == NEOC_SUCCESS);
    
    // Check reversed order
    for (size_t i = 0; i < 20; i++) {
        assert(array[i] == expected[19 - i]);
    }
    
    neoc_hash160_free(hash);
    printf("  ✅ To little endian array test passed\n");
}

// Test serialize and deserialize
static void test_serialize_and_deserialize(void) {
    printf("Testing serialize and deserialize...\n");
    
    const char *hash_str = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err = neoc_hash160_from_string(hash_str, &hash);
    assert(err == NEOC_SUCCESS);
    
    // Serialize
    neoc_binary_writer_t *writer = NULL;
    err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_hash160_serialize(hash, writer);
    assert(err == NEOC_SUCCESS);
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_len);
    assert(err == NEOC_SUCCESS);
    assert(data_len == 20);
    
    // Deserialize
    neoc_binary_reader_t *reader = NULL;
    err = neoc_binary_reader_create(data, data_len, &reader);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *deserialized = NULL;
    err = neoc_hash160_deserialize(reader, &deserialized);
    assert(err == NEOC_SUCCESS);
    
    // Verify
    const char *deserialized_str = neoc_hash160_to_string(deserialized);
    assert(strcmp(deserialized_str, hash_str) == 0);
    
    free(data);
    neoc_binary_writer_free(writer);
    neoc_binary_reader_free(reader);
    neoc_hash160_free(hash);
    neoc_hash160_free(deserialized);
    
    printf("  ✅ Serialize and deserialize test passed\n");
}

// Test equals
static void test_equals(void) {
    printf("Testing equals...\n");
    
    // Create hashes from scripts
    uint8_t script1[4] = {0x01, 0xa4, 0x02, 0xd8};
    uint8_t script2[4] = {0xd8, 0x02, 0xa4, 0x01};
    
    neoc_hash160_t *hash1 = NULL;
    neoc_error_t err = neoc_hash160_from_script(script1, sizeof(script1), &hash1);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *hash2 = NULL;
    err = neoc_hash160_from_script(script2, sizeof(script2), &hash2);
    assert(err == NEOC_SUCCESS);
    
    // Test equality
    assert(neoc_hash160_equals(hash1, hash1) == true);
    assert(neoc_hash160_equals(hash1, hash2) == false);
    assert(neoc_hash160_equals(hash2, hash1) == false);
    
    neoc_hash160_free(hash1);
    neoc_hash160_free(hash2);
    
    printf("  ✅ Equals test passed\n");
}

// Test from valid address
static void test_from_valid_address(void) {
    printf("Testing from valid address...\n");
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err = neoc_hash160_from_address("NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke", &hash);
    assert(err == NEOC_SUCCESS);
    assert(hash != NULL);
    
    // Verify hash
    size_t array_len = 0;
    const uint8_t *array = neoc_hash160_to_little_endian_array(hash, &array_len);
    assert(array_len == 20);
    
    uint8_t expected[20];
    size_t expected_len = 0;
    err = neoc_hex_decode("09a55874c2da4b86e5d49ff530a1b153eb12c7d6", expected, sizeof(expected), &expected_len);
    assert(err == NEOC_SUCCESS);
    assert(memcmp(array, expected, 20) == 0);
    
    neoc_hash160_free(hash);
    printf("  ✅ From valid address test passed\n");
}

// Test from invalid address
static void test_from_invalid_address(void) {
    printf("Testing from invalid address...\n");
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err = neoc_hash160_from_address("NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8keas", &hash);
    assert(err != NEOC_SUCCESS);
    assert(hash == NULL);
    
    printf("  ✅ From invalid address test passed\n");
}

// Test from public key bytes
static void test_from_public_key_bytes(void) {
    printf("Testing from public key bytes...\n");
    
    const char *key_hex = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";
    
    uint8_t key_bytes[33];
    size_t key_len = 0;
    neoc_error_t err = neoc_hex_decode(key_hex, key_bytes, sizeof(key_bytes), &key_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *hash = NULL;
    err = neoc_hash160_from_public_key(key_bytes, key_len, &hash);
    assert(err == NEOC_SUCCESS);
    assert(hash != NULL);
    
    // Test with multiple keys (multi-sig)
    uint8_t pubkey_bytes[33];
    size_t pubkey_len = 0;
    err = neoc_hex_decode(DEFAULT_ACCOUNT_PUBLIC_KEY, pubkey_bytes, sizeof(pubkey_bytes), &pubkey_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_public_key_t *public_key = NULL;
    err = neoc_ec_public_key_from_bytes(pubkey_bytes, pubkey_len, &public_key);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_public_key_t *keys[1] = {public_key};
    neoc_hash160_t *hash2 = NULL;
    err = neoc_hash160_from_public_keys(keys, 1, 1, &hash2);
    assert(err == NEOC_SUCCESS);
    
    const char *hash2_str = neoc_hash160_to_string(hash2);
    assert(strcmp(hash2_str, COMMITTEE_ACCOUNT_SCRIPT_HASH) == 0);
    
    neoc_ec_public_key_free(public_key);
    neoc_hash160_free(hash);
    neoc_hash160_free(hash2);
    
    printf("  ✅ From public key bytes test passed\n");
}

// Test from contract script
static void test_from_contract_script(void) {
    printf("Testing from contract script...\n");
    
    const char *script_hex = "110c21026aa8fe6b4360a67a530e23c08c6a72525afde34719c5436f9d3ced759f939a3d110b41138defaf";
    uint8_t script[256];
    size_t script_len = 0;
    neoc_error_t err = neoc_hex_decode(script_hex, script, sizeof(script), &script_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *hash = NULL;
    err = neoc_hash160_from_script(script, script_len, &hash);
    assert(err == NEOC_SUCCESS);
    assert(hash != NULL);
    
    const char *hash_str = neoc_hash160_to_string(hash);
    assert(strcmp(hash_str, "afaed076854454449770763a628f379721ea9808") == 0);
    
    // Verify little endian array
    size_t array_len = 0;
    const uint8_t *array = neoc_hash160_to_little_endian_array(hash, &array_len);
    assert(array_len == 20);
    
    // Convert to hex and verify
    char hex_buffer[41];
    err = neoc_hex_encode(array, array_len, hex_buffer, sizeof(hex_buffer));
    assert(err == NEOC_SUCCESS);
    assert(strcmp(hex_buffer, "0898ea2197378f623a7670974454448576d0aeaf") == 0);
    
    neoc_hash160_free(hash);
    printf("  ✅ From contract script test passed\n");
}

// Test to address
static void test_to_address(void) {
    printf("Testing to address...\n");
    
    uint8_t pubkey_bytes[33];
    size_t pubkey_len = 0;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PUBLIC_KEY, pubkey_bytes, sizeof(pubkey_bytes), &pubkey_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *hash = NULL;
    err = neoc_hash160_from_public_key(pubkey_bytes, pubkey_len, &hash);
    assert(err == NEOC_SUCCESS);
    
    const char *address = neoc_hash160_to_address(hash);
    assert(address != NULL);
    assert(strcmp(address, DEFAULT_ACCOUNT_ADDRESS) == 0);
    
    neoc_hash160_free(hash);
    printf("  ✅ To address test passed\n");
}

// Test compare
static void test_compare_to(void) {
    printf("Testing compare to...\n");
    
    uint8_t script1[4] = {0x01, 0xa4, 0x02, 0xd8};
    uint8_t script2[4] = {0xd8, 0x02, 0xa4, 0x01};
    uint8_t script3[4] = {0xa7, 0xb3, 0xa1, 0x91};
    
    neoc_hash160_t *hash1 = NULL;
    neoc_error_t err = neoc_hash160_from_script(script1, sizeof(script1), &hash1);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *hash2 = NULL;
    err = neoc_hash160_from_script(script2, sizeof(script2), &hash2);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash160_t *hash3 = NULL;
    err = neoc_hash160_from_script(script3, sizeof(script3), &hash3);
    assert(err == NEOC_SUCCESS);
    
    // Test comparisons
    assert(neoc_hash160_compare(hash2, hash1) > 0);
    assert(neoc_hash160_compare(hash3, hash1) > 0);
    assert(neoc_hash160_compare(hash2, hash3) > 0);
    assert(neoc_hash160_compare(hash1, hash2) < 0);
    assert(neoc_hash160_compare(hash1, hash1) == 0);
    
    neoc_hash160_free(hash1);
    neoc_hash160_free(hash2);
    neoc_hash160_free(hash3);
    
    printf("  ✅ Compare to test passed\n");
}

// Test size
static void test_size(void) {
    printf("Testing size...\n");
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err = neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash);
    assert(err == NEOC_SUCCESS);
    
    size_t size = neoc_hash160_get_size(hash);
    assert(size == 20);
    
    neoc_hash160_free(hash);
    printf("  ✅ Size test passed\n");
}

int main(void) {
    printf("\n=== Hash160Tests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_from_valid_hash();
    test_creation_throws();
    test_to_array();
    test_serialize_and_deserialize();
    test_equals();
    test_from_valid_address();
    test_from_invalid_address();
    test_from_public_key_bytes();
    test_from_contract_script();
    test_to_address();
    test_compare_to();
    test_size();
    
    tearDown();
    
    printf("\n✅ All Hash160Tests tests passed!\n\n");
    return 0;
}