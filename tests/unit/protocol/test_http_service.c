#include "unity.h"
#include "neoc/neoc.h"
#include "neoc/protocol/http/http_service.h"

static neoc_http_service_t *service = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_create(&service));
}

void tearDown(void) {
    neoc_http_service_free(service);
    service = NULL;
    neoc_cleanup();
}

void test_http_service_headers_and_url(void) {
    TEST_ASSERT_EQUAL_INT(0, neoc_http_service_get_header_count(service));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_add_header(service,
                                                       "Authorization",
                                                       "Bearer token"));
    TEST_ASSERT_EQUAL_INT(1, neoc_http_service_get_header_count(service));

    const neoc_http_header_t *header = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_get_header(service, 0, &header));
    TEST_ASSERT_EQUAL_STRING("Authorization", header->name);
    TEST_ASSERT_EQUAL_STRING("Bearer token", header->value);

    const char *value = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_find_header(service,
                                                         "Authorization",
                                                         &value));
    TEST_ASSERT_EQUAL_STRING("Bearer token", value);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_remove_header(service,
                                                          "Authorization"));
    TEST_ASSERT_EQUAL_INT(0, neoc_http_service_get_header_count(service));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_add_header(service,
                                                       "X-Test",
                                                       "1"));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_clear_headers(service));
    TEST_ASSERT_EQUAL_INT(0, neoc_http_service_get_header_count(service));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_set_url(service,
                                                     "https://example.com"));
    TEST_ASSERT_EQUAL_STRING("https://example.com",
                             neoc_http_service_get_url(service));
}

void test_http_service_from_base_wrapper(void) {
    neoc_service_t *base = neoc_http_service_get_base(service);
    TEST_ASSERT_NOT_NULL(base);

    neoc_http_service_t *wrapper = neoc_http_service_from_base(base);
    TEST_ASSERT_NOT_NULL(wrapper);
    TEST_ASSERT_EQUAL_PTR(base, neoc_http_service_get_base(wrapper));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_add_header(wrapper,
                                                       "ViaWrapper",
                                                       "yes"));
    TEST_ASSERT_EQUAL_INT(1, neoc_http_service_get_header_count(wrapper));

    neoc_http_service_free(wrapper);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_http_service_set_include_raw_responses(service, true));
    TEST_ASSERT_TRUE(neoc_http_service_includes_raw_responses(service));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_http_service_headers_and_url);
    RUN_TEST(test_http_service_from_base_wrapper);
    UNITY_END();
}
