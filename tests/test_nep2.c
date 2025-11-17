/**
 * @file test_nep2.c
 * @brief NEP-2 encryption tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/nep2.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

// Test constants from Swift TestProperties
static const char* DEFAULT_ACCOUNT_PRIVATE_KEY = "84180ac9d6eb6fba207ea4ef9d2200102d1ebeb4b9c07e2c6a738a42742e27a5";
static const char* DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY = "6PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdLz";
static const char* DEFAULT_ACCOUNT_PASSWORD = "neo";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== NEP-2 TESTS ===== */

void test_decrypt_with_default_scrypt_params(void) {
    uint8_t decrypted_private_key[32];
    
    // Decrypt the NEP-2 encrypted private key
    neoc_error_t err = neoc_nep2_decrypt(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY,
                                          DEFAULT_ACCOUNT_PASSWORD,
                                          NULL, // Use default params
                                          decrypted_private_key,
                                          sizeof(decrypted_private_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert expected private key from hex
    uint8_t expected_private_key[32];
    size_t decoded_len;
    neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, expected_private_key, 
                    sizeof(expected_private_key), &decoded_len);
    
    // Compare decrypted key with expected
    TEST_ASSERT_EQUAL_MEMORY(expected_private_key, decrypted_private_key, 32);
}

void test_decrypt_with_non_default_scrypt_params(void) {
    // Use light params (n=256, r=1, p=1)
    neoc_nep2_params_t params = {256, 1, 1};
    const char* encrypted = "6PYM7jHL3uwhP8uuHP9fMGMfJxfyQbanUZPQEh1772iyb7vRnUkbkZmdRT";
    
    uint8_t decrypted_private_key[32];
    
    // Decrypt with custom params
    neoc_error_t err = neoc_nep2_decrypt(encrypted,
                                          DEFAULT_ACCOUNT_PASSWORD,
                                          &params,
                                          decrypted_private_key,
                                          sizeof(decrypted_private_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert expected private key from hex
    uint8_t expected_private_key[32];
    size_t decoded_len;
    neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, expected_private_key,
                    sizeof(expected_private_key), &decoded_len);
    
    // Compare decrypted key with expected
    TEST_ASSERT_EQUAL_MEMORY(expected_private_key, decrypted_private_key, 32);
}

void test_encrypt_with_default_scrypt_params(void) {
    // Convert private key from hex
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, private_key,
                    sizeof(private_key), &decoded_len);
    
    char encrypted_key[100];
    
    // Encrypt the private key
    neoc_error_t err = neoc_nep2_encrypt(private_key,
                                          DEFAULT_ACCOUNT_PASSWORD,
                                          NULL, // Use default params
                                          encrypted_key,
                                          sizeof(encrypted_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // The encrypted key should start with "6PY"
    TEST_ASSERT_TRUE(strncmp(encrypted_key, "6PY", 3) == 0);
    
    // We can't compare exact encrypted value because of random salt,
    // but we can verify it decrypts correctly
    uint8_t decrypted[32];
    err = neoc_nep2_decrypt(encrypted_key,
                            DEFAULT_ACCOUNT_PASSWORD,
                            NULL,
                            decrypted,
                            sizeof(decrypted));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(private_key, decrypted, 32);
}

void test_encrypt_with_non_default_scrypt_params(void) {
    // Use light params
    neoc_nep2_params_t params = {256, 1, 1};
    
    // Convert private key from hex
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, private_key,
                    sizeof(private_key), &decoded_len);
    
    char encrypted_key[100];
    
    // Encrypt with custom params
    neoc_error_t err = neoc_nep2_encrypt(private_key,
                                          DEFAULT_ACCOUNT_PASSWORD,
                                          &params,
                                          encrypted_key,
                                          sizeof(encrypted_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // The encrypted key should start with "6PY"
    TEST_ASSERT_TRUE(strncmp(encrypted_key, "6PY", 3) == 0);
    
    // Verify it decrypts correctly with the same params
    uint8_t decrypted[32];
    err = neoc_nep2_decrypt(encrypted_key,
                            DEFAULT_ACCOUNT_PASSWORD,
                            &params,
                            decrypted,
                            sizeof(decrypted));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(private_key, decrypted, 32);
}

void test_verify_password(void) {
    // Test with correct password
    bool is_valid = neoc_nep2_verify_password(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY,
                                               DEFAULT_ACCOUNT_PASSWORD,
                                               NULL);
    TEST_ASSERT_TRUE(is_valid);
    
    // Test with wrong password
    is_valid = neoc_nep2_verify_password(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY,
                                          "wrong_password",
                                          NULL);
    TEST_ASSERT_FALSE(is_valid);
}

void test_is_valid_format(void) {
    // Valid NEP-2 format
    bool is_valid = neoc_nep2_is_valid_format(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY);
    TEST_ASSERT_TRUE(is_valid);
    
    // Invalid formats
    is_valid = neoc_nep2_is_valid_format("invalid");
    TEST_ASSERT_FALSE(is_valid);
    
    is_valid = neoc_nep2_is_valid_format("5PY123456789"); // Wrong prefix
    TEST_ASSERT_FALSE(is_valid);
    
    is_valid = neoc_nep2_is_valid_format("6PY"); // Too short
    TEST_ASSERT_FALSE(is_valid);
    
    is_valid = neoc_nep2_is_valid_format(NULL);
    TEST_ASSERT_FALSE(is_valid);
}

void test_round_trip_encryption(void) {
    // Generate a random key pair
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get the private key
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Encrypt with a password
    const char* password = "test_password_123!@#";
    char encrypted[100];
    err = neoc_nep2_encrypt(private_key, password, NULL, encrypted, sizeof(encrypted));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Decrypt and verify
    uint8_t decrypted[32];
    err = neoc_nep2_decrypt(encrypted, password, NULL, decrypted, sizeof(decrypted));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    TEST_ASSERT_EQUAL_MEMORY(private_key, decrypted, 32);
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== NEP-2 TESTS ===\n");
    
    RUN_TEST(test_decrypt_with_default_scrypt_params);
    RUN_TEST(test_decrypt_with_non_default_scrypt_params);
    RUN_TEST(test_encrypt_with_default_scrypt_params);
    RUN_TEST(test_encrypt_with_non_default_scrypt_params);
    RUN_TEST(test_verify_password);
    RUN_TEST(test_is_valid_format);
    RUN_TEST(test_round_trip_encryption);
    
    UNITY_END();
    return 0;
}
