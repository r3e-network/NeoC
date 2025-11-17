#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/script/invocation_script.h"

static void fill_signature(uint8_t *buffer, uint8_t seed) {
    for (size_t i = 0; i < NEOC_SIGNATURE_SIZE; ++i) {
        buffer[i] = (uint8_t)(seed + i);
    }
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_invocation_script_single_signature(void) {
    uint8_t signature[NEOC_SIGNATURE_SIZE] = {0};
    neoc_invocation_script_t *script = NULL;
    const uint8_t *retrieved = NULL;
    size_t len = 0;

    fill_signature(signature, 0x10);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_create_single_sig(signature,
                                                                   sizeof(signature),
                                                                   &script));
    TEST_ASSERT_NOT_NULL(script);
    TEST_ASSERT_FALSE(neoc_invocation_script_is_empty(script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_get_script(script, &retrieved, &len));
    TEST_ASSERT_EQUAL_UINT(NEOC_SIGNATURE_SIZE + 2, len); // pushdata header + payload
    TEST_ASSERT_TRUE(neoc_invocation_script_get_size(script) > 0);

    neoc_invocation_script_free(script);
}

void test_invocation_script_multi_signature_and_extract(void) {
    uint8_t sig1[NEOC_SIGNATURE_SIZE] = {0};
    uint8_t sig2[NEOC_SIGNATURE_SIZE] = {0};
    const uint8_t *sig_ptrs[2] = {sig1, sig2};
    size_t sig_lengths[2] = {sizeof(sig1), sizeof(sig2)};
    uint8_t **extracted = NULL;
    size_t *extracted_lengths = NULL;
    size_t count = 0;
    neoc_invocation_script_t *script = NULL;

    fill_signature(sig1, 0x20);
    fill_signature(sig2, 0x60);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_create_multi_sig(sig_ptrs,
                                                                  sig_lengths,
                                                                  2,
                                                                  &script));
    TEST_ASSERT_NOT_NULL(script);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_extract_signatures(script,
                                                                    &extracted,
                                                                    &extracted_lengths,
                                                                    &count));
    TEST_ASSERT_EQUAL_UINT(2, count);
    TEST_ASSERT_EQUAL_UINT(NEOC_SIGNATURE_SIZE, extracted_lengths[0]);
    TEST_ASSERT_EQUAL_MEMORY(sig1, extracted[0], NEOC_SIGNATURE_SIZE);
    TEST_ASSERT_EQUAL_MEMORY(sig2, extracted[1], NEOC_SIGNATURE_SIZE);

    neoc_invocation_script_free_signatures(extracted, extracted_lengths, count);
    neoc_invocation_script_free(script);
}

void test_invocation_script_copy_equals_and_hash(void) {
    uint8_t signature[NEOC_SIGNATURE_SIZE] = {0};
    neoc_invocation_script_t *script = NULL;
    neoc_invocation_script_t *copy = NULL;

    fill_signature(signature, 0x33);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_create_single_sig(signature,
                                                                   sizeof(signature),
                                                                   &script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_invocation_script_copy(script, &copy));

    TEST_ASSERT_TRUE(neoc_invocation_script_equals(script, copy));
    TEST_ASSERT_EQUAL_UINT(neoc_invocation_script_hash(script),
                           neoc_invocation_script_hash(copy));

    neoc_invocation_script_free(copy);
    neoc_invocation_script_free(script);
}

void test_invocation_script_serialize_deserialize(void) {
    uint8_t signature[NEOC_SIGNATURE_SIZE] = {0};
    neoc_invocation_script_t *script = NULL;
    neoc_invocation_script_t *deserialized = NULL;
    uint8_t *serialized = NULL;
    size_t serialized_len = 0;

    fill_signature(signature, 0x77);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_create_single_sig(signature,
                                                                   sizeof(signature),
                                                                   &script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_serialize(script, &serialized, &serialized_len));
    TEST_ASSERT_TRUE(serialized_len > 0);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_deserialize(serialized,
                                                             serialized_len,
                                                             &deserialized));
    TEST_ASSERT_TRUE(neoc_invocation_script_equals(script, deserialized));

    neoc_free(serialized);
    neoc_invocation_script_free(deserialized);
    neoc_invocation_script_free(script);
}

void test_invocation_script_invalid_inputs(void) {
    neoc_invocation_script_t *script = NULL;
    uint8_t signature[NEOC_SIGNATURE_SIZE] = {0};
    fill_signature(signature, 0xAA);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_invocation_script_create_single_sig(NULL,
                                                                   sizeof(signature),
                                                                   &script));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_LENGTH,
                          neoc_invocation_script_create_single_sig(signature,
                                                                   10,
                                                                   &script));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_invocation_script_create_multi_sig(NULL, NULL, 1, &script));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_invocation_script_create_single_sig(signature,
                                                                   sizeof(signature),
                                                                   &script));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_invocation_script_add_signature(NULL,
                                                                 signature,
                                                                 sizeof(signature)));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_LENGTH,
                          neoc_invocation_script_add_signature(script,
                                                               signature,
                                                               0));

    neoc_invocation_script_free(script);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_invocation_script_single_signature);
    RUN_TEST(test_invocation_script_multi_signature_and_extract);
    RUN_TEST(test_invocation_script_copy_equals_and_hash);
    RUN_TEST(test_invocation_script_serialize_deserialize);
    RUN_TEST(test_invocation_script_invalid_inputs);
    UNITY_END();
}
