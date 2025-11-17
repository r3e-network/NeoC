#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_neoc_version_and_build_info(void) {
    const char *version = neoc_get_version();
    const char *build = neoc_get_build_info();
    TEST_ASSERT_NOT_NULL(version);
    TEST_ASSERT_NOT_NULL(build);
    TEST_ASSERT_TRUE(strlen(version) > 0);
    TEST_ASSERT_TRUE(strlen(build) > 0);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_neoc_version_and_build_info);
    UNITY_END();
    return 0;
}
