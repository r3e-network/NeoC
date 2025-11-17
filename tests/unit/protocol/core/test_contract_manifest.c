#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/contract_manifest.h"
#include "neoc/protocol/contract_response_types.h"

static neoc_contract_manifest_t *manifest = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_contract_manifest_create(&manifest));
    TEST_ASSERT_NOT_NULL(manifest);
}

void tearDown(void) {
    neoc_contract_manifest_free(manifest);
    manifest = NULL;
    neoc_cleanup();
}

void test_manifest_setters_and_accessors(void) {
    TEST_ASSERT_EQUAL_STRING("Contract",
                             neoc_contract_manifest_get_name(manifest));

    const char *standards[] = {"NEP-17", "NEP-11"};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_manifest_set_standards(manifest,
                                                               standards,
                                                               2));
    const char **returned = NULL;
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_manifest_get_standards(manifest,
                                                                 &returned,
                                                                 &count));
    TEST_ASSERT_EQUAL_UINT(2, count);
    TEST_ASSERT_EQUAL_STRING("NEP-17", returned[0]);
    TEST_ASSERT_EQUAL_STRING("NEP-11", returned[1]);

    neoc_contract_method_parameter_t params[1] = {
        {.name = "from", .type = 0x07}
    };
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_manifest_add_method(manifest,
                                                             "balanceOf",
                                                             params,
                                                             1,
                                                             4,
                                                             0x05,
                                                             true));
    TEST_ASSERT_EQUAL_UINT(1, neoc_contract_manifest_get_method_count(manifest));
    const neoc_contract_method_t *method = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_manifest_get_method_by_name(manifest,
                                                                    "balanceOf",
                                                                    &method));
    TEST_ASSERT_NOT_NULL(method);
    TEST_ASSERT_EQUAL_STRING("balanceOf", neoc_contract_method_get_name(method));
    TEST_ASSERT_TRUE(neoc_contract_method_is_safe(method));
}

void test_manifest_to_json_and_clone(void) {
    char *json = NULL;
    size_t json_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_manifest_to_json(manifest, &json, &json_len));
    TEST_ASSERT_NOT_NULL(json);
    TEST_ASSERT_TRUE(json_len > 0);
    TEST_ASSERT_NOT_NULL(strstr(json, "\"name\": \"Contract\""));

    neoc_contract_manifest_t *clone = neoc_contract_manifest_clone(manifest);
    TEST_ASSERT_NOT_NULL(clone);
    TEST_ASSERT_EQUAL_STRING(neoc_contract_manifest_get_name(manifest),
                             neoc_contract_manifest_get_name(clone));

    neoc_contract_manifest_free(clone);
    neoc_free(json);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_manifest_setters_and_accessors);
    RUN_TEST(test_manifest_to_json_and_clone);
    UNITY_END();
}
