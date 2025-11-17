#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/fungible_token.h"
#include "neoc/types/neoc_hash160.h"

static void make_hash(neoc_hash160_t *hash, uint8_t seed) {
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_fungible_token_creation_sets_defaults(void) {
    neoc_hash160_t contract_hash = {{0}};
    neoc_fungible_token_t *token = NULL;
    make_hash(&contract_hash, 0x10);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_fungible_token_create(&contract_hash, &token));
    TEST_ASSERT_NOT_NULL(token);
    TEST_ASSERT_EQUAL_INT(NEOC_TOKEN_TYPE_FUNGIBLE, token->base.type);
    TEST_ASSERT_EQUAL_UINT8(8, neoc_fungible_token_get_decimals(token));
    TEST_ASSERT_EQUAL_UINT64(0, neoc_fungible_token_get_total_supply(token));
    TEST_ASSERT_NOT_NULL(token->base.contract_hash);
    TEST_ASSERT_EQUAL_MEMORY(contract_hash.data, token->base.contract_hash->data, NEOC_HASH160_SIZE);

    neoc_fungible_token_free(token);
}

void test_fungible_token_balance_and_transfer(void) {
    neoc_hash160_t contract_hash = {{0}}, from = {{0}}, to = {{0}};
    neoc_fungible_token_t *token = NULL;
    int64_t balance = -1;
    make_hash(&contract_hash, 0x20);
    make_hash(&from, 0x30);
    make_hash(&to, 0x40);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_fungible_token_create(&contract_hash, &token));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_fungible_token_balance_of(token, &from, &balance));
    TEST_ASSERT_EQUAL_INT64(0, balance);

    uint8_t payload[] = {0xAA, 0xBB};
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_fungible_token_transfer(token, &from, &to, 0, payload, sizeof(payload)));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_fungible_token_transfer(token, &from, &to, 1000000, payload, sizeof(payload)));

    neoc_fungible_token_free(token);
}

void test_fungible_token_invalid_arguments(void) {
    neoc_fungible_token_t *token = NULL;
    neoc_hash160_t hash = {{0}};
    make_hash(&hash, 0x55);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, neoc_fungible_token_create(NULL, &token));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, neoc_fungible_token_create(&hash, NULL));

    neoc_fungible_token_free(NULL);
}

void test_fungible_token_balance_rpc_arguments(void) {
    neoc_hash160_t contract_hash = {{0}}, account = {{0}};
    neoc_fungible_token_t *token = NULL;
    uint64_t balance = 0;
    make_hash(&contract_hash, 0x60);
    make_hash(&account, 0x70);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_fungible_token_create(&contract_hash, &token));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_fungible_token_balance_of_rpc(NULL, (neoc_rpc_client_t *)1, &account, &balance));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_fungible_token_balance_of_rpc(token, NULL, &account, &balance));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_fungible_token_balance_of_rpc(token, (neoc_rpc_client_t *)1, NULL, &balance));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_fungible_token_balance_of_rpc(token, (neoc_rpc_client_t *)1, &account, NULL));

    neoc_fungible_token_free(token);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fungible_token_creation_sets_defaults);
    RUN_TEST(test_fungible_token_balance_and_transfer);
    RUN_TEST(test_fungible_token_invalid_arguments);
    RUN_TEST(test_fungible_token_balance_rpc_arguments);
    UNITY_END();
}
