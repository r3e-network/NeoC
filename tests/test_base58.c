/**
 * @file test_base58.c
 * @brief Base58 encoding/decoding tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/utils/neoc_base58.h>
#include <string.h>
#include <stdio.h>

// Test vectors from Swift tests
typedef struct {
    const char* decoded;
    const char* encoded;
} base58_test_vector_t;

static const base58_test_vector_t valid_vectors[] = {
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

static const char* invalid_strings[] = {
    "0",    // Invalid character
    "O",    // Invalid character
    "I",    // Invalid character
    "l",    // Invalid character (lowercase L)
    "3mJr0", // Contains '0'
    "O3yxU", // Contains 'O'
    "3sNI",  // Contains 'I'
    "4kl8",  // Contains lowercase 'l'
    "0OIl",  // Multiple invalid chars
    "!@#$%^&*()-_=+~`" // Special characters
};

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== BASE58 ENCODING TESTS ===== */

void test_base58_encoding_for_valid_strings(void) {
    int num_vectors = sizeof(valid_vectors) / sizeof(valid_vectors[0]);
    
    for (int i = 0; i < num_vectors; i++) {
        const char* decoded = valid_vectors[i].decoded;
        const char* expected = valid_vectors[i].encoded;
        
        size_t decoded_len = strlen(decoded);

        // Calculate required buffer size
        size_t encoded_size = neoc_base58_encode_buffer_size(decoded_len);
        char* encoded = neoc_malloc(encoded_size);
        TEST_ASSERT_NOT_NULL(encoded);
        
        // Encode
        neoc_error_t err = neoc_base58_encode((const uint8_t*)decoded, decoded_len, 
                                               encoded, encoded_size);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Compare
        TEST_ASSERT_EQUAL_STRING(expected, encoded);
        
        neoc_free(encoded);
    }
}

void test_base58_decoding_for_valid_strings(void) {
    int num_vectors = sizeof(valid_vectors) / sizeof(valid_vectors[0]);
    
    for (int i = 0; i < num_vectors; i++) {
        const char* expected = valid_vectors[i].decoded;
        const char* encoded = valid_vectors[i].encoded;
        
        // Calculate required buffer size
        size_t decoded_size = neoc_base58_decode_buffer_size(encoded);
        if (decoded_size == 0) {
            decoded_size = strlen(encoded) + 1; // Fallback size
        }
        uint8_t* decoded = neoc_malloc(decoded_size);
        TEST_ASSERT_NOT_NULL(decoded);
        
        size_t actual_size;
        neoc_error_t err = neoc_base58_decode(encoded, decoded, decoded_size, &actual_size);
        
        if (strlen(encoded) == 0) {
            // Empty string might fail or return empty
            if (err == NEOC_SUCCESS) {
                TEST_ASSERT_EQUAL_INT(0, actual_size);
            }
        } else {
            TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
            
            // Compare as string
            char* result = neoc_malloc(actual_size + 1);
            memcpy(result, decoded, actual_size);
            result[actual_size] = '\0';
            TEST_ASSERT_EQUAL_STRING(expected, result);
            neoc_free(result);
        }
        
        neoc_free(decoded);
    }
}

void test_base58_decoding_for_invalid_strings(void) {
    int num_invalid = sizeof(invalid_strings) / sizeof(invalid_strings[0]);
    
    for (int i = 0; i < num_invalid; i++) {
        const char* invalid = invalid_strings[i];
        
        size_t decoded_size = neoc_base58_decode_buffer_size(invalid);
        if (decoded_size == 0) {
            // Already detected as invalid
            continue;
        }
        
        uint8_t* decoded = neoc_malloc(decoded_size + 1);
        TEST_ASSERT_NOT_NULL(decoded);
        
        size_t actual_size;
        neoc_error_t err = neoc_base58_decode(invalid, decoded, decoded_size, &actual_size);
        
        // Should fail for invalid characters
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        
        neoc_free(decoded);
    }
}

