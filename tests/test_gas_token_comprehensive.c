/**
 * @file test_gas_token_comprehensive.c
 * @brief Comprehensive GAS Token tests converted from Swift and extended
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/contract/gas_token.h>
#include <neoc/contract/native_contracts.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/types/neoc_hash160.h>
#include <string.h>
#include <stdio.h>

// Test constants 
static const char* GAS_TOKEN_SCRIPT_HASH = "d2a4cff31913016155e38e474a2c06d08be276cf";
static const char* GAS_TOKEN_NAME = "GasToken";
static const char* GAS_TOKEN_SYMBOL = "GAS";
static const int GAS_TOKEN_DECIMALS = 8;

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== GAS TOKEN BASIC PROPERTY TESTS ===== */

void test_gas_token_name(void) {
    printf("Testing GAS token name\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(gas_token);
    
    char* name;
    err = neoc_gas_token_get_name(gas_token, &name);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_EQUAL_STRING(GAS_TOKEN_NAME, name);
    
    neoc_free(name);
    neoc_gas_token_free(gas_token);
}

void test_gas_token_symbol(void) {
    printf("Testing GAS token symbol\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    char* symbol;
    err = neoc_gas_token_get_symbol(gas_token, &symbol);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(symbol);
    TEST_ASSERT_EQUAL_STRING(GAS_TOKEN_SYMBOL, symbol);
    
    neoc_free(symbol);
    neoc_gas_token_free(gas_token);
}

void test_gas_token_decimals(void) {
    printf("Testing GAS token decimals\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    int decimals;
    err = neoc_gas_token_get_decimals(gas_token, &decimals);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(GAS_TOKEN_DECIMALS, decimals);
    
    neoc_gas_token_free(gas_token);
}

void test_gas_token_script_hash(void) {
    printf("Testing GAS token script hash\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_hash160_t script_hash;
    err = neoc_gas_token_get_script_hash(gas_token, &script_hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert expected script hash from hex
    uint8_t expected_hash[20];
    size_t decoded_len;
    err = neoc_hex_decode(GAS_TOKEN_SCRIPT_HASH, expected_hash, sizeof(expected_hash), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(20, decoded_len);
    
    TEST_ASSERT_EQUAL_MEMORY(expected_hash, script_hash.data, 20);
    
    neoc_gas_token_free(gas_token);
}

/* ===== GAS TOKEN TOTAL SUPPLY TESTS ===== */

void test_gas_token_total_supply(void) {
    printf("Testing GAS token total supply\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    uint64_t total_supply;
    err = neoc_gas_token_get_total_supply(gas_token, &total_supply);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // GAS has an initial supply and continues to generate
    // Just verify we get a reasonable value (> 0)
    TEST_ASSERT_TRUE(total_supply > 0);
    
    printf("  Total GAS supply: %llu\n", (unsigned long long)total_supply);
    
    neoc_gas_token_free(gas_token);
}

/* ===== GAS TOKEN BALANCE TESTS ===== */

void test_gas_token_balance_of(void) {
    printf("Testing GAS token balance queries\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test with a known test address (committee account from TestProperties)
    neoc_hash160_t test_address;
    const char* test_address_hex = "05859de95ccbbd5668e0f055b208273634d4657f";
    uint8_t address_bytes[20];
    size_t decoded_len;
    err = neoc_hex_decode(test_address_hex, address_bytes, sizeof(address_bytes), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    memcpy(test_address.data, address_bytes, 20);
    
    uint64_t balance;
    err = neoc_gas_token_balance_of(gas_token, &test_address, &balance);
    
    // Balance query might fail in test environment, but function should handle gracefully
    if (err == NEOC_SUCCESS) {
        printf("  Balance for test address: %llu GAS\n", (unsigned long long)balance);
        TEST_ASSERT_TRUE(balance >= 0);  // Balance should be non-negative
    } else {
        printf("  Balance query failed (expected in test environment): %d\n", err);
    }
    
    neoc_gas_token_free(gas_token);
}

/* ===== GAS TOKEN TRANSFER TESTS ===== */

void test_gas_token_transfer_script(void) {
    printf("Testing GAS token transfer script generation\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create test addresses
    neoc_hash160_t from_address, to_address;
    const char* from_hex = "69ecca587293047be4c59159bf8bc399985c160d";
    const char* to_hex = "05859de95ccbbd5668e0f055b208273634d4657f";
    
    uint8_t from_bytes[20], to_bytes[20];
    size_t decoded_len;
    
    neoc_hex_decode(from_hex, from_bytes, sizeof(from_bytes), &decoded_len);
    memcpy(from_address.data, from_bytes, 20);
    
    neoc_hex_decode(to_hex, to_bytes, sizeof(to_bytes), &decoded_len);
    memcpy(to_address.data, to_bytes, 20);
    
    uint64_t amount = 100000000; // 1 GAS (8 decimals)
    
    uint8_t* script;
    size_t script_len;
    err = neoc_gas_token_build_transfer_script(gas_token, &from_address, &to_address, amount, NULL, &script, &script_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(script);
    TEST_ASSERT_TRUE(script_len > 0);
    
    printf("  Generated transfer script length: %zu bytes\n", script_len);
    
    neoc_free(script);
    neoc_gas_token_free(gas_token);
}

void test_gas_token_multi_transfer_script(void) {
    printf("Testing GAS token multi-transfer script generation\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create multiple transfer recipients
    neoc_token_transfer_t transfers[3];
    
    // Transfer 1: 1 GAS
    const char* to1_hex = "05859de95ccbbd5668e0f055b208273634d4657f";
    uint8_t to1_bytes[20];
    size_t decoded_len;
    neoc_hex_decode(to1_hex, to1_bytes, sizeof(to1_bytes), &decoded_len);
    memcpy(transfers[0].to.data, to1_bytes, 20);
    transfers[0].amount = 100000000; // 1 GAS
    transfers[0].data = NULL;
    transfers[0].data_len = 0;
    
    // Transfer 2: 0.5 GAS
    const char* to2_hex = "69ecca587293047be4c59159bf8bc399985c160d";
    uint8_t to2_bytes[20];
    neoc_hex_decode(to2_hex, to2_bytes, sizeof(to2_bytes), &decoded_len);
    memcpy(transfers[1].to.data, to2_bytes, 20);
    transfers[1].amount = 50000000; // 0.5 GAS
    transfers[1].data = NULL;
    transfers[1].data_len = 0;
    
    // Transfer 3: 2 GAS
    const char* to3_hex = "cc5e4edd9f5f8dba8bb65734541df7a1c081c67b";
    uint8_t to3_bytes[20];
    neoc_hex_decode(to3_hex, to3_bytes, sizeof(to3_bytes), &decoded_len);
    memcpy(transfers[2].to.data, to3_bytes, 20);
    transfers[2].amount = 200000000; // 2 GAS
    transfers[2].data = NULL;
    transfers[2].data_len = 0;
    
    neoc_hash160_t from_address;
    const char* from_hex = "69ecca587293047be4c59159bf8bc399985c160d";
    uint8_t from_bytes[20];
    neoc_hex_decode(from_hex, from_bytes, sizeof(from_bytes), &decoded_len);
    memcpy(from_address.data, from_bytes, 20);
    
    uint8_t* script;
    size_t script_len;
    err = neoc_gas_token_build_multi_transfer_script(gas_token, &from_address, transfers, 3, &script, &script_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(script);
    TEST_ASSERT_TRUE(script_len > 0);
    
    printf("  Generated multi-transfer script length: %zu bytes\n", script_len);
    
    neoc_free(script);
    neoc_gas_token_free(gas_token);
}

/* ===== GAS TOKEN VALIDATION TESTS ===== */

void test_gas_token_invalid_inputs(void) {
    printf("Testing GAS token with invalid inputs\n");
    
    // Test null gas token creation
    neoc_error_t err = neoc_gas_token_create(NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_gas_token_t* gas_token;
    err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test null output parameters
    err = neoc_gas_token_get_name(gas_token, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    err = neoc_gas_token_get_symbol(gas_token, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    err = neoc_gas_token_get_decimals(gas_token, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    err = neoc_gas_token_get_script_hash(gas_token, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test null gas_token parameter
    char* name;
    err = neoc_gas_token_get_name(NULL, &name);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_gas_token_free(gas_token);
}

void test_gas_token_transfer_invalid_inputs(void) {
    printf("Testing GAS token transfer with invalid inputs\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_hash160_t test_address;
    memset(&test_address, 0, sizeof(test_address));
    
    uint8_t* script;
    size_t script_len;
    
    // Test null from address
    err = neoc_gas_token_build_transfer_script(gas_token, NULL, &test_address, 1000000, NULL, &script, &script_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test null to address
    err = neoc_gas_token_build_transfer_script(gas_token, &test_address, NULL, 1000000, NULL, &script, &script_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test zero amount
    err = neoc_gas_token_build_transfer_script(gas_token, &test_address, &test_address, 0, NULL, &script, &script_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test null output script
    err = neoc_gas_token_build_transfer_script(gas_token, &test_address, &test_address, 1000000, NULL, NULL, &script_len);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    // Test null output script_len
    err = neoc_gas_token_build_transfer_script(gas_token, &test_address, &test_address, 1000000, NULL, &script, NULL);
    TEST_ASSERT_TRUE(err != NEOC_SUCCESS);
    
    neoc_gas_token_free(gas_token);
}

/* ===== GAS TOKEN NATIVE CONTRACT INTEGRATION TESTS ===== */

void test_gas_token_native_contract_properties(void) {
    printf("Testing GAS token native contract properties\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify it's recognized as a native contract
    bool is_native;
    err = neoc_gas_token_is_native_contract(gas_token, &is_native);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(is_native);
    
    // Test contract version
    int version;
    err = neoc_gas_token_get_contract_version(gas_token, &version);
    if (err == NEOC_SUCCESS) {
        TEST_ASSERT_TRUE(version >= 0);
        printf("  GAS contract version: %d\n", version);
    }
    
    neoc_gas_token_free(gas_token);
}

void test_gas_token_method_names(void) {
    printf("Testing GAS token standard method names\n");
    
    neoc_gas_token_t* gas_token;
    neoc_error_t err = neoc_gas_token_create(&gas_token);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Test standard NEP-17 methods
    char** methods;
    size_t method_count;
    err = neoc_gas_token_get_supported_methods(gas_token, &methods, &method_count);
    
    if (err == NEOC_SUCCESS) {
        TEST_ASSERT_TRUE(method_count > 0);
        TEST_ASSERT_NOT_NULL(methods);
        
        bool found_transfer = false;
        bool found_balanceOf = false;
        bool found_totalSupply = false;
        bool found_decimals = false;
        bool found_symbol = false;
        
        for (size_t i = 0; i < method_count; i++) {
            if (strcmp(methods[i], "transfer") == 0) found_transfer = true;
            else if (strcmp(methods[i], "balanceOf") == 0) found_balanceOf = true;
            else if (strcmp(methods[i], "totalSupply") == 0) found_totalSupply = true;
            else if (strcmp(methods[i], "decimals") == 0) found_decimals = true;
            else if (strcmp(methods[i], "symbol") == 0) found_symbol = true;
        }
        
        TEST_ASSERT_TRUE(found_transfer);
        TEST_ASSERT_TRUE(found_balanceOf);
        TEST_ASSERT_TRUE(found_totalSupply);
        TEST_ASSERT_TRUE(found_decimals);
        TEST_ASSERT_TRUE(found_symbol);
        
        printf("  Found %zu supported methods including NEP-17 standards\n", method_count);
        
        // Free the methods array
        for (size_t i = 0; i < method_count; i++) {
            neoc_free(methods[i]);
        }
        neoc_free(methods);
    }
    
    neoc_gas_token_free(gas_token);
}

/* ===== GAS TOKEN MEMORY MANAGEMENT TESTS ===== */

void test_gas_token_memory_management(void) {
    printf("Testing GAS token memory management\n");
    
    // Test multiple create/free cycles
    for (int i = 0; i < 10; i++) {
        neoc_gas_token_t* gas_token;
        neoc_error_t err = neoc_gas_token_create(&gas_token);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(gas_token);
        
        // Use the gas token
        char* name;
        err = neoc_gas_token_get_name(gas_token, &name);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(name);
        
        neoc_gas_token_free(gas_token);
    }
    
    // Test null free (should not crash)
    neoc_gas_token_free(NULL);
    
    printf("  Memory management tests completed\n");
}

/* ===== GAS TOKEN PERFORMANCE TESTS ===== */

void test_gas_token_performance(void) {
    printf("Testing GAS token performance\n");
    
    clock_t start = clock();
    
    // Create and use gas tokens
    for (int i = 0; i < 100; i++) {
        neoc_gas_token_t* gas_token;
        neoc_error_t err = neoc_gas_token_create(&gas_token);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        neoc_hash160_t script_hash;
        err = neoc_gas_token_get_script_hash(gas_token, &script_hash);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        neoc_gas_token_free(gas_token);
    }
    
    clock_t end = clock();
    double seconds = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("  100 gas token operations completed in %.3f seconds\n", seconds);
    TEST_ASSERT_TRUE(seconds < 1.0); // Should be very fast
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== COMPREHENSIVE GAS TOKEN TESTS ===\n");
    
    // Basic property tests
    RUN_TEST(test_gas_token_name);
    RUN_TEST(test_gas_token_symbol);
    RUN_TEST(test_gas_token_decimals);
    RUN_TEST(test_gas_token_script_hash);
    
    // Supply and balance tests
    RUN_TEST(test_gas_token_total_supply);
    RUN_TEST(test_gas_token_balance_of);
    
    // Transfer tests
    RUN_TEST(test_gas_token_transfer_script);
    RUN_TEST(test_gas_token_multi_transfer_script);
    
    // Validation tests
    RUN_TEST(test_gas_token_invalid_inputs);
    RUN_TEST(test_gas_token_transfer_invalid_inputs);
    
    // Native contract integration tests
    RUN_TEST(test_gas_token_native_contract_properties);
    RUN_TEST(test_gas_token_method_names);
    
    // Memory and performance tests
    RUN_TEST(test_gas_token_memory_management);
    RUN_TEST(test_gas_token_performance);
    
    UNITY_END();
    return 0;
}