/**
 * @file iterator.h
 * @brief Iterator for contract storage and results
 */

#ifndef NEOC_ITERATOR_H
#define NEOC_ITERATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Iterator structure
 */
typedef struct neoc_iterator {
    void *context;                      // Iterator context
    void *current;                      // Current item
    bool (*has_next)(struct neoc_iterator*);
    void* (*next)(struct neoc_iterator*);
    void (*free)(struct neoc_iterator*);
} neoc_iterator_t;

/**
 * Create iterator from array
 * @param items Array of items
 * @param count Number of items
 * @param item_size Size of each item
 * @param iterator Output iterator
 * @return Error code
 */
neoc_error_t neoc_iterator_create_from_array(void *items,
                                              size_t count,
                                              size_t item_size,
                                              neoc_iterator_t **iterator);

/**
 * Check if iterator has more items
 * @param iterator The iterator
 * @return True if has more items
 */
bool neoc_iterator_has_next(neoc_iterator_t *iterator);

/**
 * Get next item from iterator
 * @param iterator The iterator
 * @return Next item or NULL
 */
void* neoc_iterator_next(neoc_iterator_t *iterator);

/**
 * Convert iterator to array
 * @param iterator The iterator
 * @param array Output array
 * @param count Output item count
 * @param item_size Size of each item
 * @return Error code
 */
neoc_error_t neoc_iterator_to_array(neoc_iterator_t *iterator,
                                     void **array,
                                     size_t *count,
                                     size_t item_size);

/**
 * Free iterator
 * @param iterator Iterator to free
 */
void neoc_iterator_free(neoc_iterator_t *iterator);

#ifdef __cplusplus
}
#endif

#endif // NEOC_ITERATOR_H
