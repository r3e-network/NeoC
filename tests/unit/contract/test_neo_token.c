#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/neoc_token.h"
#include "neoc/types/neoc_hash160.h"

static neoc_neo_token_t *token = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_token_create(&token));
    TEST_ASSERT_NOT_NULL(token);
}

void tearDown(void) {
    neoc_neo_token_free(token);
    token = NULL;
    neoc_cleanup();
}

static void assert_hash_equals_constant(const neoc_hash160_t *hash) {
    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_EQUAL_MEMORY(NEOC_NEO_TOKEN_HASH.data,
                             hash->data,
                             NEOC_HASH160_SIZE);
}

void test_neo_token_metadata_and_overloads(void) {
    char *name = NULL;
    char *symbol_copy = NULL;
    const char *symbol_const = NULL;
    int64_t total_supply = -1;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_token_get_name(token, &name));
    TEST_ASSERT_EQUAL_STRING("NeoToken", name);

    symbol_const = neoc_neo_token_get_symbol_const();
    TEST_ASSERT_EQUAL_STRING("NEO", symbol_const);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_token_get_symbol(token, &symbol_copy));
    TEST_ASSERT_EQUAL_STRING("NEO", symbol_copy);

    TEST_ASSERT_EQUAL_UINT8(0, neoc_neo_token_get_decimals());

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_token_get_total_supply(token, &total_supply));
    TEST_ASSERT_EQUAL_INT64(NEO_TOKEN_TOTAL_SUPPLY, total_supply);

    neoc_free(name);
    neoc_free(symbol_copy);
}

void test_neo_token_contract_hash_matches_constant(void) {
    neoc_hash160_t script_hash = {{0}};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_token_get_script_hash(&script_hash));
    assert_hash_equals_constant(&script_hash);
}

void test_neo_token_balance_defaults_to_zero(void) {
    neoc_account_t *account = NULL;
    int64_t balance = -1;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_create_random(&account));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_token_get_balance(token, account, &balance));
    TEST_ASSERT_EQUAL_INT64(0, balance);

    neoc_account_free(account);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_neo_token_metadata_and_overloads);
    RUN_TEST(test_neo_token_contract_hash_matches_constant);
    RUN_TEST(test_neo_token_balance_defaults_to_zero);
    UNITY_END();
}
