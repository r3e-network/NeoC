#include "unity.h"
#include "neoc/neoc.h"
#include "neoc/transaction/witness_scope.h"

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_witness_scope_json_roundtrip(void) {
    const struct {
        neoc_witness_scope_t scope;
        const char *json;
    } cases[] = {
        {NEOC_WITNESS_SCOPE_NONE, "None"},
        {NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY, "CalledByEntry"},
        {NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS, "CustomContracts"},
        {NEOC_WITNESS_SCOPE_CUSTOM_GROUPS, "CustomGroups"},
        {NEOC_WITNESS_SCOPE_WITNESS_RULES, "WitnessRules"},
        {NEOC_WITNESS_SCOPE_GLOBAL, "Global"}
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        neoc_witness_scope_t parsed = NEOC_WITNESS_SCOPE_NONE;
        TEST_ASSERT_EQUAL_STRING(cases[i].json,
                                 neoc_witness_scope_to_json(cases[i].scope));
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                              neoc_witness_scope_from_json(cases[i].json, &parsed));
        TEST_ASSERT_EQUAL_INT(cases[i].scope, parsed);
    }

    neoc_witness_scope_t invalid;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_FORMAT,
                          neoc_witness_scope_from_json("UnknownScope", &invalid));
}

void test_witness_scope_combine_and_extract(void) {
    neoc_witness_scope_t scopes[] = {
        NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY,
        NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS,
        NEOC_WITNESS_SCOPE_WITNESS_RULES
    };
    uint8_t combined = neoc_witness_scope_combine(scopes, 3);
    TEST_ASSERT_TRUE(neoc_witness_scope_has(combined, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY));
    TEST_ASSERT_TRUE(neoc_witness_scope_has(combined, NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS));
    TEST_ASSERT_FALSE(neoc_witness_scope_has(combined, NEOC_WITNESS_SCOPE_GLOBAL));

    neoc_witness_scope_t extracted[5];
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_witness_scope_extract(combined, extracted, 5, &count));
    TEST_ASSERT_EQUAL_UINT(3, count);

    uint8_t combined_none = neoc_witness_scope_combine(NULL, 0);
    TEST_ASSERT_EQUAL_UINT8(0, combined_none);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_witness_scope_extract(NEOC_WITNESS_SCOPE_NONE,
                                                     extracted,
                                                     5,
                                                     &count));
    TEST_ASSERT_EQUAL_UINT(1, count);
    TEST_ASSERT_EQUAL_INT(NEOC_WITNESS_SCOPE_NONE, extracted[0]);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_witness_scope_extract(combined,
                                                     NULL,
                                                     5,
                                                     &count));
}

void test_witness_scope_validation(void) {
    TEST_ASSERT_TRUE(neoc_witness_scope_is_valid(NEOC_WITNESS_SCOPE_GLOBAL));
    TEST_ASSERT_FALSE(neoc_witness_scope_is_valid((neoc_witness_scope_t)0x02));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_witness_scope_json_roundtrip);
    RUN_TEST(test_witness_scope_combine_and_extract);
    RUN_TEST(test_witness_scope_validation);
    UNITY_END();
}
