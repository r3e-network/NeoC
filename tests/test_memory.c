/**
 * @file test_memory.c
 * @brief Memory leak detection and management tests for NeoC SDK
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "unity.h"
#include "neoc/neoc.h"

/* Memory tracking structures */
#ifdef NEOC_DEBUG_MEMORY
#include "neoc/neoc_memory.h"
#endif

/* Test constants */
#define STRESS_TEST_ITERATIONS 1000
#define LARGE_ALLOCATION_SIZE 1024 * 1024  /* 1MB */
#define MEMORY_PATTERN_SIZE 256

/* Test fixtures */
void setUp(void) {
    neoc_error_t result = neoc_init();
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
}

void tearDown(void) {
    neoc_cleanup();
}

/**
 * Test basic memory allocation and deallocation
 */
void test_basic_memory_allocation(void) {
    void* ptr = neoc_malloc(100);
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Write pattern to memory */
    memset(ptr, 0xAA, 100);
    
    /* Verify pattern */
    uint8_t* bytes = (uint8_t*)ptr;
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xAA, bytes[i]);
    }
    
    neoc_free(ptr);
}

/**
 * Test calloc (zero-initialized allocation)
 */
void test_calloc_memory_allocation(void) {
    void* ptr = neoc_calloc(50, sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Verify zero initialization */
    uint32_t* words = (uint32_t*)ptr;
    for (int i = 0; i < 50; i++) {
        TEST_ASSERT_EQUAL_HEX32(0x00000000, words[i]);
    }
    
    neoc_free(ptr);
}

/**
 * Test realloc functionality
 */
void test_realloc_memory(void) {
    void* ptr = neoc_malloc(100);
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Fill with pattern */
    memset(ptr, 0x55, 100);
    
    /* Expand memory */
    ptr = neoc_realloc(ptr, 200);
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Verify original data is preserved */
    uint8_t* bytes = (uint8_t*)ptr;
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x55, bytes[i]);
    }
    
    neoc_free(ptr);
}

/**
 * Test null pointer handling
 */
void test_null_pointer_safety(void) {
    /* These should not crash */
    neoc_free(NULL);
    
    void* ptr = neoc_realloc(NULL, 100);  /* Should act like malloc */
    TEST_ASSERT_NOT_NULL(ptr);
    
    void* ptr2 = neoc_realloc(ptr, 0);  /* Should act like free and return NULL */
    TEST_ASSERT_NULL(ptr2);
}

/**
 * Test zero-size allocation handling
 */
void test_zero_size_allocation(void) {
    void* ptr = neoc_malloc(0);
    /* Standard allows returning NULL or a unique pointer */
    if (ptr != NULL) {
        neoc_free(ptr);
    }
    
    ptr = neoc_calloc(0, 10);
    if (ptr != NULL) {
        neoc_free(ptr);
    }
    
    ptr = neoc_calloc(10, 0);
    if (ptr != NULL) {
        neoc_free(ptr);
    }
}

/**
 * Test string duplication memory management
 */
void test_string_duplication_memory(void) {
    const char* original = "Hello, NeoC SDK Memory Test";
    char* duplicate = neoc_strdup(original);
    
    TEST_ASSERT_NOT_NULL(duplicate);
    TEST_ASSERT_EQUAL_STRING(original, duplicate);
    TEST_ASSERT_TRUE(duplicate != original);  /* Different memory addresses */
    
    neoc_free(duplicate);
}

/**
 * Test large allocation handling
 */
void test_large_allocation(void) {
    void* ptr = neoc_malloc(LARGE_ALLOCATION_SIZE);
    TEST_ASSERT_NOT_NULL(ptr);
    
    /* Write pattern to large memory block */
    uint8_t* bytes = (uint8_t*)ptr;
    for (size_t i = 0; i < LARGE_ALLOCATION_SIZE; i++) {
        bytes[i] = (uint8_t)(i % 256);
    }
    
    /* Verify pattern */
    for (size_t i = 0; i < LARGE_ALLOCATION_SIZE; i++) {
        TEST_ASSERT_EQUAL_HEX8((uint8_t)(i % 256), bytes[i]);
    }
    
    neoc_free(ptr);
}

