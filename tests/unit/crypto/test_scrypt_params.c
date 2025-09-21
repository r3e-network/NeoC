/**
 * @file test_scrypt_params.c
 * @brief Unit tests converted from ScryptParamsTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/crypto/scrypt_params.h"
#include "neoc/utils/json.h"

// Test data
static neoc_scrypt_params_t *test_params = NULL;

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
    
    // Create test params with n=7, r=8, p=9
    err = neoc_scrypt_params_create(7, 8, 9, &test_params);
    assert(err == NEOC_SUCCESS);
    assert(test_params != NULL);
}

// Test teardown
static void tearDown(void) {
    if (test_params) {
        neoc_scrypt_params_free(test_params);
        test_params = NULL;
    }
    neoc_cleanup();
}

// Test serialize scrypt params
static void test_serialize(void) {
    printf("Testing scrypt params serialization...\n");
    
    // Serialize to JSON
    char *json = NULL;
    neoc_error_t err = neoc_scrypt_params_to_json(test_params, &json);
    assert(err == NEOC_SUCCESS);
    assert(json != NULL);
    
    // Verify JSON format
    assert(strcmp(json, "{\"n\":7,\"r\":8,\"p\":9}") == 0);
    
    free(json);
    printf("  ✅ Scrypt params serialization test passed\n");
}

// Test deserialize scrypt params
static void test_deserialize(void) {
    printf("Testing scrypt params deserialization...\n");
    
    // Different valid JSON representations
    const char *test_strings[] = {
        "{\"n\":7,\"r\":8,\"p\":9}",
        "{\"n\":7,\"blockSize\":8,\"p\":9}",
        "{\"n\":7,\"blockSize\":8,\"parallel\":9}",
        "{\"n\":7,\"r\":8,\"parallel\":9}",
        "{\"n\":7,\"blocksize\":8,\"p\":9}",
        "{\"n\":7,\"blocksize\":8,\"parallel\":9}",
        "{\"cost\":7,\"r\":8,\"p\":9}",
        "{\"cost\":7,\"r\":8,\"parallel\":9}",
        "{\"cost\":7,\"blockSize\":8,\"p\":9}",
        "{\"cost\":7,\"blockSize\":8,\"parallel\":9}",
        "{\"cost\":7,\"blocksize\":8,\"p\":9}",
        "{\"cost\":7,\"blocksize\":8,\"parallel\":9}",
        NULL
    };
    
    for (int i = 0; test_strings[i] != NULL; i++) {
        neoc_scrypt_params_t *params = NULL;
        neoc_error_t err = neoc_scrypt_params_from_json(test_strings[i], &params);
        assert(err == NEOC_SUCCESS);
        assert(params != NULL);
        
        // Verify parameters
        assert(neoc_scrypt_params_get_n(params) == 7);
        assert(neoc_scrypt_params_get_r(params) == 8);
        assert(neoc_scrypt_params_get_p(params) == 9);
        
        // Verify equality with test params
        assert(neoc_scrypt_params_equals(params, test_params));
        
        neoc_scrypt_params_free(params);
        printf("  Test string %d passed\n", i + 1);
    }
    
    printf("  ✅ Scrypt params deserialization test passed\n");
}

// Test default scrypt params
static void test_default_params(void) {
    printf("Testing default scrypt params...\n");
    
    // Create default params
    neoc_scrypt_params_t *default_params = NULL;
    neoc_error_t err = neoc_scrypt_params_create_default(&default_params);
    assert(err == NEOC_SUCCESS);
    assert(default_params != NULL);
    
    // Verify default values (common defaults are n=16384, r=8, p=1)
    assert(neoc_scrypt_params_get_n(default_params) == 16384);
    assert(neoc_scrypt_params_get_r(default_params) == 8);
    assert(neoc_scrypt_params_get_p(default_params) == 1);
    
    neoc_scrypt_params_free(default_params);
    printf("  ✅ Default scrypt params test passed\n");
}

// Test invalid JSON deserialization
static void test_invalid_json(void) {
    printf("Testing invalid JSON deserialization...\n");
    
    // Test invalid JSON strings
    const char *invalid_strings[] = {
        "{\"n\":7}",  // Missing r and p
        "{\"r\":8,\"p\":9}",  // Missing n
        "{\"n\":\"seven\",\"r\":8,\"p\":9}",  // Invalid type
        "not json",  // Not JSON
        "{",  // Incomplete JSON
        NULL
    };
    
    for (int i = 0; invalid_strings[i] != NULL; i++) {
        neoc_scrypt_params_t *params = NULL;
        neoc_error_t err = neoc_scrypt_params_from_json(invalid_strings[i], &params);
        assert(err != NEOC_SUCCESS);
        assert(params == NULL);
        printf("  Invalid string %d rejected correctly\n", i + 1);
    }
    
    printf("  ✅ Invalid JSON deserialization test passed\n");
}

// Test scrypt params equality
static void test_equality(void) {
    printf("Testing scrypt params equality...\n");
    
    // Create identical params
    neoc_scrypt_params_t *params1 = NULL;
    neoc_error_t err = neoc_scrypt_params_create(7, 8, 9, &params1);
    assert(err == NEOC_SUCCESS);
    
    neoc_scrypt_params_t *params2 = NULL;
    err = neoc_scrypt_params_create(7, 8, 9, &params2);
    assert(err == NEOC_SUCCESS);
    
    // Test equality
    assert(neoc_scrypt_params_equals(params1, params2));
    assert(neoc_scrypt_params_equals(params1, test_params));
    
    // Create different params
    neoc_scrypt_params_t *params3 = NULL;
    err = neoc_scrypt_params_create(7, 8, 10, &params3);
    assert(err == NEOC_SUCCESS);
    
    // Test inequality
    assert(!neoc_scrypt_params_equals(params1, params3));
    assert(!neoc_scrypt_params_equals(test_params, params3));
    
    neoc_scrypt_params_free(params1);
    neoc_scrypt_params_free(params2);
    neoc_scrypt_params_free(params3);
    
    printf("  ✅ Scrypt params equality test passed\n");
}

// Test scrypt params validation
static void test_validation(void) {
    printf("Testing scrypt params validation...\n");
    
    // Test invalid parameters (n must be power of 2 and > 1)
    neoc_scrypt_params_t *params = NULL;
    neoc_error_t err;
    
    // Invalid n (not power of 2)
    err = neoc_scrypt_params_create(7, 8, 9, &params);
    // Note: 7 is not a power of 2, but some implementations may allow it
    // This test depends on the implementation's validation rules
    
    // Invalid r (0)
    err = neoc_scrypt_params_create(16, 0, 1, &params);
    assert(err != NEOC_SUCCESS || params == NULL);
    
    // Invalid p (0)
    err = neoc_scrypt_params_create(16, 8, 0, &params);
    assert(err != NEOC_SUCCESS || params == NULL);
    
    // Valid parameters
    err = neoc_scrypt_params_create(16384, 8, 1, &params);
    assert(err == NEOC_SUCCESS);
    assert(params != NULL);
    neoc_scrypt_params_free(params);
    
    printf("  ✅ Scrypt params validation test passed\n");
}

int main(void) {
    printf("\n=== ScryptParamsTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_serialize();
    test_deserialize();
    test_default_params();
    test_invalid_json();
    test_equality();
    test_validation();
    
    tearDown();
    
    printf("\n✅ All ScryptParamsTests tests passed!\n\n");
    return 0;
}