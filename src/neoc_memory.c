/**
 * @file neoc_memory.c
 * @brief Memory management implementation
 */

#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct memory_stats {
    atomic_size_t total_allocated;
    atomic_size_t total_freed;
    atomic_size_t current_usage;
    atomic_size_t peak_usage;
    atomic_size_t allocation_count;
    atomic_size_t free_count;
} memory_stats_t;

typedef struct alloc_entry {
    void *ptr;
    size_t size;
    struct alloc_entry *next;
} alloc_entry_t;

static memory_stats_t stats = {0};
static neoc_allocator_t custom_allocator = {NULL, NULL, NULL};
static alloc_entry_t *alloc_head = NULL;
static atomic_flag alloc_lock = ATOMIC_FLAG_INIT;

static inline bool using_custom_allocator(void) {
    return custom_allocator.malloc_func != NULL ||
           custom_allocator.realloc_func != NULL ||
           custom_allocator.free_func != NULL;
}

static void alloc_lock_acquire(void) {
    while (atomic_flag_test_and_set(&alloc_lock)) {
        /* spin */
    }
}

static void alloc_lock_release(void) {
    atomic_flag_clear(&alloc_lock);
}

static void track_allocation(void *ptr, size_t size) {
    if (!ptr) {
        return;
    }
    alloc_entry_t *entry = malloc(sizeof(alloc_entry_t));
    if (!entry) {
        return;
    }
    entry->ptr = ptr;
    entry->size = size;
    
    alloc_lock_acquire();
    entry->next = alloc_head;
    alloc_head = entry;
    alloc_lock_release();
}

static bool untrack_allocation(void *ptr, size_t *size_out) {
    if (!ptr) {
        return false;
    }
    
    alloc_lock_acquire();
    alloc_entry_t *prev = NULL;
    alloc_entry_t *cur = alloc_head;
    while (cur) {
        if (cur->ptr == ptr) {
            if (size_out) {
                *size_out = cur->size;
            }
            if (prev) {
                prev->next = cur->next;
            } else {
                alloc_head = cur->next;
            }
            alloc_lock_release();
            free(cur);
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    alloc_lock_release();
    return false;
}

void* neoc_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    void *ptr = NULL;
    
    if (custom_allocator.malloc_func) {
        ptr = custom_allocator.malloc_func(size);
    } else {
        ptr = malloc(size);
    }
    
    if (ptr) {
        atomic_fetch_add(&stats.allocation_count, 1);
    }
    
    if (ptr && !using_custom_allocator()) {
        track_allocation(ptr, size);
        atomic_fetch_add(&stats.total_allocated, size);
        atomic_fetch_add(&stats.current_usage, size);
        
        size_t current = atomic_load(&stats.current_usage);
        size_t peak = atomic_load(&stats.peak_usage);
        if (current > peak) {
            atomic_store(&stats.peak_usage, current);
        }
    }
    
    return ptr;
}

void* neoc_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) {
        return NULL;
    }
    
    size_t total = count * size;
    void *ptr = NULL;
    
    if (custom_allocator.malloc_func) {
        ptr = custom_allocator.malloc_func(total);
        if (ptr) {
            memset(ptr, 0, total);
        }
    } else {
        ptr = calloc(count, size);
    }
    
    if (ptr) {
        atomic_fetch_add(&stats.allocation_count, 1);
    }
    
    if (ptr && !using_custom_allocator()) {
        track_allocation(ptr, total);
        atomic_fetch_add(&stats.total_allocated, total);
        atomic_fetch_add(&stats.current_usage, total);
        
        size_t current = atomic_load(&stats.current_usage);
        size_t peak = atomic_load(&stats.peak_usage);
        if (current > peak) {
            atomic_store(&stats.peak_usage, current);
        }
    }
    
    return ptr;
}

void* neoc_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return neoc_malloc(size);
    }
    
    if (size == 0) {
        neoc_free(ptr);
        return NULL;
    }
    
    if (custom_allocator.realloc_func) {
        void *new_ptr = custom_allocator.realloc_func(ptr, size);
        if (new_ptr) {
            atomic_fetch_add(&stats.allocation_count, 1);
            if (!using_custom_allocator()) {
                atomic_fetch_add(&stats.total_allocated, size);
                atomic_fetch_add(&stats.current_usage, size);
            }
        }
        return new_ptr;
    }
    
    size_t old_size = 0;
    bool had_entry = untrack_allocation(ptr, &old_size);
    
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        if (had_entry) {
            track_allocation(ptr, old_size);
        }
        return NULL;
    }
    
    track_allocation(new_ptr, size);
    atomic_fetch_add(&stats.allocation_count, 1);
    atomic_fetch_add(&stats.total_allocated, size);
    atomic_fetch_add(&stats.current_usage, size);
    
    if (had_entry && old_size > 0 && old_size > size) {
        size_t delta = old_size - size;
        atomic_fetch_add(&stats.total_freed, delta);
        atomic_fetch_sub(&stats.current_usage, delta);
    }
    
    return new_ptr;
}

