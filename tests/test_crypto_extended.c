/**
 * @file test_crypto_extended.c
 * @brief Extended cryptographic tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/crypto/ecdsa_signature.h>
#include <neoc/crypto/wif.h>
#include <neoc/crypto/nep2.h>
#include <neoc/crypto/sign.h>
#include <neoc/utils/neoc_base64.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/types/neoc_hash256.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== BASE64 TESTS (from Base64Tests.swift) ===== */

void test_base64_encode_for_string(void) {
    const char* input_hex = "150c14242dbf5e2f6ac2568b59b7822278d571b75f17be0c14242dbf5e2f6ac2568b59b7822278d571b75f17be13c00c087472616e736665720c14897720d8cd76f4f00abfa37c0edd889c208fde9b41627d5b5238";
    const char* expected_output = "FQwUJC2/Xi9qwlaLWbeCInjVcbdfF74MFCQtv14vasJWi1m3giJ41XG3Xxe+E8AMCHRyYW5zZmVyDBSJdyDYzXb08Aq/o3wO3YicII/em0FifVtSOA==";
    
    size_t hex_len = strlen(input_hex) / 2;
    uint8_t* bytes = malloc(hex_len);
    TEST_ASSERT_NOT_NULL(bytes);
    
    size_t decoded_len = hex_len;
    neoc_error_t err = neoc_hex_decode(input_hex, strlen(input_hex), bytes, hex_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    char* encoded = neoc_base64_encode_alloc(bytes, hex_len);
    TEST_ASSERT_NOT_NULL(encoded);
    TEST_ASSERT_EQUAL_STRING(expected_output, encoded);
    
    free(bytes);
    neoc_free(encoded);
}

void test_base64_decode(void) {
    const char* input_base64 = "FQwUJC2/Xi9qwlaLWbeCInjVcbdfF74MFCQtv14vasJWi1m3giJ41XG3Xxe+E8AMCHRyYW5zZmVyDBSJdyDYzXb08Aq/o3wO3YicII/em0FifVtSOA==";
    const char* expected_hex = "150c14242dbf5e2f6ac2568b59b7822278d571b75f17be0c14242dbf5e2f6ac2568b59b7822278d571b75f17be13c00c087472616e736665720c14897720d8cd76f4f00abfa37c0edd889c208fde9b41627d5b5238";
    
    size_t decoded_len;
    uint8_t* decoded = neoc_base64_decode_alloc(input_base64, &decoded_len);
    TEST_ASSERT_NOT_NULL(decoded);
    
    char* hex = neoc_hex_encode_alloc(decoded, decoded_len, false, false);
    TEST_ASSERT_NOT_NULL(hex);
    TEST_ASSERT_EQUAL_STRING(expected_hex, hex);
    
    neoc_free(decoded);
    neoc_free(hex);
}

/* ===== EC KEY PAIR TESTS (from ECKeyPairTests.swift) ===== */

void test_ec_key_pair_creation(void) {
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
    
    // Test that we can get the public key
    uint8_t* pub_key = NULL;
    size_t pub_key_len;
    err = neoc_ec_public_key_get_encoded(neoc_ec_key_pair_get_public_key(key_pair), 
                                          true, &pub_key, &pub_key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, pub_key_len);
    
    neoc_free(pub_key);
    neoc_ec_key_pair_free(key_pair);
}

void test_ec_key_pair_from_private_key(void) {
    // Test vector from Swift tests
    const char* private_key_hex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
    
    size_t priv_key_len = 32;
    uint8_t priv_key[32];
    size_t decoded_len = priv_key_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, strlen(private_key_hex), 
                                        priv_key, priv_key_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key, priv_key_len, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Verify the private key matches
    uint8_t retrieved_priv_key[32];
    size_t retrieved_len = 32;
    err = neoc_ec_key_pair_get_private_key(key_pair, retrieved_priv_key, &retrieved_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(priv_key, retrieved_priv_key, 32);
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== WIF TESTS (from WIFTests.swift) ===== */

void test_wif_encode_decode(void) {
    const char* private_key_hex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
    const char* expected_wif = "KxmqXqdpZ8FSYX4pWMGwEMUVxvLFNcpJnJTvDWMpkXrLWwGiKkFw";
    
    size_t priv_key_len = 32;
    uint8_t priv_key[32];
    size_t decoded_len = priv_key_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, strlen(private_key_hex), 
                                        priv_key, priv_key_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Encode to WIF
    char wif[52];
    err = neoc_wif_encode(priv_key, wif, sizeof(wif));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING(expected_wif, wif);
    
    // Decode from WIF
    uint8_t decoded_key[32];
    err = neoc_wif_decode(wif, decoded_key, sizeof(decoded_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(priv_key, decoded_key, 32);
}

void test_wif_invalid_decode(void) {
    const char* invalid_wif = "InvalidWIFString123";
    uint8_t decoded_key[32];
    
    neoc_error_t err = neoc_wif_decode(invalid_wif, decoded_key, sizeof(decoded_key));
    TEST_ASSERT_NOT_EQUAL(NEOC_SUCCESS, err);
}

/* ===== NEP-2 TESTS (from NEP2Tests.swift) ===== */

void test_nep2_encrypt_decrypt(void) {
    const char* private_key_hex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
    const char* password = "TestPassword123";
    
    size_t priv_key_len = 32;
    uint8_t priv_key[32];
    size_t decoded_len = priv_key_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, strlen(private_key_hex), 
                                        priv_key, priv_key_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create key pair for getting address
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key, priv_key_len, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Encrypt with NEP-2
    char encrypted[59];
    err = neoc_nep2_encrypt(priv_key, password, encrypted, sizeof(encrypted));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(58, strlen(encrypted));
    TEST_ASSERT_TRUE(strncmp(encrypted, "6P", 2) == 0);
    
    // Decrypt NEP-2
    uint8_t decrypted_key[32];
    err = neoc_nep2_decrypt(encrypted, password, decrypted_key, sizeof(decrypted_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(priv_key, decrypted_key, 32);
    
    neoc_ec_key_pair_free(key_pair);
}

void test_nep2_wrong_password(void) {
    const char* encrypted = "6PYVwtrCJWvh8qQMGa4z3EqfGnT7VBS5s6TLnJuEf5QhQ9DFECDkGDyGaD";
    const char* wrong_password = "WrongPassword";
    
    uint8_t decrypted_key[32];
    neoc_error_t err = neoc_nep2_decrypt(encrypted, wrong_password, 
                                          decrypted_key, sizeof(decrypted_key));
    TEST_ASSERT_NOT_EQUAL(NEOC_SUCCESS, err);
}

/* ===== SIGN TESTS (from SignTests.swift) ===== */

void test_sign_and_verify_message(void) {
    const char* message = "Hello, Neo blockchain!";
    const char* private_key_hex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
    
    size_t priv_key_len = 32;
    uint8_t priv_key[32];
    size_t decoded_len = priv_key_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, strlen(private_key_hex), 
                                        priv_key, priv_key_len, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create key pair
    neoc_ec_key_pair_t* key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(priv_key, priv_key_len, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Hash the message
    neoc_hash256_t hash;
    err = neoc_hash256_from_data_hash(&hash, (const uint8_t*)message, strlen(message));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Sign the hash
    neoc_ecdsa_signature_t* signature = NULL;
    uint8_t hash_bytes[32];
    neoc_hash256_to_bytes(&hash, hash_bytes, sizeof(hash_bytes));
    err = neoc_sign_hash(hash_bytes, sizeof(hash_bytes), key_pair, &signature);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(signature);
    
    // Verify the signature
    bool is_valid = false;
    err = neoc_verify_signature(hash_bytes, sizeof(hash_bytes), signature, 
                                 neoc_ec_key_pair_get_public_key(key_pair), &is_valid);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(is_valid);
    
    neoc_ecdsa_signature_free(signature);
    neoc_ec_key_pair_free(key_pair);
}

void test_sign_arbitrary_message(void) {
    const char* message = "Neo Smart Economy";
    
    // Create a new key pair
    neoc_ec_key_pair_t* key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Sign the message
    neoc_ecdsa_signature_t* signature = NULL;
    err = neoc_sign_message((const uint8_t*)message, strlen(message), key_pair, &signature);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(signature);
    
    // Verify the signature
    bool is_valid = false;
    err = neoc_verify_message((const uint8_t*)message, strlen(message), signature,
                               neoc_ec_key_pair_get_public_key(key_pair), &is_valid);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(is_valid);
    
    // Test with wrong message
    const char* wrong_message = "Wrong Message";
    err = neoc_verify_message((const uint8_t*)wrong_message, strlen(wrong_message), signature,
                               neoc_ec_key_pair_get_public_key(key_pair), &is_valid);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_FALSE(is_valid);
    
    neoc_ecdsa_signature_free(signature);
    neoc_ec_key_pair_free(key_pair);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== BASE64 TESTS ===\n");
    RUN_TEST(test_base64_encode_for_string);
    RUN_TEST(test_base64_decode);
    
    printf("\n=== EC KEY PAIR TESTS ===\n");
    RUN_TEST(test_ec_key_pair_creation);
    RUN_TEST(test_ec_key_pair_from_private_key);
    
    printf("\n=== WIF TESTS ===\n");
    RUN_TEST(test_wif_encode_decode);
    RUN_TEST(test_wif_invalid_decode);
    
    printf("\n=== NEP-2 TESTS ===\n");
    RUN_TEST(test_nep2_encrypt_decrypt);
    RUN_TEST(test_nep2_wrong_password);
    
    printf("\n=== SIGN TESTS ===\n");
    RUN_TEST(test_sign_and_verify_message);
    RUN_TEST(test_sign_arbitrary_message);
    
    return UNITY_END();
}
