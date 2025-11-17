#include "unity.h"

#include <neoc/protocol/core/response/neo_get_nep17_balances.h>

#include <string.h>

static const char *sample_balances_json =
    "{"
        "\"jsonrpc\":\"2.0\"," \
        "\"id\":1," \
        "\"result\":{"
            "\"address\":\"NXXazKH39yNFWWZF5MJ8tEN98VYHwzn7g3\"," \
            "\"balance\":["
                "{"
                    "\"assethash\":\"a48b6e1291ba24211ad11bb90ae2a10bf1fcd5a8\"," \
                    "\"name\":\"SomeToken\"," \
                    "\"symbol\":\"SOTO\"," \
                    "\"decimals\":\"4\"," \
                    "\"amount\":\"50000000000\"," \
                    "\"lastupdatedblock\":251604"
                "},"
                "{"
                    "\"assethash\":\"1aada0032aba1ef6d1f07bbd8bec1d85f5380fb3\"," \
                    "\"name\":\"RandomToken\"," \
                    "\"symbol\":\"RATO\"," \
                    "\"decimals\":\"2\"," \
                    "\"amount\":\"100000000\"," \
                    "\"lastupdatedblock\":251600"
                "}"
            "]"
        "}"
    "}";

static const char *sample_error_json =
    "{"
        "\"jsonrpc\":\"2.0\"," \
        "\"id\":7," \
        "\"error\":{"
            "\"code\":-32602," \
            "\"message\":\"invalid params\""
        "}"
    "}";

void setUp(void) {}
void tearDown(void) {}

static void assert_hash160(const char *expected, const neoc_hash160_t *actual) {
    neoc_hash160_t expected_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_string(expected, &expected_hash));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&expected_hash, actual));
}

void test_nep17_balances_parse_full_payload(void) {
    neoc_neo_get_nep17_balances_response_t *response = NULL;
    neoc_error_t err = neoc_neo_get_nep17_balances_response_from_json(sample_balances_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_TRUE(neoc_neo_get_nep17_balances_response_is_success(response));
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_STRING("NXXazKH39yNFWWZF5MJ8tEN98VYHwzn7g3", response->result->address);
    TEST_ASSERT_EQUAL_UINT(2, response->result->balance_count);

    const neoc_nep17_balance_t *first = response->result->balances[0];
    TEST_ASSERT_NOT_NULL(first);
    assert_hash160("a48b6e1291ba24211ad11bb90ae2a10bf1fcd5a8", &first->asset_hash);
    TEST_ASSERT_EQUAL_STRING("SomeToken", first->name);
    TEST_ASSERT_EQUAL_STRING("SOTO", first->symbol);
    TEST_ASSERT_EQUAL_STRING("4", first->decimals);
    TEST_ASSERT_EQUAL_STRING("50000000000", first->amount);
    TEST_ASSERT_EQUAL_UINT64(251604ULL, first->last_updated_block);

    const neoc_nep17_balance_t *second = response->result->balances[1];
    TEST_ASSERT_NOT_NULL(second);
    assert_hash160("1aada0032aba1ef6d1f07bbd8bec1d85f5380fb3", &second->asset_hash);
    TEST_ASSERT_EQUAL_STRING("RandomToken", second->name);
    TEST_ASSERT_EQUAL_STRING("RATO", second->symbol);
    TEST_ASSERT_EQUAL_STRING("2", second->decimals);
    TEST_ASSERT_EQUAL_STRING("100000000", second->amount);
    TEST_ASSERT_EQUAL_UINT64(251600ULL, second->last_updated_block);

    char *json_round_trip = NULL;
    err = neoc_neo_get_nep17_balances_response_to_json(response, &json_round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(json_round_trip);
    TEST_ASSERT_TRUE(strlen(json_round_trip) > 0);

    neoc_neo_get_nep17_balances_response_t *round_trip = NULL;
    err = neoc_neo_get_nep17_balances_response_from_json(json_round_trip, &round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_NOT_NULL(round_trip->result);
    TEST_ASSERT_EQUAL_UINT(response->result->balance_count, round_trip->result->balance_count);
    TEST_ASSERT_EQUAL_STRING(response->result->address, round_trip->result->address);

    neoc_neo_get_nep17_balances_response_free(round_trip);
    neoc_free(json_round_trip);
    neoc_neo_get_nep17_balances_response_free(response);
}

void test_nep17_balances_parse_error_payload(void) {
    neoc_neo_get_nep17_balances_response_t *response = NULL;
    neoc_error_t err = neoc_neo_get_nep17_balances_response_from_json(sample_error_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NOT_NULL(response->error);
    TEST_ASSERT_EQUAL_STRING("invalid params", response->error);
    TEST_ASSERT_NULL(response->result);

    neoc_neo_get_nep17_balances_response_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nep17_balances_parse_full_payload);
    RUN_TEST(test_nep17_balances_parse_error_payload);
    return UnityEnd();
}
