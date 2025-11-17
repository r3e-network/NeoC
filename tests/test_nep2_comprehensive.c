/**
 * @file test_nep2_comprehensive.c
 * @brief Comprehensive NEP-2 tests converted from Swift and extended
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/nep2.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <time.h>

// Test constants (from Swift TestProperties)
static const char* DEFAULT_ACCOUNT_PRIVATE_KEY = "84180ac9d6eb6fba207ea4ef9d2200102d1ebeb4b9c07e2c6a738a42742e27a5";
static const char* DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY = "6PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdLz";
static const char* DEFAULT_ACCOUNT_PASSWORD = "neo";
static const char* NON_DEFAULT_SCRYPT_ENCRYPTED = "6PYM7jHL3uwhP8uuHP9fMGMfJxfyQbanUZPQEh1772iyb7vRnUkbkZmdRT";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== NEP-2 DECRYPTION TESTS ===== */

void test_decrypt_with_default_scrypt_params(void) {
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_nep2_decrypt_key_pair(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY,
                                                  DEFAULT_ACCOUNT_PASSWORD,
                                                  NULL,
                                                  &key_pair);
    
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Get private key and verify it matches expected
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, key_len);
    
    // Convert expected private key from hex
    uint8_t expected_key[32];
    size_t decoded_len;
    err = neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, expected_key, sizeof(expected_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    TEST_ASSERT_EQUAL_MEMORY(expected_key, private_key, 32);
    
    neoc_ec_key_pair_free(key_pair);
}

