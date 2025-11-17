#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/token.h"
#include "neoc/types/neoc_hash160.h"

static void fill_test_hash(neoc_hash160_t *hash) {
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        hash->data[i] = (uint8_t)(0xA0 + i);
    }
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_token_create_and_contract_hash(void) {
    neoc_hash160_t contract_hash = {{0}};
    neoc_token_t *token = NULL;
    fill_test_hash(&contract_hash);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_create(&contract_hash,
                                            NEOC_TOKEN_TYPE_FUNGIBLE,
                                            &token));
    TEST_ASSERT_NOT_NULL(token);
    TEST_ASSERT_EQUAL_INT(NEOC_TOKEN_TYPE_FUNGIBLE, token->type);

    neoc_hash160_t *stored = neoc_token_get_contract_hash(token);
    TEST_ASSERT_NOT_NULL(stored);
    TEST_ASSERT_EQUAL_MEMORY(contract_hash.data, stored->data, NEOC_HASH160_SIZE);

    neoc_token_free(token);
}

void test_token_symbol_and_name_accessors(void) {
    neoc_hash160_t contract_hash = {{0}};
    neoc_token_t *token = NULL;
    fill_test_hash(&contract_hash);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_create(&contract_hash,
                                            NEOC_TOKEN_TYPE_GOVERNANCE,
                                            &token));
    TEST_ASSERT_NOT_NULL(token);

    token->symbol = neoc_strdup("TOK");
    token->name = neoc_strdup("Sample Token");

    TEST_ASSERT_EQUAL_STRING("TOK", neoc_token_get_symbol(token));
    TEST_ASSERT_EQUAL_STRING("Sample Token", neoc_token_get_name(token));

    neoc_token_free(token);
}

void test_token_invalid_arguments_and_free_null(void) {
    neoc_token_t *token = NULL;
    neoc_hash160_t contract_hash = {{0}};
    fill_test_hash(&contract_hash);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_token_create(NULL, NEOC_TOKEN_TYPE_FUNGIBLE, &token));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_token_create(&contract_hash, NEOC_TOKEN_TYPE_FUNGIBLE, NULL));

    neoc_token_free(NULL);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_token_create_and_contract_hash);
    RUN_TEST(test_token_symbol_and_name_accessors);
    RUN_TEST(test_token_invalid_arguments_and_free_null);
    UNITY_END();
}
