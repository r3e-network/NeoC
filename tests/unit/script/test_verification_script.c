#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/script/verification_script.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/neoc_hex.h"

static const char *PUBKEY_A = "033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b";
static const char *PUBKEY_B = "0265bf906bf385fbf3f777832e55a87991bcfbe19b097fb7c5ca2e4025a4d5e5d6";
static const char *PUBKEY_C = "031b84c5567b126440995d3ed5aaba0565d71e1834604819ff9c17f5e9d5dd078f";

static neoc_ec_public_key_t *create_public_key(const char *hex) {
    uint8_t buffer[NEOC_PUBLIC_KEY_SIZE_COMPRESSED] = {0};
    size_t decoded_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hex_decode(hex, buffer, sizeof(buffer), &decoded_len));
    TEST_ASSERT_EQUAL_UINT(NEOC_PUBLIC_KEY_SIZE_COMPRESSED, decoded_len);
    neoc_ec_public_key_t *key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_ec_public_key_from_bytes(buffer, decoded_len, &key));
    return key;
}

static void free_public_key_array(neoc_ec_public_key_t **keys, size_t count) {
    if (!keys) return;
    for (size_t i = 0; i < count; ++i) {
        neoc_ec_public_key_free(keys[i]);
    }
    neoc_free(keys);
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_verification_script_single_sig_properties(void) {
    neoc_ec_public_key_t *pubkey = create_public_key(PUBKEY_A);
    neoc_verification_script_t *script = NULL;
    const uint8_t *script_bytes = NULL;
    size_t script_len = 0;
    neoc_hash160_t expected_hash = {{0}};
    neoc_hash160_t actual_hash = {{0}};
    neoc_ec_public_key_t **keys = NULL;
    size_t key_count = 0;
    char *address = NULL;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_create_single_sig(pubkey, &script));
    TEST_ASSERT_TRUE(neoc_verification_script_is_single_sig(script));
    TEST_ASSERT_FALSE(neoc_verification_script_is_multi_sig(script));
    TEST_ASSERT_EQUAL_UINT(41, neoc_verification_script_get_size(script));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_script(script, &script_bytes, &script_len));
    TEST_ASSERT_EQUAL_UINT(40, script_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hash160_from_script(&expected_hash, script_bytes, script_len));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_script_hash(script, &actual_hash));
    TEST_ASSERT_EQUAL_MEMORY(expected_hash.data, actual_hash.data, NEOC_HASH160_SIZE);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_public_keys(script, &keys, &key_count));
    TEST_ASSERT_EQUAL_UINT(1, key_count);
    uint8_t *encoded_original = NULL;
    uint8_t *encoded_returned = NULL;
    size_t encoded_len = 0;
    size_t returned_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_ec_public_key_get_encoded(pubkey, true, &encoded_original, &encoded_len));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_ec_public_key_get_encoded(keys[0], true, &encoded_returned, &returned_len));
    TEST_ASSERT_EQUAL_UINT(encoded_len, returned_len);
    TEST_ASSERT_EQUAL_MEMORY(encoded_original, encoded_returned, encoded_len);
    neoc_free(encoded_original);
    neoc_free(encoded_returned);
    free_public_key_array(keys, key_count);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_address(script, &address));
    TEST_ASSERT_NOT_NULL(address);
    neoc_free(address);

    neoc_verification_script_free(script);
    neoc_ec_public_key_free(pubkey);
}

void test_verification_script_multi_sig_flow(void) {
    neoc_ec_public_key_t *keys_in[3] = {
        create_public_key(PUBKEY_A),
        create_public_key(PUBKEY_C),
        create_public_key(PUBKEY_B)
    };
    const char *expected_order[] = {PUBKEY_B, PUBKEY_C, PUBKEY_A};
    neoc_verification_script_t *script = NULL;
    int threshold = 0;
    int nr_accounts = 0;
    neoc_ec_public_key_t **keys_out = NULL;
    size_t key_count = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_create_multi_sig(keys_in,
                                                                    3,
                                                                    2,
                                                                    &script));
    TEST_ASSERT_TRUE(neoc_verification_script_is_multi_sig(script));
    TEST_ASSERT_FALSE(neoc_verification_script_is_single_sig(script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_signing_threshold(script, &threshold));
    TEST_ASSERT_EQUAL_INT(2, threshold);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_nr_accounts(script, &nr_accounts));
    TEST_ASSERT_EQUAL_INT(3, nr_accounts);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_get_public_keys(script,
                                                                   &keys_out,
                                                                   &key_count));
    TEST_ASSERT_EQUAL_UINT(3, key_count);
    for (size_t i = 0; i < key_count; ++i) {
        uint8_t expected_bytes[NEOC_PUBLIC_KEY_SIZE_COMPRESSED] = {0};
        size_t expected_len = 0;
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                              neoc_hex_decode(expected_order[i],
                                              expected_bytes,
                                              sizeof(expected_bytes),
                                              &expected_len));
        uint8_t *actual_bytes = NULL;
        size_t actual_len = 0;
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                              neoc_ec_public_key_get_encoded(keys_out[i], true, &actual_bytes, &actual_len));
        TEST_ASSERT_EQUAL_UINT(expected_len, actual_len);
        TEST_ASSERT_EQUAL_MEMORY(expected_bytes, actual_bytes, actual_len);
        neoc_free(actual_bytes);
    }
    free_public_key_array(keys_out, key_count);

    neoc_verification_script_free(script);
    for (size_t i = 0; i < 3; ++i) {
        neoc_ec_public_key_free(keys_in[i]);
    }
}

void test_verification_script_serialize_deserialize_and_copy(void) {
    neoc_ec_public_key_t *pubkey = create_public_key(PUBKEY_B);
    neoc_verification_script_t *script = NULL;
    neoc_verification_script_t *copy = NULL;
    neoc_verification_script_t *deserialized = NULL;
    uint8_t *serialized = NULL;
    size_t serialized_len = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_create_single_sig(pubkey, &script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_copy(script, &copy));
    TEST_ASSERT_TRUE(neoc_verification_script_equals(script, copy));
    TEST_ASSERT_EQUAL_UINT(neoc_verification_script_hash(script),
                           neoc_verification_script_hash(copy));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_serialize(script,
                                                             &serialized,
                                                             &serialized_len));
    TEST_ASSERT_TRUE(serialized_len > 0);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_verification_script_deserialize(serialized,
                                                                serialized_len,
                                                                &deserialized));
    TEST_ASSERT_TRUE(neoc_verification_script_equals(script, deserialized));

    neoc_free(serialized);
    neoc_verification_script_free(deserialized);
    neoc_verification_script_free(copy);
    neoc_verification_script_free(script);
    neoc_ec_public_key_free(pubkey);
}

void test_verification_script_invalid_arguments(void) {
    neoc_verification_script_t *script = NULL;
    neoc_ec_public_key_t *pubkey = create_public_key(PUBKEY_A);
    neoc_ec_public_key_t *keys[1] = {pubkey};
    int threshold = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_verification_script_create_single_sig(NULL, &script));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_verification_script_create_multi_sig(NULL, 1, 1, &script));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_verification_script_create_multi_sig(keys, 1, 2, &script));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_verification_script_get_signing_threshold(NULL, &threshold));

    neoc_ec_public_key_free(pubkey);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_verification_script_single_sig_properties);
    RUN_TEST(test_verification_script_multi_sig_flow);
    RUN_TEST(test_verification_script_serialize_deserialize_and_copy);
    RUN_TEST(test_verification_script_invalid_arguments);
    UNITY_END();
}
