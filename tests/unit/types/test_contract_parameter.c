/**
 * @file test_contract_parameter.c
 * @brief Unit tests converted from ContractParameterTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/types/contract_parameter_type.h"
#include "neoc/types/hash160.h"
#include "neoc/types/hash256.h"
#include "neoc/crypto/ec_public_key.h"
#include "neoc/crypto/sign.h"
#include "neoc/utils/hex.h"
#include "neoc/wallet/account.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test string from string
static void test_string_from_string(void) {
    printf("Testing string from string...\n");
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_string("value", &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_STRING);
    
    const char *value = neoc_contract_parameter_get_string(param);
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ String from string test passed\n");
}

// Test bytes from bytes
static void test_bytes_from_bytes(void) {
    printf("Testing bytes from bytes...\n");
    
    uint8_t bytes[2] = {0x01, 0x01};
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_byte_array(bytes, sizeof(bytes), &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_BYTE_ARRAY);
    
    size_t value_len = 0;
    const uint8_t *value = neoc_contract_parameter_get_byte_array(param, &value_len);
    assert(value != NULL);
    assert(value_len == 2);
    assert(memcmp(value, bytes, 2) == 0);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Bytes from bytes test passed\n");
}

// Test bytes from hex string
static void test_bytes_from_hex_string(void) {
    printf("Testing bytes from hex string...\n");
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_byte_array_from_hex("0xa602", &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_BYTE_ARRAY);
    
    size_t value_len = 0;
    const uint8_t *value = neoc_contract_parameter_get_byte_array(param, &value_len);
    assert(value != NULL);
    assert(value_len == 2);
    assert(value[0] == 0xa6);
    assert(value[1] == 0x02);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Bytes from hex string test passed\n");
}

// Test bytes equals
static void test_bytes_equals(void) {
    printf("Testing bytes equals...\n");
    
    neoc_contract_parameter_t *param1 = NULL;
    neoc_error_t err = neoc_contract_parameter_byte_array_from_hex("0x796573", &param1);
    assert(err == NEOC_SUCCESS);
    
    uint8_t bytes[3] = {0x79, 0x65, 0x73};
    neoc_contract_parameter_t *param2 = NULL;
    err = neoc_contract_parameter_byte_array(bytes, sizeof(bytes), &param2);
    assert(err == NEOC_SUCCESS);
    
    assert(neoc_contract_parameter_equals(param1, param2));
    
    neoc_contract_parameter_free(param1);
    neoc_contract_parameter_free(param2);
    printf("  ✅ Bytes equals test passed\n");
}

// Test bytes from string
static void test_bytes_from_string(void) {
    printf("Testing bytes from string...\n");
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_byte_array_from_string("Neo", &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    size_t value_len = 0;
    const uint8_t *value = neoc_contract_parameter_get_byte_array(param, &value_len);
    assert(value != NULL);
    assert(value_len == 3);
    assert(value[0] == 0x4e);  // 'N'
    assert(value[1] == 0x65);  // 'e'
    assert(value[2] == 0x6f);  // 'o'
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Bytes from string test passed\n");
}

// Test array from array
static void test_array_from_array(void) {
    printf("Testing array from array...\n");
    
    neoc_contract_parameter_t *str_param = NULL;
    neoc_error_t err = neoc_contract_parameter_string("value", &str_param);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *bytes_param = NULL;
    err = neoc_contract_parameter_byte_array_from_hex("0x0101", &bytes_param);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *params[2] = {str_param, bytes_param};
    neoc_contract_parameter_t *array_param = NULL;
    err = neoc_contract_parameter_array(params, 2, &array_param);
    assert(err == NEOC_SUCCESS);
    assert(array_param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(array_param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_ARRAY);
    
    size_t array_len = 0;
    neoc_contract_parameter_t **array = neoc_contract_parameter_get_array(array_param, &array_len);
    assert(array != NULL);
    assert(array_len == 2);
    
    neoc_contract_parameter_free(str_param);
    neoc_contract_parameter_free(bytes_param);
    neoc_contract_parameter_free(array_param);
    printf("  ✅ Array from array test passed\n");
}

// Test signature from string
static void test_signature_from_string(void) {
    printf("Testing signature from string...\n");
    
    const char *sig = "d8485d4771e9112cca6ac7e6b75fc52585a2e7ee9a702db4a39dfad0f888ea6c22b6185ceab38d8322b67737a5574d8b63f4e27b0d208f3f9efcdbf56093f213";
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_signature_from_hex(sig, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_SIGNATURE);
    
    size_t value_len = 0;
    const uint8_t *value = neoc_contract_parameter_get_signature(param, &value_len);
    assert(value != NULL);
    assert(value_len == 64);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Signature from string test passed\n");
}

// Test signature from string with 0x prefix
static void test_signature_from_string_with_0x(void) {
    printf("Testing signature from string with 0x...\n");
    
    const char *sig = "0xd8485d4771e9112cca6ac7e6b75fc52585a2e7ee9a702db4a39dfad0f888ea6c22b6185ceab38d8322b67737a5574d8b63f4e27b0d208f3f9efcdbf56093f213";
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_signature_from_hex(sig, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_SIGNATURE);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Signature from string with 0x test passed\n");
}

// Test signature from invalid length
static void test_signature_from_invalid_length(void) {
    printf("Testing signature from invalid length...\n");
    
    // Too short (63 bytes)
    const char *short_sig = "d8485d4771e9112cca6ac7e6b75fc52585a2e7ee9a702db4a39dfad0f888ea6c22b6185ceab38d8322b67737a5574d8b63f4e27b0d208f3f9efcdbf56093f2";
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_signature_from_hex(short_sig, &param);
    assert(err != NEOC_SUCCESS);
    assert(param == NULL);
    
    // Too long (65 bytes)
    const char *long_sig = "d8485d4771e9112cca6ac7e6b75fc52585a2e7ee9a702db4a39dfad0f888ea6c22b6185ceab38d8322b67737a5574d8b63f4e27b0d208f3f9efcdbf56093f213ff";
    
    err = neoc_contract_parameter_signature_from_hex(long_sig, &param);
    assert(err != NEOC_SUCCESS);
    assert(param == NULL);
    
    printf("  ✅ Signature from invalid length test passed\n");
}

// Test boolean
static void test_boolean(void) {
    printf("Testing boolean...\n");
    
    neoc_contract_parameter_t *param_false = NULL;
    neoc_error_t err = neoc_contract_parameter_bool(false, &param_false);
    assert(err == NEOC_SUCCESS);
    assert(param_false != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param_false);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_BOOLEAN);
    
    bool value = neoc_contract_parameter_get_bool(param_false);
    assert(value == false);
    
    neoc_contract_parameter_t *param_true = NULL;
    err = neoc_contract_parameter_bool(true, &param_true);
    assert(err == NEOC_SUCCESS);
    assert(param_true != NULL);
    
    value = neoc_contract_parameter_get_bool(param_true);
    assert(value == true);
    
    neoc_contract_parameter_free(param_false);
    neoc_contract_parameter_free(param_true);
    printf("  ✅ Boolean test passed\n");
}

// Test integer
static void test_integer(void) {
    printf("Testing integer...\n");
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_integer(10, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_INTEGER);
    
    int64_t value = neoc_contract_parameter_get_integer(param);
    assert(value == 10);
    
    neoc_contract_parameter_free(param);
    
    // Test negative integer
    err = neoc_contract_parameter_integer(-1, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    value = neoc_contract_parameter_get_integer(param);
    assert(value == -1);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Integer test passed\n");
}

// Test Hash160
static void test_hash160(void) {
    printf("Testing Hash160...\n");
    
    neoc_hash160_t *hash = NULL;
    neoc_error_t err = neoc_hash160_from_string("576f6f6c6f576f6f6c6f576f6f6c6f576f6f6c6f", &hash);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_hash160(hash, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_HASH160);
    
    neoc_hash160_t *value = neoc_contract_parameter_get_hash160(param);
    assert(value != NULL);
    assert(neoc_hash160_equals(value, hash));
    
    neoc_hash160_free(hash);
    neoc_contract_parameter_free(param);
    printf("  ✅ Hash160 test passed\n");
}

// Test Hash256
static void test_hash256(void) {
    printf("Testing Hash256...\n");
    
    neoc_hash256_t *hash = NULL;
    neoc_error_t err = neoc_hash256_from_string("576f6f6c6f576f6f6c6f576f6f6c6f576f6f6c6ff6c6f576f6f6c6f576f6f6cf", &hash);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_hash256(hash, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_HASH256);
    
    neoc_hash256_t *value = neoc_contract_parameter_get_hash256(param);
    assert(value != NULL);
    assert(neoc_hash256_equals(value, hash));
    
    neoc_hash256_free(hash);
    neoc_contract_parameter_free(param);
    printf("  ✅ Hash256 test passed\n");
}

// Test public key from bytes
static void test_public_key_from_bytes(void) {
    printf("Testing public key from bytes...\n");
    
    const char *key_hex = "03b4af8efe55d98b44eedfcfaa39642fd5d53ad543d18d3cc2db5880970a4654f6";
    uint8_t bytes[33];
    size_t bytes_len = 0;
    neoc_error_t err = neoc_hex_decode(key_hex, bytes, sizeof(bytes), &bytes_len);
    assert(err == NEOC_SUCCESS);
    assert(bytes_len == 33);
    
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_public_key(bytes, bytes_len, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_PUBLIC_KEY);
    
    size_t value_len = 0;
    const uint8_t *value = neoc_contract_parameter_get_public_key(param, &value_len);
    assert(value != NULL);
    assert(value_len == 33);
    assert(memcmp(value, bytes, 33) == 0);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Public key from bytes test passed\n");
}

// Test public key from string
static void test_public_key_from_string(void) {
    printf("Testing public key from string...\n");
    
    const char *key_string = "03b4af8efe55d98b44eedfcfaa39642fd5d53ad543d18d3cc2db5880970a4654f6";
    
    neoc_contract_parameter_t *param = NULL;
    neoc_error_t err = neoc_contract_parameter_public_key_from_hex(key_string, &param);
    assert(err == NEOC_SUCCESS);
    assert(param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_PUBLIC_KEY);
    
    size_t value_len = 0;
    const uint8_t *value = neoc_contract_parameter_get_public_key(param, &value_len);
    assert(value != NULL);
    assert(value_len == 33);
    
    neoc_contract_parameter_free(param);
    printf("  ✅ Public key from string test passed\n");
}

// Test public key from invalid bytes
static void test_public_key_from_invalid_bytes(void) {
    printf("Testing public key from invalid bytes...\n");
    
    // 32 bytes instead of 33
    const char *key_hex = "03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e1368";
    uint8_t bytes[32];
    size_t bytes_len = 0;
    neoc_error_t err = neoc_hex_decode(key_hex, bytes, sizeof(bytes), &bytes_len);
    assert(err == NEOC_SUCCESS);
    assert(bytes_len == 32);
    
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_public_key(bytes, bytes_len, &param);
    assert(err != NEOC_SUCCESS);
    assert(param == NULL);
    
    printf("  ✅ Public key from invalid bytes test passed\n");
}

// Test map
static void test_map(void) {
    printf("Testing map...\n");
    
    // Create key-value pairs
    neoc_contract_parameter_t *key1 = NULL;
    neoc_error_t err = neoc_contract_parameter_integer(1, &key1);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *value1 = NULL;
    err = neoc_contract_parameter_string("first", &value1);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *key2 = NULL;
    err = neoc_contract_parameter_integer(2, &key2);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *value2 = NULL;
    err = neoc_contract_parameter_string("second", &value2);
    assert(err == NEOC_SUCCESS);
    
    // Create map
    neoc_contract_parameter_t *keys[2] = {key1, key2};
    neoc_contract_parameter_t *values[2] = {value1, value2};
    
    neoc_contract_parameter_t *map_param = NULL;
    err = neoc_contract_parameter_map(keys, values, 2, &map_param);
    assert(err == NEOC_SUCCESS);
    assert(map_param != NULL);
    
    neoc_contract_parameter_type_t type = neoc_contract_parameter_get_type(map_param);
    assert(type == NEOC_CONTRACT_PARAMETER_TYPE_MAP);
    
    neoc_contract_parameter_free(key1);
    neoc_contract_parameter_free(value1);
    neoc_contract_parameter_free(key2);
    neoc_contract_parameter_free(value2);
    neoc_contract_parameter_free(map_param);
    printf("  ✅ Map test passed\n");
}

// Test equals
static void test_equals(void) {
    printf("Testing equals...\n");
    
    neoc_contract_parameter_t *param1 = NULL;
    neoc_error_t err = neoc_contract_parameter_string("value", &param1);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *param2 = NULL;
    err = neoc_contract_parameter_string("value", &param2);
    assert(err == NEOC_SUCCESS);
    
    assert(neoc_contract_parameter_equals(param1, param2));
    
    neoc_contract_parameter_t *param3 = NULL;
    err = neoc_contract_parameter_string("test", &param3);
    assert(err == NEOC_SUCCESS);
    
    assert(!neoc_contract_parameter_equals(param1, param3));
    
    neoc_contract_parameter_t *param4 = NULL;
    err = neoc_contract_parameter_integer(1, &param4);
    assert(err == NEOC_SUCCESS);
    
    assert(!neoc_contract_parameter_equals(param1, param4));
    
    neoc_contract_parameter_free(param1);
    neoc_contract_parameter_free(param2);
    neoc_contract_parameter_free(param3);
    neoc_contract_parameter_free(param4);
    printf("  ✅ Equals test passed\n");
}

int main(void) {
    printf("\n=== ContractParameterTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_string_from_string();
    test_bytes_from_bytes();
    test_bytes_from_hex_string();
    test_bytes_equals();
    test_bytes_from_string();
    test_array_from_array();
    test_signature_from_string();
    test_signature_from_string_with_0x();
    test_signature_from_invalid_length();
    test_boolean();
    test_integer();
    test_hash160();
    test_hash256();
    test_public_key_from_bytes();
    test_public_key_from_string();
    test_public_key_from_invalid_bytes();
    test_map();
    test_equals();
    
    tearDown();
    
    printf("\n✅ All ContractParameterTests tests passed!\n\n");
    return 0;
}