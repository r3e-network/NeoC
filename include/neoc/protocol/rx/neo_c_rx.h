/**
 * @file neo_c_rx.h
 * @brief NeoC reactive protocol interface for C
 * 
 * Based on Swift source: protocol/rx/NeoSwiftRx.swift
 * Provides the reactive extension protocol for NeoC
 */

#ifndef NEOC_PROTOCOL_RX_NEO_C_RX_H
#define NEOC_PROTOCOL_RX_NEO_C_RX_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/rx/json_rpc2_0_rx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations
 */
typedef struct neoc_neo_c_rx_t neoc_neo_c_rx_t;

/**
 * @brief NeoCRx protocol function table
 * 
 * Based on Swift NeoSwiftRx protocol
 * Provides function pointers for polymorphic reactive implementation
 */
typedef struct {
    /**
     * @brief Create a block publisher that emits newly created blocks
     */
    neoc_error_t (*block_publisher)(neoc_neo_c_rx_t *rx,
                                    bool full_transaction_objects,
                                    neoc_block_callback_t callback,
                                    void *user_data,
                                    neoc_subscription_t **subscription_out);
    
    /**
     * @brief Create a publisher that emits blocks within a range
     */
    neoc_error_t (*replay_blocks_publisher)(neoc_neo_c_rx_t *rx,
                                            int start_block,
                                            int end_block,
                                            bool full_transaction_objects,
                                            neoc_block_callback_t callback,
                                            void *user_data,
                                            neoc_subscription_t **subscription_out);
    
    /**
     * @brief Create a publisher that emits blocks within a range with order control
     */
    neoc_error_t (*replay_blocks_publisher_ordered)(neoc_neo_c_rx_t *rx,
                                                     int start_block,
                                                     int end_block,
                                                     bool full_transaction_objects,
                                                     bool ascending,
                                                     neoc_block_callback_t callback,
                                                     void *user_data,
                                                     neoc_subscription_t **subscription_out);
    
    /**
     * @brief Create a publisher that catches up to latest block
     */
    neoc_error_t (*catch_up_to_latest_block_publisher)(neoc_neo_c_rx_t *rx,
                                                        int start_block,
                                                        bool full_transaction_objects,
                                                        neoc_block_callback_t callback,
                                                        void *user_data,
                                                        neoc_subscription_t **subscription_out);
    
    /**
     * @brief Create a publisher that catches up and subscribes to new blocks
     */
    neoc_error_t (*catch_up_to_latest_and_subscribe_to_new_blocks_publisher)(neoc_neo_c_rx_t *rx,
                                                                              int start_block,
                                                                              bool full_transaction_objects,
                                                                              neoc_block_callback_t callback,
                                                                              void *user_data,
                                                                              neoc_subscription_t **subscription_out);
    
    /**
     * @brief Create a publisher that subscribes to new blocks only
     */
    neoc_error_t (*subscribe_to_new_blocks_publisher)(neoc_neo_c_rx_t *rx,
                                                       bool full_transaction_objects,
                                                       neoc_block_callback_t callback,
                                                       void *user_data,
                                                       neoc_subscription_t **subscription_out);
    
    /**
     * @brief Free implementation
     */
    void (*free_impl)(neoc_neo_c_rx_t *rx);
    
} neoc_neo_c_rx_vtable_t;

/**
 * @brief NeoCRx interface structure
 * 
 * Based on Swift NeoSwiftRx protocol
 * Provides reactive blockchain monitoring capabilities
 */
struct neoc_neo_c_rx_t {
    const neoc_neo_c_rx_vtable_t *vtable; /**< Virtual function table */
    void *impl_data;                          /**< Implementation-specific data */
};

/**
 * @brief Initialize a NeoSwiftRx interface
 * @param rx NeoSwiftRx structure to initialize
 * @param vtable Virtual function table
 * @param impl_data Implementation-specific data
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_init(neoc_neo_c_rx_t *rx,
                                    const neoc_neo_c_rx_vtable_t *vtable,
                                    void *impl_data);

/**
 * @brief Create a block publisher that emits newly created blocks
 * @param rx NeoSwiftRx instance
 * @param full_transaction_objects Whether to include full transaction objects
 * @param callback Block callback function
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_block_publisher(neoc_neo_c_rx_t *rx,
                                               bool full_transaction_objects,
                                               neoc_block_callback_t callback,
                                               void *user_data,
                                               neoc_subscription_t **subscription_out);

/**
 * @brief Create a publisher that emits blocks within a range (ascending order)
 * @param rx NeoSwiftRx instance
 * @param start_block Start block number
 * @param end_block End block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param callback Block callback function
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_replay_blocks_publisher(neoc_neo_c_rx_t *rx,
                                                       int start_block,
                                                       int end_block,
                                                       bool full_transaction_objects,
                                                       neoc_block_callback_t callback,
                                                       void *user_data,
                                                       neoc_subscription_t **subscription_out);

/**
 * @brief Create a publisher that emits blocks within a range with order control
 * @param rx NeoSwiftRx instance
 * @param start_block Start block number
 * @param end_block End block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param ascending Whether to emit blocks in ascending order
 * @param callback Block callback function
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_replay_blocks_publisher_ordered(neoc_neo_c_rx_t *rx,
                                                               int start_block,
                                                               int end_block,
                                                               bool full_transaction_objects,
                                                               bool ascending,
                                                               neoc_block_callback_t callback,
                                                               void *user_data,
                                                               neoc_subscription_t **subscription_out);

/**
 * @brief Create a publisher that catches up to the latest block
 * @param rx NeoSwiftRx instance
 * @param start_block Start block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param callback Block callback function
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_catch_up_to_latest_block_publisher(neoc_neo_c_rx_t *rx,
                                                                  int start_block,
                                                                  bool full_transaction_objects,
                                                                  neoc_block_callback_t callback,
                                                                  void *user_data,
                                                                  neoc_subscription_t **subscription_out);

/**
 * @brief Create a publisher that catches up to latest and subscribes to new blocks
 * @param rx NeoSwiftRx instance
 * @param start_block Start block number
 * @param full_transaction_objects Whether to include full transaction objects
 * @param callback Block callback function
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_catch_up_to_latest_and_subscribe_to_new_blocks_publisher(neoc_neo_c_rx_t *rx,
                                                                                         int start_block,
                                                                                         bool full_transaction_objects,
                                                                                         neoc_block_callback_t callback,
                                                                                         void *user_data,
                                                                                         neoc_subscription_t **subscription_out);

/**
 * @brief Create a publisher that subscribes to new blocks only
 * @param rx NeoSwiftRx instance
 * @param full_transaction_objects Whether to include full transaction objects
 * @param callback Block callback function
 * @param user_data User data for callback
 * @param subscription_out Output subscription handle
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_rx_subscribe_to_new_blocks_publisher(neoc_neo_c_rx_t *rx,
                                                                 bool full_transaction_objects,
                                                                 neoc_block_callback_t callback,
                                                                 void *user_data,
                                                                 neoc_subscription_t **subscription_out);

/**
 * @brief Free NeoSwiftRx instance
 * @param rx NeoSwiftRx instance to free
 */
void neoc_neo_c_rx_free(neoc_neo_c_rx_t *rx);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_RX_NEO_C_RX_H */
