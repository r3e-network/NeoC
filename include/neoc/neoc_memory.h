/**
 * @file neoc_memory.h
 * @brief Memory management utilities for NeoC SDK
 * 
 * Provides safe memory allocation, deallocation, and management
 * functions with leak detection and bounds checking.
 */

#ifndef NEOC_MEMORY_H
#define NEOC_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "neoc/neoc_error.h"

/**
 * @brief Function pointer type for custom memory allocator
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
typedef void* (*neoc_malloc_func_t)(size_t size);

/**
 * @brief Function pointer type for custom memory reallocator
 * 
 * @param ptr Pointer to previously allocated memory
 * @param size New size in bytes
 * @return Pointer to reallocated memory, or NULL on failure
 */
typedef void* (*neoc_realloc_func_t)(void* ptr, size_t size);

/**
 * @brief Function pointer type for custom memory deallocator
 * 
 * @param ptr Pointer to memory to free
 */
typedef void (*neoc_free_func_t)(void* ptr);

/**
 * @brief Memory allocator functions structure
 */
typedef struct {
    neoc_malloc_func_t malloc_func;   ///< Memory allocation function
    neoc_realloc_func_t realloc_func; ///< Memory reallocation function
    neoc_free_func_t free_func;       ///< Memory deallocation function
} neoc_allocator_t;

typedef struct {
    size_t total_allocated;      ///< Total bytes allocated since start
    size_t current_allocated;    ///< Currently allocated bytes
    size_t allocation_count;     ///< Number of allocation calls
    size_t free_count;           ///< Number of free calls
} neoc_memory_stats_t;

/**
 * @brief Set custom memory allocator functions
 * 
 * @param allocator Pointer to allocator functions structure
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_set_allocator(const neoc_allocator_t* allocator);

/**
 * @brief Get current memory allocator functions
 * 
 * @param allocator Pointer to store current allocator functions
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_allocator(neoc_allocator_t* allocator);

/**
 * @brief Reset to default system allocator
 * 
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_reset_allocator(void);

/**
 * @brief Allocate memory with zero initialization
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* neoc_malloc(size_t size);

/**
 * @brief Allocate memory for array with zero initialization
 * 
 * @param count Number of elements
 * @param size Size of each element
 * @return Pointer to allocated memory, or NULL on failure
 */
void* neoc_calloc(size_t count, size_t size);

/**
 * @brief Reallocate memory block
 * 
 * @param ptr Pointer to previously allocated memory
 * @param size New size in bytes
 * @return Pointer to reallocated memory, or NULL on failure
 */
void* neoc_realloc(void* ptr, size_t size);

/**
 * @brief Free allocated memory
 * 
 * @param ptr Pointer to memory to free (can be NULL)
 */
void neoc_free(void* ptr);

/**
 * @brief Securely clear memory before freeing
 * 
 * @param ptr Pointer to memory to clear and free
 * @param size Number of bytes to clear
 */
void neoc_secure_free(void* ptr, size_t size);

/**
 * @brief Duplicate a string with memory allocation
 * 
 * @param str String to duplicate
 * @return Pointer to allocated copy, or NULL on failure
 */
char* neoc_strdup(const char* str);

/**
 * @brief Duplicate a string with length limit
 * 
 * @param str String to duplicate
 * @param max_len Maximum length to copy
 * @return Pointer to allocated copy, or NULL on failure
 */
char* neoc_strndup(const char* str, size_t max_len);

/**
 * @brief Duplicate a byte array with memory allocation
 * 
 * @param data Data to duplicate
 * @param size Number of bytes to copy
 * @return Pointer to allocated copy, or NULL on failure
 */
uint8_t* neoc_memdup(const uint8_t* data, size_t size);

/**
 * @brief Secure memory comparison (constant-time)
 * 
 * @param a First memory block
 * @param b Second memory block
 * @param size Number of bytes to compare
 * @return 0 if equal, non-zero if different
 */
int neoc_secure_memcmp(const void* a, const void* b, size_t size);

/**
 * @brief Secure memory clear (prevents optimization)
 * 
 * @param ptr Memory to clear
 * @param size Number of bytes to clear
 */
void neoc_secure_memzero(void* ptr, size_t size);

/**
 * @brief Get the number of currently outstanding allocations.
 *
 * @return Outstanding allocation count.
 */
size_t neoc_get_allocation_count(void);

/**
 * @brief Print a summary of current memory leaks (if any).
 */
void neoc_print_memory_leaks(void);

/* Memory debugging support (enabled in debug builds) */

#ifdef NEOC_DEBUG_MEMORY

/**
 * @brief Enable memory leak detection
 * 
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_memory_debug_enable(void);

/**
 * @brief Disable memory leak detection
 */
void neoc_memory_debug_disable(void);

/**
 * @brief Check for memory leaks and print report
 * 
 * @return Number of leaked blocks
 */
size_t neoc_memory_debug_check_leaks(void);

/**
 * @brief Get current memory usage statistics
 * 
 * @param allocated_bytes Pointer to store number of allocated bytes
 * @param allocated_blocks Pointer to store number of allocated blocks
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_memory_debug_get_stats(size_t* allocated_bytes, size_t* allocated_blocks);

#endif /* NEOC_DEBUG_MEMORY */

neoc_error_t neoc_get_memory_stats(neoc_memory_stats_t* memory_stats);

/* Convenience macros */

/**
 * @brief Allocate memory for a specific type
 */
#define NEOC_MALLOC_TYPE(type) ((type*)neoc_malloc(sizeof(type)))

/**
 * @brief Allocate memory for an array of specific type
 */
#define NEOC_MALLOC_ARRAY(type, count) ((type*)neoc_calloc((count), sizeof(type)))

/**
 * @brief Free memory and set pointer to NULL
 */
#define NEOC_SAFE_FREE(ptr) \
    do { \
        neoc_free(ptr); \
        (ptr) = NULL; \
    } while(0)

/**
 * @brief Securely free memory and set pointer to NULL
 */
#define NEOC_SECURE_FREE(ptr, size) \
    do { \
        neoc_secure_free((ptr), (size)); \
        (ptr) = NULL; \
    } while(0)

#ifndef HAVE_STRDUP
#define HAVE_STRDUP 0
#endif

#if !HAVE_STRDUP
#define strdup neoc_strdup
#define strndup neoc_strndup
#endif

#ifdef __cplusplus
}
#endif

#endif /* NEOC_MEMORY_H */
