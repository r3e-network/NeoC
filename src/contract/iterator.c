/**
 * @file iterator.c
 * @brief Iterator implementation
 */

#include "neoc/contract/iterator.h"
#include "neoc/neoc_memory.h"
#include <string.h>

typedef struct array_iterator_context {
    void *items;
    size_t count;
    size_t item_size;
    size_t current_index;
} array_iterator_context_t;

static bool array_has_next(neoc_iterator_t *iter) {
    if (!iter || !iter->context) return false;
    array_iterator_context_t *ctx = (array_iterator_context_t*)iter->context;
    return ctx->current_index < ctx->count;
}

static void* array_next(neoc_iterator_t *iter) {
    if (!iter || !iter->context) return NULL;
    array_iterator_context_t *ctx = (array_iterator_context_t*)iter->context;
    
    if (ctx->current_index >= ctx->count) {
        return NULL;
    }
    
    void *item = (uint8_t*)ctx->items + (ctx->current_index * ctx->item_size);
    ctx->current_index++;
    return item;
}

static void array_free(neoc_iterator_t *iter) {
    if (!iter) return;
    if (iter->context) {
        neoc_free(iter->context);
    }
    neoc_free(iter);
}

neoc_error_t neoc_iterator_create_from_array(void *items,
                                              size_t count,
                                              size_t item_size,
                                              neoc_iterator_t **iterator) {
    if (!items || !iterator || count == 0 || item_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    *iterator = neoc_malloc(sizeof(neoc_iterator_t));
    if (!*iterator) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate iterator");
    }
    
    array_iterator_context_t *ctx = neoc_malloc(sizeof(array_iterator_context_t));
    if (!ctx) {
        neoc_free(*iterator);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate iterator context");
    }
    
    ctx->items = items;
    ctx->count = count;
    ctx->item_size = item_size;
    ctx->current_index = 0;
    
    (*iterator)->context = ctx;
    (*iterator)->current = NULL;
    (*iterator)->has_next = array_has_next;
    (*iterator)->next = array_next;
    (*iterator)->free = array_free;
    
    return NEOC_SUCCESS;
}

bool neoc_iterator_has_next(neoc_iterator_t *iterator) {
    return iterator && iterator->has_next ? iterator->has_next(iterator) : false;
}

void* neoc_iterator_next(neoc_iterator_t *iterator) {
    return iterator && iterator->next ? iterator->next(iterator) : NULL;
}

neoc_error_t neoc_iterator_to_array(neoc_iterator_t *iterator,
                                     void **array,
                                     size_t *count,
                                     size_t item_size) {
    if (!iterator || !array || !count || item_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    // Count items
    size_t item_count = 0;
    while (neoc_iterator_has_next(iterator)) {
        neoc_iterator_next(iterator);
        item_count++;
    }
    
    if (item_count == 0) {
        *array = NULL;
        *count = 0;
        return NEOC_SUCCESS;
    }
    
    // Allocate array
    *array = neoc_malloc(item_count * item_size);
    if (!*array) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate array");
    }
    
    // Reset iterator to beginning for array conversion
    // This requires resetting the iterator position if supported
    size_t items_copied = 0;
    void *current_pos = *array;
    
    // Iterate through all items and copy to array
    while (items_copied < item_count && iterator->has_next(iterator)) {
        void *item = iterator->next(iterator);
        if (item) {
            memcpy(current_pos, item, item_size);
            current_pos = (uint8_t *)current_pos + item_size;
            items_copied++;
        }
    }
    
    *count = item_count;
    return NEOC_SUCCESS;
}

void neoc_iterator_free(neoc_iterator_t *iterator) {
    if (iterator && iterator->free) {
        iterator->free(iterator);
    }
}
