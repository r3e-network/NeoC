/**
 * @file test_base58.c
 * @brief Unit tests converted from Base58Tests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/crypto/base58.h"
#include "neoc/crypto/hash256.h"

// Test data structure for valid encoding/decoding pairs
typedef struct {
    const char *decoded;
    const char *encoded;
} Base58TestPair;

// Valid string test pairs
static const Base58TestPair VALID_PAIRS[] = {
    {"", ""},
    {" ", "Z"},
    {"-", "n"},
    {"0", "q"},
    {"1", "r"},
    {"-1", "4SU"},
    {"11", "4k8"},
    {"abc", "ZiCa"},
    {"1234598760", "3mJr7AoUXx2Wqd"},
    {"abcdefghijklmnopqrstuvwxyz", "3yxU3u1igY8WkgtjK92fbJQCd4BZiiT1v25f"},
    {"00000000000000000000000000000000000000000000000000000000000000",
     "3sN2THZeE9Eh9eYrwkvZqNstbHGvrxSAM7gXUXvyFQP8XvQLUqNCS27icwUeDT7ckHm4FUHM2mTVh1vbLmk7y"}
};

// Invalid Base58 strings
static const char *INVALID_STRINGS[] = {
    "0",     // Contains '0'
    "O",     // Contains 'O'
    "I",     // Contains 'I'
    "l",     // Contains 'l'
    "3mJr0", // Contains '0'
    "O3yxU", // Contains 'O'
    "3sNI",  // Contains 'I'
    "4kl8",  // Contains 'l'
    "0OIl",  // Contains multiple invalid characters
    "!@#$%^&*()-_=+~`" // Special characters
};

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test Base58 encoding for valid strings
static void test_base58_encoding_for_valid_strings(void) {
    printf("Testing Base58 encoding for valid strings...\n");
    
    size_t num_tests = sizeof(VALID_PAIRS) / sizeof(VALID_PAIRS[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        const char *input = VALID_PAIRS[i].decoded;
        const char *expected = VALID_PAIRS[i].encoded;
        
        // Convert input string to bytes
        size_t input_len = strlen(input);
        uint8_t *input_bytes = (uint8_t *)input;
        
        // Encode to Base58
        char encoded[256];
        neoc_error_t err = neoc_base58_encode(input_bytes, input_len, encoded, sizeof(encoded));
        assert(err == NEOC_SUCCESS);
        
        // Compare with expected
        assert(strcmp(encoded, expected) == 0);
        
        printf("  ✅ Encoded '%s' -> '%s'\n", 
               input_len > 20 ? "..." : input, 
               strlen(expected) > 20 ? "..." : expected);
    }
    
    printf("  ✅ Base58 encoding test passed\n");
}

// Test Base58 decoding for valid strings
static void test_base58_decoding_for_valid_strings(void) {
    printf("Testing Base58 decoding for valid strings...\n");
    
    size_t num_tests = sizeof(VALID_PAIRS) / sizeof(VALID_PAIRS[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        const char *expected = VALID_PAIRS[i].decoded;
        const char *input = VALID_PAIRS[i].encoded;
        
        // Decode from Base58
        uint8_t decoded[256];
        size_t decoded_len = 0;
        neoc_error_t err = neoc_base58_decode(input, decoded, sizeof(decoded), &decoded_len);
        assert(err == NEOC_SUCCESS);
        
        // Convert bytes to string for comparison
        char result_str[256];
        memcpy(result_str, decoded, decoded_len);
        result_str[decoded_len] = '\0';
        
        // Compare with expected
        assert(strcmp(result_str, expected) == 0);
        
        printf("  ✅ Decoded '%s' -> '%s'\n", 
               strlen(input) > 20 ? "..." : input,
               decoded_len > 20 ? "..." : expected);
    }
    
    printf("  ✅ Base58 decoding test passed\n");
}

// Test Base58 decoding for invalid strings
static void test_base58_decoding_for_invalid_strings(void) {
    printf("Testing Base58 decoding for invalid strings...\n");
    
    size_t num_tests = sizeof(INVALID_STRINGS) / sizeof(INVALID_STRINGS[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        const char *invalid = INVALID_STRINGS[i];
        
        // Try to decode - should fail
        uint8_t decoded[256];
        size_t decoded_len = 0;
        neoc_error_t err = neoc_base58_decode(invalid, decoded, sizeof(decoded), &decoded_len);
        assert(err != NEOC_SUCCESS);
        
        printf("  ✅ Correctly rejected invalid string: '%s'\n", invalid);
    }
    
    printf("  ✅ Invalid Base58 decoding test passed\n");
}

// Test Base58 check encoding
static void test_base58_check_encoding(void) {
    printf("Testing Base58 check encoding...\n");
    
    // Input data from Swift test
    uint8_t input_data[] = {
        6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218, 
        133, 13, 109, 40, 194, 236, 153, 44, 61, 157, 254
    };
    size_t input_len = sizeof(input_data);
    
    const char *expected = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";
    
    // Encode with checksum
    char encoded[128];
    neoc_error_t err = neoc_base58_check_encode(input_data, input_len, encoded, sizeof(encoded));
    assert(err == NEOC_SUCCESS);
    
    // Compare with expected
    assert(strcmp(encoded, expected) == 0);
    
    printf("  ✅ Base58 check encoding test passed\n");
}

// Test Base58 check decoding
static void test_base58_check_decoding(void) {
    printf("Testing Base58 check decoding...\n");
    
    const char *input = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";
    
    // Expected output data
    uint8_t expected[] = {
        6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218,
        133, 13, 109, 40, 194, 236, 153, 44, 61, 157, 254
    };
    size_t expected_len = sizeof(expected);
    
    // Decode with checksum verification
    uint8_t decoded[128];
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base58_check_decode(input, decoded, sizeof(decoded), &decoded_len);
    assert(err == NEOC_SUCCESS);
    assert(decoded_len == expected_len);
    
    // Compare with expected
    assert(memcmp(decoded, expected, expected_len) == 0);
    
    printf("  ✅ Base58 check decoding test passed\n");
}

// Test Base58 check decoding with invalid characters
static void test_base58_check_decoding_with_invalid_characters(void) {
    printf("Testing Base58 check decoding with invalid characters...\n");
    
    const char *invalid = "0oO1lL"; // Contains invalid Base58 characters
    
    // Try to decode - should fail
    uint8_t decoded[128];
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base58_check_decode(invalid, decoded, sizeof(decoded), &decoded_len);
    assert(err != NEOC_SUCCESS);
    
    printf("  ✅ Base58 check decoding with invalid characters test passed\n");
}

// Test Base58 check decoding with invalid checksum
static void test_base58_check_decoding_with_invalid_checksum(void) {
    printf("Testing Base58 check decoding with invalid checksum...\n");
    
    // Modified last character to make checksum invalid
    const char *invalid_checksum = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtrW";
    
    // Try to decode - should fail due to checksum mismatch
    uint8_t decoded[128];
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base58_check_decode(invalid_checksum, decoded, sizeof(decoded), &decoded_len);
    assert(err != NEOC_SUCCESS);
    
    printf("  ✅ Base58 check decoding with invalid checksum test passed\n");
}

int main(void) {
    printf("\n=== Base58Tests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_base58_encoding_for_valid_strings();
    test_base58_decoding_for_valid_strings();
    test_base58_decoding_for_invalid_strings();
    test_base58_check_encoding();
    test_base58_check_decoding();
    test_base58_check_decoding_with_invalid_characters();
    test_base58_check_decoding_with_invalid_checksum();
    
    tearDown();
    
    printf("\n✅ All Base58Tests tests passed!\n\n");
    return 0;
}