void test_decrypt_with_non_default_scrypt_params(void) {
    neoc_nep2_params_t params = {
        .n = 256,
        .r = 1,
        .p = 1,
    };
    
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_nep2_decrypt_key_pair(NON_DEFAULT_SCRYPT_ENCRYPTED,
                                                  DEFAULT_ACCOUNT_PASSWORD,
                                                  &params,
                                                  &key_pair);
    
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Get private key and verify it matches expected
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert expected private key from hex
    uint8_t expected_key[32];
    size_t decoded_len;
    err = neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, expected_key, sizeof(expected_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    TEST_ASSERT_EQUAL_MEMORY(expected_key, private_key, 32);
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== NEP-2 ENCRYPTION TESTS ===== */

void test_encrypt_with_default_scrypt_params(void) {
    // Create key pair from known private key
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Encrypt with default parameters
    char* encrypted;
    err = neoc_nep2_encrypt_key_pair(key_pair, DEFAULT_ACCOUNT_PASSWORD, NULL, &encrypted);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(encrypted);
    
    // Verify it matches expected encrypted key
    TEST_ASSERT_EQUAL_STRING(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY, encrypted);
    
    neoc_free(encrypted);
    neoc_ec_key_pair_free(key_pair);
}

void test_encrypt_with_non_default_scrypt_params(void) {
    neoc_nep2_params_t params = {
        .n = 256,
        .r = 1,
        .p = 1,
    };
    
    // Create key pair from known private key
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Encrypt with non-default parameters
    char* encrypted;
    err = neoc_nep2_encrypt_key_pair(key_pair, DEFAULT_ACCOUNT_PASSWORD, &params, &encrypted);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(encrypted);
    
    // Verify it matches expected encrypted key
    TEST_ASSERT_EQUAL_STRING(NON_DEFAULT_SCRYPT_ENCRYPTED, encrypted);
    
    neoc_free(encrypted);
    neoc_ec_key_pair_free(key_pair);
}

/* ===== NEP-2 ROUND-TRIP TESTS ===== */

void test_encrypt_decrypt_round_trip(void) {
    // Create random key pair
    neoc_ec_key_pair_t* original_key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&original_key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get original private key
    uint8_t original_private_key[32];
    size_t key_len = sizeof(original_private_key);
    err = neoc_ec_key_pair_get_private_key(original_key_pair, original_private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Encrypt
    const char* password = "test_password_123";
    char* encrypted;
    err = neoc_nep2_encrypt_key_pair(original_key_pair, password, NULL, &encrypted);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(encrypted);
    
    // Decrypt
    neoc_ec_key_pair_t* decrypted_key_pair;
    err = neoc_nep2_decrypt_key_pair(encrypted, password, NULL, &decrypted_key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(decrypted_key_pair);
    
    // Get decrypted private key
    uint8_t decrypted_private_key[32];
    key_len = sizeof(decrypted_private_key);
    err = neoc_ec_key_pair_get_private_key(decrypted_key_pair, decrypted_private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify they match
    TEST_ASSERT_EQUAL_MEMORY(original_private_key, decrypted_private_key, 32);
    
    neoc_free(encrypted);
    neoc_ec_key_pair_free(original_key_pair);
    neoc_ec_key_pair_free(decrypted_key_pair);
}

/* ===== NEP-2 ERROR HANDLING TESTS ===== */

void test_decrypt_with_invalid_password(void) {
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_nep2_decrypt_key_pair(DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY,
                                                  "wrong_password",
                                                  NULL,
                                                  &key_pair);
    
    // Should fail with authentication error
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_decrypt_with_malformed_encrypted_key(void) {
    const char* malformed_keys[] = {
        "6PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdL", // too short
        "6PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdLzz", // too long
        "5PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdLz", // wrong prefix
        "",  // empty string
        "not_a_valid_base58_string!!!", // invalid base58
    };
    
    size_t num_keys = sizeof(malformed_keys) / sizeof(malformed_keys[0]);
    for (size_t i = 0; i < num_keys; i++) {
        neoc_ec_key_pair_t* key_pair;
        neoc_error_t err = neoc_nep2_decrypt_key_pair(malformed_keys[i],
                                                      DEFAULT_ACCOUNT_PASSWORD,
                                                      NULL,
                                                      &key_pair);
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    }
}

void test_encrypt_with_null_inputs(void) {
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);

    char *encrypted = NULL;

    // Null password
    err = neoc_nep2_encrypt_key_pair(key_pair, NULL, NULL, &encrypted);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    TEST_ASSERT_NULL(encrypted);
    
    // Null key pair
    encrypted = NULL;
    err = neoc_nep2_encrypt_key_pair(NULL, "password", NULL, &encrypted);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    TEST_ASSERT_NULL(encrypted);
    
    // Null output
    err = neoc_nep2_encrypt_key_pair(key_pair, "password", NULL, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== NEP-2 SCRYPT PARAMETER VALIDATION TESTS ===== */

void test_various_scrypt_parameters(void) {
    neoc_ec_key_pair_t* original_key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&original_key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test different parameter combinations
    struct {
        int n, r, p;
        bool should_succeed;
    } test_cases[] = {
        {16384, 8, 8, true},    // NEP-2 default
        {256, 1, 1, true},      // Low security (fast)
        {32768, 8, 8, true},    // Higher security
        {0, 8, 8, false},       // Invalid N
        {16384, 0, 8, false},   // Invalid r
        {16384, 8, 0, false},   // Invalid p
        {2, 1, 1, true},        // Minimal valid
    };
    
    size_t num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    for (size_t i = 0; i < num_cases; i++) {
        neoc_nep2_params_t params = {
            .n = test_cases[i].n,
            .r = test_cases[i].r,
            .p = test_cases[i].p,
        };

        char *encrypted = NULL;
        err = neoc_nep2_encrypt_key_pair(original_key_pair, "password", &params, &encrypted);
        
        if (test_cases[i].should_succeed) {
            TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
            TEST_ASSERT_NOT_NULL(encrypted);
            
            // Verify we can decrypt
            neoc_ec_key_pair_t* decrypted_key_pair;
            err = neoc_nep2_decrypt_key_pair(encrypted, "password", &params, &decrypted_key_pair);
            TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
            
            neoc_ec_key_pair_free(decrypted_key_pair);
            neoc_free(encrypted);
        } else {
            TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
            TEST_ASSERT_NULL(encrypted);
        }
    }
    
    neoc_ec_key_pair_free(original_key_pair);
}

/* ===== NEP-2 PERFORMANCE TESTS ===== */

void test_encrypt_decrypt_performance(void) {
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test with fast scrypt parameters
    neoc_nep2_params_t fast_params = {
        .n = 256,
        .r = 1,
        .p = 1,
    };
    
    clock_t start = clock();
    
    // Encrypt
    char* encrypted;
    err = neoc_nep2_encrypt_key_pair(key_pair, "password", &fast_params, &encrypted);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    clock_t encrypt_time = clock();
    
    // Decrypt
    neoc_ec_key_pair_t* decrypted_key_pair;
    err = neoc_nep2_decrypt_key_pair(encrypted, "password", &fast_params, &decrypted_key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    clock_t decrypt_time = clock();
    
    double encrypt_seconds = ((double)(encrypt_time - start)) / CLOCKS_PER_SEC;
    double decrypt_seconds = ((double)(decrypt_time - encrypt_time)) / CLOCKS_PER_SEC;
    
    // Performance assertions (generous bounds for CI)
    TEST_ASSERT_TRUE(encrypt_seconds < 5.0);  // Should be much faster with low params
    TEST_ASSERT_TRUE(decrypt_seconds < 5.0);
    
    neoc_free(encrypted);
    neoc_ec_key_pair_free(key_pair);
    neoc_ec_key_pair_free(decrypted_key_pair);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    // Decryption tests
    RUN_TEST(test_decrypt_with_default_scrypt_params);
    RUN_TEST(test_decrypt_with_non_default_scrypt_params);
    
    // Encryption tests  
    RUN_TEST(test_encrypt_with_default_scrypt_params);
    RUN_TEST(test_encrypt_with_non_default_scrypt_params);
    
    // Round-trip tests
    RUN_TEST(test_encrypt_decrypt_round_trip);
    
    // Error handling tests
    RUN_TEST(test_decrypt_with_invalid_password);
    RUN_TEST(test_decrypt_with_malformed_encrypted_key);
    RUN_TEST(test_encrypt_with_null_inputs);
    
    // Parameter validation tests
    RUN_TEST(test_various_scrypt_parameters);
    
    // Performance tests
    RUN_TEST(test_encrypt_decrypt_performance);
    
    UNITY_END();
    return 0;
}
