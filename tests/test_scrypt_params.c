/**
 * @file test_scrypt_params.c
 * @brief Scrypt parameters tests
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/nep2.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== SCRYPT PARAMS TESTS ===== */

void test_default_params(void) {
    // Test default parameters
    TEST_ASSERT_EQUAL_UINT32(16384, NEOC_NEP2_DEFAULT_PARAMS.n);
    TEST_ASSERT_EQUAL_UINT32(8, NEOC_NEP2_DEFAULT_PARAMS.r);
    TEST_ASSERT_EQUAL_UINT32(8, NEOC_NEP2_DEFAULT_PARAMS.p);
}

void test_light_params(void) {
    // Test light parameters
    TEST_ASSERT_EQUAL_UINT32(1024, NEOC_NEP2_LIGHT_PARAMS.n);
    TEST_ASSERT_EQUAL_UINT32(1, NEOC_NEP2_LIGHT_PARAMS.r);
    TEST_ASSERT_EQUAL_UINT32(1, NEOC_NEP2_LIGHT_PARAMS.p);
}

void test_custom_params(void) {
    // Create custom parameters
    neoc_nep2_params_t params = {
        .n = 7,
        .r = 8,
        .p = 9
    };
    
    TEST_ASSERT_EQUAL_UINT32(7, params.n);
    TEST_ASSERT_EQUAL_UINT32(8, params.r);
    TEST_ASSERT_EQUAL_UINT32(9, params.p);
}

void test_params_comparison(void) {
    neoc_nep2_params_t params1 = {
        .n = 100,
        .r = 200,
        .p = 300
    };
    
    neoc_nep2_params_t params2 = {
        .n = 100,
        .r = 200,
        .p = 300
    };
    
    neoc_nep2_params_t params3 = {
        .n = 100,
        .r = 200,
        .p = 400  // Different
    };
    
    // Test equality
    TEST_ASSERT_TRUE(params1.n == params2.n && 
                     params1.r == params2.r && 
                     params1.p == params2.p);
    
    // Test inequality
    TEST_ASSERT_FALSE(params1.n == params3.n && 
                      params1.r == params3.r && 
                      params1.p == params3.p);
}

void test_params_with_nep2_encryption(void) {
    // Test using custom params with NEP2 encryption
    uint8_t private_key[32];
    memset(private_key, 0x42, 32);  // Test private key
    
    const char* password = "TestPassword123";
    char encrypted_key[64];
    
    // Use custom light params
    neoc_nep2_params_t custom_params = {
        .n = 2048,
        .r = 2,
        .p = 2
    };
    
    neoc_error_t err = neoc_nep2_encrypt(private_key, password, &custom_params,
                                          encrypted_key, sizeof(encrypted_key));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_TRUE(strlen(encrypted_key) > 0);
}

void test_params_memory_cost(void) {
    // Test memory cost calculation
    neoc_nep2_params_t params = {
        .n = 16384,
        .r = 8,
        .p = 1
    };
    
    // Memory usage = 128 * r * N bytes
    // For n=16384, r=8: 128 * 8 * 16384 = 16MB
    uint64_t memory_bytes = 128ULL * params.r * params.n;
    TEST_ASSERT_EQUAL_UINT64(16777216, memory_bytes);  // 16MB
    
    // Light params memory usage
    uint64_t light_memory = 128ULL * NEOC_NEP2_LIGHT_PARAMS.r * NEOC_NEP2_LIGHT_PARAMS.n;
    TEST_ASSERT_EQUAL_UINT64(131072, light_memory);  // 128KB
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== SCRYPT PARAMS TESTS ===\n");
    
    RUN_TEST(test_default_params);
    RUN_TEST(test_light_params);
    RUN_TEST(test_custom_params);
    RUN_TEST(test_params_comparison);
    RUN_TEST(test_params_with_nep2_encryption);
    RUN_TEST(test_params_memory_cost);
    
    UNITY_END();
    return 0;
}
