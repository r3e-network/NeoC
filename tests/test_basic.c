/**
 * @file test_basic.c
 * @brief Basic functionality tests for NeoC SDK
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/neoc.h"

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            return 0; \
        } else { \
            printf("PASS: %s\n", message); \
        } \
    } while(0)

int test_initialization(void) {
    printf("\nTesting SDK initialization...\n");
    
    neoc_error_t result = neoc_init();
    TEST_ASSERT(result == NEOC_SUCCESS, "SDK initialization");
    
    const char* version = neoc_get_version();
    TEST_ASSERT(version != NULL, "Version string available");
    TEST_ASSERT(strlen(version) > 0, "Version string not empty");
    
    const char* build_info = neoc_get_build_info();
    TEST_ASSERT(build_info != NULL, "Build info available");
    TEST_ASSERT(strlen(build_info) > 0, "Build info not empty");
    
    neoc_cleanup();
    
    return 1;
}

int test_hex_encoding(void) {
    printf("\nTesting hex encoding/decoding...\n");
    
    const uint8_t test_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    const char* expected_hex = "0123456789abcdef";
    
    char hex_buffer[32];
    neoc_error_t result = neoc_hex_encode(test_data, sizeof(test_data), 
                                         hex_buffer, sizeof(hex_buffer), false, false);
    TEST_ASSERT(result == NEOC_SUCCESS, "Hex encoding succeeds");
    TEST_ASSERT(strcmp(hex_buffer, expected_hex) == 0, "Hex encoding produces correct output");
    
    uint8_t decoded_buffer[16];
    size_t decoded_length;
    result = neoc_hex_decode(hex_buffer, decoded_buffer, sizeof(decoded_buffer), &decoded_length);
    TEST_ASSERT(result == NEOC_SUCCESS, "Hex decoding succeeds");
    TEST_ASSERT(decoded_length == sizeof(test_data), "Hex decoding produces correct length");
    TEST_ASSERT(memcmp(decoded_buffer, test_data, sizeof(test_data)) == 0, "Hex decoding roundtrip");
    
    return 1;
}

int test_hash160(void) {
    printf("\nTesting Hash160 operations...\n");
    
    neoc_error_t result = neoc_init();
    TEST_ASSERT(result == NEOC_SUCCESS, "SDK initialized for hash tests");
    
    /* Test zero hash */
    neoc_hash160_t zero_hash;
    result = neoc_hash160_init_zero(&zero_hash);
    TEST_ASSERT(result == NEOC_SUCCESS, "Zero hash initialization");
    TEST_ASSERT(neoc_hash160_is_zero(&zero_hash), "Zero hash is detected as zero");
    
    /* Test from hex */
    const char* test_hex = "17694821c6e3ea8b7a7d770952e7de86c73d94c3";
    neoc_hash160_t hash_from_hex;
    result = neoc_hash160_from_hex(&hash_from_hex, test_hex);
    TEST_ASSERT(result == NEOC_SUCCESS, "Hash160 from hex string");
    TEST_ASSERT(!neoc_hash160_is_zero(&hash_from_hex), "Non-zero hash is not detected as zero");
    
    /* Test to hex */
    char hex_output[41];
    result = neoc_hash160_to_hex(&hash_from_hex, hex_output, sizeof(hex_output), false);
    TEST_ASSERT(result == NEOC_SUCCESS, "Hash160 to hex string");
    TEST_ASSERT(strcmp(hex_output, test_hex) == 0, "Hash160 hex roundtrip");
    
    /* Test copy */
    neoc_hash160_t hash_copy;
    result = neoc_hash160_copy(&hash_copy, &hash_from_hex);
    TEST_ASSERT(result == NEOC_SUCCESS, "Hash160 copy");
    TEST_ASSERT(neoc_hash160_equal(&hash_from_hex, &hash_copy), "Hash160 copy equality");
    
    /* Test comparison */
    TEST_ASSERT(!neoc_hash160_equal(&zero_hash, &hash_from_hex), "Different hashes are not equal");
    TEST_ASSERT(neoc_hash160_compare(&zero_hash, &hash_from_hex) != 0, "Hash160 comparison");
    
    neoc_cleanup();
    return 1;
}

int test_hash256(void) {
    printf("\nTesting Hash256 operations...\n");
    
    neoc_error_t result = neoc_init();
    TEST_ASSERT(result == NEOC_SUCCESS, "SDK initialized for hash256 tests");
    
    /* Test zero hash */
    neoc_hash256_t zero_hash;
    result = neoc_hash256_init_zero(&zero_hash);
    TEST_ASSERT(result == NEOC_SUCCESS, "Zero hash256 initialization");
    TEST_ASSERT(neoc_hash256_is_zero(&zero_hash), "Zero hash256 is detected as zero");
    
    /* Test from data hash */
    const char* test_data = "test data";
    neoc_hash256_t hash_from_data;
    result = neoc_hash256_from_data_hash(&hash_from_data, (const uint8_t*)test_data, strlen(test_data));
    TEST_ASSERT(result == NEOC_SUCCESS, "Hash256 from data hash");
    TEST_ASSERT(!neoc_hash256_is_zero(&hash_from_data), "Non-zero hash256 is not detected as zero");
    
    /* Test double hash */
    neoc_hash256_t double_hash;
    result = neoc_hash256_from_data_double_hash(&double_hash, (const uint8_t*)test_data, strlen(test_data));
    TEST_ASSERT(result == NEOC_SUCCESS, "Hash256 double hash");
    TEST_ASSERT(!neoc_hash256_equal(&hash_from_data, &double_hash), "Single and double hash are different");
    
    /* Test to hex */
    char hex_output[65];
    result = neoc_hash256_to_hex(&hash_from_data, hex_output, sizeof(hex_output), false);
    TEST_ASSERT(result == NEOC_SUCCESS, "Hash256 to hex string");
    TEST_ASSERT(strlen(hex_output) == 64, "Hash256 hex string has correct length");
    
    neoc_cleanup();
    return 1;
}

