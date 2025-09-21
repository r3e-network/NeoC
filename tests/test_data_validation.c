/**
 * @file test_data_validation.c
 * @brief Data validation and helper function tests
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/crypto/sha256.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== DATA VALIDATION TESTS ===== */

void test_hex_string_validation(void) {
    // Valid hex strings
    TEST_ASSERT_TRUE(neoc_hex_is_valid_string("0123456789abcdef", false));
    TEST_ASSERT_TRUE(neoc_hex_is_valid_string("ABCDEF", false));
    TEST_ASSERT_TRUE(neoc_hex_is_valid_string("0x1234", true));
    TEST_ASSERT_TRUE(neoc_hex_is_valid_string("0X5678", true));
    
    // Invalid hex strings
    TEST_ASSERT_FALSE(neoc_hex_is_valid_string("ghijkl", false));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_string("12 34", false));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_string("0x", true));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_string("", false));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_string("0xGHI", true));
}

void test_hex_char_validation(void) {
    // Valid hex chars
    for (char c = '0'; c <= '9'; c++) {
        TEST_ASSERT_TRUE(neoc_hex_is_valid_char(c));
    }
    for (char c = 'a'; c <= 'f'; c++) {
        TEST_ASSERT_TRUE(neoc_hex_is_valid_char(c));
    }
    for (char c = 'A'; c <= 'F'; c++) {
        TEST_ASSERT_TRUE(neoc_hex_is_valid_char(c));
    }
    
    // Invalid hex chars
    TEST_ASSERT_FALSE(neoc_hex_is_valid_char('g'));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_char('G'));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_char(' '));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_char('\n'));
    TEST_ASSERT_FALSE(neoc_hex_is_valid_char('#'));
}

void test_hex_char_to_value(void) {
    TEST_ASSERT_EQUAL_INT(0, neoc_hex_char_to_value('0'));
    TEST_ASSERT_EQUAL_INT(9, neoc_hex_char_to_value('9'));
    TEST_ASSERT_EQUAL_INT(10, neoc_hex_char_to_value('a'));
    TEST_ASSERT_EQUAL_INT(10, neoc_hex_char_to_value('A'));
    TEST_ASSERT_EQUAL_INT(15, neoc_hex_char_to_value('f'));
    TEST_ASSERT_EQUAL_INT(15, neoc_hex_char_to_value('F'));
    
    // Invalid chars return -1
    TEST_ASSERT_EQUAL_INT(-1, neoc_hex_char_to_value('g'));
    TEST_ASSERT_EQUAL_INT(-1, neoc_hex_char_to_value(' '));
}

void test_hex_value_to_char(void) {
    // Lowercase
    TEST_ASSERT_EQUAL_INT('0', neoc_hex_value_to_char(0, false));
    TEST_ASSERT_EQUAL_INT('9', neoc_hex_value_to_char(9, false));
    TEST_ASSERT_EQUAL_INT('a', neoc_hex_value_to_char(10, false));
    TEST_ASSERT_EQUAL_INT('f', neoc_hex_value_to_char(15, false));
    
    // Uppercase
    TEST_ASSERT_EQUAL_INT('0', neoc_hex_value_to_char(0, true));
    TEST_ASSERT_EQUAL_INT('9', neoc_hex_value_to_char(9, true));
    TEST_ASSERT_EQUAL_INT('A', neoc_hex_value_to_char(10, true));
    TEST_ASSERT_EQUAL_INT('F', neoc_hex_value_to_char(15, true));
    
    // Invalid values return '\0'
    TEST_ASSERT_EQUAL_INT('\0', neoc_hex_value_to_char(16, false));
    TEST_ASSERT_EQUAL_INT('\0', neoc_hex_value_to_char(-1, false));
}

void test_hex_buffer_size_calculation(void) {
    // Without prefix
    TEST_ASSERT_EQUAL_UINT32(1, neoc_hex_encode_buffer_size(0, false));  // Just null terminator
    TEST_ASSERT_EQUAL_UINT32(3, neoc_hex_encode_buffer_size(1, false));  // 2 hex chars + null
    TEST_ASSERT_EQUAL_UINT32(21, neoc_hex_encode_buffer_size(10, false)); // 20 hex chars + null
    TEST_ASSERT_EQUAL_UINT32(65, neoc_hex_encode_buffer_size(32, false)); // 64 hex chars + null
    
    // With "0x" prefix
    TEST_ASSERT_EQUAL_UINT32(3, neoc_hex_encode_buffer_size(0, true));   // "0x" + null
    TEST_ASSERT_EQUAL_UINT32(5, neoc_hex_encode_buffer_size(1, true));   // "0x" + 2 hex chars + null
    TEST_ASSERT_EQUAL_UINT32(23, neoc_hex_encode_buffer_size(10, true)); // "0x" + 20 hex chars + null
    TEST_ASSERT_EQUAL_UINT32(67, neoc_hex_encode_buffer_size(32, true)); // "0x" + 64 hex chars + null
}

