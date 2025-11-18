/**
 * @file test_witness.c
 * @brief Unit tests converted from WitnessTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/neoc.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test witness creation
static void test_create_witness(void) {
    printf("Testing witness creation...\n");
    
    // Sample invocation/verification scripts
    uint8_t invocation_script[] = {0x40, 0x01, 0x02, 0x03};
    uint8_t verification_script[] = {0x21, 0x04, 0x05, 0x06, 0xAC};
    
    neoc_witness_t *witness = NULL;
    neoc_error_t err = neoc_witness_create(invocation_script, sizeof(invocation_script),
                                           verification_script, sizeof(verification_script),
                                           &witness);
    assert(err == NEOC_SUCCESS);
    assert(witness != NULL);
    assert(witness->invocation_script_len == sizeof(invocation_script));
    assert(memcmp(witness->invocation_script, invocation_script, sizeof(invocation_script)) == 0);
    assert(witness->verification_script_len == sizeof(verification_script));
    assert(memcmp(witness->verification_script, verification_script, sizeof(verification_script)) == 0);
    
    // Cleanup
    neoc_witness_free(witness);
    
    printf("  ✅ Witness creation test passed\n");
}

// Test witness serialization
static void test_serialize_witness(void) {
    printf("Testing witness serialization...\n");
    
    // Create simple witness with test data
    uint8_t inv_script[] = {0x40, 0x01, 0x02, 0x03};
    uint8_t ver_script[] = {0x21, 0x04, 0x05, 0x06};
    
    neoc_witness_t *witness = NULL;
    neoc_error_t err = neoc_witness_create(inv_script, sizeof(inv_script), 
                                           ver_script, sizeof(ver_script), &witness);
    assert(err == NEOC_SUCCESS);
    
    // Serialize witness
    uint8_t *serialized = NULL;
    size_t size = 0;
    err = neoc_witness_serialize(witness, &serialized, &size);
    assert(err == NEOC_SUCCESS);
    assert(serialized != NULL);
    
    // Expected format: [inv_len][inv_script][ver_len][ver_script]
    assert(size == 1 + sizeof(inv_script) + 1 + sizeof(ver_script));
    assert(serialized[0] == sizeof(inv_script));
    assert(memcmp(serialized + 1, inv_script, sizeof(inv_script)) == 0);
    assert(serialized[1 + sizeof(inv_script)] == sizeof(ver_script));
    assert(memcmp(serialized + 2 + sizeof(inv_script), ver_script, sizeof(ver_script)) == 0);
    
    // Cleanup
    neoc_free(serialized);
    neoc_witness_free(witness);
    
    printf("  ✅ Witness serialization test passed\n");
}

// Test witness deserialization
static void test_deserialize_witness(void) {
    printf("Testing witness deserialization...\n");
    
    // Create serialized witness data
    uint8_t inv_script[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t ver_script[] = {0x05, 0x06, 0x07};
    
    size_t serialized_size = 1 + sizeof(inv_script) + 1 + sizeof(ver_script);
    uint8_t *serialized = malloc(serialized_size);
    serialized[0] = sizeof(inv_script);
    memcpy(serialized + 1, inv_script, sizeof(inv_script));
    serialized[1 + sizeof(inv_script)] = sizeof(ver_script);
    memcpy(serialized + 2 + sizeof(inv_script), ver_script, sizeof(ver_script));
    
    // Deserialize witness
    neoc_witness_t *witness = NULL;
    neoc_error_t err = neoc_witness_deserialize(serialized, serialized_size, &witness);
    assert(err == NEOC_SUCCESS);
    assert(witness != NULL);
    
    // Verify deserialized data
    assert(witness->invocation_script_len == sizeof(inv_script));
    assert(memcmp(witness->invocation_script, inv_script, sizeof(inv_script)) == 0);
    assert(witness->verification_script_len == sizeof(ver_script));
    assert(memcmp(witness->verification_script, ver_script, sizeof(ver_script)) == 0);
    
    // Cleanup
    free(serialized);
    neoc_witness_free(witness);
    
    printf("  ✅ Witness deserialization test passed\n");
}

// Test script hash from witness
static void test_script_hash_from_witness(void) {
    printf("Testing script hash from witness...\n");
    
    // Create a witness with known verification script
    uint8_t inv_script[] = {0x40, 0x01, 0x02, 0x03};
    uint8_t ver_script[] = {0x21, 0x04, 0x05, 0x06, 0x07, 0x08, 0xAC}; // Simple check sig script
    
    neoc_witness_t *witness = NULL;
    neoc_error_t err = neoc_witness_create(inv_script, sizeof(inv_script), 
                                           ver_script, sizeof(ver_script), &witness);
    assert(err == NEOC_SUCCESS);
    
    // Calculate expected hash (SHA256 then RIPEMD160 of verification script)
    neoc_hash160_t expected_hash;
    err = neoc_hash160_from_script(&expected_hash, ver_script, sizeof(ver_script));
    assert(err == NEOC_SUCCESS);

    neoc_hash160_t actual_hash;
    err = neoc_hash160_from_script(&actual_hash,
                                   witness->verification_script,
                                   witness->verification_script_len);
    assert(err == NEOC_SUCCESS);
    
    // Compare hashes
    assert(memcmp(actual_hash.data, expected_hash.data, NEOC_HASH160_SIZE) == 0);
    
    // Cleanup
    neoc_witness_free(witness);
    
    printf("  ✅ Script hash from witness test passed\n");
}

int main(void) {
    printf("\n=== WitnessTests Tests ===\n\n");
    
    setUp();
    
    // Run tests
    test_create_witness();
    test_serialize_witness();
    test_deserialize_witness();
    test_script_hash_from_witness();
    
    tearDown();
    
    printf("\n✅ All WitnessTests tests passed!\n\n");
    return 0;
}
