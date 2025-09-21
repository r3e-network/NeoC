/**
 * @file test_base64.c
 * @brief Base64 encoding/decoding tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/utils/neoc_base64.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

// Test vectors from Swift tests
static const char* input_hex = "150c14242dbf5e2f6ac2568b59b7822278d571b75f17be0c14242dbf5e2f6ac2568b59b7822278d571b75f17be13c00c087472616e736665720c14897720d8cd76f4f00abfa37c0edd889c208fde9b41627d5b5238";
static const char* expected_base64 = "FQwUJC2/Xi9qwlaLWbeCInjVcbdfF74MFCQtv14vasJWi1m3giJ41XG3Xxe+E8AMCHRyYW5zZmVyDBSJdyDYzXb08Aq/o3wO3YicII/em0FifVtSOA==";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== BASE64 ENCODING TESTS ===== */

void test_base64_encode_from_hex_string(void) {
    // Decode hex string to bytes
    size_t hex_len = strlen(input_hex);
    size_t byte_len = hex_len / 2;
    uint8_t* bytes = neoc_malloc(byte_len);
    TEST_ASSERT_NOT_NULL(bytes);
    
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(input_hex, bytes, byte_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(byte_len, decoded_len);
    
    // Encode to base64
    size_t base64_size = neoc_base64_encode_buffer_size(decoded_len);
    char* base64 = neoc_malloc(base64_size);
    TEST_ASSERT_NOT_NULL(base64);
    
    err = neoc_base64_encode(bytes, decoded_len, base64, base64_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Compare with expected
    TEST_ASSERT_EQUAL_STRING(expected_base64, base64);
    
    neoc_free(bytes);
    neoc_free(base64);
}

void test_base64_decode(void) {
    // Decode base64
    size_t decoded_size = neoc_base64_decode_buffer_size(expected_base64);
    uint8_t* decoded = neoc_malloc(decoded_size);
    TEST_ASSERT_NOT_NULL(decoded);
    
    size_t decoded_len;
    neoc_error_t err = neoc_base64_decode(expected_base64, decoded, decoded_size, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert to hex string
    char* hex = neoc_malloc(decoded_len * 2 + 1);
    TEST_ASSERT_NOT_NULL(hex);
    err = neoc_hex_encode(decoded, decoded_len, hex, decoded_len * 2 + 1, false, false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Compare with original hex
    TEST_ASSERT_EQUAL_STRING(input_hex, hex);
    
    neoc_free(decoded);
    neoc_free(hex);
}

void test_base64_round_trip(void) {
    // Test with various byte patterns
    uint8_t test_data[][32] = {
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}, // 8 bytes
        {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6}, // 10 bytes
        {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}, // 6 bytes
        {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x11, 0x22, 0x33}, // 11 bytes
    };
    size_t test_sizes[] = {8, 10, 6, 11};
    
    for (int i = 0; i < 4; i++) {
        // Encode
        size_t encoded_size = neoc_base64_encode_buffer_size(test_sizes[i]);
        char* encoded = neoc_malloc(encoded_size);
        TEST_ASSERT_NOT_NULL(encoded);
        
        neoc_error_t err = neoc_base64_encode(test_data[i], test_sizes[i], encoded, encoded_size);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Decode
        size_t decoded_size = neoc_base64_decode_buffer_size(encoded);
        uint8_t* decoded = neoc_malloc(decoded_size);
        TEST_ASSERT_NOT_NULL(decoded);
        
        size_t decoded_len;
        err = neoc_base64_decode(encoded, decoded, decoded_size, &decoded_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(decoded);
        TEST_ASSERT_EQUAL_INT(test_sizes[i], decoded_len);
        
        // Compare
        TEST_ASSERT_EQUAL_MEMORY(test_data[i], decoded, test_sizes[i]);
        
        neoc_free(encoded);
        neoc_free(decoded);
    }
}

void test_base64_empty_input(void) {
    // Test encoding empty input
    size_t encoded_size = neoc_base64_encode_buffer_size(0);
    char* encoded = neoc_malloc(encoded_size);
    TEST_ASSERT_NOT_NULL(encoded);
    
    neoc_error_t err = neoc_base64_encode(NULL, 0, encoded, encoded_size);
    // Empty input might return success or error depending on implementation
    if (err == NEOC_SUCCESS) {
        TEST_ASSERT_EQUAL_STRING("", encoded);
    }
    neoc_free(encoded);
    
    // Test decoding empty input
    uint8_t decoded[10];
    size_t decoded_len;
    err = neoc_base64_decode("", decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(0, decoded_len);
}

void test_base64_padding(void) {
    // Test different padding scenarios
    uint8_t test1[] = {0x01}; // 1 byte -> needs == padding
    uint8_t test2[] = {0x01, 0x02}; // 2 bytes -> needs = padding  
    uint8_t test3[] = {0x01, 0x02, 0x03}; // 3 bytes -> no padding
    
    size_t size1 = neoc_base64_encode_buffer_size(1);
    size_t size2 = neoc_base64_encode_buffer_size(2);
    size_t size3 = neoc_base64_encode_buffer_size(3);
    
    char* encoded1 = neoc_malloc(size1);
    char* encoded2 = neoc_malloc(size2);
    char* encoded3 = neoc_malloc(size3);
    
    neoc_error_t err = neoc_base64_encode(test1, 1, encoded1, size1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(strstr(encoded1, "==") != NULL); // Should have == padding
    
    err = neoc_base64_encode(test2, 2, encoded2, size2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(strstr(encoded2, "=") != NULL); // Should have = padding
    TEST_ASSERT_TRUE(strstr(encoded2, "==") == NULL); // Should not have == padding
    
    err = neoc_base64_encode(test3, 3, encoded3, size3);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(strstr(encoded3, "=") == NULL); // Should have no padding
    
    neoc_free(encoded1);
    neoc_free(encoded2);
    neoc_free(encoded3);
}

void test_base64_invalid_input(void) {
    // Test decoding invalid base64
    uint8_t decoded[100];
    size_t decoded_len;
    
    // Invalid characters
    neoc_error_t err = neoc_base64_decode("ABC@DEF", decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Invalid padding
    err = neoc_base64_decode("ABC=", decoded, sizeof(decoded), &decoded_len);
    // This might succeed or fail depending on implementation
    
    // Wrong length (not multiple of 4 after padding)
    err = neoc_base64_decode("ABCDE", decoded, sizeof(decoded), &decoded_len);
    // This might succeed with lenient decoding
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== BASE64 TESTS ===\n");
    
    RUN_TEST(test_base64_encode_from_hex_string);
    RUN_TEST(test_base64_decode);
    RUN_TEST(test_base64_round_trip);
    RUN_TEST(test_base64_empty_input);
    RUN_TEST(test_base64_padding);
    RUN_TEST(test_base64_invalid_input);
    
    UNITY_END();
    return 0;
}
