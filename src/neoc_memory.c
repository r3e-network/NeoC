/**
 * @file neoc_memory.c
 * @brief Memory management implementation
 */

#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

typedef struct memory_stats {
    atomic_size_t total_allocated;
    atomic_size_t total_freed;
    atomic_size_t current_usage;
    atomic_size_t peak_usage;
    atomic_size_t allocation_count;
    atomic_size_t free_count;
} memory_stats_t;

static memory_stats_t stats = {0};
static neoc_allocator_t custom_allocator = {NULL, NULL, NULL};

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
        atomic_fetch_add(&stats.total_allocated, size);
        atomic_fetch_add(&stats.current_usage, size);
        atomic_fetch_add(&stats.allocation_count, 1);
        
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
        atomic_fetch_add(&stats.total_allocated, total);
        atomic_fetch_add(&stats.current_usage, total);
        atomic_fetch_add(&stats.allocation_count, 1);
        
        size_t current = atomic_load(&stats.current_usage);
        size_t peak = atomic_load(&stats.peak_usage);
        if (current > peak) {
            atomic_store(&stats.peak_usage, current);
        }
    }
    
    return ptr;
}

void* neoc_realloc(void *ptr, size_t size) {
    void *new_ptr = NULL;
    
    if (custom_allocator.realloc_func) {
        new_ptr = custom_allocator.realloc_func(ptr, size);
    } else {
        new_ptr = realloc(ptr, size);
    }
    
    if (new_ptr && size > 0) {
        atomic_fetch_add(&stats.total_allocated, size);
        atomic_fetch_add(&stats.current_usage, size);
    }
    
    return new_ptr;
}

void neoc_free(void *ptr) {
    if (!ptr) {
        return;
    }
    
    if (custom_allocator.free_func) {
        custom_allocator.free_func(ptr);
    } else {
        free(ptr);
    }
    
    atomic_fetch_add(&stats.free_count, 1);
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
    
    size_t len = strnlen(str, n);
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

#ifdef NEOC_DEBUG_MEMORY

typedef struct {
    size_t total_allocated;      // Total bytes allocated
    size_t current_allocated;     // Currently allocated bytes
    size_t allocation_count;      // Number of allocations
    size_t free_count;           // Number of frees
} neoc_memory_stats_t;

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

// Additional function for test compatibility
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

#endif /* NEOC_DEBUG_MEMORY */
