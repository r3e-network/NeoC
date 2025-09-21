/**
 * @file block_index_polling.h
 * @brief Block index polling for real-time blockchain monitoring
 * 
 * Converted from Swift source: protocol/core/polling/BlockIndexPolling.swift
 * Provides functionality to poll for new block indices and detect blockchain updates.
 */

#ifndef NEOC_PROTOCOL_CORE_POLLING_BLOCK_INDEX_POLLING_H
#define NEOC_PROTOCOL_CORE_POLLING_BLOCK_INDEX_POLLING_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Block index polling structure
 * 
 * Manages polling state for monitoring new blocks on the blockchain.
 */
typedef struct {
    int current_block_index;            /**< Current known block index (-1 if not set) */
    bool is_initialized;                /**< Whether polling has been initialized */
    int polling_interval_ms;            /**< Polling interval in milliseconds */
} neoc_block_index_polling_t;

/**
 * @brief Block index callback function type
 * 
 * Called when new block indices are detected.
 * 
 * @param new_indices Array of new block indices
 * @param count Number of new indices
 * @param user_data User-provided data
 */
typedef void (*neoc_block_index_callback_t)(
    const int *new_indices,
    size_t count,
    void *user_data
);

/**
 * @brief Error callback function type
 * 
 * Called when polling encounters an error.
 * 
 * @param error Error code
 * @param error_message Error message
 * @param user_data User-provided data
 */
typedef void (*neoc_polling_error_callback_t)(
    neoc_error_t error,
    const char *error_message,
    void *user_data
);

/**
 * @brief Create a new block index polling instance
 * 
 * @param polling_interval_ms Polling interval in milliseconds
 * @param polling Output pointer for the created polling instance (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_index_polling_create(
    int polling_interval_ms,
    neoc_block_index_polling_t **polling
);

/**
 * @brief Free a block index polling instance
 * 
 * @param polling Polling instance to free (can be NULL)
 */
void neoc_block_index_polling_free(
    neoc_block_index_polling_t *polling
);

/**
 * @brief Start block index polling
 * 
 * Begins polling for new block indices. The callback will be invoked
 * when new blocks are detected.
 * 
 * @param polling Polling instance
 * @param neo_c NeoC instance for blockchain queries
 * @param callback Function to call when new blocks are found
 * @param error_callback Function to call on errors (can be NULL)
 * @param user_data User data passed to callbacks
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_index_polling_start(
    neoc_block_index_polling_t *polling,
    void *neo_c,  /* neoc_neo_c_t* */
    neoc_block_index_callback_t callback,
    neoc_polling_error_callback_t error_callback,
    void *user_data
);

/**
 * @brief Stop block index polling
 * 
 * @param polling Polling instance
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_index_polling_stop(
    neoc_block_index_polling_t *polling
);

/**
 * @brief Poll once for new block indices
 * 
 * Performs a single polling operation to check for new blocks.
 * This is useful for manual polling without automatic scheduling.
 * 
 * @param polling Polling instance
 * @param neo_c NeoC instance for blockchain queries
 * @param new_indices Output array of new block indices (caller must free)
 * @param count Output count of new indices
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_index_polling_poll_once(
    neoc_block_index_polling_t *polling,
    void *neo_c,  /* neoc_neo_c_t* */
    int **new_indices,
    size_t *count
);

/**
 * @brief Get current known block index
 * 
 * @param polling Polling instance
 * @param current_index Output pointer for current index
 * @return NEOC_SUCCESS if index is set, error code otherwise
 */
neoc_error_t neoc_block_index_polling_get_current_index(
    const neoc_block_index_polling_t *polling,
    int *current_index
);

/**
 * @brief Set current block index manually
 * 
 * @param polling Polling instance
 * @param index Block index to set
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_index_polling_set_current_index(
    neoc_block_index_polling_t *polling,
    int index
);

/**
 * @brief Reset polling state
 * 
 * Resets the polling instance to uninitialized state.
 * 
 * @param polling Polling instance
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_block_index_polling_reset(
    neoc_block_index_polling_t *polling
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_POLLING_BLOCK_INDEX_POLLING_H */