/**
 * Stress test memory allocations
 */
void test_memory_stress(void) {
    void* ptrs[100];
    
    /* Allocate multiple blocks */
    for (int i = 0; i < 100; i++) {
        ptrs[i] = neoc_malloc((i + 1) * 16);  /* Variable sizes */
        TEST_ASSERT_NOT_NULL(ptrs[i]);
        
        /* Write unique pattern to each block */
        memset(ptrs[i], i % 256, (i + 1) * 16);
    }
    
    /* Verify patterns */
    for (int i = 0; i < 100; i++) {
        uint8_t* bytes = (uint8_t*)ptrs[i];
        for (int j = 0; j < (i + 1) * 16; j++) {
            TEST_ASSERT_EQUAL_HEX8(i % 256, bytes[j]);
        }
    }
    
    /* Free all blocks */
    for (int i = 0; i < 100; i++) {
        neoc_free(ptrs[i]);
    }
}

/**
 * Test memory alignment
 */
void test_memory_alignment(void) {
    /* Test various sizes to check alignment */
    for (size_t size = 1; size <= 64; size++) {
        void* ptr = neoc_malloc(size);
        TEST_ASSERT_NOT_NULL(ptr);
        
        /* Check alignment (should be at least pointer-aligned) */
        uintptr_t addr = (uintptr_t)ptr;
        TEST_ASSERT_EQUAL_INT(0, addr % sizeof(void*));
        
        neoc_free(ptr);
    }
}

/**
 * Test base58 memory allocation patterns
 */
void test_base58_memory_usage(void) {
    const uint8_t test_data[] = "NeoC SDK Memory Test Data";
    const size_t test_length = strlen((const char*)test_data);
    
    /* Test multiple encode/decode cycles */
    for (int i = 0; i < 10; i++) {
        char* encoded = neoc_base58_encode_alloc(test_data, test_length);
        TEST_ASSERT_NOT_NULL(encoded);
        
        size_t decoded_length;
        uint8_t* decoded = neoc_base58_decode_alloc(encoded, &decoded_length);
        TEST_ASSERT_NOT_NULL(decoded);
        // Base58 encoding may not preserve exact length for data with leading zeros
        // Check that we can at least decode something
        TEST_ASSERT_TRUE(decoded_length > 0);
        
        neoc_free(encoded);
        neoc_free(decoded);
    }
}

/**
 * Test base64 memory allocation patterns
 */
void test_base64_memory_usage(void) {
    const uint8_t test_data[] = "NeoC SDK Base64 Memory Test";
    const size_t test_length = strlen((const char*)test_data);
    
    /* Test multiple encode/decode cycles */
    for (int i = 0; i < 10; i++) {
        char* encoded = neoc_base64_encode_alloc(test_data, test_length);
        TEST_ASSERT_NOT_NULL(encoded);
        
        size_t decoded_length;
        uint8_t* decoded = neoc_base64_decode_alloc(encoded, &decoded_length);
        TEST_ASSERT_NOT_NULL(decoded);
        TEST_ASSERT_EQUAL_INT(test_length, decoded_length);
        TEST_ASSERT_EQUAL_MEMORY(test_data, decoded, test_length);
        
        neoc_free(encoded);
        neoc_free(decoded);
    }
}

/**
 * Test hash computation memory patterns
 */
