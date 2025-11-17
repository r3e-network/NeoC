#include "unity.h"

#include <neoc/protocol/core/response/neo_get_nep11_transfers.h>

static const char *sample_transfers_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 1,\n"
    "  \"result\": {\n"
    "    \"sent\": [\n"
    "      {\n"
    "        \"tokenid\": \"1\",\n"
    "        \"timestamp\": 1554283931,\n"
    "        \"assethash\": \"1aada0032aba1ef6d1f07bbd8bec1d85f5380fb3\",\n"
    "        \"transferaddress\": \"AYwgBNMepiv5ocGcyNT4mA8zPLTQ8pDBis\",\n"
    "        \"amount\": \"100000000000\",\n"
    "        \"blockindex\": 368082,\n"
    "        \"transfernotifyindex\": 0,\n"
    "        \"txhash\": \"240ab1369712ad2782b99a02a8f9fcaa41d1e96322017ae90d0449a3ba52a564\"\n"
    "      },\n"
    "      {\n"
    "        \"tokenid\": \"2\",\n"
    "        \"timestamp\": 1554880287,\n"
    "        \"assethash\": \"1aada0032aba1ef6d1f07bbd8bec1d85f5380fb3\",\n"
    "        \"transferaddress\": \"AYwgBNMepiv5ocGcyNT4mA8zPLTQ8pDBis\",\n"
    "        \"amount\": \"100000000000\",\n"
    "        \"blockindex\": 397769,\n"
    "        \"transfernotifyindex\": 0,\n"
    "        \"txhash\": \"12fdf7ce8b2388d23ab223854cb29e5114d8288c878de23b7924880f82dfc834\"\n"
    "      }\n"
    "    ],\n"
    "    \"received\": [\n"
    "      {\n"
    "        \"tokenid\": \"3\",\n"
    "        \"timestamp\": 1555651816,\n"
    "        \"assethash\": \"600c4f5200db36177e3e8a09e9f18e2fc7d12a0f\",\n"
    "        \"transferaddress\": \"AYwgBNMepiv5ocGcyNT4mA8zPLTQ8pDBis\",\n"
    "        \"amount\": \"1000000\",\n"
    "        \"blockindex\": 436036,\n"
    "        \"transfernotifyindex\": 0,\n"
    "        \"txhash\": \"df7683ece554ecfb85cf41492c5f143215dd43ef9ec61181a28f922da06aba58\"\n"
    "      }\n"
    "    ],\n"
    "    \"address\": \"AbHgdBaWEnHkCiLtDZXjhvhaAK2cwFh5pF\"\n"
    "  }\n"
    "}";

static const char *sample_error_json =
    "{\n"
    "  \"jsonrpc\": \"2.0\",\n"
    "  \"id\": 9,\n"
    "  \"error\": {\"code\": -32602, \"message\": \"invalid params\"}\n"
    "}";

void setUp(void) {}
void tearDown(void) {}

static void assert_hash160_string(const char *expected, const neoc_hash160_t *value) {
    neoc_hash160_t expected_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_string(expected, &expected_hash));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&expected_hash, value));
}

static void assert_hash256_string(const char *expected, const neoc_hash256_t *value) {
    neoc_hash256_t expected_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash256_from_string(expected, &expected_hash));
    TEST_ASSERT_TRUE(neoc_hash256_equal(&expected_hash, value));
}

void test_nep11_transfers_parse_full_payload(void) {
    neoc_neo_get_nep11_transfers_t *response = NULL;
    neoc_error_t err = neoc_neo_get_nep11_transfers_from_json(sample_transfers_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_TRUE(neoc_neo_get_nep11_transfers_has_transfers(response));
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_STRING("AbHgdBaWEnHkCiLtDZXjhvhaAK2cwFh5pF", response->result->transfer_address);
    TEST_ASSERT_EQUAL_UINT(2, response->result->sent_count);
    TEST_ASSERT_EQUAL_UINT(1, response->result->received_count);

    neoc_nep11_transfer_t *sent = neoc_neo_get_nep11_transfers_get_sent(response, 0);
    TEST_ASSERT_NOT_NULL(sent);
    TEST_ASSERT_EQUAL_STRING("1", sent->token_id);
    TEST_ASSERT_EQUAL_INT64(1554283931LL, sent->timestamp);
    assert_hash160_string("1aada0032aba1ef6d1f07bbd8bec1d85f5380fb3", &sent->asset_hash);
    TEST_ASSERT_EQUAL_STRING("AYwgBNMepiv5ocGcyNT4mA8zPLTQ8pDBis", sent->transfer_address);
    TEST_ASSERT_EQUAL_INT64(100000000000LL, sent->amount);
    TEST_ASSERT_EQUAL_INT(368082, sent->block_index);
    TEST_ASSERT_EQUAL_INT(0, sent->transfer_notify_index);
    assert_hash256_string("240ab1369712ad2782b99a02a8f9fcaa41d1e96322017ae90d0449a3ba52a564", &sent->tx_hash);

    neoc_nep11_transfer_t *received = neoc_neo_get_nep11_transfers_get_received(response, 0);
    TEST_ASSERT_NOT_NULL(received);
    TEST_ASSERT_EQUAL_STRING("3", received->token_id);
    TEST_ASSERT_EQUAL_INT64(1555651816LL, received->timestamp);
    assert_hash160_string("600c4f5200db36177e3e8a09e9f18e2fc7d12a0f", &received->asset_hash);
    TEST_ASSERT_EQUAL_INT64(1000000LL, received->amount);

    char *json_round_trip = NULL;
    err = neoc_neo_get_nep11_transfers_to_json(response, &json_round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(json_round_trip);
    TEST_ASSERT_TRUE(strlen(json_round_trip) > 0);

    neoc_neo_get_nep11_transfers_t *round_trip = NULL;
    err = neoc_neo_get_nep11_transfers_from_json(json_round_trip, &round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_NOT_NULL(round_trip->result);
    TEST_ASSERT_EQUAL_UINT(response->result->sent_count, round_trip->result->sent_count);
    TEST_ASSERT_EQUAL_UINT(response->result->received_count, round_trip->result->received_count);
    TEST_ASSERT_EQUAL_STRING(response->result->transfer_address, round_trip->result->transfer_address);

    neoc_neo_get_nep11_transfers_free(round_trip);
    neoc_free(json_round_trip);
    neoc_neo_get_nep11_transfers_free(response);
}

void test_nep11_transfers_parse_error_payload(void) {
    neoc_neo_get_nep11_transfers_t *response = NULL;
    neoc_error_t err = neoc_neo_get_nep11_transfers_from_json(sample_error_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NOT_NULL(response->error);
    TEST_ASSERT_EQUAL_STRING("invalid params", response->error);
    TEST_ASSERT_NULL(response->result);

    neoc_neo_get_nep11_transfers_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nep11_transfers_parse_full_payload);
    RUN_TEST(test_nep11_transfers_parse_error_payload);
    return UnityEnd();
}
