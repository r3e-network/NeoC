/**
 * @file test_wif_comprehensive.c
 * @brief Comprehensive WIF (Wallet Import Format) tests converted from Swift and extended
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/wif.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/utils/neoc_base58.h>
#include <string.h>
#include <stdio.h>

// Test constants (from Swift WIFTests)
static const char* VALID_WIF = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13A";
static const char* PRIVATE_KEY_HEX = "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3a3";

// Additional test vectors
static const char* TEST_WIF_VECTORS[][2] = {
    {"L1eV34wPoj9weqhGijdDLtVQzUpWGHszXXpdU9dPuh2nRFFzFa7E", "84180ac9d6eb6fba207ea4ef9d2200102d1ebeb4b9c07e2c6a738a42742e27a5"},
    {"L3tgppXLgdaeqSGSFw1Go3skBiy8vQAM7YMXvTHsKQtE16PBncSU", "c7134d6fd8e73d819e82755c64c93788d8db0961929e025a53363c4cc02a6962"},
    {"KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn", "0000000000000000000000000000000000000000000000000000000000000001"},
    {"L5oLkpV3aqBJ4BgssVAsax1iRa77G5CVYnv9adQ6Z87te7TyUdSC", "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
};

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== WIF TO PRIVATE KEY TESTS ===== */

