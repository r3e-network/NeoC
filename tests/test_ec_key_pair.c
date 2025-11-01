/**
 * @file test_ec_key_pair.c
 * @brief EC key pair tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/crypto/wif.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== EC KEY PAIR TESTS ===== */

void test_new_public_key_from_compressed_point(void) {
    const char* encoded_point = "03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816";
    
    uint8_t point_bytes[33];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(encoded_point, point_bytes, sizeof(point_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, decoded_len);
    
    neoc_ec_public_key_t* public_key;
    err = neoc_ec_public_key_from_bytes(point_bytes, decoded_len, &public_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(public_key);
    
    // Get encoded compressed form
    uint8_t* encoded;
    size_t encoded_len;
    err = neoc_ec_public_key_get_encoded(public_key, true, &encoded, &encoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, encoded_len);
    TEST_ASSERT_EQUAL_MEMORY(point_bytes, encoded, 33);
    
    neoc_free(encoded);
    neoc_ec_public_key_free(public_key);
}

void test_new_public_key_from_uncompressed_point(void) {
    const char* uncompressed_point = 
        "04b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e1368165f4f7fb1c5862465543c06dd5a2aa414f6583f92a5cc3e1d4259df79bf6839c9";
    const char* expected_compressed = "03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816";
    
    uint8_t uncompressed_bytes[65];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(uncompressed_point, uncompressed_bytes, sizeof(uncompressed_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(65, decoded_len);
    
    neoc_ec_public_key_t* public_key;
    err = neoc_ec_public_key_from_bytes(uncompressed_bytes, decoded_len, &public_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get compressed form
    uint8_t* compressed;
    size_t compressed_len;
    err = neoc_ec_public_key_get_encoded(public_key, true, &compressed, &compressed_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, compressed_len);
    
    // Check it matches expected compressed form
    uint8_t expected_bytes[33];
    neoc_hex_decode(expected_compressed, expected_bytes, sizeof(expected_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_MEMORY(expected_bytes, compressed, 33);
    
    neoc_free(compressed);
    neoc_ec_public_key_free(public_key);
}

void test_new_public_key_from_invalid_size(void) {
    // Test with too small data (31 bytes instead of 33)
    uint8_t too_small[31];
    memset(too_small, 0x03, sizeof(too_small));
    
    neoc_ec_public_key_t* public_key;
    neoc_error_t err = neoc_ec_public_key_from_bytes(too_small, sizeof(too_small), &public_key);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
}

void test_public_key_from_hex_with_prefix(void) {
    const char* prefixed = "0x03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816";
    const char* unprefixed = prefixed + 2; // Skip "0x"
    
    uint8_t point_bytes[33];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(unprefixed, point_bytes, sizeof(point_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, decoded_len);
    
    neoc_ec_public_key_t* public_key;
    err = neoc_ec_public_key_from_bytes(point_bytes, decoded_len, &public_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(public_key);
    
    neoc_ec_public_key_free(public_key);
}

void test_public_key_wif(void) {
    const char* private_key_hex = "c7134d6fd8e73d819e82755c64c93788d8db0961929e025a53363c4cc02a6962";
    
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, decoded_len);
    
    neoc_ec_key_pair_t* key_pair;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    char* wif;
    err = neoc_ec_key_pair_export_as_wif(key_pair, &wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(wif);
    TEST_ASSERT_EQUAL_STRING("L3tgppXLgdaeqSGSFw1Go3skBiy8vQAM7YMXvTHsKQtE16PBncSU", wif);
    
    neoc_free(wif);
    neoc_ec_key_pair_free(key_pair);
}

void test_create_random_key_pair(void) {
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    TEST_ASSERT_NOT_NULL(key_pair->private_key);
    TEST_ASSERT_NOT_NULL(key_pair->public_key);
    
    // Get private key
    uint8_t private_key[32];
    size_t key_len = sizeof(private_key);
    err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(32, key_len);
    
    // Get public key
    uint8_t public_key[33];
    key_len = sizeof(public_key);
    err = neoc_ec_key_pair_get_public_key(key_pair, public_key, &key_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(33, key_len);
    
    neoc_ec_key_pair_free(key_pair);
}

void test_key_pair_from_wif(void) {
    const char* wif = "L3tgppXLgdaeqSGSFw1Go3skBiy8vQAM7YMXvTHsKQtE16PBncSU";
    
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_import_from_wif(wif, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(key_pair);
    
    // Export back to WIF and verify
    char* exported_wif;
    err = neoc_ec_key_pair_export_as_wif(key_pair, &exported_wif);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING(wif, exported_wif);
    
    neoc_free(exported_wif);
    neoc_ec_key_pair_free(key_pair);
}

void test_get_address_from_key_pair(void) {
    const char* private_key_hex = "c7134d6fd8e73d819e82755c64c93788d8db0961929e025a53363c4cc02a6962";
    
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(private_key_hex, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_ec_key_pair_t* key_pair;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    char* address;
    err = neoc_ec_key_pair_get_address(key_pair, &address);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(address);
    // Address format should be valid Neo address starting with 'N'
    TEST_ASSERT_EQUAL_INT('N', address[0]);
    
    neoc_free(address);
    neoc_ec_key_pair_free(key_pair);
}

void test_public_key_comparison(void) {
    const char* key1_hex = "03b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e136816";
    const char* key2_hex = "036b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296";
    const char* key1_uncompressed_hex = 
        "04b4af8d061b6b320cce6c63bc4ec7894dce107bfc5f5ef5c68a93b4ad1e1368165f4f7fb1c5862465543c06dd5a2aa414f6583f92a5cc3e1d4259df79bf6839c9";
    
    uint8_t key1_bytes[33], key2_bytes[33], key1_uncomp_bytes[65];
    size_t decoded_len;
    
    neoc_hex_decode(key1_hex, key1_bytes, sizeof(key1_bytes), &decoded_len);
    neoc_hex_decode(key2_hex, key2_bytes, sizeof(key2_bytes), &decoded_len);
    neoc_hex_decode(key1_uncompressed_hex, key1_uncomp_bytes, sizeof(key1_uncomp_bytes), &decoded_len);
    
    neoc_ec_public_key_t *key1, *key2, *key1_uncomp;
    neoc_ec_public_key_from_bytes(key1_bytes, 33, &key1);
    neoc_ec_public_key_from_bytes(key2_bytes, 33, &key2);
    neoc_ec_public_key_from_bytes(key1_uncomp_bytes, 65, &key1_uncomp);
    
    // Test that key1 compressed and uncompressed represent the same key
    uint8_t *encoded1, *encoded1_uncomp;
    size_t len1, len1_uncomp;
    neoc_ec_public_key_get_encoded(key1, true, &encoded1, &len1);
    neoc_ec_public_key_get_encoded(key1_uncomp, true, &encoded1_uncomp, &len1_uncomp);
    TEST_ASSERT_EQUAL_INT(33, len1);
    TEST_ASSERT_EQUAL_INT(33, len1_uncomp);
    TEST_ASSERT_EQUAL_MEMORY(encoded1, encoded1_uncomp, 33);
    
    neoc_free(encoded1);
    neoc_free(encoded1_uncomp);
    neoc_ec_public_key_free(key1);
    neoc_ec_public_key_free(key2);
    neoc_ec_public_key_free(key1_uncomp);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_new_public_key_from_compressed_point);
    RUN_TEST(test_new_public_key_from_uncompressed_point);
    RUN_TEST(test_new_public_key_from_invalid_size);
    RUN_TEST(test_public_key_from_hex_with_prefix);
    RUN_TEST(test_public_key_wif);
    RUN_TEST(test_create_random_key_pair);
    RUN_TEST(test_key_pair_from_wif);
    RUN_TEST(test_get_address_from_key_pair);
    RUN_TEST(test_public_key_comparison);
    
    UNITY_END();
    return 0;
}
