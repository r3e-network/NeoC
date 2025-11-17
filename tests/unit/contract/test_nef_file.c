#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/nef_file.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"

static const uint8_t SAMPLE_SCRIPT[] = {0x01, 0x02, 0x03, 0x04};

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

static void fill_hash160(neoc_hash160_t *hash, uint8_t seed) {
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

void test_nef_file_create_and_properties(void) {
    neoc_nef_file_t *nef = NULL;
    const uint8_t *script = NULL;
    size_t script_len = 0;
    neoc_hash256_t checksum;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_create("Neo Compiler 1.0",
                                               "https://example.com",
                                               SAMPLE_SCRIPT,
                                               sizeof(SAMPLE_SCRIPT),
                                               &nef));
    TEST_ASSERT_NOT_NULL(nef);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nef_file_get_script(nef, &script, &script_len));
    TEST_ASSERT_EQUAL_UINT(sizeof(SAMPLE_SCRIPT), script_len);
    TEST_ASSERT_EQUAL_MEMORY(SAMPLE_SCRIPT, script, sizeof(SAMPLE_SCRIPT));
    TEST_ASSERT_EQUAL_STRING("Neo Compiler 1.0", neoc_nef_file_get_compiler(nef));
    TEST_ASSERT_EQUAL_STRING("https://example.com", neoc_nef_file_get_source(nef));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nef_file_get_checksum(nef, &checksum));
    TEST_ASSERT_TRUE(neoc_nef_file_verify_checksum(nef));

    neoc_nef_file_free(nef);
}

void test_nef_file_method_tokens(void) {
    neoc_nef_file_t *nef = NULL;
    neoc_hash160_t hash = {{0}};
    const neoc_nef_method_token_t *token = NULL;

    fill_hash160(&hash, 0x10);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_create("Neo Compiler 1.0",
                                               NULL,
                                               SAMPLE_SCRIPT,
                                               sizeof(SAMPLE_SCRIPT),
                                               &nef));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_add_method_token(nef,
                                                         &hash,
                                                         "balanceOf",
                                                         2,
                                                         true,
                                                         0xFF));
    TEST_ASSERT_EQUAL_UINT(1, neoc_nef_file_get_token_count(nef));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nef_file_get_token(nef, 0, &token));
    TEST_ASSERT_EQUAL_STRING("balanceOf", token->method);
    TEST_ASSERT_EQUAL_UINT16(2, token->parameters_count);
    TEST_ASSERT_TRUE(token->has_return_value);
    TEST_ASSERT_EQUAL_UINT8(0xFF, token->call_flags);
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_OUT_OF_BOUNDS,
                          neoc_nef_file_get_token(nef, 1, &token));

    neoc_nef_file_free(nef);
}

void test_nef_file_serialization_roundtrip(void) {
    neoc_nef_file_t *nef = NULL;
    neoc_nef_file_t *loaded = NULL;
    uint8_t *bytes = NULL;
    size_t bytes_len = 0;
    const uint8_t *script = NULL;
    size_t script_len = 0;
    uint8_t large_script[400];

    for (size_t i = 0; i < sizeof(large_script); ++i) {
        large_script[i] = (uint8_t)(i & 0xFF);
    }

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_create("Neo Compiler 1.1",
                                               "src://contract",
                                               large_script,
                                               sizeof(large_script),
                                               &nef));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_to_bytes(nef, &bytes, &bytes_len));
    TEST_ASSERT_TRUE(bytes_len > 0);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_from_bytes(bytes, bytes_len, &loaded));
    TEST_ASSERT_EQUAL_STRING(neoc_nef_file_get_compiler(nef),
                             neoc_nef_file_get_compiler(loaded));
    TEST_ASSERT_EQUAL_STRING(neoc_nef_file_get_source(nef),
                             neoc_nef_file_get_source(loaded));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nef_file_get_script(loaded, &script, &script_len));
    TEST_ASSERT_EQUAL_UINT(sizeof(large_script), script_len);
    TEST_ASSERT_EQUAL_MEMORY(large_script, script, sizeof(large_script));

    neoc_free(bytes);
    neoc_nef_file_free(loaded);
    neoc_nef_file_free(nef);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nef_file_create_and_properties);
    RUN_TEST(test_nef_file_method_tokens);
    RUN_TEST(test_nef_file_serialization_roundtrip);
    UNITY_END();
}
