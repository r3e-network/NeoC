/**
 * @file integration_test_framework.h
 * @brief Integration test framework for NeoC SDK
 */

#ifndef NEOC_INTEGRATION_TEST_FRAMEWORK_H
#define NEOC_INTEGRATION_TEST_FRAMEWORK_H

#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc.h"

// Test result tracking
typedef enum {
    INTEGRATION_TEST_PASS,
    INTEGRATION_TEST_FAIL,
    INTEGRATION_TEST_SKIP,
    INTEGRATION_TEST_TIMEOUT
} integration_test_result_t;

// Test metadata
typedef struct {
    const char *name;
    const char *category;
    const char *description;
    bool requires_network;
    bool requires_neo_node;
    int timeout_seconds;
} integration_test_info_t;

// Test function signature
typedef integration_test_result_t (*integration_test_fn)(void *context);

// Test case structure
typedef struct {
    integration_test_info_t info;
    integration_test_fn setup;
    integration_test_fn test;
    integration_test_fn teardown;
    void *context;
} integration_test_case_t;

// Test suite structure
typedef struct {
    const char *name;
    const char *description;
    integration_test_case_t *tests;
    size_t test_count;
    size_t passed;
    size_t failed;
    size_t skipped;
} integration_test_suite_t;

// Neo node connection info
typedef struct {
    const char *rpc_url;
    const char *network;
    uint32_t network_magic;
    bool use_testnet;
    bool use_local;
} neo_node_config_t;

// Integration test context
typedef struct {
    neo_node_config_t node_config;
    void *neo_service;
    void *test_data;
    char *error_message;
    bool verbose;
} integration_test_context_t;

// Framework functions
neoc_error_t integration_test_init(integration_test_context_t *ctx);
void integration_test_cleanup(integration_test_context_t *ctx);

// Suite management
integration_test_suite_t *integration_test_suite_create(const char *name, const char *description);
void integration_test_suite_free(integration_test_suite_t *suite);
neoc_error_t integration_test_suite_add_test(integration_test_suite_t *suite, const integration_test_case_t *test);
integration_test_result_t integration_test_suite_run(integration_test_suite_t *suite, integration_test_context_t *ctx);

// Test execution
integration_test_result_t integration_test_run_single(const integration_test_case_t *test, integration_test_context_t *ctx);
bool integration_test_check_prerequisites(const integration_test_case_t *test, integration_test_context_t *ctx);

// Reporting
void integration_test_print_summary(const integration_test_suite_t *suite);
void integration_test_save_report(const integration_test_suite_t *suite, const char *filename);

// Assertions for integration tests
#define INTEGRATION_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("Assertion failed: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return INTEGRATION_TEST_FAIL; \
        } \
    } while (0)

#define INTEGRATION_ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("Assertion failed: expected %ld, got %ld at %s:%d\n", \
                   (long)(expected), (long)(actual), __FILE__, __LINE__); \
            return INTEGRATION_TEST_FAIL; \
        } \
    } while (0)

#define INTEGRATION_ASSERT_SUCCESS(err) \
    do { \
        if ((err) != NEOC_SUCCESS) { \
            printf("Operation failed with error: %d at %s:%d\n", err, __FILE__, __LINE__); \
            return INTEGRATION_TEST_FAIL; \
        } \
    } while (0)

// Utility functions
bool integration_test_neo_node_available(const neo_node_config_t *config);
bool integration_test_network_available(void);
void integration_test_wait_for_block(integration_test_context_t *ctx);
neoc_error_t integration_test_deploy_test_contract(integration_test_context_t *ctx, void **contract);

#endif // NEOC_INTEGRATION_TEST_FRAMEWORK_H