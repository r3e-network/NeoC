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
    
    // Create a message to sign
    uint8_t message[10];
    memset(message, 10, sizeof(message));
    
    // Create a key pair
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create(&key_pair);
    assert(err == NEOC_SUCCESS);
    assert(key_pair != NULL);
    
    // Sign the message
    uint8_t *signature = NULL;
    size_t sig_len = 0;
    err = neoc_ec_key_pair_sign(key_pair, message, sizeof(message), &signature, &sig_len);
    assert(err == NEOC_SUCCESS);
    assert(signature != NULL);
    assert(sig_len == 64);
    
    // Create invocation script from signature
    uint8_t *invocation_script = NULL;
    size_t inv_len = 0;
    err = neoc_script_create_invocation_from_signature(signature, sig_len, &invocation_script, &inv_len);
    assert(err == NEOC_SUCCESS);
    
    // Get public key
    uint8_t pubkey[65];
    size_t pubkey_len = sizeof(pubkey);
    err = neoc_ec_key_pair_get_public_key(key_pair, pubkey, &pubkey_len);
    assert(err == NEOC_SUCCESS);
    
    // Create verification script
    uint8_t *verification_script = NULL;
    size_t ver_len = 0;
    err = neoc_script_builder_build_verification_script(pubkey, pubkey_len, &verification_script, &ver_len);
    assert(err == NEOC_SUCCESS);
    
    // Create witness
    neoc_witness_t *witness = NULL;
    err = neoc_witness_create(invocation_script, inv_len, verification_script, ver_len, &witness);
    assert(err == NEOC_SUCCESS);
    assert(witness != NULL);
    
    // Cleanup
    neoc_free(signature);
    neoc_free(invocation_script);
    neoc_free(verification_script);
    neoc_witness_free(witness);
    neoc_ec_key_pair_free(key_pair);
    
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
    uint8_t *inv_out = NULL;
    size_t inv_out_len = 0;
    err = neoc_witness_get_invocation_script(witness, &inv_out, &inv_out_len);
    assert(err == NEOC_SUCCESS);
    assert(inv_out_len == sizeof(inv_script));
    assert(memcmp(inv_out, inv_script, inv_out_len) == 0);
    
    uint8_t *ver_out = NULL;
    size_t ver_out_len = 0;
    err = neoc_witness_get_verification_script(witness, &ver_out, &ver_out_len);
    assert(err == NEOC_SUCCESS);
    assert(ver_out_len == sizeof(ver_script));
    assert(memcmp(ver_out, ver_script, ver_out_len) == 0);
    
    // Cleanup
    free(serialized);
    neoc_free(inv_out);
    neoc_free(ver_out);
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
    
    // Get script hash
    neoc_hash160_t script_hash;
    err = neoc_witness_get_script_hash(witness, &script_hash);
    assert(err == NEOC_SUCCESS);
    
    // Calculate expected hash (SHA256 then RIPEMD160 of verification script)
    neoc_hash160_t expected_hash;
    err = neoc_hash160_from_script(&expected_hash, ver_script, sizeof(ver_script));
    assert(err == NEOC_SUCCESS);
    
    // Compare hashes
    assert(memcmp(script_hash.data, expected_hash.data, 20) == 0);
    
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
