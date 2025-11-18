/**
 * @file test_hash160.c
 * @brief Modernized Hash160 unit tests derived from Hash160Tests.swift
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "neoc/neoc.h"
#include "neoc/neo_constants.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/hex.h"

static const char *DEFAULT_ACCOUNT_PUBLIC_KEY =
    "033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b";
static const char *DEFAULT_ACCOUNT_ADDRESS = "NM7Aky765FG8NhhwtxjXRx7jEL1cnw7PBP";
static const char *COMMITTEE_ACCOUNT_SCRIPT_HASH = "05859de95ccbbd5668e0f055b208273634d4657f";

static void setUp(void) {
    assert(neoc_init() == NEOC_SUCCESS);
}

static void tearDown(void) {
    neoc_cleanup();
}

static void assert_hash_string(const neoc_hash160_t *hash, const char *expected) {
    char buffer[NEOC_HASH160_STRING_LENGTH];
    assert(neoc_hash160_to_string(hash, buffer, sizeof(buffer)) == NEOC_SUCCESS);
    assert(strcmp(buffer, expected) == 0);
}

static void assert_hex_bytes(const uint8_t *data, size_t len, const char *expected) {
    char buffer[NEOC_HASH160_STRING_LENGTH];
    assert(neoc_hex_encode(data, len, buffer, sizeof(buffer), false, false) == NEOC_SUCCESS);
    assert(strcmp(buffer, expected) == 0);
}

static void test_from_valid_hash(void) {
    printf("Testing create from valid hash...\n");

    neoc_hash160_t hash1;
    assert(neoc_hash160_from_string("0x23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash1) == NEOC_SUCCESS);
    assert_hash_string(&hash1, "23ba2703c53263e8d6e522dc32203339dcd8eee9");

    neoc_hash160_t hash2;
    assert(neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash2) == NEOC_SUCCESS);
    assert_hash_string(&hash2, "23ba2703c53263e8d6e522dc32203339dcd8eee9");

    printf("  ✅ Create from valid hash test passed\n");
}

static void test_creation_errors(void) {
    printf("Testing creation error handling...\n");

    neoc_hash160_t hash;
    assert(neoc_hash160_from_string("0x23ba2703c53263e8d6e522dc32203339dcd8eee", &hash) != NEOC_SUCCESS);
    assert(neoc_hash160_from_string("g3ba2703c53263e8d6e522dc32203339dcd8eee9", &hash) != NEOC_SUCCESS);
    assert(neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8ee", &hash) != NEOC_SUCCESS);
    assert(neoc_hash160_from_string("c56f33fc6ecfcd0c225c4ab356fee59390af8560be0e930faebe74a6daff7c9b", &hash) != NEOC_SUCCESS);

    printf("  ✅ Creation error handling test passed\n");
}

static void test_to_little_endian_bytes(void) {
    printf("Testing to little endian bytes...\n");

    neoc_hash160_t hash;
    assert(neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash) == NEOC_SUCCESS);

    uint8_t buffer[NEOC_HASH160_SIZE];
    assert(neoc_hash160_to_little_endian_bytes(&hash, buffer, sizeof(buffer)) == NEOC_SUCCESS);

    uint8_t expected[NEOC_HASH160_SIZE];
    size_t expected_len = 0;
    assert(neoc_hex_decode("23ba2703c53263e8d6e522dc32203339dcd8eee9", expected, sizeof(expected), &expected_len) ==
           NEOC_SUCCESS);
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        assert(buffer[i] == expected[NEOC_HASH160_SIZE - 1 - i]);
    }

    printf("  ✅ To little endian bytes test passed\n");
}

static neoc_binary_writer_t *create_writer(void) {
    neoc_binary_writer_t *writer = NULL;
    assert(neoc_binary_writer_create(64, true, &writer) == NEOC_SUCCESS);
    return writer;
}

static void test_serialize_and_deserialize(void) {
    printf("Testing serialize and deserialize...\n");

    const char *hash_str = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
    neoc_hash160_t hash;
    assert(neoc_hash160_from_string(hash_str, &hash) == NEOC_SUCCESS);

    neoc_binary_writer_t *writer = create_writer();
    assert(neoc_hash160_serialize(&hash, writer) == NEOC_SUCCESS);

    uint8_t *data = NULL;
    size_t data_len = 0;
    assert(neoc_binary_writer_to_array(writer, &data, &data_len) == NEOC_SUCCESS);
    assert(data_len == NEOC_HASH160_SIZE);

    neoc_binary_reader_t *reader = NULL;
    assert(neoc_binary_reader_create(data, data_len, &reader) == NEOC_SUCCESS);

    neoc_hash160_t deserialized;
    assert(neoc_hash160_deserialize(&deserialized, reader) == NEOC_SUCCESS);
    assert_hash_string(&deserialized, hash_str);

    neoc_binary_reader_free(reader);
    neoc_binary_writer_free(writer);
    free(data);

    printf("  ✅ Serialize and deserialize test passed\n");
}

static void test_equals(void) {
    printf("Testing equals...\n");

    uint8_t script1[4] = {0x01, 0xa4, 0x02, 0xd8};
    uint8_t script2[4] = {0xd8, 0x02, 0xa4, 0x01};

    neoc_hash160_t hash1;
    neoc_hash160_t hash2;
    assert(neoc_hash160_from_script(&hash1, script1, sizeof(script1)) == NEOC_SUCCESS);
    assert(neoc_hash160_from_script(&hash2, script2, sizeof(script2)) == NEOC_SUCCESS);

    assert(neoc_hash160_equal(&hash1, &hash1));
    assert(!neoc_hash160_equal(&hash1, &hash2));
    assert(!neoc_hash160_equal(&hash2, &hash1));

    printf("  ✅ Equals test passed\n");
}

static void test_from_valid_address(void) {
    printf("Testing from valid address...\n");

    neoc_hash160_t hash;
    assert(neoc_hash160_from_address(&hash, "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke") == NEOC_SUCCESS);

    uint8_t buffer[NEOC_HASH160_SIZE];
    assert(neoc_hash160_to_little_endian_bytes(&hash, buffer, sizeof(buffer)) == NEOC_SUCCESS);
    assert_hex_bytes(buffer, sizeof(buffer), "09a55874c2da4b86e5d49ff530a1b153eb12c7d6");

    printf("  ✅ From valid address test passed\n");
}

static void test_from_invalid_address(void) {
    printf("Testing from invalid address...\n");

    neoc_hash160_t hash;
    assert(neoc_hash160_from_address(&hash, "NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8keas") != NEOC_SUCCESS);

    printf("  ✅ From invalid address test passed\n");
}

static void test_from_public_key_bytes(void) {
    printf("Testing from public key bytes...\n");

    uint8_t key_bytes[NEOC_PUBLIC_KEY_SIZE_COMPRESSED];
    size_t key_len = 0;
    assert(neoc_hex_decode("035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50",
                           key_bytes, sizeof(key_bytes), &key_len) == NEOC_SUCCESS);
    assert(key_len == NEOC_PUBLIC_KEY_SIZE_COMPRESSED);

    neoc_hash160_t key_hash;
    assert(neoc_hash160_from_public_key(&key_hash, key_bytes) == NEOC_SUCCESS);

    uint8_t multisig_keys[1][NEOC_PUBLIC_KEY_SIZE_COMPRESSED];
    size_t multi_len = 0;
    assert(neoc_hex_decode(DEFAULT_ACCOUNT_PUBLIC_KEY, multisig_keys[0], sizeof(multisig_keys[0]), &multi_len) ==
           NEOC_SUCCESS);
    assert(multi_len == NEOC_PUBLIC_KEY_SIZE_COMPRESSED);

    neoc_hash160_t multisig_hash;
    assert(neoc_hash160_from_public_keys(
               &multisig_hash,
               (const uint8_t (*)[NEOC_PUBLIC_KEY_SIZE_COMPRESSED])multisig_keys,
               1, 1) == NEOC_SUCCESS);
    assert_hash_string(&multisig_hash, COMMITTEE_ACCOUNT_SCRIPT_HASH);

    printf("  ✅ From public key bytes test passed\n");
}

static void test_from_contract_script(void) {
    printf("Testing from contract script...\n");

    uint8_t script[256];
    size_t script_len = 0;
    assert(neoc_hex_decode(
               "110c21026aa8fe6b4360a67a530e23c08c6a72525afde34719c5436f9d3ced759f939a3d110b41138defaf",
               script, sizeof(script), &script_len) == NEOC_SUCCESS);

    neoc_hash160_t hash;
    assert(neoc_hash160_from_script(&hash, script, script_len) == NEOC_SUCCESS);
    assert_hash_string(&hash, "afaed076854454449770763a628f379721ea9808");

    uint8_t buffer[NEOC_HASH160_SIZE];
    assert(neoc_hash160_to_little_endian_bytes(&hash, buffer, sizeof(buffer)) == NEOC_SUCCESS);
    assert_hex_bytes(buffer, sizeof(buffer), "0898ea2197378f623a7670974454448576d0aeaf");

    printf("  ✅ From contract script test passed\n");
}

static void test_to_address(void) {
    printf("Testing to address...\n");

    uint8_t pubkey_bytes[NEOC_PUBLIC_KEY_SIZE_COMPRESSED];
    size_t pubkey_len = 0;
    assert(neoc_hex_decode(DEFAULT_ACCOUNT_PUBLIC_KEY, pubkey_bytes, sizeof(pubkey_bytes), &pubkey_len) ==
           NEOC_SUCCESS);

    neoc_hash160_t hash;
    assert(neoc_hash160_from_public_key(&hash, pubkey_bytes) == NEOC_SUCCESS);

    char address[NEOC_ADDRESS_MAX_LENGTH];
    assert(neoc_hash160_to_address(&hash, address, sizeof(address)) == NEOC_SUCCESS);
    assert(strcmp(address, DEFAULT_ACCOUNT_ADDRESS) == 0);

    printf("  ✅ To address test passed\n");
}

static void test_compare_to(void) {
    printf("Testing compare to...\n");

    uint8_t script1[4] = {0x01, 0xa4, 0x02, 0xd8};
    uint8_t script2[4] = {0xd8, 0x02, 0xa4, 0x01};
    uint8_t script3[4] = {0xa7, 0xb3, 0xa1, 0x91};

    neoc_hash160_t hash1, hash2, hash3;
    assert(neoc_hash160_from_script(&hash1, script1, sizeof(script1)) == NEOC_SUCCESS);
    assert(neoc_hash160_from_script(&hash2, script2, sizeof(script2)) == NEOC_SUCCESS);
    assert(neoc_hash160_from_script(&hash3, script3, sizeof(script3)) == NEOC_SUCCESS);

    assert(neoc_hash160_compare(&hash2, &hash1) != 0);
    assert(neoc_hash160_compare(&hash1, &hash1) == 0);

    printf("  ✅ Compare to test passed\n");
}

static void test_size(void) {
    printf("Testing underlying size...\n");

    neoc_hash160_t hash;
    assert(neoc_hash160_from_string("23ba2703c53263e8d6e522dc32203339dcd8eee9", &hash) == NEOC_SUCCESS);
    assert(sizeof(hash.data) == NEOC_HASH160_SIZE);

    printf("  ✅ Size test passed\n");
}

int main(void) {
    printf("\n=== Hash160Tests Tests ===\n\n");
    setUp();

    test_from_valid_hash();
    test_creation_errors();
    test_to_little_endian_bytes();
    test_serialize_and_deserialize();
    test_equals();
    test_from_valid_address();
    test_from_invalid_address();
    test_from_public_key_bytes();
    test_from_contract_script();
    test_to_address();
    test_compare_to();
    test_size();

    tearDown();
    printf("\n✅ All Hash160 tests passed!\n\n");
    return 0;
}
