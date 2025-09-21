/**
 * @file test_contract_parameter.c
 * @brief ContractParameter tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/contract/contract_parameter.h>
#include <neoc/types/neoc_hash160.h>
#include <neoc/types/neoc_hash256.h>
#include <neoc/utils/neoc_hex.h>
/* Crypto include if needed */
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== CONTRACT PARAMETER TESTS ===== */

void test_string_parameter(void) {
    neoc_contract_parameter_t* param;
    neoc_error_t err = neoc_contract_param_create_string("value", &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_STRING, param->type);
    TEST_ASSERT_EQUAL_STRING("value", param->value.string_value);
    
    neoc_contract_param_free(param);
}

void test_bytes_parameter(void) {
    uint8_t bytes[] = {0x01, 0x01};
    neoc_contract_parameter_t* param;
    neoc_error_t err = neoc_contract_param_create_byte_array(bytes, sizeof(bytes), &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_BYTE_ARRAY, param->type);
    TEST_ASSERT_EQUAL_MEMORY(bytes, param->value.byte_array.data, sizeof(bytes));
    TEST_ASSERT_EQUAL_INT(sizeof(bytes), param->value.byte_array.len);
    
    neoc_contract_param_free(param);
}

void test_bool_parameter(void) {
    neoc_contract_parameter_t* param;
    
    // Test false
    neoc_error_t err = neoc_contract_param_create_boolean(false, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_BOOLEAN, param->type);
    TEST_ASSERT_FALSE(param->value.boolean_value);
    neoc_contract_param_free(param);
    
    // Test true
    err = neoc_contract_param_create_boolean(true, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_BOOLEAN, param->type);
    TEST_ASSERT_TRUE(param->value.boolean_value);
    neoc_contract_param_free(param);
}

void test_integer_parameter(void) {
    neoc_contract_parameter_t* param;
    
    // Test positive integer
    neoc_error_t err = neoc_contract_param_create_integer(10, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_INTEGER, param->type);
    TEST_ASSERT_EQUAL_INT(10, param->value.integer_value);
    neoc_contract_param_free(param);
    
    // Test negative integer
    err = neoc_contract_param_create_integer(-1, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_INTEGER, param->type);
    TEST_ASSERT_EQUAL_INT(-1, param->value.integer_value);
    neoc_contract_param_free(param);
}

void test_hash160_parameter(void) {
    neoc_hash160_t hash;
    neoc_error_t err = neoc_hash160_from_hex(&hash, "576f6f6c6f576f6f6c6f576f6f6c6f576f6f6c6f");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_contract_parameter_t* param;
    err = neoc_contract_param_create_hash160(&hash, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_HASH160, param->type);
    TEST_ASSERT_TRUE(neoc_hash160_equal(&hash, &param->value.hash160));
    
    neoc_contract_param_free(param);
}

void test_hash256_parameter(void) {
    neoc_hash256_t hash;
    neoc_error_t err = neoc_hash256_from_hex(&hash, "576f6f6c6f576f6f6c6f576f6f6c6f576f6f6c6ff6c6f576f6f6c6f576f6f6cf");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_contract_parameter_t* param;
    err = neoc_contract_param_create_hash256(&hash, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_HASH256, param->type);
    TEST_ASSERT_TRUE(neoc_hash256_equal(&hash, &param->value.hash256));
    
    neoc_contract_param_free(param);
}

