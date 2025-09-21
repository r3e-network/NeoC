/**
 * @file test_script_builder.c
 * @brief Unit tests converted from ScriptBuilderTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "neoc/neoc.h"
#include "neoc/script/script_builder.h"
#include "neoc/script/op_code.h"
#include "neoc/script/interop_service.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/utils/hex.h"

// Helper function to create byte array of specific size
static uint8_t *create_byte_array(size_t size, uint8_t fill_value) {
    uint8_t *array = malloc(size);
    assert(array != NULL);
    memset(array, fill_value, size);
    return array;
}

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test push empty array
static void test_push_array_empty(void) {
    printf("Testing push empty array...\n");
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    assert(err == NEOC_SUCCESS);
    assert(builder != NULL);
    
    // Push empty array
    neoc_contract_parameter_t *params[0];
    err = neoc_script_builder_push_array(builder, params, 0);
    assert(err == NEOC_SUCCESS);
    
    // Get script and verify
    size_t script_len = 0;
    const uint8_t *script = neoc_script_builder_to_array(builder, &script_len);
    assert(script != NULL);
    assert(script_len == 1);
    assert(script[0] == NEOC_OPCODE_NEWARRAY0);
    
    neoc_script_builder_free(builder);
    printf("  ✅ Push empty array test passed\n");
}

// Test push byte array with different sizes
static void test_push_byte_array(void) {
    printf("Testing push byte array...\n");
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    assert(err == NEOC_SUCCESS);
    
    // Test 1 byte array
    uint8_t *data1 = create_byte_array(1, 0x01);
    err = neoc_script_builder_push_data(builder, data1, 1);
    assert(err == NEOC_SUCCESS);
    
    size_t script_len = 0;
    const uint8_t *script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len >= 2);
    assert(script[0] == 0x0c); // PUSHDATA1 prefix for small data
    assert(script[1] == 0x01); // Length
    
    free(data1);
    neoc_script_builder_reset(builder);
    
    // Test 75 byte array
    uint8_t *data75 = create_byte_array(75, 0x01);
    err = neoc_script_builder_push_data(builder, data75, 75);
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len >= 2);
    assert(script[0] == 0x0c);
    assert(script[1] == 0x4b); // 75 in hex
    
    free(data75);
    neoc_script_builder_reset(builder);
    
    // Test 256 byte array
    uint8_t *data256 = create_byte_array(256, 0x01);
    err = neoc_script_builder_push_data(builder, data256, 256);
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len >= 3);
    assert(script[0] == 0x0d); // PUSHDATA2 prefix for medium data
    
    free(data256);
    neoc_script_builder_free(builder);
    
    printf("  ✅ Push byte array test passed\n");
}

// Test push string
static void test_push_string(void) {
    printf("Testing push string...\n");
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    assert(err == NEOC_SUCCESS);
    
    // Test empty string
    err = neoc_script_builder_push_string(builder, "");
    assert(err == NEOC_SUCCESS);
    
    size_t script_len = 0;
    const uint8_t *script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len == 2);
    assert(script[0] == 0x0c);
    assert(script[1] == 0x00); // Empty string length
    
    neoc_script_builder_reset(builder);
    
    // Test single character
    err = neoc_script_builder_push_string(builder, "a");
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len == 3);
    assert(script[0] == 0x0c);
    assert(script[1] == 0x01); // Length 1
    assert(script[2] == 'a');
    
    neoc_script_builder_free(builder);
    printf("  ✅ Push string test passed\n");
}

// Test push integer
static void test_push_integer(void) {
    printf("Testing push integer...\n");
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    assert(err == NEOC_SUCCESS);
    
    // Test push 0
    err = neoc_script_builder_push_integer(builder, 0);
    assert(err == NEOC_SUCCESS);
    
    size_t script_len = 0;
    const uint8_t *script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len >= 1);
    assert(script[script_len - 1] == NEOC_OPCODE_PUSH0);
    
    // Test push 1
    err = neoc_script_builder_push_integer(builder, 1);
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script[script_len - 1] == NEOC_OPCODE_PUSH1);
    
    // Test push 16
    err = neoc_script_builder_push_integer(builder, 16);
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script[script_len - 1] == NEOC_OPCODE_PUSH16);
    
    // Test push 17 (requires encoding)
    neoc_script_builder_reset(builder);
    err = neoc_script_builder_push_integer(builder, 17);
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len == 2);
    assert(script[0] == 0x00); // Prefix for small int
    assert(script[1] == 0x11); // 17 in hex
    
    neoc_script_builder_free(builder);
    printf("  ✅ Push integer test passed\n");
}

// Test build verification script from public keys
static void test_verification_script_from_public_keys(void) {
    printf("Testing verification script from public keys...\n");
    
    const char *key1_hex = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";
    const char *key2_hex = "03eda286d19f7ee0b472afd1163d803d620a961e1581a8f2704b52c0285f6e022d";
    const char *key3_hex = "03ac81ec17f2f15fd6d193182f927c5971559c2a32b9408a06fec9e711fb7ca02e";
    
    // Create public keys
    uint8_t key1_bytes[33], key2_bytes[33], key3_bytes[33];
    size_t key1_len = 0, key2_len = 0, key3_len = 0;
    
    neoc_error_t err = neoc_hex_decode(key1_hex, key1_bytes, sizeof(key1_bytes), &key1_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_hex_decode(key2_hex, key2_bytes, sizeof(key2_bytes), &key2_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_hex_decode(key3_hex, key3_bytes, sizeof(key3_bytes), &key3_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_public_key_t *key1 = NULL, *key2 = NULL, *key3 = NULL;
    err = neoc_ec_public_key_from_bytes(key1_bytes, key1_len, &key1);
    assert(err == NEOC_SUCCESS);
    err = neoc_ec_public_key_from_bytes(key2_bytes, key2_len, &key2);
    assert(err == NEOC_SUCCESS);
    err = neoc_ec_public_key_from_bytes(key3_bytes, key3_len, &key3);
    assert(err == NEOC_SUCCESS);
    
    // Build multi-sig verification script
    neoc_ec_public_key_t *keys[3] = {key1, key2, key3};
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_build_verification_script_multi(keys, 3, 2, &script, &script_len);
    assert(err == NEOC_SUCCESS);
    assert(script != NULL);
    assert(script_len > 0);
    
    // Verify script starts with PUSH2 (for 2-of-3 multisig)
    assert(script[0] == NEOC_OPCODE_PUSH2);
    
    free(script);
    neoc_ec_public_key_free(key1);
    neoc_ec_public_key_free(key2);
    neoc_ec_public_key_free(key3);
    
    printf("  ✅ Verification script from public keys test passed\n");
}

// Test build verification script from single public key
static void test_verification_script_from_public_key(void) {
    printf("Testing verification script from single public key...\n");
    
    const char *key_hex = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";
    
    // Decode public key
    uint8_t key_bytes[33];
    size_t key_len = 0;
    neoc_error_t err = neoc_hex_decode(key_hex, key_bytes, sizeof(key_bytes), &key_len);
    assert(err == NEOC_SUCCESS);
    
    // Build verification script
    uint8_t *script = NULL;
    size_t script_len = 0;
    err = neoc_script_builder_build_verification_script(key_bytes, key_len, &script, &script_len);
    assert(err == NEOC_SUCCESS);
    assert(script != NULL);
    assert(script_len > 0);
    
    // Verify script structure
    assert(script[0] == NEOC_OPCODE_PUSHDATA1); // PUSHDATA1 for public key
    assert(script[1] == 0x21); // 33 bytes length
    assert(memcmp(script + 2, key_bytes, 33) == 0); // Public key data
    assert(script[35] == NEOC_OPCODE_SYSCALL); // SYSCALL opcode
    
    free(script);
    printf("  ✅ Verification script from single public key test passed\n");
}

// Test push boolean
static void test_push_boolean(void) {
    printf("Testing push boolean...\n");
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    assert(err == NEOC_SUCCESS);
    
    // Push true
    err = neoc_script_builder_push_boolean(builder, true);
    assert(err == NEOC_SUCCESS);
    
    size_t script_len = 0;
    const uint8_t *script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len >= 1);
    assert(script[script_len - 1] == NEOC_OPCODE_PUSH1); // true is PUSH1
    
    // Push false
    err = neoc_script_builder_push_boolean(builder, false);
    assert(err == NEOC_SUCCESS);
    
    script = neoc_script_builder_to_array(builder, &script_len);
    assert(script[script_len - 1] == NEOC_OPCODE_PUSH0); // false is PUSH0
    
    neoc_script_builder_free(builder);
    printf("  ✅ Push boolean test passed\n");
}

// Test opcode operations
static void test_opcode_operations(void) {
    printf("Testing opcode operations...\n");
    
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    assert(err == NEOC_SUCCESS);
    
    // Add various opcodes
    err = neoc_script_builder_opcode(builder, NEOC_OPCODE_NOP);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_script_builder_opcode(builder, NEOC_OPCODE_DUP);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_script_builder_opcode(builder, NEOC_OPCODE_DROP);
    assert(err == NEOC_SUCCESS);
    
    // Verify script
    size_t script_len = 0;
    const uint8_t *script = neoc_script_builder_to_array(builder, &script_len);
    assert(script_len == 3);
    assert(script[0] == NEOC_OPCODE_NOP);
    assert(script[1] == NEOC_OPCODE_DUP);
    assert(script[2] == NEOC_OPCODE_DROP);
    
    neoc_script_builder_free(builder);
    printf("  ✅ Opcode operations test passed\n");
}

int main(void) {
    printf("\n=== ScriptBuilderTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_push_array_empty();
    test_push_byte_array();
    test_push_string();
    test_push_integer();
    test_verification_script_from_public_keys();
    test_verification_script_from_public_key();
    test_push_boolean();
    test_opcode_operations();
    
    tearDown();
    
    printf("\n✅ All ScriptBuilderTests tests passed!\n\n");
    return 0;
}
