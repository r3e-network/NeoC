#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/protocol/core/response.h"

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_response_result_flow(void) {
    neoc_response_t *response = neoc_response_create(7);
    TEST_ASSERT_NOT_NULL(response);
    int *value = neoc_malloc(sizeof(int));
    TEST_ASSERT_NOT_NULL(value);
    *value = 1234;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_response_set_result(response, value));
    TEST_ASSERT_FALSE(neoc_response_has_error(response));
    void *out = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_response_get_result(response, &out));
    TEST_ASSERT_EQUAL_PTR(value, out);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_response_set_raw(response, "{\"result\":1234}"));
    TEST_ASSERT_NOT_NULL(response->raw_response);

    neoc_response_free(response);
}

void test_response_error_and_to_string(void) {
    neoc_response_t *response = neoc_response_create(5);
    TEST_ASSERT_NOT_NULL(response);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_response_set_error(response,
                                                  -100,
                                                  "Boom",
                                                  "details"));
    TEST_ASSERT_TRUE(neoc_response_has_error(response));

    void *out = (void*)0xDE;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_PROTOCOL,
                          neoc_response_get_result(response, &out));

    neoc_rpc_error_t *err = response->error;
    TEST_ASSERT_NOT_NULL(err);
    char *err_str = neoc_rpc_error_to_string(err);
    TEST_ASSERT_NOT_NULL(err_str);
    TEST_ASSERT_NOT_NULL(strstr(err_str, "Boom"));
    neoc_free(err_str);

    neoc_response_free(response);
}

void test_rpc_error_create_and_free(void) {
    neoc_rpc_error_t *error = neoc_rpc_error_create(-1, "msg", NULL);
    TEST_ASSERT_NOT_NULL(error);
    TEST_ASSERT_EQUAL_INT(-1, error->code);
    TEST_ASSERT_EQUAL_STRING("msg", error->message);
    TEST_ASSERT_NULL(error->data);
    neoc_rpc_error_free(error);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_response_result_flow);
    RUN_TEST(test_response_error_and_to_string);
    RUN_TEST(test_rpc_error_create_and_free);
    UNITY_END();
}
