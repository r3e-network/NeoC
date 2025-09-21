/**
 * @file test_comprehensive.c
 * @brief Comprehensive test suite for NeoC SDK with test vectors from NeoSwift
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "unity.h"
#include "neoc/neoc.h"

/* Test vector structure for Base58 tests */
typedef struct {
    const char* decoded;
    const char* encoded;
} base58_test_vector_t;

/* Test vectors ported from NeoSwift Base58Tests.swift */
static const base58_test_vector_t base58_vectors[] = {
    {"", ""},
    {" ", "Z"},
    {"-", "n"},
    {"0", "q"},
    {"1", "r"},
    {"-1", "4SU"},
    {"11", "4k8"},
    {"abc", "ZiCa"},
    {"1234598760", "3mJr7AoUXx2Wqd"},
    {"abcdefghijklmnopqrstuvwxyz", "3yxU3u1igY8WkgtjK92fbJQCd4BZiiT1v25f"},
    {"00000000000000000000000000000000000000000000000000000000000000",
     "3sN2THZeE9Eh9eYrwkvZqNstbHGvrxSAM7gXUXvyFQP8XvQLUqNCS27icwUeDT7ckHm4FUHM2mTVh1vbLmk7y"}
};

static const char* base58_invalid_strings[] = {
    "0", "O", "I", "l", "3mJr0", "O3yxU", "3sNI", "4kl8", "0OIl", "!@#$%^&*()-_=+~`"
};

/* Hash160 test vectors */
static const char* hash160_valid_hex = "17694821c6e3ea8b7a7d770952e7de86c73d94c3";
// static const char* hash160_address = "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke";
// static const char* hash160_pubkey = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";

/* Hash256 test vectors */
static const char* hash256_valid_hex = "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a";

/* Base58Check test vectors */
static const uint8_t base58check_input_data[] = {
    6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218, 133, 13, 109, 40, 194, 236, 153, 44, 61, 157, 254
};
static const char* base58check_expected = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";

