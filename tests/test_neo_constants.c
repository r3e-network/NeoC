/**
 * @file test_neo_constants.c
 * @brief Neo blockchain constants validation tests
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/types/neoc_hash160.h>
#include <neoc/utils/neoc_hex.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== NEO CONSTANTS TESTS ===== */

void test_neo_token_constants(void) {
    const char* expected_neo_hash = NEOC_NEO_TOKEN_HASH_HEX;
    TEST_ASSERT_EQUAL_STRING("ef4073a0f2b305a38ec4050e4d3d28bc40ea63f5", expected_neo_hash);
}

void test_gas_token_constants(void) {
    const char* expected_gas_hash = NEOC_GAS_TOKEN_HASH_HEX;
    TEST_ASSERT_EQUAL_STRING("d2a4cff31913016155e38e474a2c06d08be276cf", expected_gas_hash);
}

void test_address_version(void) {
    // Neo N3 address version should be 0x35 (53 decimal)
    TEST_ASSERT_EQUAL_UINT8(0x35, NEOC_ADDRESS_VERSION);
}

void test_max_transaction_size(void) {
    // Maximum transaction size
    TEST_ASSERT_TRUE(NEOC_MAX_TRANSACTION_SIZE >= 102400); // At least 100KB
    TEST_ASSERT_TRUE(NEOC_MAX_TRANSACTION_SIZE <= 1048576); // At most 1MB
}

void test_max_script_size(void) {
    // Maximum script size
    TEST_ASSERT_TRUE(NEOC_MAX_SCRIPT_SIZE >= 65536); // At least 64KB
    TEST_ASSERT_TRUE(NEOC_MAX_SCRIPT_SIZE <= 1048576); // At most 1MB
}

void test_opcode_values(void) {
    // Test some key opcodes
    #ifdef NEOC_OP_PUSH0
    TEST_ASSERT_EQUAL_UINT8(0x10, NEOC_OP_PUSH0);
    #endif
    
    #ifdef NEOC_OP_PUSH1
    TEST_ASSERT_EQUAL_UINT8(0x11, NEOC_OP_PUSH1);
    #endif
    
    #ifdef NEOC_OP_PUSHDATA1
    TEST_ASSERT_EQUAL_UINT8(0x0C, NEOC_OP_PUSHDATA1);
    #endif
    
    #ifdef NEOC_OP_PUSHDATA2
    TEST_ASSERT_EQUAL_UINT8(0x0D, NEOC_OP_PUSHDATA2);
    #endif
    
    #ifdef NEOC_OP_PUSHDATA4
    TEST_ASSERT_EQUAL_UINT8(0x0E, NEOC_OP_PUSHDATA4);
    #endif
    
    #ifdef NEOC_OP_SYSCALL
    TEST_ASSERT_EQUAL_UINT8(0x41, NEOC_OP_SYSCALL);
    #endif
    
    #ifdef NEOC_OP_RET
    TEST_ASSERT_EQUAL_UINT8(0x40, NEOC_OP_RET);
    #endif
}

void test_crypto_constants(void) {
    // Test crypto-related constants
    TEST_ASSERT_EQUAL_UINT32(32, NEOC_PRIVATE_KEY_SIZE);
    
    TEST_ASSERT_EQUAL_UINT32(33, NEOC_PUBLIC_KEY_SIZE_COMPRESSED);
    
    TEST_ASSERT_EQUAL_UINT32(65, NEOC_PUBLIC_KEY_SIZE_UNCOMPRESSED);
    
    TEST_ASSERT_EQUAL_UINT32(64, NEOC_SIGNATURE_SIZE);
    
    TEST_ASSERT_EQUAL_UINT32(20, NEOC_HASH160_SIZE);
    
    TEST_ASSERT_EQUAL_UINT32(32, NEOC_HASH256_SIZE);
}

void test_address_constants(void) {
    // Test address-related constants
    TEST_ASSERT_TRUE(NEOC_ADDRESS_MAX_LENGTH >= 34);
    TEST_ASSERT_TRUE(NEOC_ADDRESS_MAX_LENGTH <= 64);
    
    TEST_ASSERT_TRUE(NEOC_WIF_MAX_LENGTH >= 52);
    TEST_ASSERT_TRUE(NEOC_WIF_MAX_LENGTH <= 64);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== NEO CONSTANTS TESTS ===\n");
    
    RUN_TEST(test_neo_token_constants);
    RUN_TEST(test_gas_token_constants);
    RUN_TEST(test_address_version);
    RUN_TEST(test_max_transaction_size);
    RUN_TEST(test_max_script_size);
    RUN_TEST(test_opcode_values);
    RUN_TEST(test_crypto_constants);
    RUN_TEST(test_address_constants);
    
    UNITY_END();
    return 0;
}
