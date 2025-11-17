#include "unity.h"
#include "neoc/neoc.h"
#include "neoc/protocol/rx/json_rpc2_0_rx.h"

static neoc_json_rpc2_0_rx_t *rx = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    rx = neoc_json_rpc2_0_rx_create(NULL, NULL);
    TEST_ASSERT_NOT_NULL(rx);
}

void tearDown(void) {
    neoc_json_rpc2_0_rx_free(rx);
    rx = NULL;
    neoc_cleanup();
}

void test_rx_creation_and_simple_getter(void) {
    TEST_ASSERT_NULL(neoc_json_rpc2_0_rx_get_neo_c(rx));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NOT_IMPLEMENTED,
                          neoc_json_rpc2_0_rx_get_latest_block_index(rx, NULL));
}

static bool dummy_block_index_callback(int block_index, neoc_error_t error, void *user_data) {
    (void)block_index;
    (void)user_data;
    return error == NEOC_SUCCESS;
}

void test_rx_block_index_subscription_returns_not_implemented(void) {
    neoc_subscription_t *subscription = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NOT_IMPLEMENTED,
                          neoc_json_rpc2_0_rx_block_index_publisher(rx,
                                                                    1000,
                                                                    dummy_block_index_callback,
                                                                    NULL,
                                                                    &subscription));
    TEST_ASSERT_NULL(subscription);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_rx_creation_and_simple_getter);
    RUN_TEST(test_rx_block_index_subscription_returns_not_implemented);
    UNITY_END();
}