void test_signature_parameter(void) {
    const char* sig_hex = "d8485d4771e9112cca6ac7e6b75fc52585a2e7ee9a702db4a39dfad0f888ea6c22b6185ceab38d8322b67737a5574d8b63f4e27b0d208f3f9efcdbf56093f213";
    
    uint8_t sig_bytes[64];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(sig_hex, sig_bytes, sizeof(sig_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(64, decoded_len);
    
    neoc_contract_parameter_t* param;
    err = neoc_contract_param_create_signature(sig_bytes, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_SIGNATURE, param->type);
    TEST_ASSERT_EQUAL_MEMORY(sig_bytes, param->value.signature.data, 64);
    
    neoc_contract_param_free(param);
}

void test_public_key_parameter(void) {
    const char* pubkey_hex = "03b4af8efe55d98b44eedfcfaa39642fd5d53ad543d18d3cc2db5880970a4654f6";
    
    uint8_t pubkey_bytes[33];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(pubkey_hex, pubkey_bytes, sizeof(pubkey_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, decoded_len);
    
    neoc_contract_parameter_t* param;
    err = neoc_contract_param_create_public_key(pubkey_bytes, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_PUBLIC_KEY, param->type);
    TEST_ASSERT_EQUAL_MEMORY(pubkey_bytes, param->value.public_key.data, 33);
    
    neoc_contract_param_free(param);
}

void test_array_parameter(void) {
    // Create array elements
    neoc_contract_parameter_t* params[2];
    neoc_error_t err = neoc_contract_param_create_string("value", &params[0]);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    uint8_t bytes[] = {0x01, 0x01};
    err = neoc_contract_param_create_byte_array(bytes, sizeof(bytes), &params[1]);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create array parameter
    neoc_contract_parameter_t* array_param;
    err = neoc_contract_param_create_array(params, 2, &array_param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(array_param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_ARRAY, array_param->type);
    TEST_ASSERT_EQUAL_INT(2, array_param->value.array.count);
    
    // Verify array contents
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_STRING, array_param->value.array.items[0]->type);
    TEST_ASSERT_EQUAL_STRING("value", array_param->value.array.items[0]->value.string_value);
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_BYTE_ARRAY, array_param->value.array.items[1]->type);
    
    neoc_contract_param_free(array_param);
    neoc_contract_param_free(params[0]);
    neoc_contract_param_free(params[1]);
}

void test_empty_array_parameter(void) {
    neoc_contract_parameter_t* array_param;
    neoc_error_t err = neoc_contract_param_create_array(NULL, 0, &array_param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(array_param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_ARRAY, array_param->type);
    TEST_ASSERT_EQUAL_INT(0, array_param->value.array.count);
    
    neoc_contract_param_free(array_param);
}

void test_map_parameter(void) {
    // Create map keys and values
    neoc_contract_parameter_t* keys[2];
    neoc_contract_parameter_t* values[2];
    
    neoc_error_t err = neoc_contract_param_create_integer(1, &keys[0]);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    err = neoc_contract_param_create_string("first", &values[0]);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_contract_param_create_integer(2, &keys[1]);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    err = neoc_contract_param_create_string("second", &values[1]);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create map parameter
    neoc_contract_parameter_t* map_param;
    err = neoc_contract_param_create_map(keys, values, 2, &map_param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(map_param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_MAP, map_param->type);
    TEST_ASSERT_EQUAL_INT(2, map_param->value.map.count);
    
    neoc_contract_param_free(map_param);
    neoc_contract_param_free(keys[0]);
    neoc_contract_param_free(values[0]);
    neoc_contract_param_free(keys[1]);
    neoc_contract_param_free(values[1]);
}

void test_any_parameter(void) {
    neoc_contract_parameter_t* param;
    neoc_error_t err = neoc_contract_param_create_any(NULL, &param);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(param);
    
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_ANY, param->type);
    
    neoc_contract_param_free(param);
}

void test_parameter_types(void) {
    // Test that all parameter type constants are defined correctly
    TEST_ASSERT_EQUAL_INT(0x00, NEOC_CONTRACT_PARAM_ANY);
    TEST_ASSERT_EQUAL_INT(0x10, NEOC_CONTRACT_PARAM_BOOLEAN);
    TEST_ASSERT_EQUAL_INT(0x11, NEOC_CONTRACT_PARAM_INTEGER);
    TEST_ASSERT_EQUAL_INT(0x12, NEOC_CONTRACT_PARAM_BYTE_ARRAY);
    TEST_ASSERT_EQUAL_INT(0x13, NEOC_CONTRACT_PARAM_STRING);
    TEST_ASSERT_EQUAL_INT(0x14, NEOC_CONTRACT_PARAM_HASH160);
    TEST_ASSERT_EQUAL_INT(0x15, NEOC_CONTRACT_PARAM_HASH256);
    TEST_ASSERT_EQUAL_INT(0x16, NEOC_CONTRACT_PARAM_PUBLIC_KEY);
    TEST_ASSERT_EQUAL_INT(0x17, NEOC_CONTRACT_PARAM_SIGNATURE);
    TEST_ASSERT_EQUAL_INT(0x20, NEOC_CONTRACT_PARAM_ARRAY);
    TEST_ASSERT_EQUAL_INT(0x22, NEOC_CONTRACT_PARAM_MAP);
    TEST_ASSERT_EQUAL_INT(0x30, NEOC_CONTRACT_PARAM_INTEROP_INTERFACE);
    TEST_ASSERT_EQUAL_INT(0xFF, NEOC_CONTRACT_PARAM_VOID);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== CONTRACT PARAMETER TESTS ===\n");
    
    RUN_TEST(test_string_parameter);
    RUN_TEST(test_bytes_parameter);
    RUN_TEST(test_bool_parameter);
    RUN_TEST(test_integer_parameter);
    RUN_TEST(test_hash160_parameter);
    RUN_TEST(test_hash256_parameter);
    RUN_TEST(test_signature_parameter);
    RUN_TEST(test_public_key_parameter);
    RUN_TEST(test_array_parameter);
    RUN_TEST(test_empty_array_parameter);
    RUN_TEST(test_map_parameter);
    RUN_TEST(test_any_parameter);
    RUN_TEST(test_parameter_types);
    
    UNITY_END();
    return 0;
}
