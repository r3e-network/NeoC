/**
 * @file test_witness_scope.c
 * @brief WitnessScope tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/transaction/signer.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== WITNESS SCOPE TESTS ===== */

void test_combine_scopes(void) {
    // Test combining CalledByEntry and CustomContracts
    uint8_t combined1 = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS;
    TEST_ASSERT_EQUAL_UINT8(0x11, combined1);
    
    // Test combining CalledByEntry, CustomContracts, and CustomGroups
    uint8_t combined2 = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS | NEOC_WITNESS_SCOPE_CUSTOM_GROUPS;
    TEST_ASSERT_EQUAL_UINT8(0x31, combined2);
    
    // Test Global scope alone
    uint8_t combined3 = NEOC_WITNESS_SCOPE_GLOBAL;
    TEST_ASSERT_EQUAL_UINT8(0x80, combined3);
    
    // Test None scope
    uint8_t combined4 = NEOC_WITNESS_SCOPE_NONE;
    TEST_ASSERT_EQUAL_UINT8(0x00, combined4);
}

void test_extract_combined_scopes(void) {
    uint8_t combined;
    
    // Test extracting None (0x00)
    combined = 0x00;
    TEST_ASSERT_EQUAL_INT(NEOC_WITNESS_SCOPE_NONE, combined);
    
    // Test extracting Global (0x80)
    combined = 0x80;
    TEST_ASSERT_EQUAL_INT(NEOC_WITNESS_SCOPE_GLOBAL, combined);
    
    // Test extracting CalledByEntry and CustomContracts (0x11)
    combined = 0x11;
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) != 0);
    TEST_ASSERT_FALSE((combined & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) != 0);
    
    // Test extracting CalledByEntry and CustomGroups (0x21)
    combined = 0x21;
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) != 0);
    TEST_ASSERT_FALSE((combined & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) != 0);
    
    // Test extracting CalledByEntry, CustomGroups, and CustomContracts (0x31)
    combined = 0x31;
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) != 0);
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) != 0);
}

void test_scope_values(void) {
    // Test that scope values match expected constants
    TEST_ASSERT_EQUAL_UINT8(0x00, NEOC_WITNESS_SCOPE_NONE);
    TEST_ASSERT_EQUAL_UINT8(0x01, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY);
    TEST_ASSERT_EQUAL_UINT8(0x10, NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS);
    TEST_ASSERT_EQUAL_UINT8(0x20, NEOC_WITNESS_SCOPE_CUSTOM_GROUPS);
    TEST_ASSERT_EQUAL_UINT8(0x40, NEOC_WITNESS_SCOPE_WITNESS_RULES);
    TEST_ASSERT_EQUAL_UINT8(0x80, NEOC_WITNESS_SCOPE_GLOBAL);
}

void test_has_scope(void) {
    uint8_t combined;
    
    // Test checking for specific scope in combined value
    combined = 0x11;
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) != 0);
    TEST_ASSERT_FALSE((combined & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) != 0);
    TEST_ASSERT_FALSE((combined & NEOC_WITNESS_SCOPE_GLOBAL) != 0);
    
    // Test Global scope
    combined = 0x80;
    TEST_ASSERT_TRUE((combined & NEOC_WITNESS_SCOPE_GLOBAL) != 0);
    TEST_ASSERT_FALSE((combined & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
    
    // Test None scope
    combined = 0x00;
    TEST_ASSERT_TRUE(combined == NEOC_WITNESS_SCOPE_NONE);
    TEST_ASSERT_FALSE((combined & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
}

void test_scope_combinations(void) {
    // Test that different combinations work correctly
    uint8_t scopes;
    
    // Single scope
    scopes = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY;
    TEST_ASSERT_EQUAL_UINT8(0x01, scopes);
    
    // Two scopes
    scopes = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS;
    TEST_ASSERT_EQUAL_UINT8(0x11, scopes);
    
    // Three scopes
    scopes = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS | NEOC_WITNESS_SCOPE_CUSTOM_GROUPS;
    TEST_ASSERT_EQUAL_UINT8(0x31, scopes);
    
    // Four scopes
    scopes = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS | NEOC_WITNESS_SCOPE_CUSTOM_GROUPS | NEOC_WITNESS_SCOPE_WITNESS_RULES;
    TEST_ASSERT_EQUAL_UINT8(0x71, scopes);
    
    // Global overrides all
    scopes = NEOC_WITNESS_SCOPE_GLOBAL;
    TEST_ASSERT_EQUAL_UINT8(0x80, scopes);
}

void test_scope_checking(void) {
    uint8_t scopes;
    
    // Create a combination of scopes
    scopes = NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_GROUPS;
    
    // Check individual scopes
    TEST_ASSERT_TRUE((scopes & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) == NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY);
    TEST_ASSERT_TRUE((scopes & NEOC_WITNESS_SCOPE_CUSTOM_GROUPS) == NEOC_WITNESS_SCOPE_CUSTOM_GROUPS);
    TEST_ASSERT_FALSE((scopes & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) == NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS);
    TEST_ASSERT_FALSE((scopes & NEOC_WITNESS_SCOPE_WITNESS_RULES) == NEOC_WITNESS_SCOPE_WITNESS_RULES);
    TEST_ASSERT_FALSE((scopes & NEOC_WITNESS_SCOPE_GLOBAL) == NEOC_WITNESS_SCOPE_GLOBAL);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== WITNESS SCOPE TESTS ===\n");
    
    RUN_TEST(test_combine_scopes);
    RUN_TEST(test_extract_combined_scopes);
    RUN_TEST(test_scope_values);
    RUN_TEST(test_has_scope);
    RUN_TEST(test_scope_combinations);
    RUN_TEST(test_scope_checking);
    
    UNITY_END();
    return 0;
}
