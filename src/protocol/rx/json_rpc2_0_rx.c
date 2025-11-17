#include "neoc/protocol/rx/json_rpc2_0_rx.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

static neoc_error_t rx_not_implemented(void) {
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "Reactive JSON-RPC functionality not implemented");
}

neoc_json_rpc2_0_rx_t *neoc_json_rpc2_0_rx_create(neoc_neo_c_t *neo_c, void *executor_service) {
    neoc_json_rpc2_0_rx_t *rx = neoc_calloc(1, sizeof(neoc_json_rpc2_0_rx_t));
    if (!rx) {
        neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate reactive client");
        return NULL;
    }
    rx->neo_c = neo_c;
    rx->executor_service = executor_service;
    rx->subscriptions = NULL;
    rx->subscription_count = 0;
    return rx;
}

neoc_error_t neoc_json_rpc2_0_rx_block_index_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                       int polling_interval,
                                                       neoc_block_index_callback_t callback,
                                                       void *user_data,
                                                       neoc_subscription_t **subscription_out) {
    (void)rx; (void)polling_interval; (void)callback; (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_implemented();
}

neoc_error_t neoc_json_rpc2_0_rx_block_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                 bool full_transaction_objects,
                                                 int polling_interval,
                                                 neoc_block_callback_t callback,
                                                 void *user_data,
                                                 neoc_subscription_t **subscription_out) {
    (void)rx; (void)full_transaction_objects; (void)polling_interval;
    (void)callback; (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_implemented();
}

neoc_error_t neoc_json_rpc2_0_rx_replay_blocks_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                         int start_block,
                                                         int end_block,
                                                         bool full_transaction_objects,
                                                         bool ascending,
                                                         neoc_block_callback_t callback,
                                                         void *user_data,
                                                         neoc_subscription_t **subscription_out) {
    (void)rx; (void)start_block; (void)end_block;
    (void)full_transaction_objects; (void)ascending;
    (void)callback; (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_implemented();
}

neoc_error_t neoc_json_rpc2_0_rx_catch_up_to_latest_and_subscribe(neoc_json_rpc2_0_rx_t *rx,
                                                                  int start_block,
                                                                  bool full_transaction_objects,
                                                                  int polling_interval,
                                                                  neoc_block_callback_t callback,
                                                                  void *user_data,
                                                                  neoc_subscription_t **subscription_out) {
    (void)rx; (void)start_block; (void)full_transaction_objects;
    (void)polling_interval; (void)callback; (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_implemented();
}

neoc_error_t neoc_json_rpc2_0_rx_catch_up_to_latest_block_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                                    int start_block,
                                                                    bool full_transaction_objects,
                                                                    neoc_block_callback_t callback,
                                                                    void *user_data,
                                                                    neoc_subscription_t **subscription_out) {
    (void)rx; (void)start_block; (void)full_transaction_objects;
    (void)callback; (void)user_data;
    if (subscription_out) {
        *subscription_out = NULL;
    }
    return rx_not_implemented();
}

neoc_error_t neoc_json_rpc2_0_rx_get_latest_block_index(neoc_json_rpc2_0_rx_t *rx, int *block_index_out) {
    (void)rx;
    if (block_index_out) {
        *block_index_out = -1;
    }
    return rx_not_implemented();
}

neoc_error_t neoc_json_rpc2_0_rx_get_latest_block_index_async(neoc_json_rpc2_0_rx_t *rx,
                                                              neoc_block_index_callback_t callback,
                                                              void *user_data) {
    (void)rx; (void)callback; (void)user_data;
    return rx_not_implemented();
}

neoc_error_t neoc_subscription_cancel(neoc_subscription_t *subscription) {
    (void)subscription;
    return rx_not_implemented();
}

bool neoc_subscription_is_active(const neoc_subscription_t *subscription) {
    return subscription ? subscription->is_active : false;
}

void neoc_subscription_free(neoc_subscription_t *subscription) {
    if (!subscription) {
        return;
    }
    if (subscription->cancel) {
        subscription->cancel(subscription);
    }
    neoc_free(subscription);
}

neoc_error_t neoc_json_rpc2_0_rx_cancel_all_subscriptions(neoc_json_rpc2_0_rx_t *rx) {
    (void)rx;
    return rx_not_implemented();
}

neoc_neo_c_t *neoc_json_rpc2_0_rx_get_neo_c(neoc_json_rpc2_0_rx_t *rx) {
    return rx ? rx->neo_c : NULL;
}

void neoc_json_rpc2_0_rx_free(neoc_json_rpc2_0_rx_t *rx) {
    if (!rx) {
        return;
    }
    if (rx->subscriptions) {
        for (size_t i = 0; i < rx->subscription_count; ++i) {
            neoc_subscription_free(rx->subscriptions[i]);
        }
        neoc_free(rx->subscriptions);
    }
    neoc_free(rx);
}
