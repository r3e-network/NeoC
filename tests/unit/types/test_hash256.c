/**
 * @file test_hash256.c
 * @brief Hash256 unit tests adapted from NeoSwift Hash256Tests.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "neoc/neoc.h"
#include "neoc/neo_constants.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/utils/hex.h"

static const char *HELLO_SHA256 = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
static const char *HELLO_SHA256D = "9595c9df90075148eb06860365df33584b75bff782a510c6cd4883a419833d50";

static void setUp(void) {
    assert(neoc_init() == NEOC_SUCCESS);
}

static void tearDown(void) {
    neoc_cleanup();
}

static void assert_hash_string(const neoc_hash256_t *hash, const char *expected) {
    char buffer[NEOC_HASH256_STRING_LENGTH];
    assert(neoc_hash256_to_string(hash, buffer, sizeof(buffer)) == NEOC_SUCCESS);
    assert(strcmp(buffer, expected) == 0);
}

static void test_from_valid_string(void) {
    printf("Testing Hash256 from string...\n");

    neoc_hash256_t hash;
    assert(neoc_hash256_from_string(HELLO_SHA256, &hash) == NEOC_SUCCESS);
    assert_hash_string(&hash, HELLO_SHA256);

    assert(neoc_hash256_from_string("0x9595c9df90075148eb06860365df33584b75bff782a510c6cd4883a419833d50",
                                    &hash) == NEOC_SUCCESS);
    assert_hash_string(&hash, HELLO_SHA256D);

    printf("  ✅ Hash256 from string test passed\n");
}

static void test_invalid_string(void) {
    printf("Testing invalid Hash256 strings...\n");

    neoc_hash256_t hash;
    assert(neoc_hash256_from_string("1234", &hash) != NEOC_SUCCESS);
    assert(neoc_hash256_from_string("0xzz0024dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824", &hash) !=
           NEOC_SUCCESS);

    printf("  ✅ Hash256 error handling test passed\n");
}

static void test_to_little_endian(void) {
    printf("Testing little-endian conversion...\n");

    neoc_hash256_t hash;
    assert(neoc_hash256_from_string(HELLO_SHA256, &hash) == NEOC_SUCCESS);

    uint8_t buffer[NEOC_HASH256_SIZE];
    assert(neoc_hash256_to_little_endian_bytes(&hash, buffer, sizeof(buffer)) == NEOC_SUCCESS);

    uint8_t expected[NEOC_HASH256_SIZE];
    size_t expected_len = 0;
    assert(neoc_hex_decode(HELLO_SHA256, expected, sizeof(expected), &expected_len) == NEOC_SUCCESS);
    for (size_t i = 0; i < NEOC_HASH256_SIZE; ++i) {
        assert(buffer[i] == expected[NEOC_HASH256_SIZE - 1 - i]);
    }

    printf("  ✅ Little-endian conversion test passed\n");
}

static neoc_binary_writer_t *create_writer(void) {
    neoc_binary_writer_t *writer = NULL;
    assert(neoc_binary_writer_create(64, true, &writer) == NEOC_SUCCESS);
    return writer;
}

static void test_serialize_deserialize(void) {
    printf("Testing Hash256 serialize/deserialize...\n");

    neoc_hash256_t hash;
    assert(neoc_hash256_from_string(HELLO_SHA256, &hash) == NEOC_SUCCESS);

    neoc_binary_writer_t *writer = create_writer();
    assert(neoc_hash256_serialize(&hash, writer) == NEOC_SUCCESS);

    uint8_t *data = NULL;
    size_t data_len = 0;
    assert(neoc_binary_writer_to_array(writer, &data, &data_len) == NEOC_SUCCESS);
    assert(data_len == NEOC_HASH256_SIZE);

    neoc_binary_reader_t *reader = NULL;
    assert(neoc_binary_reader_create(data, data_len, &reader) == NEOC_SUCCESS);

    neoc_hash256_t deserialized;
    assert(neoc_hash256_deserialize(&deserialized, reader) == NEOC_SUCCESS);
    assert_hash_string(&deserialized, HELLO_SHA256);

    free(data);
    neoc_binary_reader_free(reader);
    neoc_binary_writer_free(writer);

    printf("  ✅ Serialize/deserialize test passed\n");
}

static void test_hashing(void) {
    printf("Testing SHA256 and double SHA256 creation...\n");

    const uint8_t data[] = "hello";
    neoc_hash256_t hash;
    assert(neoc_hash256_from_data_hash(&hash, data, sizeof("hello") - 1) == NEOC_SUCCESS);
    assert_hash_string(&hash, HELLO_SHA256);

    assert(neoc_hash256_from_data_double_hash(&hash, data, sizeof("hello") - 1) == NEOC_SUCCESS);
    assert_hash_string(&hash, HELLO_SHA256D);

    printf("  ✅ Hash creation tests passed\n");
}

static void test_compare_and_zero(void) {
    printf("Testing comparison and zero checks...\n");

    neoc_hash256_t hash1;
    neoc_hash256_t hash2;
    assert(neoc_hash256_from_string(HELLO_SHA256, &hash1) == NEOC_SUCCESS);
    assert(neoc_hash256_copy(&hash2, &hash1) == NEOC_SUCCESS);

    assert(neoc_hash256_equal(&hash1, &hash2));
    assert(neoc_hash256_compare(&hash1, &hash2) == 0);

    neoc_hash256_t zero;
    assert(neoc_hash256_init_zero(&zero) == NEOC_SUCCESS);
    assert(neoc_hash256_is_zero(&zero));
    assert(!neoc_hash256_is_zero(&hash1));

    printf("  ✅ Compare/zero tests passed\n");
}

int main(void) {
    printf("\n=== Hash256Tests ===\n\n");
    setUp();

    test_from_valid_string();
    test_invalid_string();
    test_to_little_endian();
    test_serialize_deserialize();
    test_hashing();
    test_compare_and_zero();

    tearDown();
    printf("\n✅ All Hash256 tests passed!\n\n");
    return 0;
}
