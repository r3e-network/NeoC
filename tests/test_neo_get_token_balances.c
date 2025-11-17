#include "unity.h"

#include <neoc/neoc.h>
#include <neoc/protocol/core/response/neo_get_token_balances.h>
#include <neoc/types/neoc_hash160.h>

static const char *kTokenBalancesJson =
    "{"
        "\"address\":\"NVGUZ7AbcD1FJcVg1mJisNmzu6Y9f9Dz3S\","
        "\"balance\":["
            "{"
                "\"assethash\":\"0xa6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6\""
            "},"
            "{"
                "\"assethash\":\"0xb7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7\""
            "}"
        "]"
    "}";

static const char *kTokenBalancesResponseJson =
    "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":42,"
        "\"result\":{"
            "\"address\":\"NVGUZ7AbcD1FJcVg1mJisNmzu6Y9f9Dz3S\","
            "\"balance\":["
                "{"
                    "\"assethash\":\"0xa6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6\""
                "},"
                "{"
                    "\"assethash\":\"0xb7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7\""
                "}"
            "]"
        "}"
    "}";

static const char *kTokenBalancesErrorResponseJson =
    "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":7,"
        "\"error\":{"
            "\"code\":-32602,"
            "\"message\":\"invalid params\""
        "}"
    "}";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

static void assert_hash160_equals(const char *expected, const neoc_hash160_t *actual) {
    neoc_hash160_t expected_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_string(expected, &expected_hash));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&expected_hash, actual));
}

void test_token_balances_from_json(void) {
    neoc_token_balances_t *balances = NULL;
    neoc_error_t err = neoc_token_balances_from_json(kTokenBalancesJson, &balances);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(balances);
    TEST_ASSERT_NOT_NULL(balances->address);
    TEST_ASSERT_EQUAL_STRING("NVGUZ7AbcD1FJcVg1mJisNmzu6Y9f9Dz3S", balances->address);
    TEST_ASSERT_EQUAL_UINT(2, balances->balances_count);
    TEST_ASSERT_NOT_NULL(balances->balances);
    TEST_ASSERT_NOT_NULL(balances->balances[0].asset_hash);
    TEST_ASSERT_NOT_NULL(balances->balances[1].asset_hash);

    assert_hash160_equals("0xa6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6a6", balances->balances[0].asset_hash);
    assert_hash160_equals("0xb7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7", balances->balances[1].asset_hash);

    const neoc_token_balance_t *matched = NULL;
    neoc_hash160_t search_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_string("0xb7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7", &search_hash));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_balances_find_asset(balances, &search_hash, &matched));
    TEST_ASSERT_NOT_NULL(matched);
    assert_hash160_equals("0xb7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7b7", matched->asset_hash);

    neoc_token_balances_free(balances);
}

void test_token_balances_find_asset_missing(void) {
    neoc_token_balances_t *balances = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_token_balances_from_json(kTokenBalancesJson, &balances));

    neoc_hash160_t search_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_string("0x1111111111111111111111111111111111111111", &search_hash));
    const neoc_token_balance_t *matched = NULL;
    neoc_error_t err = neoc_token_balances_find_asset(balances, &search_hash, &matched);
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NOT_FOUND, err);
    TEST_ASSERT_NULL(matched);

    neoc_token_balances_free(balances);
}

