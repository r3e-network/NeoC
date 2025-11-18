/**
 * @file test_gas_token.c
 * @brief Unit tests converted from GasTokenTests.swift
 */

#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/gas_token.h"
#include "neoc/types/neoc_hash160.h"

#define EXPECTED_TOTAL_SUPPLY 5200000000000000ULL

static neoc_gas_token_t *token = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_gas_token_create(&token));
    TEST_ASSERT_NOT_NULL(token);
}

void tearDown(void) {
    neoc_gas_token_free(token);
    token = NULL;
    neoc_cleanup();
}

void test_gas_token_metadata(void) {
    char *name = NULL;
    char *symbol = NULL;
    int decimals = -1;
    uint64_t supply = 0;
    bool is_native = false;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_gas_token_get_name(token, &name));
    TEST_ASSERT_EQUAL_STRING("GasToken", name);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_gas_token_get_symbol(token, &symbol));
    TEST_ASSERT_EQUAL_STRING("GAS", symbol);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_gas_token_get_decimals(token, &decimals));
    TEST_ASSERT_EQUAL_INT(NEOC_GAS_TOKEN_DECIMALS, decimals);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_gas_token_get_total_supply(token, &supply));
    TEST_ASSERT_EQUAL_UINT64(EXPECTED_TOTAL_SUPPLY, supply);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_gas_token_is_native_contract(token, &is_native));
    TEST_ASSERT_TRUE(is_native);

    neoc_free(name);
    neoc_free(symbol);
}

void test_gas_token_transfer_script_builders(void) {
    neoc_hash160_t from = {{0}};
    neoc_hash160_t to = {{0}};
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        from.data[i] = (uint8_t)i;
        to.data[i] = (uint8_t)(0xAB + i);
    }

    uint8_t *script = NULL;
    size_t script_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_gas_token_build_transfer_script(token,
                                                               &from,
                                                               &to,
                                                               100000000,
                                                               NULL,
                                                               0,
                                                               &script,
                                                               &script_len));
    TEST_ASSERT_NOT_NULL(script);
    TEST_ASSERT_TRUE(script_len > 0);
    neoc_free(script);

    neoc_gas_token_transfer_request_t requests[2] = {
        {.to = to, .amount = 500000000, .data = NULL, .data_len = 0},
        {.to = from, .amount = 1000000000, .data = NULL, .data_len = 0}
    };
    script = NULL;
    script_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_gas_token_build_multi_transfer_script(token,
                                                                     &from,
                                                                     requests,
                                                                     2,
                                                                     &script,
                                                                     &script_len));
    TEST_ASSERT_NOT_NULL(script);
    TEST_ASSERT_TRUE(script_len > 0);
    neoc_free(script);
}

void test_gas_token_supported_methods(void) {
    char **methods = NULL;
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_gas_token_get_supported_methods(token,
                                                               &methods,
                                                               &count));
    static const char *expected[] = {
        "symbol",
        "decimals",
        "totalSupply",
        "balanceOf",
        "transfer",
        "refuel"
    };
    TEST_ASSERT_EQUAL_UINT(sizeof(expected) / sizeof(expected[0]), count);
    for (size_t i = 0; i < count; ++i) {
        TEST_ASSERT_EQUAL_STRING(expected[i], methods[i]);
        neoc_free(methods[i]);
    }
    neoc_free(methods);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_gas_token_metadata);
    RUN_TEST(test_gas_token_transfer_script_builders);
    RUN_TEST(test_gas_token_supported_methods);
    return UnityEnd();
}
