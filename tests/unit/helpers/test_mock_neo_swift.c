#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

static void assert_non_empty_string(const char *value) {
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_GREATER_THAN_INT(0, (int)strlen(value));
}

void test_mock_neoswift_environment_ready(void) {
    assert_non_empty_string(neoc_get_version());
    assert_non_empty_string(neoc_get_build_info());
}

void test_mock_neoswift_multiple_init_cleanup(void) {
    neoc_cleanup();
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    assert_non_empty_string(neoc_get_version());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mock_neoswift_environment_ready);
    RUN_TEST(test_mock_neoswift_multiple_init_cleanup);
    return UNITY_END();
}
