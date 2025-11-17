#include "neoc/protocol/rx/neo_c_rx.h"
#include "neoc/neoc_error.h"

static neoc_error_t rx_not_supported(void) {
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "NeoC reactive extensions not implemented");
}

neoc_error_t neoc_neo_c_rx_init(neoc_neo_c_rx_t *rx,
                                const neoc_neo_c_rx_vtable_t *vtable,
                                void *impl_data) {
    if (!rx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "rx is NULL");
    }
    rx->vtable = vtable;
    rx->impl_data = impl_data;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_c_rx_block_publisher(neoc_neo_c_rx_t *rx,
                                           bool full_transaction_objects,
                                           neoc_block_callback_t callback,
                                           void *user_data,
                                           neoc_subscription_t **subscription_out) {
    (void)rx;
    (void)full_transaction_objects;
    (void)callback;
    (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_supported();
}

neoc_error_t neoc_neo_c_rx_replay_blocks_publisher(neoc_neo_c_rx_t *rx,
                                                   int start_block,
                                                   int end_block,
                                                   bool full_transaction_objects,
                                                   neoc_block_callback_t callback,
                                                   void *user_data,
                                                   neoc_subscription_t **subscription_out) {
    (void)rx;
    (void)start_block;
    (void)end_block;
    (void)full_transaction_objects;
    (void)callback;
    (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_supported();
}

neoc_error_t neoc_neo_c_rx_replay_blocks_publisher_ordered(neoc_neo_c_rx_t *rx,
                                                           int start_block,
                                                           int end_block,
                                                           bool full_transaction_objects,
                                                           bool ascending,
                                                           neoc_block_callback_t callback,
                                                           void *user_data,
                                                           neoc_subscription_t **subscription_out) {
    (void)rx;
    (void)start_block;
    (void)end_block;
    (void)full_transaction_objects;
    (void)ascending;
    (void)callback;
    (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_supported();
}

neoc_error_t neoc_neo_c_rx_catch_up_to_latest_block_publisher(neoc_neo_c_rx_t *rx,
                                                              int start_block,
                                                              bool full_transaction_objects,
                                                              neoc_block_callback_t callback,
                                                              void *user_data,
                                                              neoc_subscription_t **subscription_out) {
    (void)rx;
    (void)start_block;
    (void)full_transaction_objects;
    (void)callback;
    (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_supported();
}

neoc_error_t neoc_neo_c_rx_catch_up_to_latest_and_subscribe_to_new_blocks_publisher(
    neoc_neo_c_rx_t *rx,
    int start_block,
    bool full_transaction_objects,
    neoc_block_callback_t callback,
    void *user_data,
    neoc_subscription_t **subscription_out) {
    (void)rx;
    (void)start_block;
    (void)full_transaction_objects;
    (void)callback;
    (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_supported();
}

neoc_error_t neoc_neo_c_rx_subscribe_to_new_blocks_publisher(neoc_neo_c_rx_t *rx,
                                                             bool full_transaction_objects,
                                                             neoc_block_callback_t callback,
                                                             void *user_data,
                                                             neoc_subscription_t **subscription_out) {
    (void)rx;
    (void)full_transaction_objects;
    (void)callback;
    (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_supported();
}

void neoc_neo_c_rx_free(neoc_neo_c_rx_t *rx) {
    (void)rx;
}