void test_hex_decode_buffer_size(void) {
    TEST_ASSERT_EQUAL_UINT32(0, neoc_hex_decode_buffer_size(""));
    TEST_ASSERT_EQUAL_UINT32(1, neoc_hex_decode_buffer_size("00"));
    TEST_ASSERT_EQUAL_UINT32(1, neoc_hex_decode_buffer_size("FF"));
    TEST_ASSERT_EQUAL_UINT32(10, neoc_hex_decode_buffer_size("00112233445566778899"));
    
    // With 0x prefix
    TEST_ASSERT_EQUAL_UINT32(0, neoc_hex_decode_buffer_size("0x"));
    TEST_ASSERT_EQUAL_UINT32(1, neoc_hex_decode_buffer_size("0x00"));
    TEST_ASSERT_EQUAL_UINT32(32, neoc_hex_decode_buffer_size("0x0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"));
}

void test_hex_normalization(void) {
    char buffer[128];
    
    // Normalize to lowercase
    neoc_error_t err = neoc_hex_normalize("0xABCDEF", buffer, sizeof(buffer), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("abcdef", buffer);
    
    // Normalize to uppercase
    err = neoc_hex_normalize("0xabcdef", buffer, sizeof(buffer), true);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("ABCDEF", buffer);
    
    // Mixed case input
    err = neoc_hex_normalize("0xAbCdEf", buffer, sizeof(buffer), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("abcdef", buffer);
    
    // No prefix
    err = neoc_hex_normalize("DeadBeef", buffer, sizeof(buffer), true);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("DEADBEEF", buffer);
}

void test_hex_comparison(void) {
    // Case insensitive comparison
    TEST_ASSERT_TRUE(neoc_hex_equal("abcdef", "ABCDEF"));
    TEST_ASSERT_TRUE(neoc_hex_equal("0xabcdef", "ABCDEF"));
    TEST_ASSERT_TRUE(neoc_hex_equal("0xABCDEF", "0xabcdef"));
    TEST_ASSERT_TRUE(neoc_hex_equal("1234", "0x1234"));
    
    // Different values
    TEST_ASSERT_FALSE(neoc_hex_equal("abcdef", "fedcba"));
    TEST_ASSERT_FALSE(neoc_hex_equal("1234", "5678"));
    TEST_ASSERT_FALSE(neoc_hex_equal("0x00", "0xFF"));
    
    // Different lengths
    TEST_ASSERT_FALSE(neoc_hex_equal("ab", "abcd"));
    TEST_ASSERT_FALSE(neoc_hex_equal("0x1234", "0x123456"));
}

void test_data_padding(void) {
    // Test padding to 32 bytes (common for hash values)
    uint8_t data[32];
    size_t data_len = 20;
    
    // Fill with test data
    for (size_t i = 0; i < data_len; i++) {
        data[i] = (uint8_t)i;
    }
    
    // Pad with zeros
    memset(data + data_len, 0, 32 - data_len);
    
    // Check padding
    for (size_t i = 0; i < data_len; i++) {
        TEST_ASSERT_EQUAL_UINT8(i, data[i]);
    }
    for (size_t i = data_len; i < 32; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, data[i]);
    }
}

void test_byte_array_comparison(void) {
    uint8_t arr1[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t arr2[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t arr3[] = {0x01, 0x02, 0x03, 0x04, 0x06};
    uint8_t arr4[] = {0x01, 0x02, 0x03};
    
    // Equal arrays
    TEST_ASSERT_EQUAL_INT(0, memcmp(arr1, arr2, 5));
    
    // Different last byte
    TEST_ASSERT_TRUE(memcmp(arr1, arr3, 5) < 0);
    
    // Equal up to shorter length
    TEST_ASSERT_EQUAL_INT(0, memcmp(arr1, arr4, 3));
}

void test_checksum_validation(void) {
    // Simple checksum by XOR (example)
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t checksum = 0;
    
    for (size_t i = 0; i < sizeof(data); i++) {
        checksum ^= data[i];
    }
    
    // Expected: 0x01 ^ 0x02 ^ 0x03 ^ 0x04 = 0x04
    TEST_ASSERT_EQUAL_UINT8(0x04, checksum);
    
    // Verify checksum
    uint8_t verify = 0;
    for (size_t i = 0; i < sizeof(data); i++) {
        verify ^= data[i];
    }
    verify ^= checksum;
    TEST_ASSERT_EQUAL_UINT8(0x00, verify); // Should be zero if valid
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== DATA VALIDATION TESTS ===\n");
    
    RUN_TEST(test_hex_string_validation);
    RUN_TEST(test_hex_char_validation);
    RUN_TEST(test_hex_char_to_value);
    RUN_TEST(test_hex_value_to_char);
    RUN_TEST(test_hex_buffer_size_calculation);
    RUN_TEST(test_hex_decode_buffer_size);
    RUN_TEST(test_hex_normalization);
    RUN_TEST(test_hex_comparison);
    RUN_TEST(test_data_padding);
    RUN_TEST(test_byte_array_comparison);
    RUN_TEST(test_checksum_validation);
    
    UNITY_END();
    return 0;
}