void test_base58_check_encoding(void) {
    uint8_t input_data[] = {
        6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218, 
        133, 13, 109, 40, 194, 236, 153, 44, 61, 157, 254
    };
    const char* expected_output = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";
    
    size_t encoded_size = neoc_base58_check_encode_buffer_size(sizeof(input_data));
    char* encoded = neoc_malloc(encoded_size);
    TEST_ASSERT_NOT_NULL(encoded);
    
    neoc_error_t err = neoc_base58_check_encode(input_data, sizeof(input_data), 
                                                 encoded, encoded_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING(expected_output, encoded);
    
    neoc_free(encoded);
}

void test_base58_check_decoding(void) {
    const char* input_string = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";
    uint8_t expected_output[] = {
        6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218, 
        133, 13, 109, 40, 194, 236, 153, 44, 61, 157, 254
    };
    
    size_t decoded_size = neoc_base58_check_decode_buffer_size(input_string);
    uint8_t* decoded = neoc_malloc(decoded_size);
    TEST_ASSERT_NOT_NULL(decoded);
    
    size_t actual_size;
    neoc_error_t err = neoc_base58_check_decode(input_string, decoded, 
                                                 decoded_size, &actual_size);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(sizeof(expected_output), actual_size);
    TEST_ASSERT_EQUAL_MEMORY(expected_output, decoded, actual_size);
    
    neoc_free(decoded);
}

void test_base58_check_decoding_with_invalid_characters(void) {
    const char* invalid = "0oO1lL";
    
    size_t decoded_size = neoc_base58_check_decode_buffer_size(invalid);
    if (decoded_size > 0) {
        uint8_t* decoded = neoc_malloc(decoded_size);
        TEST_ASSERT_NOT_NULL(decoded);
        
        size_t actual_size;
        neoc_error_t err = neoc_base58_check_decode(invalid, decoded, 
                                                     decoded_size, &actual_size);
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        
        neoc_free(decoded);
    }
}

void test_base58_check_decoding_with_invalid_checksum(void) {
    // Last character changed from 'w' to 'W' to break checksum
    const char* invalid = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtrW";
    
    size_t decoded_size = neoc_base58_check_decode_buffer_size(invalid);
    uint8_t* decoded = neoc_malloc(decoded_size);
    TEST_ASSERT_NOT_NULL(decoded);
    
    size_t actual_size;
    neoc_error_t err = neoc_base58_check_decode(invalid, decoded, 
                                                 decoded_size, &actual_size);
    // Should fail due to checksum mismatch
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_free(decoded);
}

void test_base58_round_trip(void) {
    // Test various byte patterns for round-trip encoding/decoding
    uint8_t test_patterns[][38] = {
        {0x00, 0x00, 0x00, 0x00}, // Leading zeros
        {0xFF, 0xFF, 0xFF, 0xFF}, // All ones
        {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09},
        // Pattern that was failing: repeated 0xAB
        {0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB},
        // WIF payload example (0x80 || 32 * 0x00 || 0x01 || checksum 0x69f436de)
        {
            0x80,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x69, 0xF4, 0x36, 0xDE
        }
    };
    size_t pattern_sizes[] = {4, 4, 8, 10, 8, 38};
    
    for (int i = 0; i < 6; i++) {
        // Encode
        size_t encoded_size = neoc_base58_encode_buffer_size(pattern_sizes[i]);
        char* encoded = neoc_malloc(encoded_size);
        TEST_ASSERT_NOT_NULL(encoded);
        
        neoc_error_t err = neoc_base58_encode(test_patterns[i], pattern_sizes[i],
                                               encoded, encoded_size);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Decode
        size_t decoded_size = neoc_base58_decode_buffer_size(encoded);
        uint8_t* decoded = neoc_malloc(decoded_size);
        TEST_ASSERT_NOT_NULL(decoded);
        
        size_t actual_size;
        err = neoc_base58_decode(encoded, decoded, decoded_size, &actual_size);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);

        char size_message[64];
        snprintf(size_message, sizeof(size_message),
                 "pattern %d expected %zu got %zu", i, pattern_sizes[i], actual_size);
        UNITY_TEST_ASSERT_EQUAL_INT((int)pattern_sizes[i], (int)actual_size,
                                    __LINE__, size_message);

        if (memcmp(test_patterns[i], decoded, actual_size) != 0) {
            size_t mismatch_index = 0;
            while (mismatch_index < actual_size &&
                   test_patterns[i][mismatch_index] == decoded[mismatch_index]) {
                mismatch_index++;
            }

            char diff_message[256];
            if (mismatch_index < actual_size) {
                snprintf(diff_message, sizeof(diff_message),
                         "pattern %d encoded %s byte %zu expected %02X got %02X (size %zu)",
                         i, encoded, mismatch_index,
                         test_patterns[i][mismatch_index], decoded[mismatch_index],
                         actual_size);
            } else {
                snprintf(diff_message, sizeof(diff_message),
                         "pattern %d mismatch after %zu bytes (encoded %s)",
                         i, actual_size, encoded);
            }
            TEST_FAIL_MESSAGE(diff_message);
        }

        TEST_ASSERT_EQUAL_MEMORY(test_patterns[i], decoded, actual_size);
        
        neoc_free(encoded);
        neoc_free(decoded);
    }
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_base58_encoding_for_valid_strings);
    RUN_TEST(test_base58_decoding_for_valid_strings);
    RUN_TEST(test_base58_decoding_for_invalid_strings);
    RUN_TEST(test_base58_check_encoding);
    RUN_TEST(test_base58_check_decoding);
    RUN_TEST(test_base58_check_decoding_with_invalid_characters);
    RUN_TEST(test_base58_check_decoding_with_invalid_checksum);
    RUN_TEST(test_base58_round_trip);
    
    UNITY_END();
    return 0;
}