int test_base58(void) {
    printf("\nTesting Base58 encoding/decoding...\n");
    
    // Initialize SDK for Base58Check (needs SHA256)
    neoc_init();
    
    const uint8_t test_data[] = "Hello World";
    const size_t test_length = strlen((const char*)test_data);
    
    /* Test encoding */
    char* encoded = neoc_base58_encode_alloc(test_data, test_length);
    TEST_ASSERT(encoded != NULL, "Base58 encoding allocation");
    TEST_ASSERT(strlen(encoded) > 0, "Base58 encoded string not empty");
    
    /* Test decoding */
    size_t decoded_length;
    uint8_t* decoded = neoc_base58_decode_alloc(encoded, &decoded_length);
    TEST_ASSERT(decoded != NULL, "Base58 decoding allocation");
    TEST_ASSERT(decoded_length == test_length, "Base58 decoded length correct");
    TEST_ASSERT(memcmp(decoded, test_data, test_length) == 0, "Base58 roundtrip");
    
    neoc_free(encoded);
    neoc_free(decoded);
    
    /* Test Base58Check */
    char* check_encoded = neoc_base58_check_encode_alloc(test_data, test_length);
    TEST_ASSERT(check_encoded != NULL, "Base58Check encoding allocation");
    
    uint8_t* check_decoded = neoc_base58_check_decode_alloc(check_encoded, &decoded_length);
    TEST_ASSERT(check_decoded != NULL, "Base58Check decoding allocation");
    TEST_ASSERT(decoded_length == test_length, "Base58Check decoded length correct");
    TEST_ASSERT(memcmp(check_decoded, test_data, test_length) == 0, "Base58Check roundtrip");
    
    neoc_free(check_encoded);
    neoc_free(check_decoded);
    
    neoc_cleanup();
    return 1;
}

int test_base64(void) {
    printf("\nTesting Base64 encoding/decoding...\n");
    
    const uint8_t test_data[] = "Hello World";
    const size_t test_length = strlen((const char*)test_data);
    
    /* Test encoding */
    char* encoded = neoc_base64_encode_alloc(test_data, test_length);
    TEST_ASSERT(encoded != NULL, "Base64 encoding allocation");
    TEST_ASSERT(strlen(encoded) > 0, "Base64 encoded string not empty");
    
    /* Test decoding */
    size_t decoded_length;
    uint8_t* decoded = neoc_base64_decode_alloc(encoded, &decoded_length);
    TEST_ASSERT(decoded != NULL, "Base64 decoding allocation");
    TEST_ASSERT(decoded_length == test_length, "Base64 decoded length correct");
    TEST_ASSERT(memcmp(decoded, test_data, test_length) == 0, "Base64 roundtrip");
    
    neoc_free(encoded);
    neoc_free(decoded);
    
    return 1;
}

int test_error_handling(void) {
    printf("\nTesting error handling...\n");
    
    /* Test invalid hex */
    neoc_hash160_t hash;
    neoc_error_t result = neoc_hash160_from_hex(&hash, "invalid_hex");
    TEST_ASSERT(result == NEOC_ERROR_INVALID_HEX, "Invalid hex string error");
    
    /* Test null pointer */
    result = neoc_hash160_from_hex(NULL, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT(result == NEOC_ERROR_NULL_POINTER, "Null pointer error");
    
    /* Test buffer too small */
    char small_buffer[5];
    result = neoc_hex_encode((const uint8_t*)"test", 4, small_buffer, sizeof(small_buffer), false, false);
    TEST_ASSERT(result == NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small error");
    
    return 1;
}

int main(void) {
    printf("NeoC SDK Basic Tests\n");
    printf("===================\n");
    
    int tests_passed = 0;
    int total_tests = 0;
    
    total_tests++;
    if (test_initialization()) tests_passed++;
    
    total_tests++;
    if (test_hex_encoding()) tests_passed++;
    
    total_tests++;
    if (test_hash160()) tests_passed++;
    
    total_tests++;
    if (test_hash256()) tests_passed++;
    
    total_tests++;
    if (test_base58()) tests_passed++;
    
    total_tests++;
    if (test_base64()) tests_passed++;
    
    total_tests++;
    if (test_error_handling()) tests_passed++;
    
    printf("\n===================\n");
    printf("Test Results: %d/%d passed\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}

