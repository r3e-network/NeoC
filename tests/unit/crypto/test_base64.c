/**
 * @file test_base64.c
 * @brief Unit tests converted from Base64Tests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include "neoc/neoc.h"
#include "neoc/crypto/base64.h"
#include "neoc/utils/hex.h"

// Test data
static const char *INPUT_HEX = "150c14242dbf5e2f6ac2568b59b7822278d571b75f17be0c14242dbf5e2f6ac2568b59b7822278d571b75f17be13c00c087472616e736665720c14897720d8cd76f4f00abfa37c0edd889c208fde9b41627d5b5238";
static const char *OUTPUT_BASE64 = "FQwUJC2/Xi9qwlaLWbeCInjVcbdfF74MFCQtv14vasJWi1m3giJ41XG3Xxe+E8AMCHRyYW5zZmVyDBSJdyDYzXb08Aq/o3wO3YicII/em0FifVtSOA==";

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test Base64 encode for hex string
static void test_base64_encode_for_string(void) {
    printf("Testing Base64 encode for hex string...\n");
    
    // Convert hex string to bytes
    size_t hex_len = strlen(INPUT_HEX) / 2;
    uint8_t *bytes = malloc(hex_len);
    assert(bytes != NULL);
    
    size_t decoded_len = 0;
    neoc_error_t err = neoc_hex_decode(INPUT_HEX, bytes, hex_len, &decoded_len);
    assert(err == NEOC_SUCCESS);
    assert(decoded_len == hex_len);
    
    // Encode to Base64
    size_t output_size = ((decoded_len + 2) / 3) * 4 + 1;
    char *encoded = malloc(output_size);
    assert(encoded != NULL);
    
    err = neoc_base64_encode(bytes, decoded_len, encoded, output_size);
    assert(err == NEOC_SUCCESS);
    
    // Compare with expected
    assert(strcmp(encoded, OUTPUT_BASE64) == 0);
    
    free(bytes);
    free(encoded);
    
    printf("  ✅ Base64 encode for hex string test passed\n");
}

// Test Base64 encode for bytes
static void test_base64_encode_for_bytes(void) {
    printf("Testing Base64 encode for bytes...\n");
    
    // Convert hex string to bytes
    size_t hex_len = strlen(INPUT_HEX) / 2;
    uint8_t *bytes = malloc(hex_len);
    assert(bytes != NULL);
    
    size_t decoded_len = 0;
    neoc_error_t err = neoc_hex_decode(INPUT_HEX, bytes, hex_len, &decoded_len);
    assert(err == NEOC_SUCCESS);
    
    // Encode bytes to Base64
    size_t output_size = ((decoded_len + 2) / 3) * 4 + 1;
    char *encoded = malloc(output_size);
    assert(encoded != NULL);
    
    err = neoc_base64_encode(bytes, decoded_len, encoded, output_size);
    assert(err == NEOC_SUCCESS);
    
    // Compare with expected
    assert(strcmp(encoded, OUTPUT_BASE64) == 0);
    
    free(bytes);
    free(encoded);
    
    printf("  ✅ Base64 encode for bytes test passed\n");
}

// Test Base64 decode
static void test_base64_decode(void) {
    printf("Testing Base64 decode...\n");
    
    // Decode from Base64
    size_t input_len = strlen(OUTPUT_BASE64);
    size_t output_size = (input_len * 3) / 4 + 1;
    uint8_t *decoded = malloc(output_size);
    assert(decoded != NULL);
    
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base64_decode(OUTPUT_BASE64, decoded, output_size, &decoded_len);
    assert(err == NEOC_SUCCESS);
    
    // Convert decoded bytes to hex
    char *hex_output = malloc(decoded_len * 2 + 1);
    assert(hex_output != NULL);
    
    err = neoc_hex_encode(decoded, decoded_len, hex_output, decoded_len * 2 + 1, false, false);
    assert(err == NEOC_SUCCESS);
    
    // Compare with expected (case-insensitive)
    assert(strcasecmp(hex_output, INPUT_HEX) == 0);
    
    free(decoded);
    free(hex_output);
    
    printf("  ✅ Base64 decode test passed\n");
}

int main(void) {
    printf("\n=== Base64Tests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_base64_encode_for_string();
    test_base64_encode_for_bytes();
    test_base64_decode();
    
    tearDown();
    
    printf("\n✅ All Base64Tests tests passed!\n\n");
    return 0;
}
