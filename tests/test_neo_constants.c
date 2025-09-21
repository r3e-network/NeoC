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
    // Check if NEO token hash is defined
    #ifdef NEOC_NEO_TOKEN_HASH
    // Neo token script hash
    const char* expected_neo_hash = "ef4073a0f2b305a38ec4050e4d3d28bc40ea63f5";
    char neo_hash_str[41];
    neoc_hash160_to_hex(&NEOC_NEO_TOKEN_HASH, neo_hash_str, sizeof(neo_hash_str));
    TEST_ASSERT_EQUAL_STRING(expected_neo_hash, neo_hash_str);
    #else
    TEST_IGNORE_MESSAGE("NEO token hash constant not defined");
    #endif
}

void test_gas_token_constants(void) {
    // Check if GAS token hash is defined
    #ifdef NEOC_GAS_TOKEN_HASH
    // GAS token script hash
    const char* expected_gas_hash = "d2a4cff31913016155e38e474a2c06d08be276cf";
    char gas_hash_str[41];
    neoc_hash160_to_hex(&NEOC_GAS_TOKEN_HASH, gas_hash_str, sizeof(gas_hash_str));
    TEST_ASSERT_EQUAL_STRING(expected_gas_hash, gas_hash_str);
    #else
    TEST_IGNORE_MESSAGE("GAS token hash constant not defined");
    #endif
}

void test_address_version(void) {
    // Neo N3 address version should be 0x35 (53 decimal)
    #ifdef NEOC_ADDRESS_VERSION
    TEST_ASSERT_EQUAL_UINT8(0x35, NEOC_ADDRESS_VERSION);
    #else
    // Try to test if it's defined somewhere else
    const uint8_t expected_version = 0x35;
    TEST_IGNORE_MESSAGE("Address version constant not verified");
    #endif
}

void test_max_transaction_size(void) {
    // Maximum transaction size
    #ifdef NEOC_MAX_TRANSACTION_SIZE
    TEST_ASSERT_TRUE(NEOC_MAX_TRANSACTION_SIZE >= 102400); // At least 100KB
    TEST_ASSERT_TRUE(NEOC_MAX_TRANSACTION_SIZE <= 1048576); // At most 1MB
    #else
    TEST_IGNORE_MESSAGE("Max transaction size constant not defined");
    #endif
}

void test_max_script_size(void) {
    // Maximum script size
    #ifdef NEOC_MAX_SCRIPT_SIZE
    TEST_ASSERT_TRUE(NEOC_MAX_SCRIPT_SIZE >= 65536); // At least 64KB
    TEST_ASSERT_TRUE(NEOC_MAX_SCRIPT_SIZE <= 1048576); // At most 1MB
    #else
    TEST_IGNORE_MESSAGE("Max script size constant not defined");
    #endif
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
    #ifdef NEOC_PRIVATE_KEY_SIZE
    TEST_ASSERT_EQUAL_UINT32(32, NEOC_PRIVATE_KEY_SIZE);
    #endif
    
    #ifdef NEOC_PUBLIC_KEY_SIZE_COMPRESSED
    TEST_ASSERT_EQUAL_UINT32(33, NEOC_PUBLIC_KEY_SIZE_COMPRESSED);
    #endif
    
    #ifdef NEOC_PUBLIC_KEY_SIZE_UNCOMPRESSED
    TEST_ASSERT_EQUAL_UINT32(65, NEOC_PUBLIC_KEY_SIZE_UNCOMPRESSED);
    #endif
    
    #ifdef NEOC_SIGNATURE_SIZE
    TEST_ASSERT_EQUAL_UINT32(64, NEOC_SIGNATURE_SIZE);
    #endif
    
    #ifdef NEOC_HASH160_SIZE
    TEST_ASSERT_EQUAL_UINT32(20, NEOC_HASH160_SIZE);
    #endif
    
    #ifdef NEOC_HASH256_SIZE
    TEST_ASSERT_EQUAL_UINT32(32, NEOC_HASH256_SIZE);
    #endif
}

void test_address_constants(void) {
    // Test address-related constants
    #ifdef NEOC_ADDRESS_MAX_LENGTH
    TEST_ASSERT_TRUE(NEOC_ADDRESS_MAX_LENGTH >= 34);
    TEST_ASSERT_TRUE(NEOC_ADDRESS_MAX_LENGTH <= 64);
    #else
    TEST_IGNORE_MESSAGE("Address max length constant not defined");
    #endif
    
    #ifdef NEOC_WIF_MAX_LENGTH
    TEST_ASSERT_TRUE(NEOC_WIF_MAX_LENGTH >= 52);
    TEST_ASSERT_TRUE(NEOC_WIF_MAX_LENGTH <= 64);
    #else
    TEST_IGNORE_MESSAGE("WIF max length constant not defined");
    #endif
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