/* Test fixtures */
void setUp(void) {
    neoc_error_t result = neoc_init();
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== BASE58 COMPREHENSIVE TESTS ===== */

void test_base58_encoding_valid_vectors(void) {
    const size_t num_vectors = sizeof(base58_vectors) / sizeof(base58_vectors[0]);
    
    for (size_t i = 0; i < num_vectors; i++) {
        const base58_test_vector_t* vector = &base58_vectors[i];
        const uint8_t* data = (const uint8_t*)vector->decoded;
        size_t data_length = strlen(vector->decoded);
        
        char* encoded = neoc_base58_encode_alloc(data, data_length);
        TEST_ASSERT_NOT_NULL(encoded);
        TEST_ASSERT_EQUAL_STRING(vector->encoded, encoded);
        
        neoc_free(encoded);
    }
}

void test_base58_decoding_valid_vectors(void) {
    const size_t num_vectors = sizeof(base58_vectors) / sizeof(base58_vectors[0]);
    
    for (size_t i = 0; i < num_vectors; i++) {
        const base58_test_vector_t* vector = &base58_vectors[i];
        
        size_t decoded_length;
        uint8_t* decoded = neoc_base58_decode_alloc(vector->encoded, &decoded_length);
        
        if (strlen(vector->decoded) == 0 && strlen(vector->encoded) == 0) {
            /* Empty string case - may return NULL */
            if (decoded != NULL) {
                TEST_ASSERT_EQUAL_INT(0, decoded_length);
                neoc_free(decoded);
            }
        } else {
            TEST_ASSERT_NOT_NULL(decoded);
            TEST_ASSERT_EQUAL_INT(strlen(vector->decoded), decoded_length);
            TEST_ASSERT_EQUAL_MEMORY(vector->decoded, decoded, decoded_length);
            neoc_free(decoded);
        }
    }
}

void test_base58_decoding_invalid_strings(void) {
    const size_t num_invalid = sizeof(base58_invalid_strings) / sizeof(base58_invalid_strings[0]);
    
    for (size_t i = 0; i < num_invalid; i++) {
        size_t decoded_length;
        uint8_t* decoded = neoc_base58_decode_alloc(base58_invalid_strings[i], &decoded_length);
        
        /* Invalid strings should return NULL */
        TEST_ASSERT_NULL(decoded);
    }
}

void test_base58_check_encoding(void) {
    char* encoded = neoc_base58_check_encode_alloc(base58check_input_data, 
                                                   sizeof(base58check_input_data));
    TEST_ASSERT_NOT_NULL(encoded);
    TEST_ASSERT_EQUAL_STRING(base58check_expected, encoded);
    neoc_free(encoded);
}

void test_base58_check_decoding(void) {
    size_t decoded_length;
    uint8_t* decoded = neoc_base58_check_decode_alloc(base58check_expected, &decoded_length);
    
    TEST_ASSERT_NOT_NULL(decoded);
    TEST_ASSERT_EQUAL_INT(sizeof(base58check_input_data), decoded_length);
    TEST_ASSERT_EQUAL_MEMORY(base58check_input_data, decoded, decoded_length);
    neoc_free(decoded);
}

void test_base58_check_invalid_characters(void) {
    size_t decoded_length;
    uint8_t* decoded = neoc_base58_check_decode_alloc("0oO1lL", &decoded_length);
    TEST_ASSERT_NULL(decoded);
}

void test_base58_check_invalid_checksum(void) {
    /* Modified checksum (last character changed) */
    size_t decoded_length;
    uint8_t* decoded = neoc_base58_check_decode_alloc("tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtrW", 
                                                      &decoded_length);
    TEST_ASSERT_NULL(decoded);
}

/* ===== HASH160 COMPREHENSIVE TESTS ===== */

void test_hash160_from_valid_hex(void) {
    neoc_hash160_t hash;
    
    /* Test with 0x prefix */
    char hex_with_prefix[43];
    snprintf(hex_with_prefix, sizeof(hex_with_prefix), "0x%s", hash160_valid_hex);
    
    neoc_error_t result = neoc_hash160_from_hex(&hash, hex_with_prefix);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    char output[41];
    result = neoc_hash160_to_hex(&hash, output, sizeof(output), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING(hash160_valid_hex, output);
    
    /* Test without 0x prefix */
    result = neoc_hash160_from_hex(&hash, hash160_valid_hex);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash160_to_hex(&hash, output, sizeof(output), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING(hash160_valid_hex, output);
}

void test_hash160_creation_errors(void) {
    neoc_hash160_t hash;
    neoc_error_t result;
    
    /* Test with valid odd-length hex (should succeed with padding) */
    result = neoc_hash160_from_hex(&hash, "0x23ba2703c53263e8d6e522dc32203339dcd8eee");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash160_from_hex(&hash, "g3ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_HEX, result);
    
    /* Test wrong length - too short */
    result = neoc_hash160_from_hex(&hash, "23ba2703c53263e8d6e522dc32203339dcd8ee");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, result);
    
    /* Test wrong length - too long */
    result = neoc_hash160_from_hex(&hash, "c56f33fc6ecfcd0c225c4ab356fee59390af8560be0e930faebe74a6daff7c9b");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL, result);
}

void test_hash160_to_little_endian_array(void) {
    neoc_hash160_t hash;
    neoc_error_t result = neoc_hash160_from_hex(&hash, "23ba2703c53263e8d6e522dc32203339dcd8eee9");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Expected reversed bytes */
    uint8_t expected[] = {0xe9, 0xee, 0xd8, 0xdc, 0x39, 0x33, 0x20, 0x32, 0xdc, 0x22, 
                         0xe5, 0xd6, 0xe8, 0x63, 0x32, 0xc5, 0x03, 0x27, 0xba, 0x23};
    
    uint8_t actual[NEOC_HASH160_SIZE];
    result = neoc_hash160_to_little_endian_bytes(&hash, actual, 20);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_MEMORY(expected, actual, NEOC_HASH160_SIZE);
}

void test_hash160_serialization(void) {
    neoc_hash160_t hash, deserialized_hash;
    const char* test_string = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
    
    neoc_error_t result = neoc_hash160_from_hex(&hash, test_string);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Serialize to little-endian bytes */
    uint8_t serialized[NEOC_HASH160_SIZE];
    result = neoc_hash160_to_little_endian_bytes(&hash, serialized, 20);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Reverse the bytes back to big-endian for comparison */
    uint8_t reversed[NEOC_HASH160_SIZE];
    for (int i = 0; i < NEOC_HASH160_SIZE; i++) {
        reversed[i] = serialized[NEOC_HASH160_SIZE - 1 - i];
    }
    
    /* Deserialize from reversed bytes */
    result = neoc_hash160_from_bytes(&deserialized_hash, reversed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Verify they're equal */
    TEST_ASSERT_TRUE(neoc_hash160_equal(&hash, &deserialized_hash));
    
    char output[41];
    result = neoc_hash160_to_hex(&deserialized_hash, output, sizeof(output), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING(test_string, output);
}

void test_hash160_comparison(void) {
    neoc_hash160_t hash1, hash2, hash3;
    
    /* Create test hashes */
    neoc_error_t result = neoc_hash160_from_hex(&hash1, "01a402d800000000000000000000000000000000");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash160_from_hex(&hash2, "d802a40100000000000000000000000000000000");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash160_from_hex(&hash3, "a7b3a19100000000000000000000000000000000");
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Test equality */
    TEST_ASSERT_FALSE(neoc_hash160_equal(&hash1, &hash2));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&hash1, &hash1));
    
    /* Test comparison */
    TEST_ASSERT_TRUE(neoc_hash160_compare(&hash2, &hash1) > 0);
    TEST_ASSERT_TRUE(neoc_hash160_compare(&hash3, &hash1) > 0);
    TEST_ASSERT_TRUE(neoc_hash160_compare(&hash2, &hash3) > 0);
}

void test_hash160_zero_hash(void) {
    neoc_hash160_t zero_hash;
    neoc_error_t result = neoc_hash160_init_zero(&zero_hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_TRUE(neoc_hash160_is_zero(&zero_hash));
    
    /* Verify all bytes are zero */
    uint8_t bytes[NEOC_HASH160_SIZE];
    result = neoc_hash160_to_little_endian_bytes(&zero_hash, bytes, 20);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    for (int i = 0; i < NEOC_HASH160_SIZE; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, bytes[i]);
    }
}

/* ===== HASH256 COMPREHENSIVE TESTS ===== */

void test_hash256_from_valid_hex(void) {
    neoc_hash256_t hash;
    
    /* Test with 0x prefix */
    char hex_with_prefix[67];
    snprintf(hex_with_prefix, sizeof(hex_with_prefix), "0x%s", hash256_valid_hex);
    
    neoc_error_t result = neoc_hash256_from_hex(&hash, hex_with_prefix);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    char output[65];
    result = neoc_hash256_to_hex(&hash, output, sizeof(output), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING(hash256_valid_hex, output);
    
    /* Test without 0x prefix */
    result = neoc_hash256_from_hex(&hash, hash256_valid_hex);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash256_to_hex(&hash, output, sizeof(output), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING(hash256_valid_hex, output);
}

void test_hash256_creation_errors(void) {
    neoc_hash256_t hash;
    neoc_error_t result;
    
    /* Test wrong length - too long (65 chars) */
    result = neoc_hash256_from_hex(&hash, "b804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21ae");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL, result);
    
    result = neoc_hash256_from_hex(&hash, "g804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_HEX, result);
    
    /* Test wrong length - too short (62 chars after 0x = 31 bytes) */
    result = neoc_hash256_from_hex(&hash, "0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a2");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, result);
    
    /* Test wrong length - too long (66 chars after 0x = 33 bytes) */
    result = neoc_hash256_from_hex(&hash, "0xb804a98220c69ab4674e97142beeeb00909113d417b9d6a67c12b71a3974a21a12");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL, result);
}

void test_hash256_from_bytes(void) {
    neoc_hash256_t hash;
    
    /* Convert hex string to bytes */
    uint8_t bytes[NEOC_HASH256_SIZE];
    size_t bytes_length;
    neoc_error_t result = neoc_hex_decode(hash256_valid_hex, bytes, sizeof(bytes), &bytes_length);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_INT(NEOC_HASH256_SIZE, bytes_length);
    
    /* Create hash from bytes */
    result = neoc_hash256_from_bytes(&hash, bytes);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Verify it matches */
    char output[65];
    result = neoc_hash256_to_hex(&hash, output, sizeof(output), false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING(hash256_valid_hex, output);
}

void test_hash256_data_hashing(void) {
    const char* test_data = "test data for hashing";
    neoc_hash256_t hash_single, hash_double;
    
    /* Single hash */
    neoc_error_t result = neoc_hash256_from_data_hash(&hash_single, 
                                                      (const uint8_t*)test_data, 
                                                      strlen(test_data));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_FALSE(neoc_hash256_is_zero(&hash_single));
    
    /* Double hash */
    result = neoc_hash256_from_data_double_hash(&hash_double, 
                                                (const uint8_t*)test_data, 
                                                strlen(test_data));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_FALSE(neoc_hash256_is_zero(&hash_double));
    
    /* They should be different */
    TEST_ASSERT_FALSE(neoc_hash256_equal(&hash_single, &hash_double));
}

void test_hash256_comparison(void) {
    neoc_hash256_t hash1, hash2, hash3;
    
    /* Create test hashes with known ordering */
    const char* hex1 = "1aa274391ab7127ca6d6b917d413919000ebee2b14974e67b49ac62082a904b8";
    const char* hex2 = "b43034ab680d646f8b6ca71647aa6ba167b2eb0b3757e545f6c2715787b13272";
    const char* hex3 = "f4609b99e171190c22adcf70c88a7a14b5b530914d2398287bd8bb7ad95a661c";
    
    neoc_error_t result = neoc_hash256_from_hex(&hash1, hex1);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash256_from_hex(&hash2, hex2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    result = neoc_hash256_from_hex(&hash3, hex3);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    
    /* Test equality */
    TEST_ASSERT_FALSE(neoc_hash256_equal(&hash1, &hash2));
    TEST_ASSERT_TRUE(neoc_hash256_equal(&hash1, &hash1));
    
    /* Test comparison (using big-endian byte comparison) */
    TEST_ASSERT_TRUE(neoc_hash256_compare(&hash1, &hash2) < 0);
    TEST_ASSERT_TRUE(neoc_hash256_compare(&hash3, &hash1) > 0);
    TEST_ASSERT_TRUE(neoc_hash256_compare(&hash3, &hash2) > 0);
}

/* ===== HEX ENCODING COMPREHENSIVE TESTS ===== */

void test_hex_encoding_comprehensive(void) {
    /* Test various data sizes and patterns */
    const uint8_t test_cases[][32] = {
        {0x00}, /* Single zero */
        {0xFF}, /* Single max */
        {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}, /* Classic pattern */
        {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, /* Full range */
    };
    
    const char* expected_results[] = {
        "00",
        "ff", 
        "0123456789abcdef",
        "00112233445566778899aabbccddeeff"
    };
    
    const size_t test_sizes[] = {1, 1, 8, 16};
    
    for (int i = 0; i < 4; i++) {
        char hex_buffer[65];
        neoc_error_t result = neoc_hex_encode(test_cases[i], test_sizes[i], 
                                             hex_buffer, sizeof(hex_buffer), false, false);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
        TEST_ASSERT_EQUAL_STRING(expected_results[i], hex_buffer);
        
        /* Test roundtrip */
        uint8_t decoded_buffer[32];
        size_t decoded_length;
        result = neoc_hex_decode(hex_buffer, decoded_buffer, sizeof(decoded_buffer), &decoded_length);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
        TEST_ASSERT_EQUAL_INT(test_sizes[i], decoded_length);
        TEST_ASSERT_EQUAL_MEMORY(test_cases[i], decoded_buffer, test_sizes[i]);
    }
}

void test_hex_encoding_with_prefix_and_uppercase(void) {
    const uint8_t test_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    char hex_buffer[20];
    
    /* Test with prefix, uppercase */
    neoc_error_t result = neoc_hex_encode(test_data, sizeof(test_data), 
                                         hex_buffer, sizeof(hex_buffer), true, true);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING("0x0123456789ABCDEF", hex_buffer);
    
    /* Test without prefix, uppercase */
    result = neoc_hex_encode(test_data, sizeof(test_data), 
                            hex_buffer, sizeof(hex_buffer), true, false);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING("0123456789ABCDEF", hex_buffer);
    
    /* Test with prefix, lowercase */
    result = neoc_hex_encode(test_data, sizeof(test_data), 
                            hex_buffer, sizeof(hex_buffer), false, true);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    TEST_ASSERT_EQUAL_STRING("0x0123456789abcdef", hex_buffer);
}

/* ===== BASE64 COMPREHENSIVE TESTS ===== */

void test_base64_encoding_comprehensive(void) {
    /* Test vectors from RFC 4648 */
    const char* test_strings[] = {
        "",
        "f",
        "fo",
        "foo", 
        "foob",
        "fooba",
        "foobar"
    };
    
    const char* expected_encoded[] = {
        "",
        "Zg==",
        "Zm8=", 
        "Zm9v",
        "Zm9vYg==",
        "Zm9vYmE=",
        "Zm9vYmFy"
    };
    
    const size_t num_tests = sizeof(test_strings) / sizeof(test_strings[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        const uint8_t* data = (const uint8_t*)test_strings[i];
        size_t data_length = strlen(test_strings[i]);
        
        char* encoded = neoc_base64_encode_alloc(data, data_length);
        if (data_length == 0) {
            /* Empty string may return NULL */
            if (encoded != NULL) {
                TEST_ASSERT_EQUAL_STRING("", encoded);
                neoc_free(encoded);
            }
        } else {
            TEST_ASSERT_NOT_NULL(encoded);
            TEST_ASSERT_EQUAL_STRING(expected_encoded[i], encoded);
            
            /* Test decoding */
            size_t decoded_length;
            uint8_t* decoded = neoc_base64_decode_alloc(encoded, &decoded_length);
            TEST_ASSERT_NOT_NULL(decoded);
            TEST_ASSERT_EQUAL_INT(data_length, decoded_length);
            TEST_ASSERT_EQUAL_MEMORY(data, decoded, data_length);
            
            neoc_free(encoded);
            neoc_free(decoded);
        }
    }
}

/* ===== PERFORMANCE AND EDGE CASE TESTS ===== */

void test_large_data_encoding(void) {
    const size_t large_size = 1024 * 16; /* 16KB */
    uint8_t* large_data = neoc_malloc(large_size);
    TEST_ASSERT_NOT_NULL(large_data);
    
    /* Fill with pattern */
    for (size_t i = 0; i < large_size; i++) {
        large_data[i] = (uint8_t)(i % 256);
    }
    
    /* Test Base64 */
    clock_t start = clock();
    char* encoded = neoc_base64_encode_alloc(large_data, large_size);
    clock_t end = clock();
    double encode_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    TEST_ASSERT_NOT_NULL(encoded);
    printf("Base64 encoding 16KB took: %f seconds\n", encode_time);
    
    /* Test Base64 decode */
    start = clock();
    size_t decoded_length;
    uint8_t* decoded = neoc_base64_decode_alloc(encoded, &decoded_length);
    end = clock();
    double decode_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    TEST_ASSERT_NOT_NULL(decoded);
    TEST_ASSERT_EQUAL_INT(large_size, decoded_length);
    TEST_ASSERT_EQUAL_MEMORY(large_data, decoded, large_size);
    printf("Base64 decoding 16KB took: %f seconds\n", decode_time);
    
    neoc_free(large_data);
    neoc_free(encoded);
    neoc_free(decoded);
}

void test_hash_performance(void) {
    const size_t data_size = 1024 * 64; /* 64KB */
    uint8_t* data = neoc_malloc(data_size);
    TEST_ASSERT_NOT_NULL(data);
    
    /* Fill with pseudo-random data */
    for (size_t i = 0; i < data_size; i++) {
        data[i] = (uint8_t)(i * 197 + 13); /* Simple PRNG */
    }
    
    /* Test Hash256 performance */
    clock_t start = clock();
    neoc_hash256_t hash;
    neoc_error_t result = neoc_hash256_from_data_hash(&hash, data, data_size);
    clock_t end = clock();
    
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    double hash_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Hashing 64KB took: %f seconds\n", hash_time);
    
    /* Test double hash performance */
    start = clock();
    result = neoc_hash256_from_data_double_hash(&hash, data, data_size);
    end = clock();
    
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    double double_hash_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Double hashing 64KB took: %f seconds\n", double_hash_time);
    
    neoc_free(data);
}

/* ===== ERROR HANDLING COMPREHENSIVE TESTS ===== */

void test_comprehensive_error_handling(void) {
    /* Test all error conditions systematically */
    
    /* Hex encoding errors */
    char small_buffer[5];
    neoc_error_t result = neoc_hex_encode((const uint8_t*)"test", 4, 
                                         small_buffer, sizeof(small_buffer), false, false);
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL, result);
    
    /* Hash160 errors */
    neoc_hash160_t hash160;
    result = neoc_hash160_from_hex(NULL, "1234567890abcdef1234567890abcdef12345678");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NULL_POINTER, result);
    
    result = neoc_hash160_from_hex(&hash160, NULL);
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NULL_POINTER, result);
    
    result = neoc_hash160_from_hex(&hash160, "invalid_hex_string");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_HEX, result);
    
    /* Hash256 errors */
    neoc_hash256_t hash256;
    result = neoc_hash256_from_hex(NULL, "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NULL_POINTER, result);
    
    result = neoc_hash256_from_data_hash(&hash256, NULL, 10);
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NULL_POINTER, result);
}

/* Main test runner */
int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== BASE58 COMPREHENSIVE TESTS ===\n");
    RUN_TEST(test_base58_encoding_valid_vectors);
    RUN_TEST(test_base58_decoding_valid_vectors);
    RUN_TEST(test_base58_decoding_invalid_strings);
    RUN_TEST(test_base58_check_encoding);
    RUN_TEST(test_base58_check_decoding);
    RUN_TEST(test_base58_check_invalid_characters);
    RUN_TEST(test_base58_check_invalid_checksum);
    
    printf("\n=== HASH160 COMPREHENSIVE TESTS ===\n");
    RUN_TEST(test_hash160_from_valid_hex);
    RUN_TEST(test_hash160_creation_errors);
    RUN_TEST(test_hash160_to_little_endian_array);
    RUN_TEST(test_hash160_serialization);
    RUN_TEST(test_hash160_comparison);
    RUN_TEST(test_hash160_zero_hash);
    
    printf("\n=== HASH256 COMPREHENSIVE TESTS ===\n");
    RUN_TEST(test_hash256_from_valid_hex);
    RUN_TEST(test_hash256_creation_errors);
    RUN_TEST(test_hash256_from_bytes);
    RUN_TEST(test_hash256_data_hashing);
    RUN_TEST(test_hash256_comparison);
    
    printf("\n=== HEX ENCODING COMPREHENSIVE TESTS ===\n");
    RUN_TEST(test_hex_encoding_comprehensive);
    RUN_TEST(test_hex_encoding_with_prefix_and_uppercase);
    
    printf("\n=== BASE64 COMPREHENSIVE TESTS ===\n");
    RUN_TEST(test_base64_encoding_comprehensive);
    
    printf("\n=== PERFORMANCE TESTS ===\n");
    RUN_TEST(test_large_data_encoding);
    RUN_TEST(test_hash_performance);
    
    printf("\n=== ERROR HANDLING TESTS ===\n");
    RUN_TEST(test_comprehensive_error_handling);
    
    UNITY_END();
}