void test_valid_wif_to_private_key(void) {
    printf("Testing WIF to private key conversion\n");
    
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    neoc_error_t err = neoc_wif_to_private_key(VALID_WIF, private_key, &key_len);
    
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, key_len);
    
    // Convert expected private key from hex and compare
    uint8_t expected_key[32];
    size_t decoded_len;
    err = neoc_hex_decode(PRIVATE_KEY_HEX, expected_key, sizeof(expected_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    TEST_ASSERT_EQUAL_MEMORY(expected_key, private_key, 32);
}

void test_multiple_wif_vectors(void) {
    printf("Testing multiple WIF test vectors\n");
    
    size_t num_vectors = sizeof(TEST_WIF_VECTORS) / sizeof(TEST_WIF_VECTORS[0]);
    for (size_t i = 0; i < num_vectors; i++) {
        const char* wif = TEST_WIF_VECTORS[i][0];
        const char* expected_hex = TEST_WIF_VECTORS[i][1];
        
        // Convert WIF to private key
        uint8_t private_key[32];
        size_t key_len = sizeof(private_key);
        neoc_error_t err = neoc_wif_to_private_key(wif, private_key, &key_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_EQUAL_INT(32, key_len);
        
        // Convert expected hex to bytes
        uint8_t expected_key[32];
        size_t decoded_len;
        err = neoc_hex_decode(expected_hex, expected_key, sizeof(expected_key), &decoded_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        TEST_ASSERT_EQUAL_MEMORY(expected_key, private_key, 32);
        
        printf("  Vector %zu: WIF %s -> key %s ✓\n", i + 1, wif, expected_hex);
    }
}

/* ===== PRIVATE KEY TO WIF TESTS ===== */

void test_valid_private_key_to_wif(void) {
    printf("Testing private key to WIF conversion\n");
    
    // Convert hex private key to bytes
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(PRIVATE_KEY_HEX, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    // Convert to WIF
    char* wif;
    err = neoc_private_key_to_wif(private_key, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    TEST_ASSERT_EQUAL_STRING(VALID_WIF, wif);
    
    neoc_free(wif);
}

void test_private_key_to_wif_vectors(void) {
    printf("Testing private key to WIF conversion with vectors\n");
    
    size_t num_vectors = sizeof(TEST_WIF_VECTORS) / sizeof(TEST_WIF_VECTORS[0]);
    for (size_t i = 0; i < num_vectors; i++) {
        const char* expected_wif = TEST_WIF_VECTORS[i][0];
        const char* private_key_hex = TEST_WIF_VECTORS[i][1];
        
        // Convert hex to bytes
        uint8_t private_key[32];
        size_t decoded_len;
        neoc_error_t err = neoc_hex_decode(private_key_hex, private_key, sizeof(private_key), &decoded_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Convert to WIF
        char* wif;
        err = neoc_private_key_to_wif(private_key, &wif);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(wif);
        TEST_ASSERT_EQUAL_STRING(expected_wif, wif);
        
        neoc_free(wif);
        printf("  Vector %zu: key %s -> WIF %s ✓\n", i + 1, private_key_hex, expected_wif);
    }
}

/* ===== WIF ROUND-TRIP TESTS ===== */

void test_wif_round_trip(void) {
    printf("Testing WIF round-trip conversion\n");
    
    // Generate random private key
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get private key
    uint8_t original_private_key[32];
    size_t key_len = sizeof(original_private_key);
    err = neoc_ec_key_pair_get_private_key(key_pair, original_private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert to WIF
    char* wif;
    err = neoc_private_key_to_wif(original_private_key, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    
    // Convert back to private key
    uint8_t restored_private_key[32];
    key_len = sizeof(restored_private_key);
    err = neoc_wif_to_private_key(wif, restored_private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, key_len);
    
    // Verify they match
    TEST_ASSERT_EQUAL_MEMORY(original_private_key, restored_private_key, 32);
    
    neoc_free(wif);
    neoc_ec_key_pair_free(key_pair);
}

/* ===== WIF ERROR HANDLING TESTS ===== */

void test_wrongly_sized_wifs(void) {
    printf("Testing WIF with invalid sizes\n");
    
    // Too large (extra characters)
    const char* too_large = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13Ahc7S";
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    neoc_error_t err = neoc_wif_to_private_key(too_large, private_key, &key_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Too small (missing characters)
    const char* too_small = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWML";
    err = neoc_wif_to_private_key(too_small, private_key, &key_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_wrong_first_byte_wif(void) {
    printf("Testing WIF with wrong first byte\n");
    
    // Decode the valid WIF, modify first byte, re-encode
    uint8_t decoded[38];  // WIF decoded is typically 37-38 bytes
    size_t decoded_len = sizeof(decoded);
    neoc_error_t err = neoc_base58_decode(VALID_WIF, decoded, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Change first byte (should be 0x80 for mainnet)
    decoded[0] = 0x81;  // Wrong network byte
    
    char* wrong_wif;
    err = neoc_base58_encode(decoded, decoded_len, &wrong_wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Try to convert - should fail
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    err = neoc_wif_to_private_key(wrong_wif, private_key, &key_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_free(wrong_wif);
}

void test_wrong_byte_33_wif(void) {
    printf("Testing WIF with wrong compression flag\n");
    
    // Decode the valid WIF, modify compression flag, re-encode
    uint8_t decoded[38];
    size_t decoded_len = sizeof(decoded);
    neoc_error_t err = neoc_base58_decode(VALID_WIF, decoded, &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    if (decoded_len >= 34) {
        // Change compression flag byte (typically byte 33, should be 0x01)
        decoded[33] = 0x00;  // Wrong compression flag
        
        char* wrong_wif;
        err = neoc_base58_encode(decoded, decoded_len, &wrong_wif);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Try to convert - should fail
        uint8_t private_key[32];
        size_t key_len = sizeof(private_key);
        err = neoc_wif_to_private_key(wrong_wif, private_key, &key_len);
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        
        neoc_free(wrong_wif);
    }
}

void test_wrongly_sized_private_key(void) {
    printf("Testing private key to WIF with wrong size\n");
    
    // Try with 31 bytes instead of 32
    uint8_t wrongly_sized_key[31];
    memset(wrongly_sized_key, 0xAB, sizeof(wrongly_sized_key));
    
    char* wif;
    neoc_error_t err = neoc_private_key_to_wif_sized(wrongly_sized_key, sizeof(wrongly_sized_key), &wif);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_invalid_wif_formats(void) {
    printf("Testing various invalid WIF formats\n");
    
    const char* invalid_wifs[] = {
        "",  // Empty string
        "1", // Too short
        "not_a_valid_base58_string!!!", // Invalid base58 characters
        "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ", // Bitcoin WIF (wrong network)
        "6PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdLz", // NEP-2 encrypted key (not WIF)
    };
    
    size_t num_invalid = sizeof(invalid_wifs) / sizeof(invalid_wifs[0]);
    for (size_t i = 0; i < num_invalid; i++) {
        uint8_t private_key[32];
        size_t key_len = sizeof(private_key);
        neoc_error_t err = neoc_wif_to_private_key(invalid_wifs[i], private_key, &key_len);
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        printf("  Invalid WIF %zu: \"%s\" ✓\n", i + 1, invalid_wifs[i]);
    }
}

/* ===== WIF NULL INPUT TESTS ===== */

void test_wif_with_null_inputs(void) {
    printf("Testing WIF functions with null inputs\n");
    
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    char* wif;
    
    // Null WIF input
    neoc_error_t err = neoc_wif_to_private_key(NULL, private_key, &key_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Null output buffer
    err = neoc_wif_to_private_key(VALID_WIF, NULL, &key_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Null size pointer
    err = neoc_wif_to_private_key(VALID_WIF, private_key, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Null private key input
    err = neoc_private_key_to_wif(NULL, &wif);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Null output pointer
    memset(private_key, 0xAB, sizeof(private_key));
    err = neoc_private_key_to_wif(private_key, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

/* ===== WIF EDGE CASE TESTS ===== */

void test_wif_edge_cases(void) {
    printf("Testing WIF edge cases\n");
    
    // Test with all zeros private key
    uint8_t zero_key[32];
    memset(zero_key, 0x00, sizeof(zero_key));
    char* zero_wif;
    neoc_error_t err = neoc_private_key_to_wif(zero_key, &zero_wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(zero_wif);
    
    // Convert back
    uint8_t restored_zero_key[32];
    size_t key_len = sizeof(restored_zero_key);
    err = neoc_wif_to_private_key(zero_wif, restored_zero_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(zero_key, restored_zero_key, 32);
    
    neoc_free(zero_wif);
    
    // Test with all 0xFF private key
    uint8_t max_key[32];
    memset(max_key, 0xFF, sizeof(max_key));
    char* max_wif;
    err = neoc_private_key_to_wif(max_key, &max_wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(max_wif);
    
    // Convert back
    uint8_t restored_max_key[32];
    key_len = sizeof(restored_max_key);
    err = neoc_wif_to_private_key(max_wif, restored_max_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_MEMORY(max_key, restored_max_key, 32);
    
    neoc_free(max_wif);
}

/* ===== WIF INTEGRATION WITH EC KEY PAIR ===== */

void test_wif_integration_with_ec_key_pair(void) {
    printf("Testing WIF integration with EC key pairs\n");
    
    // Create key pair from WIF
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_import_from_wif(VALID_WIF, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Export back to WIF
    char* exported_wif;
    err = neoc_ec_key_pair_export_as_wif(key_pair, &exported_wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(exported_wif);
    TEST_ASSERT_EQUAL_STRING(VALID_WIF, exported_wif);
    
    // Get private key and verify it matches expected
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    uint8_t expected_key[32];
    size_t decoded_len;
    neoc_hex_decode(PRIVATE_KEY_HEX, expected_key, sizeof(expected_key), &decoded_len);
    TEST_ASSERT_EQUAL_MEMORY(expected_key, private_key, 32);
    
    neoc_free(exported_wif);
    neoc_ec_key_pair_free(key_pair);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== COMPREHENSIVE WIF TESTS ===\n");
    
    // Basic conversion tests
    RUN_TEST(test_valid_wif_to_private_key);
    RUN_TEST(test_multiple_wif_vectors);
    RUN_TEST(test_valid_private_key_to_wif);
    RUN_TEST(test_private_key_to_wif_vectors);
    
    // Round-trip tests
    RUN_TEST(test_wif_round_trip);
    
    // Error handling tests
    RUN_TEST(test_wrongly_sized_wifs);
    RUN_TEST(test_wrong_first_byte_wif);
    RUN_TEST(test_wrong_byte_33_wif);
    RUN_TEST(test_wrongly_sized_private_key);
    RUN_TEST(test_invalid_wif_formats);
    RUN_TEST(test_wif_with_null_inputs);
    
    // Edge case tests
    RUN_TEST(test_wif_edge_cases);
    
    // Integration tests
    RUN_TEST(test_wif_integration_with_ec_key_pair);
    
    UNITY_END();
    return 0;
}