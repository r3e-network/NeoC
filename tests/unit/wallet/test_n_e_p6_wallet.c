/**
 * @file test_n_e_p6_wallet.c
 * @brief Unit tests converted from NEP6WalletTests.swift
 */

#include "unity.h"
#include <stdio.h>
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/wallet/nep6_wallet.h"
#include "neoc/wallet/nep6/nep6_wallet.h"
#include "neoc/wallet/nep6/nep6_contract.h"

static neoc_nep6_contract_t* create_test_contract(void) {
    neoc_nep6_parameter_t param = {
        .name = "signature",
        .type = NEOC_PARAM_TYPE_SIGNATURE
    };
    neoc_nep6_contract_t *contract = NULL;
    neoc_error_t err = neoc_nep6_contract_create("deadbeef", &param, 1, false, &contract);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    return contract;
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_nep6_wallet_struct_json_roundtrip(void) {
    neoc_nep6_wallet_struct_t *wallet = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_create("StructWallet", "1.0", &wallet));
    wallet->scrypt.n = 16384;
    wallet->scrypt.r = 8;
    wallet->scrypt.p = 1;

    neoc_nep6_contract_t *contract = create_test_contract();
    neoc_nep6_account_t *account = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_account_create("NLnyLtep7jwyq1qhNPkwXbJpurC4jUT8ke",
                                                   "Primary",
                                                   true,
                                                   false,
                                                   "6PYVEi6ZGdsLoCYbbGWqoYef7VWMbKwcew86m5fpxnZRUD8tEjainBgQW1",
                                                   contract,
                                                   &account));
    contract = NULL; // ownership transferred
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_add_account(wallet, account));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_to_json(wallet, &json));

    neoc_nep6_wallet_struct_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_from_json(json, &parsed));

    TEST_ASSERT_TRUE(neoc_nep6_wallet_struct_equals(wallet, parsed));

    neoc_nep6_wallet_struct_free(parsed);
    neoc_nep6_wallet_struct_free(wallet);
    neoc_free(json);
}

void test_nep6_wallet_struct_to_runtime(void) {
    neoc_nep6_wallet_struct_t *wallet_struct = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_create("RuntimeWallet", "2.0", &wallet_struct));

    neoc_nep6_contract_t *contract_a = create_test_contract();
    neoc_nep6_account_t *account_a = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_account_create("NWcx4EfYdfqn5jNjDz8AHE6hWtWdUGDdmy",
                                                   "AccountA",
                                                   true,
                                                   false,
                                                   "6PYSQWBqZE5oEFdMGCJ3xR7bz6ezz814oKE7GqwB9i5uhtUzkshe9B6YGB",
                                                   contract_a,
                                                   &account_a));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_add_account(wallet_struct, account_a));

    neoc_nep6_contract_t *contract_b = create_test_contract();
    neoc_nep6_account_t *account_b = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_account_create("NbUgTSFvPmsRxmGeWpuuGeJUoRoi6PErcM",
                                                   "AccountB",
                                                   false,
                                                   false,
                                                   "6PYM7jHL4ChfgWLsXTvQVa2PVLh4q14izVQpmTLGi7DpzQjjMvJ6rAHLuK",
                                                   contract_b,
                                                   &account_b));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_struct_add_account(wallet_struct, account_b));

    neoc_nep6_wallet_t *runtime = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep6_wallet_from_struct(wallet_struct, &runtime));

    TEST_ASSERT_EQUAL_STRING("RuntimeWallet", neoc_nep6_wallet_get_name(runtime));
    TEST_ASSERT_EQUAL_STRING("2.0", neoc_nep6_wallet_get_version(runtime));
    size_t count = neoc_nep6_wallet_get_account_count(runtime);
    TEST_ASSERT_EQUAL_UINT(2, count);

    neoc_nep6_account_t *first = neoc_nep6_wallet_get_account(runtime, 0);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_TRUE(neoc_nep6_account_is_default(first));

    neoc_nep6_wallet_free(runtime);
    neoc_nep6_wallet_struct_free(wallet_struct);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nep6_wallet_struct_json_roundtrip);
    RUN_TEST(test_nep6_wallet_struct_to_runtime);
    return UNITY_END();
}
