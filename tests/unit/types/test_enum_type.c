#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/utils/enum.h"

static const neoc_byte_enum_entry_t STATUS_ENTRIES[] = {
    {0x00, "pending", "Pending"},
    {0x01, "confirmed", "Confirmed"},
    {0xFF, NULL, "Unknown"}
};

static const neoc_byte_enum_def_t STATUS_ENUM = {
    STATUS_ENTRIES,
    sizeof(STATUS_ENTRIES) / sizeof(STATUS_ENTRIES[0]),
    "TransactionStatus"
};

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_enum_find_by_members(void) {
    const neoc_byte_enum_entry_t *entry = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_find_by_byte(&STATUS_ENUM, 0x01, &entry));
    TEST_ASSERT_EQUAL_STRING("Confirmed", entry->name);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_find_by_json_value(&STATUS_ENUM, "pending", &entry));
    TEST_ASSERT_EQUAL_UINT8(0x00, entry->byte_value);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_find_by_name(&STATUS_ENUM, "Unknown", &entry));
    TEST_ASSERT_EQUAL_UINT8(0xFF, entry->byte_value);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NOT_FOUND,
                          neoc_enum_find_by_byte(&STATUS_ENUM, 0x02, &entry));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NOT_FOUND,
                          neoc_enum_find_by_json_value(&STATUS_ENUM, "missing", &entry));
}

void test_enum_json_parsing_and_encoding(void) {
    const neoc_byte_enum_entry_t *entry = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_parse_json(&STATUS_ENUM, "\"confirmed\"", &entry));
    TEST_ASSERT_EQUAL_UINT8(0x01, entry->byte_value);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_parse_json(&STATUS_ENUM, "0xFF", &entry));
    TEST_ASSERT_EQUAL_STRING("Unknown", entry->name);

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_enum_to_json(entry, &json));
    TEST_ASSERT_EQUAL_STRING("255", json);
    neoc_free(json);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_parse_json(&STATUS_ENUM, "  \"pending\"  ", &entry));
    TEST_ASSERT_EQUAL_STRING("Pending", entry->name);
}

void test_enum_utility_helpers(void) {
    TEST_ASSERT_TRUE(neoc_enum_contains_byte(&STATUS_ENUM, 0x00));
    TEST_ASSERT_FALSE(neoc_enum_contains_byte(&STATUS_ENUM, 0x02));
    TEST_ASSERT_TRUE(neoc_enum_contains_json_value(&STATUS_ENUM, "pending"));
    TEST_ASSERT_FALSE(neoc_enum_contains_json_value(&STATUS_ENUM, "mystery"));

    const neoc_byte_enum_entry_t *entries = NULL;
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_enum_get_all_cases(&STATUS_ENUM, &entries, &count));
    TEST_ASSERT_EQUAL_UINT(STATUS_ENUM.count, count);
    TEST_ASSERT_EQUAL_PTR(STATUS_ENTRIES, entries);
    TEST_ASSERT_EQUAL_STRING("TransactionStatus",
                             neoc_enum_get_type_name(&STATUS_ENUM));
    TEST_ASSERT_EQUAL_UINT(3, neoc_enum_get_case_count(&STATUS_ENUM));

    uint32_t hash_a = neoc_enum_hash(&STATUS_ENTRIES[0]);
    uint32_t hash_b = neoc_enum_hash(&STATUS_ENTRIES[1]);
    TEST_ASSERT_TRUE(hash_a != hash_b);
    TEST_ASSERT_TRUE(neoc_enum_equals(&STATUS_ENTRIES[0], &STATUS_ENTRIES[0]));
    TEST_ASSERT_FALSE(neoc_enum_equals(&STATUS_ENTRIES[0], &STATUS_ENTRIES[1]));
}

void test_enum_validate_definition_errors(void) {
    static const neoc_byte_enum_entry_t DUP_ENTRIES[] = {
        {0x00, "value", "CaseA"},
        {0x00, "value2", "CaseB"}
    };
    static const neoc_byte_enum_def_t DUP_ENUM = {DUP_ENTRIES, 2, "DupEnum"};
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_STATE,
                          neoc_enum_validate_definition(&DUP_ENUM));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_enum_find_by_members);
    RUN_TEST(test_enum_json_parsing_and_encoding);
    RUN_TEST(test_enum_utility_helpers);
    RUN_TEST(test_enum_validate_definition_errors);
    UNITY_END();
}
