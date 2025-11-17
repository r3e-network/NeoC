#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/non_fungible_token.h"
#include "neoc/types/neoc_hash160.h"

static neoc_non_fungible_token_t *token = NULL;
static neoc_hash160_t contract_hash;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        contract_hash.data[i] = (uint8_t)(i + 1);
    }
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_create(&contract_hash, false, &token));
    TEST_ASSERT_NOT_NULL(token);
}

void tearDown(void) {
    neoc_nft_free(token);
    token = NULL;
    neoc_cleanup();
}

void test_nft_creation_sets_base_fields(void) {
    TEST_ASSERT_EQUAL_PTR(&contract_hash, token->base.contract_hash);
    TEST_ASSERT_EQUAL_INT(NEOC_TOKEN_TYPE_NON_FUNGIBLE, token->base.type);
    TEST_ASSERT_FALSE(token->divisible);
}

void test_nft_symbol_defaults_without_rpc(void) {
    char *symbol = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_symbol(token, &symbol));
    TEST_ASSERT_NOT_NULL(symbol);
    TEST_ASSERT_EQUAL_STRING("NFT", symbol);
    TEST_ASSERT_NOT_NULL(token->base.symbol);
    TEST_ASSERT_EQUAL_STRING("NFT", token->base.symbol);
    neoc_free(symbol);
}

void test_nft_decimals_and_total_supply_without_rpc(void) {
    uint8_t decimals = 99;
    uint64_t supply = 1234;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_decimals(token, &decimals));
    TEST_ASSERT_EQUAL_UINT8(0, decimals);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_total_supply(token, &supply));
    TEST_ASSERT_EQUAL_UINT64(0, supply);
}

void test_nft_balance_owners_and_tokens_without_rpc(void) {
    neoc_hash160_t owner = {{0}};
    int64_t balance = -1;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_balance_of(token, &owner, &balance));
    TEST_ASSERT_EQUAL_INT64(0, balance);

    neoc_iterator_t *iterator = (neoc_iterator_t *)0x1;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_tokens_of(token, &owner, &iterator));
    TEST_ASSERT_EQUAL_PTR(NULL, iterator);

    uint8_t token_id[] = {0x01, 0x02, 0x03, 0x04};
    neoc_hash160_t **owners = (neoc_hash160_t **)0x1;
    size_t owners_count = 55;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nft_owner_of(token,
                                            token_id,
                                            sizeof(token_id),
                                            &owners,
                                            &owners_count));
    TEST_ASSERT_EQUAL_PTR(NULL, owners);
    TEST_ASSERT_EQUAL_UINT(0, owners_count);
}

void test_nft_set_rpc_client_updates_pointer(void) {
    void *dummy_client = (void *)0x1234;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nft_set_rpc_client(token, dummy_client));
    TEST_ASSERT_EQUAL_PTR(dummy_client, token->rpc_client);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nft_creation_sets_base_fields);
    RUN_TEST(test_nft_symbol_defaults_without_rpc);
    RUN_TEST(test_nft_decimals_and_total_supply_without_rpc);
    RUN_TEST(test_nft_balance_owners_and_tokens_without_rpc);
    RUN_TEST(test_nft_set_rpc_client_updates_pointer);
    UNITY_END();
}
