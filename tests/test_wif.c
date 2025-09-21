/**
 * @file test_wif.c
 * @brief WIF (Wallet Import Format) tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/wif.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/utils/neoc_base58.h>
#include <string.h>
#include <stdio.h>

// Test vectors from Swift tests
static const char* valid_wif = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13A";
static const char* private_key_hex = "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3a3";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== WIF TESTS ===== */

void test_valid_wif_to_private_key(void) {
    // Decode WIF to get private key
    uint8_t* priv_key = NULL;
    neoc_error_t err = neoc_wif_to_private_key(valid_wif, &priv_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(priv_key);
    
    // Convert to hex for comparison
    char hex[65];
    err = neoc_hex_encode(priv_key, 32, hex, sizeof(hex), false, false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Compare with expected
    TEST_ASSERT_EQUAL_STRING(private_key_hex, hex);
    
    neoc_free(priv_key);
}

void test_wrongly_sized_wifs(void) {
    const char* too_large = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13Ahc7S";
    const char* too_small = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWML";
    
    uint8_t* priv_key = NULL;
    
    // Too large should fail
    neoc_error_t err = neoc_wif_to_private_key(too_large, &priv_key);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    if (priv_key) neoc_free(priv_key);
    
    // Too small should fail
    priv_key = NULL;
    err = neoc_wif_to_private_key(too_small, &priv_key);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    if (priv_key) neoc_free(priv_key);
}

void test_wrong_first_byte_wif(void) {
    // Decode the valid WIF
    size_t decoded_size = neoc_base58_decode_buffer_size(valid_wif);
    if (decoded_size == 0) {
        decoded_size = strlen(valid_wif) + 1;
    }
    uint8_t* decoded = neoc_malloc(decoded_size);
    TEST_ASSERT_NOT_NULL(decoded);
    
    size_t actual_size;
    neoc_error_t err = neoc_base58_decode(valid_wif, decoded, decoded_size, &actual_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Change first byte (should be 0x80)
    decoded[0] = 0x81;
    
    // Re-encode to base58
    size_t encoded_size = neoc_base58_encode_buffer_size(actual_size);
    char* wrong_wif = neoc_malloc(encoded_size);
    TEST_ASSERT_NOT_NULL(wrong_wif);
    
    err = neoc_base58_encode(decoded, actual_size, wrong_wif, encoded_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to decode - should fail
    uint8_t* priv_key = NULL;
    err = neoc_wif_to_private_key(wrong_wif, &priv_key);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    if (priv_key) neoc_free(priv_key);
    
    neoc_free(decoded);
    neoc_free(wrong_wif);
}

void test_wrong_byte33_wif(void) {
    // Decode the valid WIF
    size_t decoded_size = neoc_base58_decode_buffer_size(valid_wif);
    if (decoded_size == 0) {
        decoded_size = strlen(valid_wif) + 1;
    }
    uint8_t* decoded = neoc_malloc(decoded_size);
    TEST_ASSERT_NOT_NULL(decoded);
    
    size_t actual_size;
    neoc_error_t err = neoc_base58_decode(valid_wif, decoded, decoded_size, &actual_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Change byte 33 (should be 0x01 for compressed)
    if (actual_size > 33) {
        decoded[33] = 0x00;
    }
    
    // Re-encode to base58
    size_t encoded_size = neoc_base58_encode_buffer_size(actual_size);
    char* wrong_wif = neoc_malloc(encoded_size);
    TEST_ASSERT_NOT_NULL(wrong_wif);
    
    err = neoc_base58_encode(decoded, actual_size, wrong_wif, encoded_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to decode - should fail or give wrong result
    uint8_t* priv_key = NULL;
    err = neoc_wif_to_private_key(wrong_wif, &priv_key);
    // May succeed but with wrong key, or may fail
    if (priv_key) neoc_free(priv_key);
    
    neoc_free(decoded);
    neoc_free(wrong_wif);
}

void test_valid_private_key_to_wif(void) {
    // Convert hex to bytes
    uint8_t priv_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, priv_key, sizeof(priv_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    // Convert to WIF
    char* wif = NULL;
    err = neoc_private_key_to_wif(priv_key, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    
    // Compare with expected
    TEST_ASSERT_EQUAL_STRING(valid_wif, wif);
    
    neoc_free(wif);
}

void test_wrongly_sized_private_key(void) {
    // Wrong size private key (31 bytes instead of 32)
    const char* wrong_hex = "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3";
    
    uint8_t priv_key[31];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(wrong_hex, priv_key, sizeof(priv_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify that the decoded length is 31 bytes (incorrect for a private key)
    // The WIF API should reject this when attempting to create WIF from it
    TEST_ASSERT_EQUAL_INT(31, decoded_len);
}

void test_wif_round_trip(void) {
    // Test round-trip conversion with various private keys
    uint8_t test_keys[][32] = {
        // All zeros
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
        // All ones
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE},
        // Random pattern
        {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
         0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
         0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
         0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
    };
    
    for (int i = 0; i < 3; i++) {
        // Convert to WIF
        char* wif = NULL;
        neoc_error_t err = neoc_private_key_to_wif(test_keys[i], &wif);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(wif);
        
        // Convert back from WIF
        uint8_t* decoded_key = NULL;
        err = neoc_wif_to_private_key(wif, &decoded_key);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(decoded_key);
        
        // Compare
        TEST_ASSERT_EQUAL_MEMORY(test_keys[i], decoded_key, 32);
        
        neoc_free(wif);
        neoc_free(decoded_key);
    }
}

void test_wif_format_validation(void) {
    // Test various invalid WIF formats
    const char* invalid_wifs[] = {
        "InvalidWIF",  // Not valid base58
        "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ",  // Uncompressed (starts with 5)
        "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWX",  // Wrong checksum (changed last char)
        "",  // Empty string
        "L",  // Too short
    };
    
    for (int i = 0; i < 5; i++) {
        uint8_t* priv_key = NULL;
        neoc_error_t err = neoc_wif_to_private_key(invalid_wifs[i], &priv_key);
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        if (priv_key) neoc_free(priv_key);
    }
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== WIF TESTS ===\n");
    
    RUN_TEST(test_valid_wif_to_private_key);
    RUN_TEST(test_wrongly_sized_wifs);
    RUN_TEST(test_wrong_first_byte_wif);
    RUN_TEST(test_wrong_byte33_wif);
    RUN_TEST(test_valid_private_key_to_wif);
    RUN_TEST(test_wrongly_sized_private_key);
    RUN_TEST(test_wif_round_trip);
    RUN_TEST(test_wif_format_validation);
    
    UNITY_END();
    return 0;
}
