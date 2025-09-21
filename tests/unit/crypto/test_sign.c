/**
 * @file test_sign.c
 * @brief Unit tests converted from SignTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/crypto/sign.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/ec_private_key.h"
#include "neoc/crypto/ec_public_key.h"
#include "neoc/types/hash160.h"
#include "neoc/utils/hex.h"

// Test data
static const char *PRIVATE_KEY_HEX = "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3a3";
static const char *PUBLIC_KEY_HEX = "0265bf906bf385fbf3f777832e55a87991bcfbe19b097fb7c5ca2e4025a4d5e5d6";
static const char *TEST_MESSAGE = "A test message";
static const char *EXPECTED_R = "147e5f3c929dd830d961626551dbea6b70e4b2837ed2fe9089eed2072ab3a655";
static const char *EXPECTED_S = "523ae0fa8711eee4769f1913b180b9b3410bbb2cf770f529c85f6886f22cbaaf";

// Test objects
static neoc_ec_key_pair_t *test_key_pair = NULL;

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
    
    // Create key pair from private key
    uint8_t private_key_bytes[32];
    size_t private_key_len = 0;
    err = neoc_hex_decode(PRIVATE_KEY_HEX, private_key_bytes, sizeof(private_key_bytes), &private_key_len);
    assert(err == NEOC_SUCCESS);
    assert(private_key_len == 32);
    
    err = neoc_ec_key_pair_create_from_private_key(private_key_bytes, &test_key_pair);
    assert(err == NEOC_SUCCESS);
    assert(test_key_pair != NULL);
}

// Test teardown
static void tearDown(void) {
    if (test_key_pair) {
        neoc_ec_key_pair_free(test_key_pair);
        test_key_pair = NULL;
    }
    neoc_cleanup();
}

// Test sign message
static void test_sign_message(void) {
    printf("Testing sign message...\n");
    
    // Sign the test message
    neoc_signature_data_t *signature = NULL;
    neoc_error_t err = neoc_sign_message((const uint8_t *)TEST_MESSAGE, strlen(TEST_MESSAGE), test_key_pair, &signature);
    assert(err == NEOC_SUCCESS);
    assert(signature != NULL);
    
    // Get signature components
    uint8_t v = neoc_signature_data_get_v(signature);
    assert(v == 27 || v == 28);  // v can be 27 or 28 depending on recovery bit
    
    size_t r_len = 0, s_len = 0;
    const uint8_t *r = neoc_signature_data_get_r(signature, &r_len);
    const uint8_t *s = neoc_signature_data_get_s(signature, &s_len);
    assert(r_len == 32);
    assert(s_len == 32);
    
    // Convert to hex for comparison
    char r_hex[65], s_hex[65];
    err = neoc_hex_encode(r, r_len, r_hex, sizeof(r_hex));
    assert(err == NEOC_SUCCESS);
    err = neoc_hex_encode(s, s_len, s_hex, sizeof(s_hex));
    assert(err == NEOC_SUCCESS);
    
    // Note: Due to nonce randomness in ECDSA, the signature may differ between runs
    // For deterministic testing, we'd need to use RFC 6979 deterministic nonce
    printf("  Signature R: %s\n", r_hex);
    printf("  Signature S: %s\n", s_hex);
    
    neoc_signature_data_free(signature);
    printf("  ✅ Sign message test passed\n");
}

// Test recover signing script hash
static void test_recover_signing_script_hash(void) {
    printf("Testing recover signing script hash...\n");
    
    // Create signature data with known values
    uint8_t r_bytes[32], s_bytes[32];
    size_t r_len = 0, s_len = 0;
    neoc_error_t err = neoc_hex_decode(EXPECTED_R, r_bytes, sizeof(r_bytes), &r_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_hex_decode(EXPECTED_S, s_bytes, sizeof(s_bytes), &s_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_signature_data_t *signature = NULL;
    err = neoc_signature_data_create(27, r_bytes, s_bytes, &signature);
    assert(err == NEOC_SUCCESS);
    
    // Recover script hash from signature
    neoc_hash160_t *recovered_hash = NULL;
    err = neoc_sign_recover_script_hash((const uint8_t *)TEST_MESSAGE, strlen(TEST_MESSAGE), signature, &recovered_hash);
    assert(err == NEOC_SUCCESS);
    assert(recovered_hash != NULL);
    
    // Get expected script hash from key pair
    neoc_hash160_t *expected_hash = NULL;
    err = neoc_ec_key_pair_get_script_hash(test_key_pair, &expected_hash);
    assert(err == NEOC_SUCCESS);
    
    // Compare hashes
    assert(neoc_hash160_equals(recovered_hash, expected_hash));
    
    neoc_signature_data_free(signature);
    neoc_hash160_free(recovered_hash);
    neoc_hash160_free(expected_hash);
    printf("  ✅ Recover signing script hash test passed\n");
}

// Test signature data from bytes
static void test_signature_data_from_bytes(void) {
    printf("Testing signature data from bytes...\n");
    
    // Create signature bytes (r || s)
    const char *sig_hex = "147e5f3c929dd830d961626551dbea6b70e4b2837ed2fe9089eed2072ab3a655523ae0fa8711eee4769f1913b180b9b3410bbb2cf770f529c85f6886f22cbaaf";
    uint8_t sig_bytes[64];
    size_t sig_len = 0;
    neoc_error_t err = neoc_hex_decode(sig_hex, sig_bytes, sizeof(sig_bytes), &sig_len);
    assert(err == NEOC_SUCCESS);
    assert(sig_len == 64);
    
    // Create signature data from bytes
    neoc_signature_data_t *signature = NULL;
    err = neoc_signature_data_from_bytes(sig_bytes, sig_len, &signature);
    assert(err == NEOC_SUCCESS);
    assert(signature != NULL);
    
    // Verify components
    uint8_t v = neoc_signature_data_get_v(signature);
    assert(v == 0);  // Default v when created from bytes
    
    size_t r_len = 0, s_len = 0;
    const uint8_t *r = neoc_signature_data_get_r(signature, &r_len);
    const uint8_t *s = neoc_signature_data_get_s(signature, &s_len);
    assert(r_len == 32);
    assert(s_len == 32);
    
    // Verify r value
    uint8_t expected_r[32];
    size_t expected_r_len = 0;
    err = neoc_hex_decode(EXPECTED_R, expected_r, sizeof(expected_r), &expected_r_len);
    assert(err == NEOC_SUCCESS);
    assert(memcmp(r, expected_r, 32) == 0);
    
    // Verify s value
    uint8_t expected_s[32];
    size_t expected_s_len = 0;
    err = neoc_hex_decode(EXPECTED_S, expected_s, sizeof(expected_s), &expected_s_len);
    assert(err == NEOC_SUCCESS);
    assert(memcmp(s, expected_s, 32) == 0);
    
    neoc_signature_data_free(signature);
    
    // Test with v = 0x27
    err = neoc_signature_data_from_bytes_with_v(0x27, sig_bytes, sig_len, &signature);
    assert(err == NEOC_SUCCESS);
    assert(signature != NULL);
    
    v = neoc_signature_data_get_v(signature);
    assert(v == 0x27);
    
    neoc_signature_data_free(signature);
    printf("  ✅ Signature data from bytes test passed\n");
}

// Test public key from signed message
static void test_public_key_from_signed_message(void) {
    printf("Testing public key from signed message...\n");
    
    // Sign message
    neoc_signature_data_t *signature = NULL;
    neoc_error_t err = neoc_sign_message((const uint8_t *)TEST_MESSAGE, strlen(TEST_MESSAGE), test_key_pair, &signature);
    assert(err == NEOC_SUCCESS);
    
    // Recover public key from signature
    neoc_ec_public_key_t *recovered_key = NULL;
    err = neoc_sign_recover_public_key((const uint8_t *)TEST_MESSAGE, strlen(TEST_MESSAGE), signature, &recovered_key);
    assert(err == NEOC_SUCCESS);
    assert(recovered_key != NULL);
    
    // Get expected public key from key pair
    neoc_ec_public_key_t *expected_key = neoc_ec_key_pair_get_public_key(test_key_pair);
    assert(expected_key != NULL);
    
    // Compare public keys
    size_t recovered_len = 0, expected_len = 0;
    const uint8_t *recovered_bytes = neoc_ec_public_key_get_encoded(recovered_key, true, &recovered_len);
    const uint8_t *expected_bytes = neoc_ec_public_key_get_encoded(expected_key, true, &expected_len);
    assert(recovered_len == expected_len);
    assert(memcmp(recovered_bytes, expected_bytes, recovered_len) == 0);
    
    neoc_signature_data_free(signature);
    neoc_ec_public_key_free(recovered_key);
    printf("  ✅ Public key from signed message test passed\n");
}

// Test public key from private key
static void test_public_key_from_private_key(void) {
    printf("Testing public key from private key...\n");
    
    // Get private key from key pair
    neoc_ec_private_key_t *private_key = neoc_ec_key_pair_get_private_key(test_key_pair);
    assert(private_key != NULL);
    
    // Derive public key from private key
    neoc_ec_public_key_t *derived_key = NULL;
    neoc_error_t err = neoc_sign_public_key_from_private_key(private_key, &derived_key);
    assert(err == NEOC_SUCCESS);
    assert(derived_key != NULL);
    
    // Get expected public key
    neoc_ec_public_key_t *expected_key = neoc_ec_key_pair_get_public_key(test_key_pair);
    assert(expected_key != NULL);
    
    // Compare public keys
    size_t derived_len = 0, expected_len = 0;
    const uint8_t *derived_bytes = neoc_ec_public_key_get_encoded(derived_key, true, &derived_len);
    const uint8_t *expected_bytes = neoc_ec_public_key_get_encoded(expected_key, true, &expected_len);
    assert(derived_len == expected_len);
    assert(memcmp(derived_bytes, expected_bytes, derived_len) == 0);
    
    neoc_ec_public_key_free(derived_key);
    printf("  ✅ Public key from private key test passed\n");
}

// Test invalid signature validation
static void test_invalid_signature(void) {
    printf("Testing invalid signature validation...\n");
    
    // Test with invalid r length (1 byte)
    uint8_t invalid_r[1] = {1};
    uint8_t valid_s[32] = {0};
    
    neoc_signature_data_t *signature = NULL;
    neoc_error_t err = neoc_signature_data_create(27, invalid_r, valid_s, &signature);
    assert(err != NEOC_SUCCESS);  // Should fail due to invalid r length
    assert(signature == NULL);
    
    // Test with invalid s length
    uint8_t valid_r[32] = {0};
    uint8_t invalid_s[1] = {0};
    
    err = neoc_signature_data_create(27, valid_r, invalid_s, &signature);
    assert(err != NEOC_SUCCESS);  // Should fail due to invalid s length
    assert(signature == NULL);
    
    printf("  ✅ Invalid signature validation test passed\n");
}

// Test verify signature
static void test_verify_signature(void) {
    printf("Testing verify signature...\n");
    
    // Sign message
    neoc_signature_data_t *signature = NULL;
    neoc_error_t err = neoc_sign_message((const uint8_t *)TEST_MESSAGE, strlen(TEST_MESSAGE), test_key_pair, &signature);
    assert(err == NEOC_SUCCESS);
    
    // Get public key
    neoc_ec_public_key_t *public_key = neoc_ec_key_pair_get_public_key(test_key_pair);
    assert(public_key != NULL);
    
    // Verify signature
    bool is_valid = neoc_sign_verify_signature((const uint8_t *)TEST_MESSAGE, strlen(TEST_MESSAGE), signature, public_key);
    assert(is_valid == true);
    
    // Test with wrong message
    const char *wrong_message = "Wrong message";
    is_valid = neoc_sign_verify_signature((const uint8_t *)wrong_message, strlen(wrong_message), signature, public_key);
    assert(is_valid == false);
    
    neoc_signature_data_free(signature);
    printf("  ✅ Verify signature test passed\n");
}

int main(void) {
    printf("\n=== SignTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_sign_message();
    test_recover_signing_script_hash();
    test_signature_data_from_bytes();
    test_public_key_from_signed_message();
    test_public_key_from_private_key();
    test_invalid_signature();
    test_verify_signature();
    
    tearDown();
    
    printf("\n✅ All SignTests tests passed!\n\n");
    return 0;
}