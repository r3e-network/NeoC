#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/protocol/core/request.h"

static void *const SERVICE_PTR = (void*)0x1234;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_request_create_and_to_json(void) {
    neoc_request_set_counter(1);
    neoc_request_t *request = neoc_request_create("getblockcount", NULL, SERVICE_PTR);
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_INT(1, request->id);
    TEST_ASSERT_EQUAL_PTR(SERVICE_PTR, request->service);

    char *json = neoc_request_to_json(request);
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"method\":\"getblockcount\""));
    TEST_ASSERT_NOT_NULL(strstr(json, "\"params\":[]"));

    neoc_free(json);
    neoc_request_free(request);
}

void test_request_from_json_parses_fields(void) {
    const char *json = "{\"jsonrpc\":\"2.0\",\"method\":\"getblock\",\"params\":[1],\"id\":99}";
    neoc_request_t *request = neoc_request_from_json(json, SERVICE_PTR);
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("getblock", request->method);
    TEST_ASSERT_EQUAL_INT(99, request->id);
    TEST_ASSERT_EQUAL_PTR(SERVICE_PTR, request->service);

    char *request_json = neoc_request_to_json(request);
    TEST_ASSERT_NOT_NULL(request_json);
    TEST_ASSERT_NOT_NULL(strstr(request_json, "\"params\":[1]"));

    neoc_free(request_json);
    neoc_request_free(request);
}

void test_request_counter_controls(void) {
    neoc_request_set_counter(42);
    neoc_request_t *a = neoc_request_create("methodA", "[]", NULL);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_INT(42, a->id);
    TEST_ASSERT_EQUAL_INT(43, neoc_request_get_counter());

    neoc_request_t *b = neoc_request_create("methodB", "[]", NULL);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_INT(43, b->id);
    TEST_ASSERT_EQUAL_INT(44, neoc_request_get_counter());

    neoc_request_free(a);
    neoc_request_free(b);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_request_create_and_to_json);
    RUN_TEST(test_request_from_json_parses_fields);
    RUN_TEST(test_request_counter_controls);
    UNITY_END();
}
