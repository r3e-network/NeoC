#include "unity.h"

#include <neoc/protocol/core/response/neo_get_nep11_balances.h>

static const char *sample_balances_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 3,\n"
    "  \"result\": {\n"
    "    \"address\": \"NXXazKH39yNFWWZF5MJ8tEN98VYHwzn7g3\",\n"
    "    \"balance\": [\n"
    "      {\n"
    "        \"assethash\": \"a48b6e1291ba24211ad11bb90ae2a10bf1fcd5a8\",\n"
    "        \"name\": \"FunnyCats\",\n"
    "        \"symbol\": \"FCS\",\n"
    "        \"decimals\": \"0\",\n"
    "        \"tokens\": [\n"
    "          {\"tokenid\": \"1\", \"amount\": \"1\", \"lastupdatedblock\": 12345},\n"
    "          {\"tokenid\": \"2\", \"amount\": \"1\", \"lastupdatedblock\": 123456}\n"
    "        ]\n"
    "      },\n"
    "      {\n"
    "        \"assethash\": \"1aada0032aba1ef6d1f07bbd8bec1d85f5380fb3\",\n"
    "        \"name\": \"CuteNeoKittens\",\n"
    "        \"symbol\": \"CNKS\",\n"
    "        \"decimals\": \"4\",\n"
    "        \"tokens\": [\n"
    "          {\"tokenid\": \"4\", \"amount\": \"10000\", \"lastupdatedblock\": 12345},\n"
    "          {\"tokenid\": \"10\", \"amount\": \"6500\", \"lastupdatedblock\": 654321}\n"
    "        ]\n"
    "      }\n"
    "    ]\n"
    "  }\n"
    "}";

static const char *sample_error_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 11,\n"
    "  \"error\": {\"code\": -32602, \"message\": \"invalid params\"}\n"
    "}";

void setUp(void) {}
void tearDown(void) {}

static void assert_hash160_match(const char *expected, const neoc_hash160_t *hash) {
    neoc_hash160_t expected_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_string(expected, &expected_hash));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&expected_hash, hash));
}

void test_nep11_balances_parse_full_payload(void) {
    neoc_neo_get_nep11_balances_t *response = NULL;
    neoc_error_t err = neoc_neo_get_nep11_balances_from_json(sample_balances_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_TRUE(neoc_neo_get_nep11_balances_has_balances(response));
    TEST_ASSERT_EQUAL_INT(3, response->id);
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_STRING("NXXazKH39yNFWWZF5MJ8tEN98VYHwzn7g3", response->result->address);
    TEST_ASSERT_EQUAL_UINT(2, response->result->balances_count);

    neoc_nep11_balance_t *first = neoc_neo_get_nep11_balances_get_balance(response, 0);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_STRING("FunnyCats", first->name);
    TEST_ASSERT_EQUAL_STRING("FCS", first->symbol);
    TEST_ASSERT_EQUAL_STRING("0", first->decimals);
    assert_hash160_match("a48b6e1291ba24211ad11bb90ae2a10bf1fcd5a8", &first->asset_hash);
    TEST_ASSERT_EQUAL_UINT(2, first->tokens_count);
    TEST_ASSERT_EQUAL_STRING("1", first->tokens[0].token_id);
    TEST_ASSERT_EQUAL_STRING("1", first->tokens[0].amount);
    TEST_ASSERT_EQUAL_INT64(12345LL, first->tokens[0].last_updated_block);

    neoc_nep11_balance_t *second = neoc_neo_get_nep11_balances_get_balance(response, 1);
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_EQUAL_STRING("CuteNeoKittens", second->name);
    TEST_ASSERT_EQUAL_STRING("CNKS", second->symbol);
    TEST_ASSERT_EQUAL_UINT(2, second->tokens_count);
    TEST_ASSERT_EQUAL_STRING("10", second->tokens[1].token_id);
    TEST_ASSERT_EQUAL_STRING("6500", second->tokens[1].amount);
    TEST_ASSERT_EQUAL_INT64(654321LL, second->tokens[1].last_updated_block);

    char *json_round_trip = NULL;
    err = neoc_neo_get_nep11_balances_to_json(response, &json_round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(json_round_trip);
    TEST_ASSERT_TRUE(strlen(json_round_trip) > 0);

    neoc_neo_get_nep11_balances_t *round_trip = NULL;
    err = neoc_neo_get_nep11_balances_from_json(json_round_trip, &round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_NOT_NULL(round_trip->result);
    TEST_ASSERT_EQUAL_UINT(response->result->balances_count, round_trip->result->balances_count);
    TEST_ASSERT_EQUAL_STRING(response->result->address, round_trip->result->address);

    neoc_neo_get_nep11_balances_free(round_trip);
    neoc_free(json_round_trip);
    neoc_neo_get_nep11_balances_free(response);
}

void test_nep11_balances_parse_error_payload(void) {
    neoc_neo_get_nep11_balances_t *response = NULL;
    neoc_error_t err = neoc_neo_get_nep11_balances_from_json(sample_error_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NOT_NULL(response->error);
    TEST_ASSERT_EQUAL_STRING("invalid params", response->error);
    TEST_ASSERT_NULL(response->result);

    neoc_neo_get_nep11_balances_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nep11_balances_parse_full_payload);
    RUN_TEST(test_nep11_balances_parse_error_payload);
    return UnityEnd();
}
