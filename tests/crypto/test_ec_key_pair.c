/**
 * @file test_ec_key_pair.c
 * @brief Unit tests for EC key pair functionality
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/ec_public_key.h"
#include "neoc/crypto/ec_private_key.h"
#include "neoc/utils/hex.h"
#include "neoc/neoc_constants.h"
#include "neoc/neoc_memory.h"

#define ENCODED_POINT "03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816"
#define UNCOMPRESSED_POINT "04b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e1368165f4f7fb1c5862465543c06dd5a2aa414f6583f92a5cc3e1d4259df79bf6839c9"

static void test_new_public_key_from_point(void) {
    printf("Testing new public key from point...\n");
    
    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t result = neoc_ec_public_key_from_hex(&public_key, ENCODED_POINT);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    char hex_buffer[256];
    result = neoc_ec_public_key_to_hex(public_key, hex_buffer, sizeof(hex_buffer), true);
    assert(result == NEOC_SUCCESS);
    assert(strcmp(hex_buffer, ENCODED_POINT) == 0);
    
    neoc_ec_public_key_free(public_key);
    printf("✓ test_new_public_key_from_point passed\n");
}

static void test_new_public_key_from_uncompressed_point(void) {
    printf("Testing new public key from uncompressed point...\n");
    
    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t result = neoc_ec_public_key_from_hex(&public_key, UNCOMPRESSED_POINT);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    char hex_buffer[256];
    result = neoc_ec_public_key_to_hex(public_key, hex_buffer, sizeof(hex_buffer), true);
    assert(result == NEOC_SUCCESS);
    assert(strcmp(hex_buffer, ENCODED_POINT) == 0);
    
    neoc_ec_public_key_free(public_key);
    printf("✓ test_new_public_key_from_uncompressed_point passed\n");
}

static void test_new_public_key_from_string_with_invalid_size(void) {
    printf("Testing new public key from string with invalid size...\n");
    
    // Create string that's too small
    char too_small[64];
    strncpy(too_small, ENCODED_POINT, sizeof(too_small) - 1);
    too_small[strlen(ENCODED_POINT) - 2] = '\0';  // Remove last 2 chars
    
    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t result = neoc_ec_public_key_from_hex(&public_key, too_small);
    assert(result != NEOC_SUCCESS);
    assert(public_key == NULL);
    
    printf("✓ test_new_public_key_from_string_with_invalid_size passed\n");
}

static void test_new_public_key_from_point_with_hex_prefix(void) {
    printf("Testing new public key from point with hex prefix...\n");
    
    const char *prefixed = "0x03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816";
    
    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t result = neoc_ec_public_key_from_hex(&public_key, prefixed);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    char hex_buffer[256];
    result = neoc_ec_public_key_to_hex(public_key, hex_buffer, sizeof(hex_buffer), true);
    assert(result == NEOC_SUCCESS);
    assert(strcmp(hex_buffer, ENCODED_POINT) == 0);
    
    neoc_ec_public_key_free(public_key);
    printf("✓ test_new_public_key_from_point_with_hex_prefix passed\n");
}

static void test_serialize_public_key(void) {
    printf("Testing serialize public key...\n");
    
    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t result = neoc_ec_public_key_from_hex(&public_key, ENCODED_POINT);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    uint8_t buffer[NEOC_PUBLIC_KEY_SIZE_COMPRESSED];
    size_t size;
    result = neoc_ec_public_key_to_bytes(public_key, buffer, sizeof(buffer), &size, true);
    assert(result == NEOC_SUCCESS);
    assert(size == NEOC_PUBLIC_KEY_SIZE_COMPRESSED);
    
    // Convert to hex and compare
    char hex_buffer[256];
    neoc_hex_encode(buffer, size, hex_buffer, sizeof(hex_buffer));
    assert(strcmp(hex_buffer, ENCODED_POINT) == 0);
    
    neoc_ec_public_key_free(public_key);
    printf("✓ test_serialize_public_key passed\n");
}

static void test_deserialize_public_key(void) {
    printf("Testing deserialize public key...\n");
    
    const char *hex_data = "036b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296";
    uint8_t data[NEOC_PUBLIC_KEY_SIZE_COMPRESSED];
    size_t data_len = sizeof(data);
    
    neoc_error_t result = neoc_hex_decode(hex_data, strlen(hex_data), data, &data_len);
    assert(result == NEOC_SUCCESS);
    
    neoc_ec_public_key_t *public_key = NULL;
    result = neoc_ec_public_key_from_bytes(&public_key, data, data_len);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    // Verify it's a valid public key
    bool is_valid;
    result = neoc_ec_public_key_is_valid(public_key, &is_valid);
    assert(result == NEOC_SUCCESS);
    assert(is_valid);
    
    neoc_ec_public_key_free(public_key);
    printf("✓ test_deserialize_public_key passed\n");
}

static void test_public_key_size(void) {
    printf("Testing public key size...\n");
    
    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t result = neoc_ec_public_key_from_hex(&public_key, ENCODED_POINT);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    size_t compressed_size, uncompressed_size;
    result = neoc_ec_public_key_get_size(public_key, &compressed_size, true);
    assert(result == NEOC_SUCCESS);
    assert(compressed_size == NEOC_PUBLIC_KEY_SIZE_COMPRESSED);
    
    result = neoc_ec_public_key_get_size(public_key, &uncompressed_size, false);
    assert(result == NEOC_SUCCESS);
    assert(uncompressed_size == NEOC_PUBLIC_KEY_SIZE_UNCOMPRESSED);
    
    neoc_ec_public_key_free(public_key);
    printf("✓ test_public_key_size passed\n");
}

static void test_public_key_equality(void) {
    printf("Testing public key equality...\n");
    
    neoc_ec_public_key_t *key1 = NULL;
    neoc_ec_public_key_t *key2 = NULL;
    neoc_ec_public_key_t *key3 = NULL;
    
    neoc_error_t result = neoc_ec_public_key_from_hex(&key1, ENCODED_POINT);
    assert(result == NEOC_SUCCESS);
    
    result = neoc_ec_public_key_from_hex(&key2, ENCODED_POINT);
    assert(result == NEOC_SUCCESS);
    
    // Different key
    result = neoc_ec_public_key_from_hex(&key3, 
        "02b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816");
    assert(result == NEOC_SUCCESS);
    
    bool is_equal;
    result = neoc_ec_public_key_equals(key1, key2, &is_equal);
    assert(result == NEOC_SUCCESS);
    assert(is_equal);
    
    result = neoc_ec_public_key_equals(key1, key3, &is_equal);
    assert(result == NEOC_SUCCESS);
    assert(!is_equal);
    
    neoc_ec_public_key_free(key1);
    neoc_ec_public_key_free(key2);
    neoc_ec_public_key_free(key3);
    printf("✓ test_public_key_equality passed\n");
}

static void test_key_pair_generation(void) {
    printf("Testing key pair generation...\n");
    
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t result = neoc_ec_key_pair_create_random(&key_pair);
    assert(result == NEOC_SUCCESS);
    assert(key_pair != NULL);
    
    // Get public and private keys
    neoc_ec_public_key_t *public_key = NULL;
    neoc_ec_private_key_t *private_key = NULL;
    
    result = neoc_ec_key_pair_get_public_key(key_pair, &public_key);
    assert(result == NEOC_SUCCESS);
    assert(public_key != NULL);
    
    result = neoc_ec_key_pair_get_private_key(key_pair, &private_key);
    assert(result == NEOC_SUCCESS);
    assert(private_key != NULL);
    
    // Verify keys are valid
    bool is_valid;
    result = neoc_ec_public_key_is_valid(public_key, &is_valid);
    assert(result == NEOC_SUCCESS);
    assert(is_valid);
    
    neoc_ec_key_pair_free(key_pair);
    printf("✓ test_key_pair_generation passed\n");
}

static void test_key_pair_from_private_key(void) {
    printf("Testing key pair from private key...\n");
    
    const char *private_key_hex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
    uint8_t private_key_bytes[32];
    size_t key_len = sizeof(private_key_bytes);
    
    neoc_error_t result = neoc_hex_decode(private_key_hex, strlen(private_key_hex), 
                                           private_key_bytes, &key_len);
    assert(result == NEOC_SUCCESS);
    
    neoc_ec_key_pair_t *key_pair = NULL;
    result = neoc_ec_key_pair_from_private_key(&key_pair, private_key_bytes, key_len);
    assert(result == NEOC_SUCCESS);
    assert(key_pair != NULL);
    
    // Verify we can get the same private key back
    neoc_ec_private_key_t *private_key = NULL;
    result = neoc_ec_key_pair_get_private_key(key_pair, &private_key);
    assert(result == NEOC_SUCCESS);
    
    uint8_t retrieved_bytes[32];
    size_t retrieved_len;
    result = neoc_ec_private_key_to_bytes(private_key, retrieved_bytes, 
                                           sizeof(retrieved_bytes), &retrieved_len);
    assert(result == NEOC_SUCCESS);
    assert(retrieved_len == 32);
    assert(memcmp(private_key_bytes, retrieved_bytes, 32) == 0);
    
    neoc_ec_key_pair_free(key_pair);
    printf("✓ test_key_pair_from_private_key passed\n");
}

int main(void) {
    printf("\n=== EC Key Pair Tests ===\n\n");
    
    // Initialize NeoC library
    neoc_error_t result = neoc_init();
    if (result != NEOC_SUCCESS) {
        printf("Failed to initialize NeoC library: %d\n", result);
        return 1;
    }
    
    // Run tests
    test_new_public_key_from_point();
    test_new_public_key_from_uncompressed_point();
    test_new_public_key_from_string_with_invalid_size();
    test_new_public_key_from_point_with_hex_prefix();
    test_serialize_public_key();
    test_deserialize_public_key();
    test_public_key_size();
    test_public_key_equality();
    test_key_pair_generation();
    test_key_pair_from_private_key();
    
    // Cleanup
    neoc_cleanup();
    
    printf("\n✅ All EC Key Pair tests passed!\n\n");
    return 0;
}
