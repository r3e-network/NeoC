#include "neoc/protocol/core/polling/block_index_polling.h"
#include "neoc/protocol/neo_c.h"
#include "neoc/protocol/core/neo.h"
#include <string.h>

neoc_error_t neoc_block_index_polling_create(
    int polling_interval_ms,
    neoc_block_index_polling_t **polling) {
    if (!polling) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Polling output is NULL");
    }
    *polling = NULL;

    neoc_block_index_polling_t *obj = neoc_calloc(1, sizeof(neoc_block_index_polling_t));
    if (!obj) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate block index polling");
    }

    obj->current_block_index = -1;
    obj->polling_interval_ms = polling_interval_ms > 0 ? polling_interval_ms : 15000;
    obj->is_initialized = true;
    *polling = obj;
    return NEOC_SUCCESS;
}

void neoc_block_index_polling_free(
    neoc_block_index_polling_t *polling) {
    if (!polling) return;
    neoc_free(polling);
}

neoc_error_t neoc_block_index_polling_start(
    neoc_block_index_polling_t *polling,
    void *neo_c,
    neoc_block_index_callback_t callback,
    neoc_polling_error_callback_t error_callback,
    void *user_data) {
    if (!polling || !neo_c || !callback) {
        if (error_callback) {
            error_callback(NEOC_ERROR_INVALID_ARGUMENT, "Invalid polling arguments", user_data);
        }
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid polling arguments");
    }

    uint32_t latest = 0;
    neoc_error_t err = neoc_neo_get_block_count((neoc_neo_client_t *)neo_c, &latest);
    if (err == NEOC_SUCCESS) {
        polling->current_block_index = (int)latest - 1;
        int new_index = (int)latest - 1;
        callback(&new_index, 1, user_data);
    } else if (error_callback) {
        const neoc_error_info_t *info = neoc_get_last_error();
        error_callback(err, info ? info->message : "Polling error", user_data);
    }
    return err;
}

neoc_error_t neoc_block_index_polling_stop(
    neoc_block_index_polling_t *polling) {
    if (!polling) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Polling is NULL");
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_block_index_polling_poll_once(
    neoc_block_index_polling_t *polling,
    void *neo_c,
    int **new_indices,
    size_t *count) {
    if (!polling || !neo_c || !new_indices || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid poll_once arguments");
    }
    *new_indices = NULL;
    *count = 0;

    uint32_t latest = 0;
    neoc_error_t err = neoc_neo_get_block_count((neoc_neo_client_t *)neo_c, &latest);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    int current = polling->current_block_index;
    if ((int)latest <= current) {
        return NEOC_SUCCESS;
    }

    size_t diff = (size_t)(latest - current);
    int *indices = neoc_calloc(diff, sizeof(int));
    if (!indices) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate indices");
    }
    for (size_t i = 0; i < diff; ++i) {
        indices[i] = current + 1 + (int)i;
    }

    polling->current_block_index = latest;
    *new_indices = indices;
    *count = diff;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_block_index_polling_get_current_index(
    const neoc_block_index_polling_t *polling,
    int *current_index) {
    if (!polling || !current_index) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid polling get");
    }
    *current_index = polling->current_block_index;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_block_index_polling_set_current_index(
    neoc_block_index_polling_t *polling,
    int index) {
    if (!polling) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Polling is NULL");
    }
    polling->current_block_index = index;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_block_index_polling_reset(
    neoc_block_index_polling_t *polling) {
    if (!polling) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Polling is NULL");
    }
    polling->current_block_index = -1;
    polling->is_initialized = false;
    return NEOC_SUCCESS;
}
