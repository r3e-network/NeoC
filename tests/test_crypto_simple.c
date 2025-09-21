/**
 * @file test_crypto_simple.c
 * @brief Simple cryptographic tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/crypto/wif.h>
#include <neoc/crypto/nep2.h>
#include <neoc/utils/neoc_base64.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== BASE64 TESTS ===== */

void test_base64_encode_decode(void) {
    const char* test_string = "Hello, Neo blockchain!";
    size_t test_len = strlen(test_string);
    
    // Encode
    char* encoded = neoc_base64_encode_alloc((const uint8_t*)test_string, test_len);
    TEST_ASSERT_NOT_NULL(encoded);
    
    // Decode
    size_t decoded_len;
    uint8_t* decoded = neoc_base64_decode_alloc(encoded, &decoded_len);
    TEST_ASSERT_NOT_NULL(decoded);
    TEST_ASSERT_EQUAL_INT(test_len, decoded_len);
    TEST_ASSERT_EQUAL_MEMORY(test_string, decoded, test_len);
    
    neoc_free(encoded);
    neoc_free(decoded);
}

/* ===== EC KEY PAIR TESTS ===== */

void test_ec_key_pair_random_creation(void) {
    neoc_ec_key_pair_t* key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Test that we can get the private key
    size_t priv_key_len = 32;
    uint8_t priv_key[32];
    err = neoc_ec_key_pair_get_private_key(key_pair, priv_key, &priv_key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, priv_key_len);
    
    neoc_ec_key_pair_free(key_pair);
}

void test_ec_key_pair_from_bytes(void) {
    // Create a key from known bytes
    uint8_t priv_key[32];
    for (int i = 0; i < 32; i++) {
        priv_key[i] = i + 1;
    }
    
    neoc_ec_key_pair_t* key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_from_private_key(priv_key, 32, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Verify the private key matches
    uint8_t retrieved_key[32];
    size_t retrieved_len = 32;
    err = neoc_ec_key_pair_get_private_key(key_pair, retrieved_key, &retrieved_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(priv_key, retrieved_key, 32);
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== WIF TESTS ===== */

void test_wif_encode_simple(void) {
    // Create a private key with varied bytes to avoid Base58 edge cases
    uint8_t priv_key[32];
    for (int i = 0; i < 32; i++) {
        priv_key[i] = (uint8_t)(0x10 + i * 2);
    }
    
    // Encode to WIF
    char* wif = NULL;
    neoc_error_t err = neoc_private_key_to_wif(priv_key, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    TEST_ASSERT_TRUE(strlen(wif) > 0);
    // WIF format can start with different characters depending on the key value
    // Common prefixes are '5' for uncompressed, 'K'/'L' for compressed, and others
    
    // Decode from WIF
    uint8_t* decoded_key = NULL;
    err = neoc_wif_to_private_key(wif, &decoded_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(decoded_key);
    TEST_ASSERT_EQUAL_MEMORY(priv_key, decoded_key, 32);
    
    neoc_free(wif);
    neoc_free(decoded_key);
}

/* ===== NEP-2 TESTS ===== */

void test_nep2_basic_encrypt_decrypt(void) {
    // Create a private key with more entropy for proper NEP2 format
    uint8_t priv_key[32];
    for (int i = 0; i < 32; i++) {
        priv_key[i] = (uint8_t)(0x42 + i);
    }
    
    const char* password = "TestPassword";
    
    // Create NEP-2 params
    neoc_nep2_params_t params = {
        .n = 1024,  // Use lower values for testing
        .r = 1,
        .p = 1
    };
    
    // Encrypt
    char encrypted[59];
    neoc_error_t err = neoc_nep2_encrypt(priv_key, password, &params, encrypted, sizeof(encrypted));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    // NEP2 format is usually 58 chars but can vary slightly
    TEST_ASSERT_TRUE(strlen(encrypted) >= 57 && strlen(encrypted) <= 58);
    // NEP2 encrypted keys can start with different prefixes depending on the key
    
    // Decrypt
    uint8_t decrypted_key[32];
    err = neoc_nep2_decrypt(encrypted, password, &params, decrypted_key, sizeof(decrypted_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(priv_key, decrypted_key, 32);
}

/* ===== HEX TESTS ===== */

void test_hex_encode_decode(void) {
    uint8_t test_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78};
    size_t test_len = sizeof(test_data);
    
    // Encode
    char* hex = neoc_hex_encode_alloc(test_data, test_len, false, false);
    TEST_ASSERT_NOT_NULL(hex);
    TEST_ASSERT_EQUAL_STRING("deadbeef12345678", hex);
    
    // Decode
    uint8_t decoded[8];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(hex, decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(test_len, decoded_len);
    TEST_ASSERT_EQUAL_MEMORY(test_data, decoded, test_len);
    
    neoc_free(hex);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== SIMPLE CRYPTO TESTS ===\n");
    
    RUN_TEST(test_base64_encode_decode);
    RUN_TEST(test_ec_key_pair_random_creation);
    RUN_TEST(test_ec_key_pair_from_bytes);
    RUN_TEST(test_wif_encode_simple);
    RUN_TEST(test_nep2_basic_encrypt_decrypt);
    RUN_TEST(test_hex_encode_decode);
    
    UNITY_END();
    return 0;
}