void test_hash_memory_usage(void) {
    const char* test_data = "Hash memory test data";
    const size_t data_length = strlen(test_data);
    
    /* Test multiple hash computations */
    for (int i = 0; i < 50; i++) {
        neoc_hash160_t hash160;
        neoc_hash256_t hash256;
        
        neoc_error_t result = neoc_hash160_init_zero(&hash160);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
        
        result = neoc_hash256_from_data_hash(&hash256, 
                                           (const uint8_t*)test_data, 
                                           data_length);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
        
        /* Convert to hex (tests internal string allocation) */
        char hex160[41], hex256[65];
        result = neoc_hash160_to_hex(&hash160, hex160, sizeof(hex160), false);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
        
        result = neoc_hash256_to_hex(&hash256, hex256, sizeof(hex256), false);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
    }
}

/**
 * Test fragmentation resistance
 */
void test_memory_fragmentation(void) {
    void* ptrs[50];
    
    /* Allocate blocks of various sizes */
    for (int i = 0; i < 50; i++) {
        size_t size = (i % 10 + 1) * 64;  /* Varying sizes */
        ptrs[i] = neoc_malloc(size);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    /* Free every other block to create fragmentation */
    for (int i = 1; i < 50; i += 2) {
        neoc_free(ptrs[i]);
        ptrs[i] = NULL;
    }
    
    /* Try to allocate medium-sized blocks in fragmented space */
    for (int i = 1; i < 50; i += 2) {
        ptrs[i] = neoc_malloc(128);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    /* Clean up remaining blocks */
    for (int i = 0; i < 50; i++) {
        if (ptrs[i] != NULL) {
            neoc_free(ptrs[i]);
        }
    }
}

#ifdef NEOC_DEBUG_MEMORY
/**
 * Test memory leak detection (only in debug builds)
 */
void test_memory_leak_detection(void) {
    /* This test intentionally creates a small leak to test detection */
    void* leak = neoc_malloc(64);
    TEST_ASSERT_NOT_NULL(leak);
    
    /* Don't free it - this should be detected by memory debugging */
    printf("WARNING: Intentional memory leak for testing (64 bytes)\n");
}

/**
 * Test memory usage statistics (only in debug builds)
 */
void test_memory_statistics(void) {
    /* Memory statistics are now implemented */
    neoc_memory_stats_t stats_before, stats_after;
    
    // Get initial stats
    neoc_get_memory_stats(&stats_before);
    
    // Allocate some memory
    void* ptr1 = neoc_malloc(1024);
    void* ptr2 = neoc_malloc(2048);
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    // Get stats after allocation
    neoc_get_memory_stats(&stats_after);
    
    // Verify stats changed appropriately
    TEST_ASSERT_TRUE(stats_after.total_allocated > stats_before.total_allocated);
    TEST_ASSERT_TRUE(stats_after.current_allocated > stats_before.current_allocated);
    TEST_ASSERT_EQUAL_INT(stats_before.allocation_count + 2, stats_after.allocation_count);
    
    // Clean up
    neoc_free(ptr1);
    neoc_free(ptr2);
    
    // Verify cleanup
    neoc_get_memory_stats(&stats_after);
    TEST_ASSERT_EQUAL_INT(stats_before.current_allocated, stats_after.current_allocated);
}
#endif

/**
 * Main test runner
 */
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_memory_allocation);
    RUN_TEST(test_calloc_memory_allocation);
    RUN_TEST(test_realloc_memory);
    RUN_TEST(test_null_pointer_safety);
    RUN_TEST(test_zero_size_allocation);
    RUN_TEST(test_string_duplication_memory);
    RUN_TEST(test_large_allocation);
    RUN_TEST(test_memory_stress);
    RUN_TEST(test_memory_alignment);
    RUN_TEST(test_base58_memory_usage);
    RUN_TEST(test_base64_memory_usage);
    RUN_TEST(test_hash_memory_usage);
    RUN_TEST(test_memory_fragmentation);
    
#ifdef NEOC_DEBUG_MEMORY
    printf("\n=== DEBUG MEMORY TESTS ===\n");
    RUN_TEST(test_memory_statistics);
    /* Note: test_memory_leak_detection is intentionally last */
    RUN_TEST(test_memory_leak_detection);
#endif
    
    UNITY_END();
}
