#include "unity.h"
#include <string.h>

#include "neoc/neoc.h"
#include "neoc/protocol/neo_c_express.h"
#include "neoc/protocol/core/request.h"
#include "neoc/types/neoc_hash160.h"

static neoc_neo_c_t *create_fake_base(void) {
    neoc_neo_c_t *base = neoc_calloc(1, sizeof(neoc_neo_c_t));
    TEST_ASSERT_NOT_NULL(base);
    base->neo_c_service = neoc_calloc(1, sizeof(neoc_service_t));
    TEST_ASSERT_NOT_NULL(base->neo_c_service);
    base->config = NULL;
    base->neo_c_rx = NULL;
    return base;
}

static void fill_hash(neoc_hash160_t *hash) {
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        hash->data[i] = (uint8_t)(i + 1);
    }
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_express_create_and_free(void) {
    neoc_neo_c_t *base = create_fake_base();
    neoc_neo_c_express_t *express = neoc_neo_c_express_create(base);
    TEST_ASSERT_NOT_NULL(express);
    TEST_ASSERT_EQUAL_PTR(base, neoc_neo_c_express_get_base(express));
    neoc_neo_c_express_free(express);
}

void test_express_request_builders(void) {
    neoc_neo_c_t *base = create_fake_base();
    neoc_service_t *service = base->neo_c_service;
    neoc_neo_c_express_t *express = neoc_neo_c_express_create(base);
    TEST_ASSERT_NOT_NULL(express);

    neoc_request_t *request = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_get_populated_blocks(express, &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expressgetpopulatedblocks", request->method);
    TEST_ASSERT_EQUAL_PTR(service, request->service);
    neoc_request_free(request);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_get_nep17_contracts(express, &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expressgetnep17contracts", request->method);
    neoc_request_free(request);

    neoc_hash160_t hash = {{0}};
    fill_hash(&hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_get_contract_storage(express, &hash, &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expressgetcontractstorage", request->method);
    TEST_ASSERT_NOT_NULL(request->params);
    char hash_hex[NEOC_HASH160_STRING_LENGTH];
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hash160_to_string(&hash, hash_hex, sizeof(hash_hex)));
    char expected_params[NEOC_HASH160_STRING_LENGTH + 4];
    snprintf(expected_params, sizeof(expected_params), "[\"%s\"]", hash_hex);
    TEST_ASSERT_EQUAL_STRING(expected_params, request->params);
    neoc_request_free(request);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_list_contracts(express, &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expresslistcontracts", request->method);
    neoc_request_free(request);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_create_checkpoint(express, "checkpoint.neoexp", &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expresscreatecheckpoint", request->method);
    TEST_ASSERT_EQUAL_STRING("[\"checkpoint.neoexp\"]", request->params);
    neoc_request_free(request);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_list_oracle_requests(express, &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expresslistoraclerequests", request->method);
    neoc_request_free(request);

    uint64_t oracle_id = 42;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_create_oracle_response_tx(express,
                                                                      (const neoc_transaction_attribute_t *)&oracle_id,
                                                                      &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expresscreateoracleresponsetx", request->method);
    TEST_ASSERT_NOT_NULL(request->params);
    TEST_ASSERT_NOT_NULL(strstr(request->params, "\"id\":42"));
    neoc_request_free(request);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_express_shutdown(express, &request));
    TEST_ASSERT_NOT_NULL(request);
    TEST_ASSERT_EQUAL_STRING("expressshutdown", request->method);
    neoc_request_free(request);

    neoc_neo_c_express_free(express);
}

void test_express_error_handling(void) {
    neoc_neo_c_t *base = create_fake_base();
    neoc_neo_c_express_t *express = neoc_neo_c_express_create(base);
    TEST_ASSERT_NOT_NULL(express);

    neoc_request_t *request = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_neo_c_express_get_contract_storage(express, NULL, &request));
    TEST_ASSERT_NULL(request);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_neo_c_express_get_populated_blocks(express, NULL));

    neoc_neo_c_express_free(express);
}

void test_express_build_helpers(void) {
    neoc_service_t *service = neoc_calloc(1, sizeof(neoc_service_t));
    TEST_ASSERT_NOT_NULL(service);
    neoc_neo_c_express_t *express = neoc_neo_c_express_build(service);
    TEST_ASSERT_NOT_NULL(express);
    neoc_neo_c_express_free(express);

    service = neoc_calloc(1, sizeof(neoc_service_t));
    neoc_neo_c_config_t *config = neoc_neo_c_config_create();
    TEST_ASSERT_NOT_NULL(service);
    TEST_ASSERT_NOT_NULL(config);
    express = neoc_neo_c_express_build_with_config(service, config);
    TEST_ASSERT_NOT_NULL(express);
    neoc_neo_c_express_free(express);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_express_create_and_free);
    RUN_TEST(test_express_request_builders);
    RUN_TEST(test_express_error_handling);
    RUN_TEST(test_express_build_helpers);
    return UnityEnd();
}