void test_token_balances_response_success(void) {
    neoc_neo_get_token_balances_response_t *response = NULL;
    neoc_error_t err = neoc_neo_get_token_balances_response_from_json(
        kTokenBalancesResponseJson, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(42, response->id);
    TEST_ASSERT_NULL(response->error);
    TEST_ASSERT_EQUAL_INT(0, response->error_code);
    TEST_ASSERT_NOT_NULL(response->jsonrpc);
    TEST_ASSERT_EQUAL_STRING("2.0", response->jsonrpc);
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_STRING("NVGUZ7AbcD1FJcVg1mJisNmzu6Y9f9Dz3S", response->result->address);
    TEST_ASSERT_EQUAL_UINT(2, response->result->balances_count);

    neoc_neo_get_token_balances_response_free(response);
}

void test_token_balances_response_error(void) {
    neoc_neo_get_token_balances_response_t *response = NULL;
    neoc_error_t err = neoc_neo_get_token_balances_response_from_json(
        kTokenBalancesErrorResponseJson, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(7, response->id);
    TEST_ASSERT_NOT_NULL(response->error);
    TEST_ASSERT_EQUAL_STRING("invalid params", response->error);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NULL(response->result);

    neoc_neo_get_token_balances_response_free(response);
}

void test_token_balances_to_json_round_trip(void) {
    neoc_token_balances_t *balances = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_balances_from_json(kTokenBalancesJson, &balances));

    char *serialized = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_balances_to_json(balances, &serialized));
    TEST_ASSERT_NOT_NULL(serialized);

    neoc_token_balances_t *round_trip = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_balances_from_json(serialized, &round_trip));

    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_EQUAL_STRING(balances->address, round_trip->address);
    TEST_ASSERT_EQUAL_UINT(balances->balances_count, round_trip->balances_count);
    for (size_t i = 0; i < balances->balances_count; ++i) {
        const neoc_token_balance_t *lhs = &balances->balances[i];
        const neoc_token_balance_t *rhs = &round_trip->balances[i];
        if (lhs->asset_hash && rhs->asset_hash) {
            TEST_ASSERT_TRUE(neoc_hash160_equal(lhs->asset_hash, rhs->asset_hash));
        } else {
            TEST_ASSERT_TRUE(lhs->asset_hash == NULL && rhs->asset_hash == NULL);
        }
    }

    neoc_free(serialized);
    neoc_token_balances_free(round_trip);
    neoc_token_balances_free(balances);
}

void test_token_balances_response_to_json_round_trip(void) {
    neoc_neo_get_token_balances_response_t *response = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_get_token_balances_response_from_json(
                              kTokenBalancesResponseJson, &response));

    char *serialized = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_get_token_balances_response_to_json(response, &serialized));
    TEST_ASSERT_NOT_NULL(serialized);

    neoc_neo_get_token_balances_response_t *round_trip = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_get_token_balances_response_from_json(serialized, &round_trip));

    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_EQUAL_INT(response->id, round_trip->id);
    TEST_ASSERT_EQUAL_INT(response->error_code, round_trip->error_code);
    if (response->jsonrpc && round_trip->jsonrpc) {
        TEST_ASSERT_EQUAL_STRING(response->jsonrpc, round_trip->jsonrpc);
    } else {
        TEST_ASSERT_TRUE(response->jsonrpc == NULL && round_trip->jsonrpc == NULL);
    }

    if (response->result && round_trip->result) {
        TEST_ASSERT_EQUAL_STRING(response->result->address, round_trip->result->address);
        TEST_ASSERT_EQUAL_UINT(response->result->balances_count,
                               round_trip->result->balances_count);
        for (size_t i = 0; i < response->result->balances_count; ++i) {
            const neoc_token_balance_t *lhs = &response->result->balances[i];
            const neoc_token_balance_t *rhs = &round_trip->result->balances[i];
            if (lhs->asset_hash && rhs->asset_hash) {
                TEST_ASSERT_TRUE(neoc_hash160_equal(lhs->asset_hash, rhs->asset_hash));
            } else {
                TEST_ASSERT_TRUE(lhs->asset_hash == NULL && rhs->asset_hash == NULL);
            }
        }
    } else {
        TEST_ASSERT_TRUE(response->result == NULL && round_trip->result == NULL);
    }

    neoc_free(serialized);
    neoc_neo_get_token_balances_response_free(round_trip);
    neoc_neo_get_token_balances_response_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_token_balances_from_json);
    RUN_TEST(test_token_balances_find_asset_missing);
    RUN_TEST(test_token_balances_response_success);
    RUN_TEST(test_token_balances_response_error);
    RUN_TEST(test_token_balances_to_json_round_trip);
    RUN_TEST(test_token_balances_response_to_json_round_trip);
    return UnityEnd();
}
