#include "unity.h"

#include <string.h>

#include <neoc/protocol/core/response/neo_find_states.h>

void setUp(void) {
}

void tearDown(void) {
}

static void assert_result_entry(const neoc_find_states_result_t *entry,
                                const char *expected_key,
                                const char *expected_value) {
    TEST_ASSERT_NOT_NULL(entry);
    TEST_ASSERT_NOT_NULL(entry->key);
    TEST_ASSERT_NOT_NULL(entry->value);
    TEST_ASSERT_EQUAL_STRING(expected_key, entry->key);
    TEST_ASSERT_EQUAL_STRING(expected_value, entry->value);
}

void test_find_states_response_parses_full_payload(void) {
    const char *json =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":42,"
        "\"result\":{"
            "\"firstProof\":\"FIRST_PROOF_DATA\","
            "\"lastProof\":\"LAST_PROOF_DATA\","
            "\"truncated\":true,"
            "\"results\":["
                "{\"key\":\"AAEC\",\"value\":\"AQID\"},"
                "{\"key\":\"BBEE\",\"value\":\"CCGG\"}"
            "]"
        "}"
        "}";

    neoc_find_states_response_t *response = NULL;
    neoc_error_t err = neoc_find_states_response_from_json(json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);

    TEST_ASSERT_NOT_NULL(response->jsonrpc);
    TEST_ASSERT_EQUAL_STRING("2.0", response->jsonrpc);
    TEST_ASSERT_EQUAL_INT(42, response->id);
    TEST_ASSERT_EQUAL_INT(0, response->error_code);
    TEST_ASSERT_NULL(response->error_message);
    TEST_ASSERT_NOT_NULL(response->result);

    neoc_find_states_t *states = response->result;
    TEST_ASSERT_EQUAL_STRING("FIRST_PROOF_DATA", states->first_proof);
    TEST_ASSERT_EQUAL_STRING("LAST_PROOF_DATA", states->last_proof);
    TEST_ASSERT_TRUE(states->truncated);
    TEST_ASSERT_EQUAL_UINT(2, states->results_count);
    assert_result_entry(states->results[0], "AAEC", "AQID");
    assert_result_entry(states->results[1], "BBEE", "CCGG");

    neoc_find_states_response_free(response);
}

void test_find_states_response_handles_minimal_payload(void) {
    const char *json =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":7,"
        "\"result\":{"
            "\"results\":[]"
        "}"
        "}";

    neoc_find_states_response_t *response = NULL;
    neoc_error_t err = neoc_find_states_response_from_json(json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_NOT_NULL(response->jsonrpc);
    TEST_ASSERT_EQUAL_STRING("2.0", response->jsonrpc);
    TEST_ASSERT_EQUAL_INT(7, response->id);
    TEST_ASSERT_NOT_NULL(response->result);

    neoc_find_states_t *states = response->result;
    TEST_ASSERT_NULL(states->first_proof);
    TEST_ASSERT_NULL(states->last_proof);
    TEST_ASSERT_FALSE(states->truncated);
    TEST_ASSERT_EQUAL_UINT(0, states->results_count);

    neoc_find_states_response_free(response);
}

void test_find_states_response_parses_error_object(void) {
    const char *json =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":99,"
        "\"error\":{"
            "\"code\":-32602,"
            "\"message\":\"Invalid params\""
        "}"
        "}";

    neoc_find_states_response_t *response = NULL;
    neoc_error_t err = neoc_find_states_response_from_json(json, &response);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_EQUAL_INT(-32602, response->error_code);
    TEST_ASSERT_NOT_NULL(response->error_message);
    TEST_ASSERT_EQUAL_STRING("Invalid params", response->error_message);
    TEST_ASSERT_NULL(response->result);

    neoc_find_states_response_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_find_states_response_parses_full_payload);
    RUN_TEST(test_find_states_response_handles_minimal_payload);
    RUN_TEST(test_find_states_response_parses_error_object);
    return UnityEnd();
}
