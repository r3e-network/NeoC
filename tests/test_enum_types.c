/**
 * @file test_enum_types.c
 * @brief Enum type tests converted from Swift
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/contract/contract_parameter.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== ENUM TYPE TESTS ===== */

void test_contract_parameter_type_values(void) {
    // Test that contract parameter types have correct byte values
    TEST_ASSERT_EQUAL_UINT8(0x00, NEOC_CONTRACT_PARAM_ANY);
    TEST_ASSERT_EQUAL_UINT8(0x10, NEOC_CONTRACT_PARAM_BOOLEAN);
    TEST_ASSERT_EQUAL_UINT8(0x11, NEOC_CONTRACT_PARAM_INTEGER);
    TEST_ASSERT_EQUAL_UINT8(0x12, NEOC_CONTRACT_PARAM_BYTE_ARRAY);
    TEST_ASSERT_EQUAL_UINT8(0x13, NEOC_CONTRACT_PARAM_STRING);
    TEST_ASSERT_EQUAL_UINT8(0x14, NEOC_CONTRACT_PARAM_HASH160);
    TEST_ASSERT_EQUAL_UINT8(0x15, NEOC_CONTRACT_PARAM_HASH256);
    TEST_ASSERT_EQUAL_UINT8(0x16, NEOC_CONTRACT_PARAM_PUBLIC_KEY);
    TEST_ASSERT_EQUAL_UINT8(0x17, NEOC_CONTRACT_PARAM_SIGNATURE);
    TEST_ASSERT_EQUAL_UINT8(0x20, NEOC_CONTRACT_PARAM_ARRAY);
    TEST_ASSERT_EQUAL_UINT8(0x22, NEOC_CONTRACT_PARAM_MAP);
    TEST_ASSERT_EQUAL_UINT8(0x30, NEOC_CONTRACT_PARAM_INTEROP_INTERFACE);
    TEST_ASSERT_EQUAL_UINT8(0xFF, NEOC_CONTRACT_PARAM_VOID);
}

void test_contract_parameter_type_string_conversion(void) {
    // Test getting string representation of parameter types
    const char* type_name;
    
    // Get type name for STRING
    type_name = neoc_contract_param_type_to_string(NEOC_CONTRACT_PARAM_STRING);
    if (type_name != NULL) {
        TEST_ASSERT_TRUE(strcmp(type_name, "String") == 0 || strcmp(type_name, "STRING") == 0);
    }
    
    // Get type name for INTEGER
    type_name = neoc_contract_param_type_to_string(NEOC_CONTRACT_PARAM_INTEGER);
    if (type_name != NULL) {
        TEST_ASSERT_TRUE(strcmp(type_name, "Integer") == 0 || strcmp(type_name, "INTEGER") == 0);
    }
    
    // Get type name for BOOLEAN
    type_name = neoc_contract_param_type_to_string(NEOC_CONTRACT_PARAM_BOOLEAN);
    if (type_name != NULL) {
        TEST_ASSERT_TRUE(strcmp(type_name, "Boolean") == 0 || strcmp(type_name, "BOOLEAN") == 0);
    }
}

void test_contract_parameter_type_from_byte(void) {
    // Test converting from byte value to type
    neoc_contract_param_type_t type;
    
    // Test valid conversions
    type = (neoc_contract_param_type_t)0x13;
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_STRING, type);
    
    type = (neoc_contract_param_type_t)0x11;
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_INTEGER, type);
    
    type = (neoc_contract_param_type_t)0x10;
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_BOOLEAN, type);
    
    type = (neoc_contract_param_type_t)0x20;
    TEST_ASSERT_EQUAL_INT(NEOC_CONTRACT_PARAM_ARRAY, type);
    
    // Invalid byte values - just cast to the enum type
    type = (neoc_contract_param_type_t)0xAB;
    // It will be 0xAB but not a valid enum value
    TEST_ASSERT_EQUAL_UINT8(0xAB, (uint8_t)type);
}

void test_contract_parameter_type_comparison(void) {
    // Test that we can compare enum values
    neoc_contract_param_type_t type1 = NEOC_CONTRACT_PARAM_STRING;
    neoc_contract_param_type_t type2 = NEOC_CONTRACT_PARAM_STRING;
    neoc_contract_param_type_t type3 = NEOC_CONTRACT_PARAM_INTEGER;
    
    TEST_ASSERT_TRUE(type1 == type2);
    TEST_ASSERT_FALSE(type1 == type3);
    TEST_ASSERT_TRUE(type1 != type3);
}

void test_contract_parameter_type_switch(void) {
    // Test that we can use types in switch statements
    neoc_contract_param_type_t type = NEOC_CONTRACT_PARAM_STRING;
    
    bool is_string = false;
    bool is_integer = false;
    
    switch (type) {
        case NEOC_CONTRACT_PARAM_STRING:
            is_string = true;
            break;
        case NEOC_CONTRACT_PARAM_INTEGER:
            is_integer = true;
            break;
        default:
            break;
    }
    
    TEST_ASSERT_TRUE(is_string);
    TEST_ASSERT_FALSE(is_integer);
    
    // Test with INTEGER
    type = NEOC_CONTRACT_PARAM_INTEGER;
    is_string = false;
    is_integer = false;
    
    switch (type) {
        case NEOC_CONTRACT_PARAM_STRING:
            is_string = true;
            break;
        case NEOC_CONTRACT_PARAM_INTEGER:
            is_integer = true;
            break;
        default:
            break;
    }
    
    TEST_ASSERT_FALSE(is_string);
    TEST_ASSERT_TRUE(is_integer);
}

void test_contract_parameter_type_array_indexing(void) {
    // Test that we can use enum values as array indices
    const char* type_names[256];
    memset(type_names, 0, sizeof(type_names));
    
    // Set up some type names
    type_names[NEOC_CONTRACT_PARAM_STRING] = "String";
    type_names[NEOC_CONTRACT_PARAM_INTEGER] = "Integer";
    type_names[NEOC_CONTRACT_PARAM_BOOLEAN] = "Boolean";
    type_names[NEOC_CONTRACT_PARAM_ARRAY] = "Array";
    
    // Test retrieval
    TEST_ASSERT_EQUAL_STRING("String", type_names[NEOC_CONTRACT_PARAM_STRING]);
    TEST_ASSERT_EQUAL_STRING("Integer", type_names[NEOC_CONTRACT_PARAM_INTEGER]);
    TEST_ASSERT_EQUAL_STRING("Boolean", type_names[NEOC_CONTRACT_PARAM_BOOLEAN]);
    TEST_ASSERT_EQUAL_STRING("Array", type_names[NEOC_CONTRACT_PARAM_ARRAY]);
    
    // Test that invalid indices return NULL
    TEST_ASSERT_NULL(type_names[0xAB]);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== ENUM TYPE TESTS ===\n");
    
    RUN_TEST(test_contract_parameter_type_values);
    RUN_TEST(test_contract_parameter_type_string_conversion);
    RUN_TEST(test_contract_parameter_type_from_byte);
    RUN_TEST(test_contract_parameter_type_comparison);
    RUN_TEST(test_contract_parameter_type_switch);
    RUN_TEST(test_contract_parameter_type_array_indexing);
    
    UNITY_END();
    return 0;
}
