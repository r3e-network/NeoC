#include "unity.h"

#include <neoc/protocol/core/response/neo_get_next_block_validators.h>

static const char *sample_validators_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 1,\n"
    "  \"result\": [\n"
    "    {\n"
    "      \"publickey\": \"03f1ec3c1e283e880de6e9c489f0f27c19007c53385aaa4c0c917c320079edadf2\",\n"
    "      \"votes\": \"0\",\n"
    "      \"active\": false\n"
    "    },\n"
    "    {\n"
    "      \"publickey\": \"02494f3ff953e45ca4254375187004f17293f90a1aa4b1a89bc07065bc1da521f6\",\n"
    "      \"votes\": \"91600000\",\n"
    "      \"active\": true\n"
    "    }\n"
    "  ]\n"
    "}";

static const char *sample_empty_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 67,\n"
    "  \"result\": []\n"
    "}";

static const char *sample_error_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 5,\n"
    "  \"error\": {\"code\": -32602, \"message\": \"invalid params\"}\n"
    "}";

void setUp(void) {}
void tearDown(void) {}

void test_next_block_validators_parse_full_payload(void) {
    neoc_neo_get_next_block_validators_t *response = NULL;
    neoc_error_t err = neoc_neo_get_next_block_validators_from_json(sample_validators_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_TRUE(neoc_neo_get_next_block_validators_has_validators(response));
    TEST_ASSERT_EQUAL_UINT(2, neoc_neo_get_next_block_validators_get_count(response));

    neoc_validator_t *first = neoc_neo_get_next_block_validators_get_validator(response, 0);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_STRING("03f1ec3c1e283e880de6e9c489f0f27c19007c53385aaa4c0c917c320079edadf2", neoc_validator_get_public_key(first));
    TEST_ASSERT_EQUAL_STRING("0", neoc_validator_get_votes(first));
    TEST_ASSERT_FALSE(neoc_validator_is_active(first));

    neoc_validator_t *second = neoc_neo_get_next_block_validators_get_validator(response, 1);
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_EQUAL_STRING("02494f3ff953e45ca4254375187004f17293f90a1aa4b1a89bc07065bc1da521f6", neoc_validator_get_public_key(second));
    TEST_ASSERT_EQUAL_STRING("91600000", neoc_validator_get_votes(second));
    TEST_ASSERT_TRUE(neoc_validator_is_active(second));

    TEST_ASSERT_EQUAL_UINT(1, neoc_neo_get_next_block_validators_get_active_count(response));

    char *round_trip = NULL;
    err = neoc_neo_get_next_block_validators_to_json(response, &round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_TRUE(strlen(round_trip) > 0);

    neoc_neo_get_next_block_validators_t *parsed = NULL;
    err = neoc_neo_get_next_block_validators_from_json(round_trip, &parsed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(parsed);
    TEST_ASSERT_EQUAL_UINT(neoc_neo_get_next_block_validators_get_count(response),
                           neoc_neo_get_next_block_validators_get_count(parsed));
    TEST_ASSERT_EQUAL_UINT(neoc_neo_get_next_block_validators_get_active_count(response),
                           neoc_neo_get_next_block_validators_get_active_count(parsed));

    neoc_neo_get_next_block_validators_free(parsed);
    neoc_free(round_trip);
    neoc_neo_get_next_block_validators_free(response);
}

void test_next_block_validators_parse_empty_payload(void) {
    neoc_neo_get_next_block_validators_t *response = NULL;
    neoc_error_t err = neoc_neo_get_next_block_validators_from_json(sample_empty_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_UINT(0, neoc_neo_get_next_block_validators_get_count(response));
    TEST_ASSERT_EQUAL_UINT(0, neoc_neo_get_next_block_validators_get_active_count(response));

    neoc_neo_get_next_block_validators_free(response);
}

void test_next_block_validators_parse_error_payload(void) {
    neoc_neo_get_next_block_validators_t *response = NULL;
    neoc_error_t err = neoc_neo_get_next_block_validators_from_json(sample_error_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NOT_NULL(response->error);
    TEST_ASSERT_EQUAL_STRING("invalid params", response->error);
    TEST_ASSERT_NULL(response->result);

    neoc_neo_get_next_block_validators_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_next_block_validators_parse_full_payload);
    RUN_TEST(test_next_block_validators_parse_empty_payload);
    RUN_TEST(test_next_block_validators_parse_error_payload);
    return UnityEnd();
}
