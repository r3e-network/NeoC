/**
 * @file test_witness_rule.c
 * @brief Unit tests converted from WitnessRuleTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/witnessrule/witness_rule.h"
#include "neoc/witnessrule/witness_condition.h"
#include "neoc/crypto/ec_public_key.h"
#include "neoc/neo_constants.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/hex.h"

// Test data
// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

static neoc_ec_public_key_t *create_public_key_from_hex(const char *hex) {
    uint8_t buffer[NEOC_PUBLIC_KEY_SIZE_UNCOMPRESSED];
    size_t decoded_len = 0;
    neoc_error_t err = neoc_hex_decode(hex, buffer, sizeof(buffer), &decoded_len);
    assert(err == NEOC_SUCCESS);

    neoc_ec_public_key_t *key = NULL;
    err = neoc_ec_public_key_from_bytes(buffer, decoded_len, &key);
    assert(err == NEOC_SUCCESS);
    return key;
}

static neoc_binary_writer_t *create_writer(void) {
    neoc_binary_writer_t *writer = NULL;
    assert(neoc_binary_writer_create(32, true, &writer) == NEOC_SUCCESS);
    return writer;
}

// Test boolean condition decode
static void test_decode_boolean_condition(void) {
    printf("Testing decode boolean condition...\n");
    
    // Create a boolean false condition
    neoc_witness_condition_t *condition = NULL;
    neoc_error_t err = neoc_witness_condition_create_boolean(false, &condition);
    assert(err == NEOC_SUCCESS);
    assert(condition != NULL);
    
    // Create a witness rule with allow action
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_ALLOW, condition, &rule);
    assert(err == NEOC_SUCCESS);
    assert(rule != NULL);
    
    // Verify action
    neoc_witness_action_t action = neoc_witness_rule_get_action(rule);
    assert(action == NEOC_WITNESS_ACTION_ALLOW);
    
    // Verify condition type and value
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(condition);
    assert(type == NEOC_WITNESS_CONDITION_BOOLEAN);
    
    bool bool_value = false;
    err = neoc_witness_condition_get_boolean(condition, &bool_value);
    assert(err == NEOC_SUCCESS);
    assert(bool_value == false);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(condition);
    
    printf("  ✅ Decode boolean condition test passed\n");
}

// Test NOT condition decode
static void test_decode_not_condition(void) {
    printf("Testing decode NOT condition...\n");
    
    // Create CalledByEntry condition
    neoc_witness_condition_t *inner_condition = NULL;
    neoc_error_t err = neoc_witness_condition_create_called_by_entry(&inner_condition);
    assert(err == NEOC_SUCCESS);
    
    // Create NOT(CalledByEntry) condition
    neoc_witness_condition_t *not_condition = NULL;
    err = neoc_witness_condition_create_not(inner_condition, &not_condition);
    assert(err == NEOC_SUCCESS);
    
    // Create NOT(NOT(CalledByEntry)) condition
    neoc_witness_condition_t *double_not = NULL;
    err = neoc_witness_condition_create_not(not_condition, &double_not);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_ALLOW, double_not, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(double_not);
    assert(type == NEOC_WITNESS_CONDITION_NOT);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(inner_condition);
    neoc_witness_condition_free(not_condition);
    neoc_witness_condition_free(double_not);
    
    printf("  ✅ Decode NOT condition test passed\n");
}

// Test AND condition decode
static void test_decode_and_condition(void) {
    printf("Testing decode AND condition...\n");
    
    // Create conditions for AND
    neoc_witness_condition_t *cond1 = NULL;
    neoc_error_t err = neoc_witness_condition_create_called_by_entry(&cond1);
    assert(err == NEOC_SUCCESS);
    
    // Create group condition with public key
    neoc_ec_public_key_t *pub_key =
        create_public_key_from_hex("021821807f923a3da004fb73871509d7635bcc05f41edef2a3ca5c941d8bbc1231");
    
    neoc_witness_condition_t *cond2 = NULL;
    err = neoc_witness_condition_create_group(pub_key, &cond2);
    assert(err == NEOC_SUCCESS);
    
    neoc_witness_condition_t *cond3 = NULL;
    err = neoc_witness_condition_create_boolean(true, &cond3);
    assert(err == NEOC_SUCCESS);
    
    // Create AND condition
    neoc_witness_condition_t *conditions[3] = { cond1, cond2, cond3 };
    neoc_witness_condition_t *and_condition = NULL;
    err = neoc_witness_condition_create_and(conditions, 3, &and_condition);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_ALLOW, and_condition, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(and_condition);
    assert(type == NEOC_WITNESS_CONDITION_AND);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(cond1);
    neoc_witness_condition_free(cond2);
    neoc_witness_condition_free(cond3);
    neoc_witness_condition_free(and_condition);
    neoc_ec_public_key_free(pub_key);
    
    printf("  ✅ Decode AND condition test passed\n");
}

// Test OR condition decode
static void test_decode_or_condition(void) {
    printf("Testing decode OR condition...\n");
    
    // Create group condition
    neoc_ec_public_key_t *pub_key =
        create_public_key_from_hex("023be7b6742268f4faca4835718f3232ddc976855d5ef273524cea36f0e8d102f3");
    
    neoc_witness_condition_t *cond1 = NULL;
    neoc_error_t err = neoc_witness_condition_create_group(pub_key, &cond1);
    assert(err == NEOC_SUCCESS);
    
    neoc_witness_condition_t *cond2 = NULL;
    err = neoc_witness_condition_create_called_by_entry(&cond2);
    assert(err == NEOC_SUCCESS);
    
    // Create OR condition
    neoc_witness_condition_t *conditions[2] = { cond1, cond2 };
    neoc_witness_condition_t *or_condition = NULL;
    err = neoc_witness_condition_create_or(conditions, 2, &or_condition);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_DENY, or_condition, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(or_condition);
    assert(type == NEOC_WITNESS_CONDITION_OR);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(cond1);
    neoc_witness_condition_free(cond2);
    neoc_witness_condition_free(or_condition);
    neoc_ec_public_key_free(pub_key);
    
    printf("  ✅ Decode OR condition test passed\n");
}

// Test script hash condition
static void test_decode_script_hash(void) {
    printf("Testing decode script hash...\n");
    
    // Create script hash
    neoc_hash160_t hash;
    neoc_error_t err = neoc_hash160_from_string("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5", &hash);
    assert(err == NEOC_SUCCESS);
    
    // Create script hash condition
    neoc_witness_condition_t *condition = NULL;
    err = neoc_witness_condition_create_script_hash(&hash, &condition);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_ALLOW, condition, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(condition);
    assert(type == NEOC_WITNESS_CONDITION_SCRIPT_HASH);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(condition);
    (void)hash;
    
    printf("  ✅ Decode script hash test passed\n");
}

// Test boolean condition serialize/deserialize
static void test_boolean_condition_serialize_deserialize(void) {
    printf("Testing boolean condition serialize/deserialize...\n");
    
    // Create boolean true condition
    neoc_witness_condition_t *condition = NULL;
    neoc_error_t err = neoc_witness_condition_create_boolean(true, &condition);
    assert(err == NEOC_SUCCESS);
    
    // Expected bytes: 0x00 (type) 0x01 (true)
    uint8_t expected_bytes[] = { 0x00, 0x01 };
    
    // Serialize
    neoc_binary_writer_t *writer = create_writer();
    
    err = neoc_witness_condition_serialize(condition, writer);
    assert(err == NEOC_SUCCESS);
    
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    err = neoc_binary_writer_to_array(writer, &bytes, &bytes_len);
    assert(err == NEOC_SUCCESS);
    assert(bytes_len == sizeof(expected_bytes));
    assert(memcmp(bytes, expected_bytes, bytes_len) == 0);
    
    // Deserialize
    neoc_binary_reader_t *reader = NULL;
    err = neoc_binary_reader_create(bytes, bytes_len, &reader);
    assert(err == NEOC_SUCCESS);
    
    neoc_witness_condition_t *deserialized = NULL;
    err = neoc_witness_condition_deserialize(reader, &deserialized);
    assert(err == NEOC_SUCCESS);
    
    // Verify deserialized condition
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(deserialized);
    assert(type == NEOC_WITNESS_CONDITION_BOOLEAN);
    
    bool bool_value = false;
    err = neoc_witness_condition_get_boolean(deserialized, &bool_value);
    assert(err == NEOC_SUCCESS);
    assert(bool_value == true);
    
    // Cleanup
    free(bytes);
    neoc_binary_writer_free(writer);
    neoc_binary_reader_free(reader);
    neoc_witness_condition_free(condition);
    neoc_witness_condition_free(deserialized);
    
    printf("  ✅ Boolean condition serialize/deserialize test passed\n");
}

// Test NOT condition serialize/deserialize
static void test_not_condition_serialize_deserialize(void) {
    printf("Testing NOT condition serialize/deserialize...\n");
    
    // Create boolean true condition
    neoc_witness_condition_t *inner = NULL;
    neoc_error_t err = neoc_witness_condition_create_boolean(true, &inner);
    assert(err == NEOC_SUCCESS);
    
    // Create NOT condition
    neoc_witness_condition_t *condition = NULL;
    err = neoc_witness_condition_create_not(inner, &condition);
    assert(err == NEOC_SUCCESS);
    
    // Expected bytes: 0x01 (NOT type) 0x00 (boolean type) 0x01 (true)
    uint8_t expected_bytes[] = { 0x01, 0x00, 0x01 };
    
    // Serialize
    neoc_binary_writer_t *writer = create_writer();
    
    err = neoc_witness_condition_serialize(condition, writer);
    assert(err == NEOC_SUCCESS);
    
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    err = neoc_binary_writer_to_array(writer, &bytes, &bytes_len);
    assert(err == NEOC_SUCCESS);
    assert(bytes_len == sizeof(expected_bytes));
    assert(memcmp(bytes, expected_bytes, bytes_len) == 0);
    
    // Cleanup
    free(bytes);
    neoc_binary_writer_free(writer);
    neoc_witness_condition_free(inner);
    neoc_witness_condition_free(condition);
    
    printf("  ✅ NOT condition serialize/deserialize test passed\n");
}

// Test called by entry condition
static void test_called_by_entry_condition(void) {
    printf("Testing called by entry condition...\n");
    
    // Create called by entry condition
    neoc_witness_condition_t *condition = NULL;
    neoc_error_t err = neoc_witness_condition_create_called_by_entry(&condition);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_DENY, condition, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(condition);
    assert(type == NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY);
    
    // Serialize
    neoc_binary_writer_t *writer = create_writer();
    
    err = neoc_witness_condition_serialize(condition, writer);
    assert(err == NEOC_SUCCESS);
    
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    err = neoc_binary_writer_to_array(writer, &bytes, &bytes_len);
    assert(err == NEOC_SUCCESS);
    
    // Expected: 0x20 (CalledByEntry type)
    assert(bytes_len == 1);
    assert(bytes[0] == 0x20);
    
    // Cleanup
    free(bytes);
    neoc_binary_writer_free(writer);
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(condition);
    
    printf("  ✅ Called by entry condition test passed\n");
}

// Test called by contract condition
static void test_called_by_contract_condition(void) {
    printf("Testing called by contract condition...\n");
    
    // Create hash
    neoc_hash160_t hash;
    neoc_error_t err = neoc_hash160_from_string("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63e4", &hash);
    assert(err == NEOC_SUCCESS);
    
    // Create called by contract condition
    neoc_witness_condition_t *condition = NULL;
    err = neoc_witness_condition_create_called_by_contract(&hash, &condition);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_ALLOW, condition, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(condition);
    assert(type == NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(condition);
    (void)hash;
    
    printf("  ✅ Called by contract condition test passed\n");
}

// Test called by group condition
static void test_called_by_group_condition(void) {
    printf("Testing called by group condition...\n");
    
    // Create public key
    neoc_ec_public_key_t *pub_key =
        create_public_key_from_hex("035a1ced7ae274a881c3f479452c8bca774c89f653d54c5c5959a01371a8c696fd");
    
    // Create called by group condition
    neoc_witness_condition_t *condition = NULL;
    neoc_error_t err = neoc_witness_condition_create_called_by_group(pub_key, &condition);
    assert(err == NEOC_SUCCESS);
    
    // Create witness rule
    neoc_witness_rule_t *rule = NULL;
    err = neoc_witness_rule_create(NEOC_WITNESS_ACTION_DENY, condition, &rule);
    assert(err == NEOC_SUCCESS);
    
    // Verify condition type
    neoc_witness_condition_type_t type = neoc_witness_condition_get_type(condition);
    assert(type == NEOC_WITNESS_CONDITION_CALLED_BY_GROUP);
    
    // Cleanup
    neoc_witness_rule_free(rule);
    neoc_witness_condition_free(condition);
    neoc_ec_public_key_free(pub_key);
    
    printf("  ✅ Called by group condition test passed\n");
}

int main(void) {
    printf("\n=== WitnessRuleTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_decode_boolean_condition();
    test_decode_not_condition();
    test_decode_and_condition();
    test_decode_or_condition();
    test_decode_script_hash();
    test_boolean_condition_serialize_deserialize();
    test_not_condition_serialize_deserialize();
    test_called_by_entry_condition();
    test_called_by_contract_condition();
    test_called_by_group_condition();
    
    tearDown();
    
    printf("\n✅ All WitnessRuleTests tests passed!\n\n");
    return 0;
}
