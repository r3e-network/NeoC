#include "unity.h"
#include "neoc/serialization/neo_serializable.h"
#include "neoc/protocol/stack_item.h"
#include "neoc/protocol/contract_response_types.h"
#include "neoc/protocol/rpc_response_types.h"
#include "neoc/utils/neoc_numeric_utils.h"
#include <string.h>
#include <stdlib.h>

/**
 * @file test_new_components.c
 * @brief Test suite for new Neo C SDK components
 * 
 * This file tests the newly implemented components including:
 * - NeoSerializable protocol implementation
 * - StackItem functionality
 * - Contract response types
 * - RPC response types  
 * - Numeric utilities
 * 
 * @author NeoC SDK Team
 * @version 1.1.0
 * @date 2024
 */

void setUp(void) {
    // Set up test fixtures, if any
}

void tearDown(void) {
    // Clean up after each test
}

// Stack Item Tests
void test_stack_item_create_boolean(void) {
    neoc_stack_item_t *item = NULL;
    
    // Test creating a boolean stack item
    neoc_error_t result = neoc_stack_item_create_boolean(true, &item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL(NEOC_STACK_ITEM_BOOLEAN, neoc_stack_item_get_type(item));
    
    // Test getting the boolean value
    bool value;
    result = neoc_stack_item_get_boolean(item, &value);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_TRUE(value);
    
    neoc_stack_item_free(item);
}

void test_stack_item_create_integer(void) {
    neoc_stack_item_t *item = NULL;
    
    // Test creating an integer stack item
    neoc_error_t result = neoc_stack_item_create_integer_int32(42, &item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL(NEOC_STACK_ITEM_INTEGER, neoc_stack_item_get_type(item));
    
    // Test getting the integer value
    int32_t value;
    result = neoc_stack_item_get_int32(item, &value);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(42, value);
    
    neoc_stack_item_free(item);
}

void test_stack_item_create_byte_string(void) {
    neoc_stack_item_t *item = NULL;
    const uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    
    // Test creating a byte string stack item
    neoc_error_t result = neoc_stack_item_create_byte_string(test_data, sizeof(test_data), &item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(item);
    TEST_ASSERT_EQUAL(NEOC_STACK_ITEM_BYTE_STRING, neoc_stack_item_get_type(item));
    
    // Test getting the byte array
    const uint8_t *data;
    size_t length;
    result = neoc_stack_item_get_byte_array(item, &data, &length);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(sizeof(test_data), length);
    TEST_ASSERT_EQUAL_MEMORY(test_data, data, sizeof(test_data));
    
    neoc_stack_item_free(item);
}

void test_stack_item_create_array(void) {
    neoc_stack_item_t *item1 = NULL, *item2 = NULL, *array_item = NULL;
    neoc_stack_item_t *items[2];
    
    // Create some items for the array
    neoc_error_t result = neoc_stack_item_create_boolean(true, &item1);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    result = neoc_stack_item_create_integer_int32(123, &item2);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    items[0] = item1;
    items[1] = item2;
    
    // Create array stack item
    result = neoc_stack_item_create_array(items, 2, &array_item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(array_item);
    TEST_ASSERT_EQUAL(NEOC_STACK_ITEM_ARRAY, neoc_stack_item_get_type(array_item));
    
    // Test getting the array
    neoc_stack_item_t **array_items;
    size_t count;
    result = neoc_stack_item_get_array(array_item, &array_items, &count);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_NOT_NULL(array_items);
    
    // Clean up
    neoc_stack_item_free(item1);
    neoc_stack_item_free(item2);
    neoc_stack_item_free(array_item);
}

void test_stack_item_equals(void) {
    neoc_stack_item_t *item1 = NULL, *item2 = NULL, *item3 = NULL;
    
    // Create identical items
    neoc_error_t result = neoc_stack_item_create_integer_int32(42, &item1);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    result = neoc_stack_item_create_integer_int32(42, &item2);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    result = neoc_stack_item_create_integer_int32(99, &item3);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    // Test equality
    bool equal;
    result = neoc_stack_item_equals(item1, item2, &equal);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_TRUE(equal);
    
    result = neoc_stack_item_equals(item1, item3, &equal);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_FALSE(equal);
    
    neoc_stack_item_free(item1);
    neoc_stack_item_free(item2);
    neoc_stack_item_free(item3);
}

// NeoSerializable Tests
void test_serializable_validate(void) {
    neoc_stack_item_t *item = NULL;
    
    // Create a stack item (which implements NeoSerializable)
    neoc_error_t result = neoc_stack_item_create_boolean(true, &item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    // Test validation
    result = neoc_serializable_validate(&item->base);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    neoc_stack_item_free(item);
}

void test_serializable_get_size(void) {
    neoc_stack_item_t *item = NULL;
    
    // Create a stack item
    neoc_error_t result = neoc_stack_item_create_boolean(true, &item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    // Test getting size
    size_t size;
    result = neoc_serializable_get_size(&item->base, &size);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_GREATER_THAN(0, size);
    
    neoc_stack_item_free(item);
}

void test_serializable_to_array(void) {
    neoc_stack_item_t *item = NULL;
    
    // Create a stack item
    neoc_error_t result = neoc_stack_item_create_boolean(true, &item);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    // Test serialization to array
    uint8_t *data;
    size_t len;
    result = neoc_serializable_to_array(&item->base, &data, &len);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_GREATER_THAN(0, len);
    
    free(data);
    neoc_stack_item_free(item);
}

// Numeric Utility Tests
void test_var_int_size(void) {
    TEST_ASSERT_EQUAL(1, neoc_var_int_size(0));
    TEST_ASSERT_EQUAL(1, neoc_var_int_size(252));
    TEST_ASSERT_EQUAL(3, neoc_var_int_size(253));
    TEST_ASSERT_EQUAL(3, neoc_var_int_size(65535));
    TEST_ASSERT_EQUAL(5, neoc_var_int_size(65536));
    TEST_ASSERT_EQUAL(5, neoc_var_int_size(0xFFFFFFFF));
    TEST_ASSERT_EQUAL(9, neoc_var_int_size(0x100000000ULL));
}

void test_encode_decode_var_int(void) {
    uint8_t buffer[16];
    size_t bytes_written, bytes_consumed;
    uint64_t decoded_value;
    
    // Test small value
    neoc_error_t result = neoc_encode_var_int(42, buffer, sizeof(buffer), &bytes_written);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(1, bytes_written);
    TEST_ASSERT_EQUAL(42, buffer[0]);
    
    result = neoc_decode_var_int(buffer, bytes_written, &decoded_value, &bytes_consumed);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(42, decoded_value);
    TEST_ASSERT_EQUAL(1, bytes_consumed);
    
    // Test medium value
    result = neoc_encode_var_int(1000, buffer, sizeof(buffer), &bytes_written);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(3, bytes_written);
    TEST_ASSERT_EQUAL(0xFD, buffer[0]);
    
    result = neoc_decode_var_int(buffer, bytes_written, &decoded_value, &bytes_consumed);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL(1000, decoded_value);
    TEST_ASSERT_EQUAL(3, bytes_consumed);
}

void test_bytes_to_uint_conversion(void) {
    uint8_t bytes_le[] = {0x34, 0x12}; // 0x1234 in little-endian
    uint8_t bytes_be[] = {0x12, 0x34}; // 0x1234 in big-endian
    
    TEST_ASSERT_EQUAL(0x1234, neoc_bytes_to_uint16_le(bytes_le));
    TEST_ASSERT_EQUAL(0x1234, neoc_bytes_to_uint16_be(bytes_be));
    
    uint8_t out_buffer[4];
    neoc_uint16_to_bytes_le(0x1234, out_buffer);
    TEST_ASSERT_EQUAL(0x34, out_buffer[0]);
    TEST_ASSERT_EQUAL(0x12, out_buffer[1]);
    
    neoc_uint16_to_bytes_be(0x1234, out_buffer);
    TEST_ASSERT_EQUAL(0x12, out_buffer[0]);
    TEST_ASSERT_EQUAL(0x34, out_buffer[1]);
}

void test_reverse_bytes(void) {
    uint8_t bytes[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t original[] = {0x01, 0x02, 0x03, 0x04};
    
    neoc_reverse_bytes(bytes, 4);
    TEST_ASSERT_EQUAL(0x04, bytes[0]);
    TEST_ASSERT_EQUAL(0x03, bytes[1]);
    TEST_ASSERT_EQUAL(0x02, bytes[2]);
    TEST_ASSERT_EQUAL(0x01, bytes[3]);
    
    uint8_t reversed[4];
    neoc_copy_reverse_bytes(original, reversed, 4);
    TEST_ASSERT_EQUAL(0x04, reversed[0]);
    TEST_ASSERT_EQUAL(0x03, reversed[1]);
    TEST_ASSERT_EQUAL(0x02, reversed[2]);
    TEST_ASSERT_EQUAL(0x01, reversed[3]);
    
    // Original should be unchanged
    TEST_ASSERT_EQUAL(0x01, original[0]);
    TEST_ASSERT_EQUAL(0x02, original[1]);
    TEST_ASSERT_EQUAL(0x03, original[2]);
    TEST_ASSERT_EQUAL(0x04, original[3]);
}

void test_big_int_compare(void) {
    uint8_t a[] = {0x34, 0x12}; // 0x1234 in little-endian
    uint8_t b[] = {0x56, 0x12}; // 0x1256 in little-endian
    uint8_t c[] = {0x34, 0x12}; // 0x1234 in little-endian (same as a)
    
    TEST_ASSERT_EQUAL(-1, neoc_big_int_compare(a, 2, b, 2)); // a < b
    TEST_ASSERT_EQUAL(1, neoc_big_int_compare(b, 2, a, 2));  // b > a
    TEST_ASSERT_EQUAL(0, neoc_big_int_compare(a, 2, c, 2));  // a == c
}

void test_big_int_is_zero(void) {
    uint8_t zero[] = {0x00, 0x00};
    uint8_t nonzero[] = {0x01, 0x00};
    
    TEST_ASSERT_TRUE(neoc_big_int_is_zero(zero, 2));
    TEST_ASSERT_FALSE(neoc_big_int_is_zero(nonzero, 2));
}

void test_big_int_to_int32(void) {
    uint8_t positive[] = {0x2A, 0x00, 0x00, 0x00}; // 42 in little-endian
    uint8_t negative[] = {0x2A, 0x00, 0x00, 0x00}; // Will be treated as -42
    
    int32_t result;
    neoc_error_t err = neoc_big_int_to_int32(positive, 4, false, &result);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL(42, result);
    
    err = neoc_big_int_to_int32(negative, 4, true, &result);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL(-42, result);
}

void test_int32_to_big_int(void) {
    uint8_t *bytes;
    size_t length;
    bool is_negative;
    
    neoc_error_t result = neoc_int32_to_big_int(42, &bytes, &length, &is_negative);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(bytes);
    TEST_ASSERT_GREATER_THAN(0, length);
    TEST_ASSERT_FALSE(is_negative);
    TEST_ASSERT_EQUAL(0x2A, bytes[0]); // 42 in little-endian
    
    free(bytes);
    
    result = neoc_int32_to_big_int(-42, &bytes, &length, &is_negative);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(bytes);
    TEST_ASSERT_GREATER_THAN(0, length);
    TEST_ASSERT_TRUE(is_negative);
    TEST_ASSERT_EQUAL(0x2A, bytes[0]); // 42 in little-endian (magnitude)
    
    free(bytes);
}

void test_safe_math_operations(void) {
    int64_t result;
    
    // Test safe addition
    neoc_error_t err = neoc_safe_add_int64(100, 200, &result);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL(300, result);
    
    // Test overflow detection
    err = neoc_safe_add_int64(INT64_MAX, 1, &result);
    TEST_ASSERT_EQUAL(NEOC_OVERFLOW, err);
    
    // Test safe multiplication
    err = neoc_safe_mul_int64(100, 200, &result);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL(20000, result);
    
    // Test multiplication overflow
    err = neoc_safe_mul_int64(INT64_MAX, 2, &result);
    TEST_ASSERT_EQUAL(NEOC_OVERFLOW, err);
}

void test_in_range(void) {
    TEST_ASSERT_TRUE(neoc_in_range(50, 0, 100));
    TEST_ASSERT_TRUE(neoc_in_range(0, 0, 100));
    TEST_ASSERT_TRUE(neoc_in_range(100, 0, 100));
    TEST_ASSERT_FALSE(neoc_in_range(-1, 0, 100));
    TEST_ASSERT_FALSE(neoc_in_range(101, 0, 100));
}

// Contract Response Types Tests
void test_contract_manifest_create(void) {
    neoc_contract_manifest_t *manifest = NULL;
    
    neoc_error_t result = neoc_contract_manifest_create(&manifest);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(manifest);
    
    // Test setting name
    result = neoc_contract_manifest_set_name(manifest, "TestContract");
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    neoc_contract_manifest_free(manifest);
}

void test_contract_nef_create(void) {
    neoc_contract_nef_t *nef = NULL;
    
    neoc_error_t result = neoc_contract_nef_create(&nef);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(nef);
    
    // Test setting compiler
    result = neoc_contract_nef_set_compiler(nef, "neow3j-3.0.0");
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    
    neoc_contract_nef_free(nef);
}

void test_contract_state_create(void) {
    neoc_contract_state_t *state = NULL;
    
    neoc_error_t result = neoc_contract_state_create(&state);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(state);
    
    neoc_contract_state_free(state);
}

// RPC Response Types Tests  
void test_version_info_create(void) {
    neoc_version_info_t *version_info = NULL;
    
    neoc_error_t result = neoc_version_info_create(&version_info);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(version_info);
    
    neoc_version_info_free(version_info);
}

void test_block_create(void) {
    neoc_block_t *block = NULL;
    
    neoc_error_t result = neoc_block_create(&block);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, result);
    TEST_ASSERT_NOT_NULL(block);
    
    neoc_block_free(block);
}

void test_nep17_balances_result_create(void) {
    neoc_nep17_balances_result_t *result = NULL;
    
    neoc_error_t err = neoc_nep17_balances_result_create(&result);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(result);
    
    neoc_nep17_balances_result_free(result);
}

void test_invocation_result_create(void) {
    neoc_invocation_result_t *result = NULL;
    
    neoc_error_t err = neoc_invocation_result_create(&result);
    TEST_ASSERT_EQUAL(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(result);
    
    neoc_invocation_result_free(result);
}

// Test runner
int main(void) {
    UNITY_BEGIN();
    
    // Stack Item Tests
    RUN_TEST(test_stack_item_create_boolean);
    RUN_TEST(test_stack_item_create_integer);
    RUN_TEST(test_stack_item_create_byte_string);
    RUN_TEST(test_stack_item_create_array);
    RUN_TEST(test_stack_item_equals);
    
    // NeoSerializable Tests
    RUN_TEST(test_serializable_validate);
    RUN_TEST(test_serializable_get_size);
    RUN_TEST(test_serializable_to_array);
    
    // Numeric Utility Tests
    RUN_TEST(test_var_int_size);
    RUN_TEST(test_encode_decode_var_int);
    RUN_TEST(test_bytes_to_uint_conversion);
    RUN_TEST(test_reverse_bytes);
    RUN_TEST(test_big_int_compare);
    RUN_TEST(test_big_int_is_zero);
    RUN_TEST(test_big_int_to_int32);
    RUN_TEST(test_int32_to_big_int);
    RUN_TEST(test_safe_math_operations);
    RUN_TEST(test_in_range);
    
    // Contract Response Types Tests
    RUN_TEST(test_contract_manifest_create);
    RUN_TEST(test_contract_nef_create);
    RUN_TEST(test_contract_state_create);
    
    // RPC Response Types Tests
    RUN_TEST(test_version_info_create);
    RUN_TEST(test_block_create);
    RUN_TEST(test_nep17_balances_result_create);
    RUN_TEST(test_invocation_result_create);
    
    return UNITY_END();
}
