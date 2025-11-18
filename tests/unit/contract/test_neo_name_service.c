#include "unity.h"
#include <string.h>
#include <stdint.h>
#include "neoc/neoc.h"
#include "neoc/contract/neoc_name_service.h"
#include "neoc/types/neoc_hash160.h"

#define MOCK_SCRIPT_BUFFER 128

typedef enum {
    MOCK_RPC_NONE,
    MOCK_RPC_BOOLEAN,
    MOCK_RPC_STRING,
    MOCK_RPC_INTEGER
} mock_rpc_mode_t;

typedef struct mock_rpc_client {
    neoc_error_t (*invoke_script)(void *client, const uint8_t *script, size_t len, void *result);
    mock_rpc_mode_t mode;
    bool invoked;
    bool boolean_value;
    uint64_t integer_value;
    const char *string_value;
    neoc_error_t return_code;
    size_t script_length;
    uint8_t script_prefix[MOCK_SCRIPT_BUFFER];
} mock_rpc_client_t;

static void make_hash(neoc_hash160_t *hash, uint8_t seed) {
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

static neoc_error_t mock_invoke_script(void *client, const uint8_t *script, size_t len, void *result) {
    mock_rpc_client_t *mock = (mock_rpc_client_t *)client;
    mock->invoked = true;
    mock->script_length = len;
    size_t copy_len = len < MOCK_SCRIPT_BUFFER ? len : MOCK_SCRIPT_BUFFER;
    if (copy_len > 0) {
        memcpy(mock->script_prefix, script, copy_len);
    }

    if (mock->return_code != NEOC_SUCCESS) {
        return mock->return_code;
    }

    switch (mock->mode) {
    case MOCK_RPC_BOOLEAN: {
        struct { int type; bool boolean_value; } *res = result;
        res->type = 0;
        res->boolean_value = mock->boolean_value;
        break;
    }
    case MOCK_RPC_STRING: {
        struct { int type; const char *string_value; } *res = result;
        res->type = 0;
        res->string_value = mock->string_value;
        break;
    }
    case MOCK_RPC_INTEGER: {
        struct { int type; uint64_t integer_value; } *res = result;
        res->type = 0;
        res->integer_value = mock->integer_value;
        break;
    }
    default:
        break;
    }

    return NEOC_SUCCESS;
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_neoc_nns_create_and_free(void) {
    neoc_neo_name_service_t *nns = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));
    TEST_ASSERT_NOT_NULL(nns);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_set_rpc_client(nns, NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, neoc_nns_set_rpc_client(NULL, NULL));
    neoc_nns_free(nns);
}

void test_neoc_nns_resolve_requires_rpc(void) {
    neoc_neo_name_service_t *nns = NULL;
    char *result = (char *)0x1;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));
    TEST_ASSERT_NOT_NULL(nns);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NETWORK,
                          neoc_nns_resolve(nns, "example.neo", NEOC_NNS_RECORD_TYPE_A, &result));
    TEST_ASSERT_NULL(result);

    neoc_nns_free(nns);
}

void test_neoc_nns_resolve_returns_value_via_rpc(void) {
    neoc_neo_name_service_t *nns = NULL;
    char *result = NULL;
    mock_rpc_client_t mock = {
        .invoke_script = mock_invoke_script,
        .mode = MOCK_RPC_STRING,
        .string_value = "1.2.3.4",
        .return_code = NEOC_SUCCESS
    };

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_set_rpc_client(nns, &mock));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nns_resolve(nns, "example.neo", NEOC_NNS_RECORD_TYPE_A, &result));
    TEST_ASSERT_TRUE(mock.invoked);
    TEST_ASSERT_TRUE(mock.script_length > 0);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("1.2.3.4", result);

    neoc_free(result);
    neoc_nns_free(nns);
}

void test_neoc_nns_is_available_uses_rpc_result(void) {
    neoc_neo_name_service_t *nns = NULL;
    bool available = false;
    mock_rpc_client_t mock = {
        .invoke_script = mock_invoke_script,
        .mode = MOCK_RPC_BOOLEAN,
        .boolean_value = true,
        .return_code = NEOC_SUCCESS
    };

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_set_rpc_client(nns, &mock));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_is_available(nns, "new.neo", &available));
    TEST_ASSERT_TRUE(mock.invoked);
    TEST_ASSERT_TRUE(available);

    neoc_nns_free(nns);
}

static uint64_t gas_amount(uint64_t amount) {
    return amount * 100000000ULL;
}

void test_neoc_nns_get_price_fallback_tiers(void) {
    neoc_neo_name_service_t *nns = NULL;
    uint64_t price = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NETWORK, neoc_nns_get_price(nns, 2, &price));
    TEST_ASSERT_EQUAL_UINT64(gas_amount(1000), price);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NETWORK, neoc_nns_get_price(nns, 3, &price));
    TEST_ASSERT_EQUAL_UINT64(gas_amount(500), price);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NETWORK, neoc_nns_get_price(nns, 4, &price));
    TEST_ASSERT_EQUAL_UINT64(gas_amount(200), price);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NETWORK, neoc_nns_get_price(nns, 5, &price));
    TEST_ASSERT_EQUAL_UINT64(gas_amount(60), price);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NETWORK, neoc_nns_get_price(nns, 6, &price));
    TEST_ASSERT_EQUAL_UINT64(gas_amount(10), price);

    neoc_nns_free(nns);
}

void test_neoc_nns_get_price_with_rpc_result(void) {
    neoc_neo_name_service_t *nns = NULL;
    uint64_t price = 0;
    mock_rpc_client_t mock = {
        .invoke_script = mock_invoke_script,
        .mode = MOCK_RPC_INTEGER,
        .integer_value = 424242,
        .return_code = NEOC_SUCCESS
    };

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_set_rpc_client(nns, &mock));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_get_price(nns, 8, &price));
    TEST_ASSERT_TRUE(mock.invoked);
    TEST_ASSERT_EQUAL_UINT64(424242, price);

    neoc_nns_free(nns);
}

void test_neoc_nns_register_and_set_record(void) {
    neoc_neo_name_service_t *nns = NULL;
    neoc_hash160_t owner = {{0}};

    make_hash(&owner, 0x20);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_create(&nns));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nns_register(nns, "example.neo", &owner));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nns_set_record(nns, "example.neo", NEOC_NNS_RECORD_TYPE_TXT, "hello"));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_nns_register(NULL, "example.neo", &owner));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_nns_register(nns, NULL, &owner));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_nns_register(nns, "example.neo", NULL));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_nns_set_record(NULL, "example.neo", NEOC_NNS_RECORD_TYPE_A, "1.2.3.4"));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_nns_set_record(nns, NULL, NEOC_NNS_RECORD_TYPE_A, "1.2.3.4"));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_nns_set_record(nns, "example.neo", NEOC_NNS_RECORD_TYPE_A, NULL));

    neoc_nns_free(nns);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_neoc_nns_create_and_free);
    RUN_TEST(test_neoc_nns_resolve_requires_rpc);
    RUN_TEST(test_neoc_nns_resolve_returns_value_via_rpc);
    RUN_TEST(test_neoc_nns_is_available_uses_rpc_result);
    RUN_TEST(test_neoc_nns_get_price_fallback_tiers);
    RUN_TEST(test_neoc_nns_get_price_with_rpc_result);
    RUN_TEST(test_neoc_nns_register_and_set_record);
    return UnityEnd();
}