void neoc_free(void *ptr) {
    if (!ptr) {
        return;
    }
    
    if (custom_allocator.free_func) {
        custom_allocator.free_func(ptr);
        atomic_fetch_add(&stats.free_count, 1);
        return;
    }
    
    size_t size = 0;
    bool tracked = untrack_allocation(ptr, &size);
    free(ptr);
    
    atomic_fetch_add(&stats.free_count, 1);
    if (tracked) {
        atomic_fetch_add(&stats.total_freed, size);
        atomic_fetch_sub(&stats.current_usage, size);
    }
}

char* neoc_strdup(const char *str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str) + 1;
    char *copy = neoc_malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

char* neoc_strndup(const char *str, size_t n) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    if (len > n) {
        len = n;
    }
    char *copy = neoc_malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len);
        copy[len] = '\0';
    }
    return copy;
}

uint8_t* neoc_memdup(const uint8_t *data, size_t size) {
    if (!data || size == 0) {
        return NULL;
    }
    
    uint8_t *copy = neoc_malloc(size);
    if (copy) {
        memcpy(copy, data, size);
    }
    return copy;
}

neoc_error_t neoc_set_allocator(const neoc_allocator_t *allocator) {
    if (!allocator) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    custom_allocator = *allocator;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_get_allocator(neoc_allocator_t *allocator) {
    if (!allocator) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    *allocator = custom_allocator;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_reset_allocator(void) {
    memset(&custom_allocator, 0, sizeof(custom_allocator));
    return NEOC_SUCCESS;
}

void neoc_secure_memzero(void *ptr, size_t size) {
    if (ptr && size > 0) {
        volatile uint8_t *p = ptr;
        while (size--) {
            *p++ = 0;
        }
    }
}

void neoc_secure_free(void *ptr, size_t size) {
    if (ptr) {
        neoc_secure_memzero(ptr, size);
        neoc_free(ptr);
    }
}

int neoc_secure_memcmp(const void *a, const void *b, size_t size) {
    if (!a || !b) {
        return -1;
    }
    
    const uint8_t *pa = a;
    const uint8_t *pb = b;
    uint8_t result = 0;
    
    for (size_t i = 0; i < size; i++) {
        result |= pa[i] ^ pb[i];
    }
    
    return result != 0;
}

size_t neoc_get_allocation_count(void) {
    size_t allocs = atomic_load(&stats.allocation_count);
    size_t frees = atomic_load(&stats.free_count);
    if (allocs > frees) {
        return allocs - frees;
    }
    return 0;
}

void neoc_print_memory_leaks(void) {
    size_t outstanding = neoc_get_allocation_count();
    if (outstanding == 0) {
        fprintf(stderr, "[NeoC] No outstanding allocations detected.\n");
        return;
    }

    size_t alloc_calls = atomic_load(&stats.allocation_count);
    size_t free_calls = atomic_load(&stats.free_count);
    size_t total_bytes = atomic_load(&stats.total_allocated);

    fprintf(stderr,
            "[NeoC] Outstanding allocations detected: %zu block(s) "
            "(alloc calls: %zu, free calls: %zu, total bytes allocated: %zu)\n",
            outstanding, alloc_calls, free_calls, total_bytes);
}

#ifdef NEOC_DEBUG_MEMORY

neoc_error_t neoc_memory_debug_enable(void) {
    // Memory debugging is always enabled in this implementation
    return NEOC_SUCCESS;
}

void neoc_memory_debug_disable(void) {
    // Reset statistics
    atomic_store(&stats.total_allocated, 0);
    atomic_store(&stats.total_freed, 0);
    atomic_store(&stats.current_usage, 0);
    atomic_store(&stats.peak_usage, 0);
    atomic_store(&stats.allocation_count, 0);
    atomic_store(&stats.free_count, 0);
}

size_t neoc_memory_debug_check_leaks(void) {
    size_t current = atomic_load(&stats.current_usage);
    size_t alloc_count = atomic_load(&stats.allocation_count);
    size_t free_count = atomic_load(&stats.free_count);
    
    // Return number of leaked blocks (allocations - frees)
    if (alloc_count > free_count) {
        return alloc_count - free_count;
    }
    return 0;
}

neoc_error_t neoc_memory_debug_get_stats(size_t* allocated_bytes, size_t* allocated_blocks) {
    if (!allocated_bytes || !allocated_blocks) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    *allocated_bytes = atomic_load(&stats.current_usage);
    *allocated_blocks = atomic_load(&stats.allocation_count) - atomic_load(&stats.free_count);
    
    return NEOC_SUCCESS;
}

#endif /* NEOC_DEBUG_MEMORY */

neoc_error_t neoc_get_memory_stats(neoc_memory_stats_t* memory_stats) {
    if (!memory_stats) {
        return NEOC_ERROR_NULL_POINTER;
    }

    memory_stats->total_allocated = atomic_load(&stats.total_allocated);
    memory_stats->current_allocated = atomic_load(&stats.current_usage);
    memory_stats->allocation_count = atomic_load(&stats.allocation_count);
    memory_stats->free_count = atomic_load(&stats.free_count);

    return NEOC_SUCCESS;
}
