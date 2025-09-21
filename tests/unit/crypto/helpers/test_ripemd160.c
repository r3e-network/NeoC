/**
 * @file test_ripemd160.c
 * @brief Unit tests converted from RIPEMD160Tests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/crypto/helpers/ripemd160.h"
#include "neoc/utils/hex.h"

// Test vectors from https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
typedef struct {
    const char *message;
    const char *expected_hash;
} test_vector_t;

static const test_vector_t test_vectors[] = {
    {"", "9c1185a5c5e9fc54612808977ee8f548b2258d31"},
    {"a", "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe"},
    {"abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"},
    {"message digest", "5d0689ef49d2fae572b881b123a85ffa21595f36"},
    {"abcdefghijklmnopqrstuvwxyz", "f71c27109c692c1b56bbdceb5b9d2865b3708dbc"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "12a053384a9c0c88e405a06c27dcf49ada62eb2b"},
    {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "b0e20b6e3116640286ed3a87a5713079b21f5189"},
    {NULL, NULL}  // Special handling for repeated patterns
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

// Helper function to create repeated string
static char* create_repeated_string(const char *pattern, int count) {
    size_t pattern_len = strlen(pattern);
    size_t total_len = pattern_len * count;
    char *result = malloc(total_len + 1);
    assert(result != NULL);
    
    for (int i = 0; i < count; i++) {
        memcpy(result + (i * pattern_len), pattern, pattern_len);
    }
    result[total_len] = '\0';
    return result;
}

// Test RIPEMD160 with test vectors
static void test_ripemd160_vectors(void) {
    printf("Testing RIPEMD160 with test vectors...\n");
    
    // Test standard vectors
    for (int i = 0; test_vectors[i].message != NULL; i++) {
        const char *message = test_vectors[i].message;
        const char *expected = test_vectors[i].expected_hash;
        
        // Calculate RIPEMD160 hash
        uint8_t hash[20];
        neoc_error_t err = neoc_ripemd160((const uint8_t *)message, strlen(message), hash);
        assert(err == NEOC_SUCCESS);
        
        // Convert to hex
        char hash_hex[41];
        err = neoc_hex_encode(hash, 20, hash_hex, sizeof(hash_hex));
        assert(err == NEOC_SUCCESS);
        
        // Compare
        assert(strcmp(hash_hex, expected) == 0);
        printf("  Test vector %d passed\n", i + 1);
    }
    
    // Test "1234567890" repeated 8 times
    char *repeated = create_repeated_string("1234567890", 8);
    uint8_t hash[20];
    neoc_error_t err = neoc_ripemd160((const uint8_t *)repeated, strlen(repeated), hash);
    assert(err == NEOC_SUCCESS);
    free(repeated);
    
    char hash_hex[41];
    err = neoc_hex_encode(hash, 20, hash_hex, sizeof(hash_hex));
    assert(err == NEOC_SUCCESS);
    assert(strcmp(hash_hex, "9b752e45573d4b39f4dbd3323cab82bf63326bfb") == 0);
    printf("  Test vector 8 (repeated) passed\n");
    
    // Test "a" repeated 1,000,000 times
    char *million_a = create_repeated_string("a", 1000000);
    err = neoc_ripemd160((const uint8_t *)million_a, strlen(million_a), hash);
    assert(err == NEOC_SUCCESS);
    free(million_a);
    
    err = neoc_hex_encode(hash, 20, hash_hex, sizeof(hash_hex));
    assert(err == NEOC_SUCCESS);
    assert(strcmp(hash_hex, "52783243c1697bdbe16d37f97f68f08325dc1528") == 0);
    printf("  Test vector 9 (million a's) passed\n");
    
    printf("  ✅ RIPEMD160 test vectors passed\n");
}

// Test empty input
static void test_ripemd160_empty(void) {
    printf("Testing RIPEMD160 with empty input...\n");
    
    uint8_t hash[20];
    neoc_error_t err = neoc_ripemd160(NULL, 0, hash);
    assert(err == NEOC_SUCCESS);
    
    char hash_hex[41];
    err = neoc_hex_encode(hash, 20, hash_hex, sizeof(hash_hex));
    assert(err == NEOC_SUCCESS);
    
    assert(strcmp(hash_hex, "9c1185a5c5e9fc54612808977ee8f548b2258d31") == 0);
    
    printf("  ✅ RIPEMD160 empty input test passed\n");
}

// Test single byte input
static void test_ripemd160_single_byte(void) {
    printf("Testing RIPEMD160 with single byte...\n");
    
    uint8_t data = 0x61; // 'a'
    uint8_t hash[20];
    neoc_error_t err = neoc_ripemd160(&data, 1, hash);
    assert(err == NEOC_SUCCESS);
    
    char hash_hex[41];
    err = neoc_hex_encode(hash, 20, hash_hex, sizeof(hash_hex));
    assert(err == NEOC_SUCCESS);
    
    assert(strcmp(hash_hex, "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe") == 0);
    
    printf("  ✅ RIPEMD160 single byte test passed\n");
}

// Test binary data
static void test_ripemd160_binary(void) {
    printf("Testing RIPEMD160 with binary data...\n");
    
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t hash[20];
    neoc_error_t err = neoc_ripemd160(data, sizeof(data), hash);
    assert(err == NEOC_SUCCESS);
    
    // Verify hash was computed (check it's not all zeros)
    int all_zeros = 1;
    for (int i = 0; i < 20; i++) {
        if (hash[i] != 0) {
            all_zeros = 0;
            break;
        }
    }
    assert(!all_zeros);
    
    printf("  ✅ RIPEMD160 binary data test passed\n");
}

int main(void) {
    printf("\n=== RIPEMD160Tests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_ripemd160_vectors();
    test_ripemd160_empty();
    test_ripemd160_single_byte();
    test_ripemd160_binary();
    
    tearDown();
    
    printf("\n✅ All RIPEMD160Tests tests passed!\n\n");
    return 0;
}