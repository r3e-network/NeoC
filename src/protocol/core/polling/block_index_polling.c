#include "neoc/protocol/core/polling/block_index_polling.h"

static neoc_error_t neoc_block_index_polling_not_impl(const char *fn) {
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          fn ? fn : "Block index polling not implemented");
}

neoc_error_t neoc_block_index_polling_create(
    int polling_interval_ms,
    neoc_block_index_polling_t **polling) {
    (void)polling_interval_ms;
    if (polling) {
        *polling = NULL;
    }
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_create");
}

void neoc_block_index_polling_free(
    neoc_block_index_polling_t *polling) {
    (void)polling;
}

neoc_error_t neoc_block_index_polling_start(
    neoc_block_index_polling_t *polling,
    void *neo_c,
    neoc_block_index_callback_t callback,
    neoc_polling_error_callback_t error_callback,
    void *user_data) {
    (void)polling;
    (void)neo_c;
    (void)callback;
    (void)error_callback;
    (void)user_data;
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_start");
}

neoc_error_t neoc_block_index_polling_stop(
    neoc_block_index_polling_t *polling) {
    (void)polling;
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_stop");
}

neoc_error_t neoc_block_index_polling_poll_once(
    neoc_block_index_polling_t *polling,
    void *neo_c,
    int **new_indices,
    size_t *count) {
    (void)polling;
    (void)neo_c;
    if (new_indices) {
        *new_indices = NULL;
    }
    if (count) {
        *count = 0;
    }
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_poll_once");
}

neoc_error_t neoc_block_index_polling_get_current_index(
    const neoc_block_index_polling_t *polling,
    int *current_index) {
    (void)polling;
    if (current_index) {
        *current_index = -1;
    }
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_get_current_index");
}

neoc_error_t neoc_block_index_polling_set_current_index(
    neoc_block_index_polling_t *polling,
    int index) {
    (void)polling;
    (void)index;
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_set_current_index");
}

neoc_error_t neoc_block_index_polling_reset(
    neoc_block_index_polling_t *polling) {
    (void)polling;
    return neoc_block_index_polling_not_impl("neoc_block_index_polling_reset");
}

