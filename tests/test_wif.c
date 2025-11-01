/**
 * @file test_wif.c
 * @brief WIF (Wallet Import Format) tests converted from Swift and aligned with Unity style.
 */

#include "unity.h"
#include <string.h>
#include <neoc/neoc.h>
#include <neoc/neoc_memory.h>
#include <neoc/crypto/wif.h>
#include <neoc/utils/neoc_base58.h>
#include <neoc/utils/neoc_hex.h>

#define VALID_WIF "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13A"
#define PRIVATE_KEY_HEX "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3a3"

static void decode_private_key_hex(uint8_t *buffer, size_t buffer_size) {
    size_t decoded_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hex_decode(PRIVATE_KEY_HEX, buffer, buffer_size, &decoded_len));
    TEST_ASSERT_EQUAL_UINT(32u, decoded_len);
}

static char *encode_mutated_base58(uint8_t *payload, size_t length) {
    size_t encoded_capacity = neoc_base58_encode_buffer_size(length);
    char *encoded = neoc_calloc(encoded_capacity, 1);
    TEST_ASSERT_NOT_NULL(encoded);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_base58_encode(payload, length, encoded, encoded_capacity));
    return encoded;
}

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

void test_valid_wif_to_private_key(void) {
    uint8_t *private_key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_wif_to_private_key(VALID_WIF, &private_key));
    TEST_ASSERT_NOT_NULL(private_key);

    uint8_t expected[32];
    decode_private_key_hex(expected, sizeof(expected));
    TEST_ASSERT_EQUAL_MEMORY(expected, private_key, sizeof(expected));

    neoc_free(private_key);
}

void test_wrongly_sized_wifs(void) {
    const char *too_large = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13Ahc7S";
    const char *too_small = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWML";

    uint8_t *private_key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_FORMAT,
                          neoc_wif_to_private_key(too_large, &private_key));
    TEST_ASSERT_NULL(private_key);

    private_key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_FORMAT,
                          neoc_wif_to_private_key(too_small, &private_key));
    TEST_ASSERT_NULL(private_key);
}

void test_wrong_first_byte_wif(void) {
    size_t decoded_capacity = neoc_base58_decode_buffer_size(VALID_WIF);
    uint8_t *decoded = neoc_calloc(decoded_capacity, 1);
    size_t decoded_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_base58_decode(VALID_WIF, decoded, decoded_capacity, &decoded_len));
    TEST_ASSERT_TRUE(decoded_len > 0);

    decoded[0] ^= 0x01U;
    char *mutated_wif = encode_mutated_base58(decoded, decoded_len);

    uint8_t *private_key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_FORMAT,
                          neoc_wif_to_private_key(mutated_wif, &private_key));
    TEST_ASSERT_NULL(private_key);

    neoc_free(mutated_wif);
    neoc_free(decoded);
}

void test_wrong_byte33_wif(void) {
    size_t decoded_capacity = neoc_base58_decode_buffer_size(VALID_WIF);
    uint8_t *decoded = neoc_calloc(decoded_capacity, 1);
    size_t decoded_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_base58_decode(VALID_WIF, decoded, decoded_capacity, &decoded_len));
    TEST_ASSERT_TRUE(decoded_len > 33);

    decoded[33] ^= 0x01U;
    char *mutated_wif = encode_mutated_base58(decoded, decoded_len);

    uint8_t *private_key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_FORMAT,
                          neoc_wif_to_private_key(mutated_wif, &private_key));
    TEST_ASSERT_NULL(private_key);

    neoc_free(mutated_wif);
    neoc_free(decoded);
}

void test_valid_private_key_to_wif(void) {
    uint8_t private_key[32];
    decode_private_key_hex(private_key, sizeof(private_key));

    char *wif = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_private_key_to_wif(private_key, &wif));
    TEST_ASSERT_NOT_NULL(wif);
    TEST_ASSERT_EQUAL_STRING(VALID_WIF, wif);

    neoc_free(wif);
}

void test_wrongly_sized_private_key(void) {
    const char *wrong_hex = "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3";
    uint8_t buffer[32];
    size_t decoded_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hex_decode(wrong_hex, buffer, sizeof(buffer), &decoded_len));
    TEST_ASSERT_EQUAL_UINT(31u, decoded_len);

    char *wif = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_SIZE,
                          neoc_private_key_to_wif_len(buffer, decoded_len, &wif));
    TEST_ASSERT_NULL(wif);
}

void test_wif_round_trip(void) {
    static const uint8_t test_keys[][32] = {
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
        },
        {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE
        },
        {
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
            0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
        }
    };

    for (size_t i = 0; i < sizeof(test_keys) / sizeof(test_keys[0]); ++i) {
        char *wif = NULL;
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_private_key_to_wif(test_keys[i], &wif));
        TEST_ASSERT_NOT_NULL(wif);

        uint8_t *decoded_key = NULL;
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_wif_to_private_key(wif, &decoded_key));
        TEST_ASSERT_NOT_NULL(decoded_key);
        TEST_ASSERT_EQUAL_MEMORY(test_keys[i], decoded_key, 32);

        neoc_free(decoded_key);
        neoc_free(wif);
    }
}

void test_wif_format_validation(void) {
    const char *invalid_wifs[] = {
        "InvalidWIF",
        "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ",
        "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWX",
        "",
        "L"
    };

    for (size_t i = 0; i < sizeof(invalid_wifs) / sizeof(invalid_wifs[0]); ++i) {
        uint8_t *private_key = NULL;
        neoc_error_t err = neoc_wif_to_private_key(invalid_wifs[i], &private_key);
        TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
        TEST_ASSERT_NULL(private_key);
    }
}

int main(void) {
    UNITY_BEGIN();
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
