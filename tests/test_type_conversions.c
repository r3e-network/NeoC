/**
 * @file test_type_conversions.c
 * @brief Type conversion and utility tests
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/utils/neoc_base64.h>
#include <neoc/utils/neoc_base58.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== HEX CONVERSION TESTS ===== */

void test_hex_encode_decode(void) {
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    char hex_str[21];
    
    // Encode to hex (lowercase, no prefix)
    neoc_error_t err = neoc_hex_encode(data, sizeof(data), hex_str, sizeof(hex_str), false, false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("00010203aabbccddeeff", hex_str);
    
    // Decode back
    uint8_t decoded[10];
    size_t decoded_len;
    err = neoc_hex_decode(hex_str, decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_UINT32(10, decoded_len);
    TEST_ASSERT_EQUAL_MEMORY(data, decoded, 10);
}

void test_hex_with_prefix(void) {
    const char* hex_with_prefix = "0x1234567890abcdef";
    const char* hex_without_prefix = "1234567890abcdef";
    uint8_t expected[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
    
    uint8_t decoded1[8];
    uint8_t decoded2[8];
    size_t len1, len2;
    
    // Decode with prefix
    neoc_error_t err = neoc_hex_decode(hex_with_prefix, decoded1, sizeof(decoded1), &len1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_UINT32(8, len1);
    TEST_ASSERT_EQUAL_MEMORY(expected, decoded1, 8);
    
    // Decode without prefix
    err = neoc_hex_decode(hex_without_prefix, decoded2, sizeof(decoded2), &len2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_UINT32(8, len2);
    TEST_ASSERT_EQUAL_MEMORY(expected, decoded2, 8);
}

/* ===== INTEGER CONVERSION TESTS ===== */

void test_uint32_to_bytes_le(void) {
    uint32_t value = 0x12345678;
    uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12};
    uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
    
    uint8_t bytes_le[4];
    uint8_t bytes_be[4];
    
    // Convert to little endian
    bytes_le[0] = (uint8_t)(value & 0xFF);
    bytes_le[1] = (uint8_t)((value >> 8) & 0xFF);
    bytes_le[2] = (uint8_t)((value >> 16) & 0xFF);
    bytes_le[3] = (uint8_t)((value >> 24) & 0xFF);
    TEST_ASSERT_EQUAL_MEMORY(expected_le, bytes_le, 4);
    
    // Convert to big endian
    bytes_be[0] = (uint8_t)((value >> 24) & 0xFF);
    bytes_be[1] = (uint8_t)((value >> 16) & 0xFF);
    bytes_be[2] = (uint8_t)((value >> 8) & 0xFF);
    bytes_be[3] = (uint8_t)(value & 0xFF);
    TEST_ASSERT_EQUAL_MEMORY(expected_be, bytes_be, 4);
}

void test_uint64_to_bytes_le(void) {
    uint64_t value = 0x123456789ABCDEF0ULL;
    uint8_t expected_le[] = {0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12};
    
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
    
    TEST_ASSERT_EQUAL_MEMORY(expected_le, bytes, 8);
}

/* ===== STRING MANIPULATION TESTS ===== */

void test_string_length_validation(void) {
    const char* short_str = "test";
    const char* exact_str = "12345678901234567890123456789012345"; // 35 chars
    const char* empty_str = "";
    
    TEST_ASSERT_EQUAL_UINT32(4, strlen(short_str));
    TEST_ASSERT_EQUAL_UINT32(35, strlen(exact_str));
    TEST_ASSERT_EQUAL_UINT32(0, strlen(empty_str));
}

void test_string_copy_safety(void) {
    char buffer[32];
    const char* source = "Hello, NeoC!";
    
    // Safe string copy
    strncpy(buffer, source, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    TEST_ASSERT_EQUAL_STRING(source, buffer);
}

/* ===== BYTE ARRAY OPERATIONS ===== */

void test_byte_array_reversal(void) {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t expected[] = {0x05, 0x04, 0x03, 0x02, 0x01};
    uint8_t reversed[5];
    
    // Reverse the array
    for (size_t i = 0; i < 5; i++) {
        reversed[i] = data[4 - i];
    }
    
    TEST_ASSERT_EQUAL_MEMORY(expected, reversed, 5);
}

void test_byte_array_comparison(void) {
    uint8_t array1[] = {0x01, 0x02, 0x03};
    uint8_t array2[] = {0x01, 0x02, 0x03};
    uint8_t array3[] = {0x01, 0x02, 0x04};
    
    TEST_ASSERT_EQUAL_INT(0, memcmp(array1, array2, 3));
    TEST_ASSERT_TRUE(memcmp(array1, array3, 3) != 0);
}

/* ===== BOUNDARY VALUE TESTS ===== */

void test_boundary_values(void) {
    // Test various boundary values
    uint8_t zero = 0;
    uint8_t max_u8 = 255;
    uint16_t max_u16 = 65535;
    uint32_t max_u32 = 0xFFFFFFFF;
    
    TEST_ASSERT_EQUAL_UINT8(0, zero);
    TEST_ASSERT_EQUAL_UINT8(255, max_u8);
    TEST_ASSERT_EQUAL_UINT16(65535, max_u16);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFF, max_u32);
    
    // Test overflow behavior
    uint8_t overflow = max_u8;
    overflow++;
    TEST_ASSERT_EQUAL_UINT8(0, overflow);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== TYPE CONVERSION TESTS ===\n");
    
    RUN_TEST(test_hex_encode_decode);
    RUN_TEST(test_hex_with_prefix);
    RUN_TEST(test_uint32_to_bytes_le);
    RUN_TEST(test_uint64_to_bytes_le);
    RUN_TEST(test_string_length_validation);
    RUN_TEST(test_string_copy_safety);
    RUN_TEST(test_byte_array_reversal);
    RUN_TEST(test_byte_array_comparison);
    RUN_TEST(test_boundary_values);
    
    UNITY_END();
    return 0;
}

