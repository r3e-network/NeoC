#ifndef NEOC_ARRAY_H
#define NEOC_ARRAY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Dynamic array structure
typedef struct neoc_array {
    void** items;
    size_t count;
    size_t capacity;
    void (*free_func)(void*);  // Optional custom free function for items
} neoc_array_t;

// Create array
neoc_array_t* neoc_array_create(void);

// Create array with initial capacity
neoc_array_t* neoc_array_create_with_capacity(size_t capacity);

// Create array with custom free function
neoc_array_t* neoc_array_create_with_free_func(void (*free_func)(void*));

// Free array
void neoc_array_free(neoc_array_t* array);

// Add item to array
bool neoc_array_add(neoc_array_t* array, void* item);

// Insert item at index
bool neoc_array_insert(neoc_array_t* array, size_t index, void* item);

// Remove item at index
void* neoc_array_remove(neoc_array_t* array, size_t index);

// Remove item by value
bool neoc_array_remove_item(neoc_array_t* array, void* item);

// Get item at index
void* neoc_array_get(const neoc_array_t* array, size_t index);

// Set item at index
bool neoc_array_set(neoc_array_t* array, size_t index, void* item);

// Find item in array
int neoc_array_index_of(const neoc_array_t* array, void* item);

// Check if array contains item
bool neoc_array_contains(const neoc_array_t* array, void* item);

// Clear array (remove all items)
void neoc_array_clear(neoc_array_t* array);

// Get array size
size_t neoc_array_size(const neoc_array_t* array);

// Check if array is empty
bool neoc_array_is_empty(const neoc_array_t* array);

// Sort array with custom comparator
void neoc_array_sort(neoc_array_t* array, int (*compare)(const void*, const void*));

// Reverse array
void neoc_array_reverse(neoc_array_t* array);

// Map function over array (create new array)
neoc_array_t* neoc_array_map(const neoc_array_t* array, void* (*map_func)(void*));

// Filter array (create new array)
neoc_array_t* neoc_array_filter(const neoc_array_t* array, bool (*filter_func)(void*));

// Reduce array to single value
void* neoc_array_reduce(const neoc_array_t* array, 
                        void* (*reduce_func)(void*, void*), 
                        void* initial);

// For each item in array
void neoc_array_foreach(const neoc_array_t* array, void (*func)(void*));

// Clone array (shallow copy)
neoc_array_t* neoc_array_clone(const neoc_array_t* array);

// Clone array (deep copy with clone function)
neoc_array_t* neoc_array_deep_clone(const neoc_array_t* array, void* (*clone_func)(void*));

// Convert array to C array
void** neoc_array_to_c_array(const neoc_array_t* array);

// Create array from C array
neoc_array_t* neoc_array_from_c_array(void** items, size_t count);

// Byte array utilities
uint8_t* neoc_byte_array_concat(const uint8_t* a, size_t a_len, 
                                 const uint8_t* b, size_t b_len);
uint8_t* neoc_byte_array_slice(const uint8_t* array, size_t start, size_t end);
bool neoc_byte_array_equals(const uint8_t* a, size_t a_len,
                            const uint8_t* b, size_t b_len);
void neoc_byte_array_reverse(uint8_t* array, size_t len);

#ifdef __cplusplus
}
#endif

#endif // NEOC_ARRAY_H
