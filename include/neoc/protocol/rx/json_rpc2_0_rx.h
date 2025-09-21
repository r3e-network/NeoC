/**
 * @file json_rpc2_0_rx.h
 * @brief JSON-RPC 2.0 reactive-style implementation for C
 * 
 * Based on Swift source: protocol/rx/JsonRpc2_0Rx.swift
 * Provides reactive-style block polling and subscription functionality
 */

#ifndef NEOC_PROTOCOL_RX_JSON_RPC2_0_RX_H
#define NEOC_PROTOCOL_RX_JSON_RPC2_0_RX_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/neo_c.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations
 */
typedef struct neoc_json_rpc2_0_rx_t neoc_json_rpc2_0_rx_t;
typedef struct neoc_neo_get_block_t neoc_neo_get_block_t;
typedef struct neoc_subscription_t neoc_subscription_t;

/**
 * @brief Block callback function
 * @param block Block data (do not free)
 * @param error Error code (NEOC_SUCCESS on success)
 * @param user_data User-provided data
 * @return true to continue subscription, false to stop
 */
typedef bool (*neoc_block_callback_t)(const neoc_neo_get_block_t *block, neoc_error_t error, void *user_data);

/**
 * @brief Block index callback function
 * @param block_index Block index
 * @param error Error code (NEOC_SUCCESS on success)
 * @param user_data User-provided data
 * @return true to continue subscription, false to stop
 */
typedef bool (*neoc_block_index_callback_t)(int block_index, neoc_error_t error, void *user_data);

/**
 * @brief Subscription handle for managing active subscriptions
 */
struct neoc_subscription_t {
    void *impl_data;                        /**< Implementation-specific data */
    bool is_active;                         /**< Whether subscription is active */
    bool (*cancel)(neoc_subscription_t *subscription); /**< Cancel function */
};

/**
 * @brief JSON-RPC 2.0 reactive implementation
 * 
 * Based on Swift JsonRpc2_0Rx class
 * Provides reactive-style block polling and subscription
 */
struct neoc_json_rpc2_0_rx_t {
    neoc_neo_c_t *neo_c;            /**< Associated NeoC client */
    void *executor_service;                 /**< Executor for scheduling */
    neoc_subscription_t **subscriptions;   /**< Active subscriptions */
    size_t subscription_count;              /**< Number of active subscriptions */
};

/**
 * @brief Create a new JSON-RPC reactive client
 * @param neo_c Associated NeoC client
 * @param executor_service Executor for scheduling (can be NULL)
 * @return Newly allocated reactive client or NULL on failure
 */
neoc_json_rpc2_0_rx_t *neoc_json_rpc2_0_rx_create(neoc_neo_c_t *neo_c, void *executor_service);

/**
 * @brief Start block index polling
 * @param rx Reactive client
 * @param polling_interval Polling interval in milliseconds
 * @param callback Block index callback
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_block_index_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                       int polling_interval,
                                                       neoc_block_index_callback_t callback,
                                                       void *user_data,
                                                       neoc_subscription_t **subscription_out);

/**
 * @brief Start block polling
 * @param rx Reactive client
 * @param full_transaction_objects Whether to include full transaction objects
 * @param polling_interval Polling interval in milliseconds
 * @param callback Block callback
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_block_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                 bool full_transaction_objects,
                                                 int polling_interval,
                                                 neoc_block_callback_t callback,
                                                 void *user_data,
                                                 neoc_subscription_t **subscription_out);

/**
 * @brief Replay blocks within a range
 * @param rx Reactive client
 * @param start_block Start block number
 * @param end_block End block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param ascending Whether to replay in ascending order
 * @param callback Block callback
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_replay_blocks_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                         int start_block,
                                                         int end_block,
                                                         bool full_transaction_objects,
                                                         bool ascending,
                                                         neoc_block_callback_t callback,
                                                         void *user_data,
                                                         neoc_subscription_t **subscription_out);

/**
 * @brief Catch up to latest block and then start subscription
 * @param rx Reactive client
 * @param start_block Start block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param polling_interval Polling interval for new blocks in milliseconds
 * @param callback Block callback
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_catch_up_to_latest_and_subscribe(neoc_json_rpc2_0_rx_t *rx,
                                                                  int start_block,
                                                                  bool full_transaction_objects,
                                                                  int polling_interval,
                                                                  neoc_block_callback_t callback,
                                                                  void *user_data,
                                                                  neoc_subscription_t **subscription_out);

/**
 * @brief Catch up to latest block only (no ongoing subscription)
 * @param rx Reactive client
 * @param start_block Start block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param callback Block callback
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_catch_up_to_latest_block_publisher(neoc_json_rpc2_0_rx_t *rx,
                                                                    int start_block,
                                                                    bool full_transaction_objects,
                                                                    neoc_block_callback_t callback,
                                                                    void *user_data,
                                                                    neoc_subscription_t **subscription_out);

/**
 * @brief Get latest block index
 * @param rx Reactive client
 * @param block_index_out Output for latest block index
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_get_latest_block_index(neoc_json_rpc2_0_rx_t *rx, int *block_index_out);

/**
 * @brief Get latest block index asynchronously
 * @param rx Reactive client
 * @param callback Block index callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_json_rpc2_0_rx_get_latest_block_index_async(neoc_json_rpc2_0_rx_t *rx,
                                                             neoc_block_index_callback_t callback,
                                                             void *user_data);

/**
 * @brief Cancel a subscription
 * @param subscription Subscription to cancel
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_subscription_cancel(neoc_subscription_t *subscription);

/**
 * @brief Check if subscription is active
 * @param subscription Subscription to check
 * @return true if active, false otherwise
 */
bool neoc_subscription_is_active(const neoc_subscription_t *subscription);

/**
 * @brief Free subscription handle
 * @param subscription Subscription to free
 */
void neoc_subscription_free(neoc_subscription_t *subscription);

/**
 * @brief Cancel all active subscriptions
 * @param rx Reactive client
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_json_rpc2_0_rx_cancel_all_subscriptions(neoc_json_rpc2_0_rx_t *rx);

/**
 * @brief Get the associated NeoSwift client
 * @param rx Reactive client
 * @return NeoSwift client (do not free)
 */
neoc_neo_c_t *neoc_json_rpc2_0_rx_get_neo_c(neoc_json_rpc2_0_rx_t *rx);

/**
 * @brief Free JSON-RPC reactive client
 * @param rx Reactive client to free
 */
void neoc_json_rpc2_0_rx_free(neoc_json_rpc2_0_rx_t *rx);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_RX_JSON_RPC2_0_RX_H */
