#include "unity.h"

#include <neoc/protocol/core/response/neo_get_mem_pool.h>
#include <neoc/types/neoc_hash256.h>

static const char *sample_full_json =
    "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":67,"
        "\"result\":{"
            "\"height\":5492,"
            "\"verified\":["
                "\"0x9786cce0dddb524c40ddbdd5e31a41ed1f6b5c8a683c122f627ca4a007a7cf4e\","
                "\"0xb488ad25eb474f89d5ca3f985cc047ca96bc7373a6d3da8c0f192722896c1cd7\""
            "],"
            "\"unverified\":["
                "\"0x9786cce0dddb524c40ddbdd5e31a41ed1f6b5c8a683c122f627ca4a007a7cf4e\","
                "\"0xb488ad25eb474f89d5ca3f985cc047ca96bc7373a6d3da8c0f192722896c1cd7\""
            "]"
        "}"
    "}";

static const char *sample_empty_json =
    "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":82,"
        "\"result\":{"
            "\"height\":\"5492\","
            "\"verified\":[],"
            "\"unverified\":[]"
        "}"
    "}";

static const char *sample_error_json =
    "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":12,"
        "\"error\":{"
            "\"code\":-32602,"
            "\"message\":\"invalid params\""
        "}"
    "}";

void setUp(void) {}
void tearDown(void) {}

static void assert_hash_equals(const char *expected, const neoc_hash256_t *actual) {
    neoc_hash256_t expected_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash256_from_string(expected, &expected_hash));
    TEST_ASSERT_TRUE(neoc_hash256_equal(&expected_hash, actual));
}

void test_mem_pool_parses_full_payload(void) {
    neoc_neo_get_mem_pool_t *response = NULL;
    neoc_error_t err = neoc_neo_get_mem_pool_from_json(sample_full_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(67, response->id);
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_INT(5492, response->result->height);
    TEST_ASSERT_EQUAL_UINT(2, response->result->verified_count);
    TEST_ASSERT_EQUAL_UINT(2, response->result->unverified_count);

    neoc_hash256_t hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_get_mem_pool_get_verified_transaction(response, 0, &hash));
    assert_hash_equals("0x9786cce0dddb524c40ddbdd5e31a41ed1f6b5c8a683c122f627ca4a007a7cf4e", &hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_get_mem_pool_get_unverified_transaction(response, 1, &hash));
    assert_hash_equals("0xb488ad25eb474f89d5ca3f985cc047ca96bc7373a6d3da8c0f192722896c1cd7", &hash);

    char *json_round_trip = NULL;
    err = neoc_neo_get_mem_pool_to_json(response, &json_round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(json_round_trip);
    TEST_ASSERT_TRUE(strlen(json_round_trip) > 0);

    neoc_neo_get_mem_pool_t *round_trip = NULL;
    err = neoc_neo_get_mem_pool_from_json(json_round_trip, &round_trip);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(round_trip);
    TEST_ASSERT_NOT_NULL(round_trip->result);
    TEST_ASSERT_EQUAL_INT(response->result->height, round_trip->result->height);
    TEST_ASSERT_EQUAL_UINT(response->result->verified_count, round_trip->result->verified_count);
    TEST_ASSERT_EQUAL_UINT(response->result->unverified_count, round_trip->result->unverified_count);

    neoc_neo_get_mem_pool_free(round_trip);
    neoc_free(json_round_trip);
    neoc_neo_get_mem_pool_free(response);
}

void test_mem_pool_parses_empty_payload(void) {
    neoc_neo_get_mem_pool_t *response = NULL;
    neoc_error_t err = neoc_neo_get_mem_pool_from_json(sample_empty_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_INT(5492, response->result->height);
    TEST_ASSERT_EQUAL_UINT(0, response->result->verified_count);
    TEST_ASSERT_EQUAL_UINT(0, response->result->unverified_count);
    TEST_ASSERT_EQUAL_UINT(0, neoc_neo_get_mem_pool_get_total_count(response));

    neoc_neo_get_mem_pool_free(response);
}

void test_mem_pool_parses_error(void) {
    neoc_neo_get_mem_pool_t *response = NULL;
    neoc_error_t err = neoc_neo_get_mem_pool_from_json(sample_error_json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NOT_NULL(response->error);
    TEST_ASSERT_EQUAL_STRING("invalid params", response->error);
    TEST_ASSERT_NULL(response->result);

    neoc_neo_get_mem_pool_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mem_pool_parses_full_payload);
    RUN_TEST(test_mem_pool_parses_empty_payload);
    RUN_TEST(test_mem_pool_parses_error);
    return UnityEnd();
}
