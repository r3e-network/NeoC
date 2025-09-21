#include <stdlib.h>
#include <string.h>
#include "../../include/neoc/utils/neoc_array.h"
#include "../../include/neoc/neoc_memory.h"

// Create array
neoc_array_t* neoc_array_create(void) {
    return neoc_array_create_with_capacity(16);
}

// Create array with initial capacity
neoc_array_t* neoc_array_create_with_capacity(size_t capacity) {
    neoc_array_t* array = neoc_malloc(sizeof(neoc_array_t));
    if (!array) {
        return NULL;
    }
    
    array->items = neoc_malloc(sizeof(void*) * capacity);
    if (!array->items) {
        neoc_free(array);
        return NULL;
    }
    
    array->count = 0;
    array->capacity = capacity;
    array->free_func = NULL;
    
    return array;
}

// Create array with custom free function
neoc_array_t* neoc_array_create_with_free_func(void (*free_func)(void*)) {
    neoc_array_t* array = neoc_array_create();
    if (array) {
        array->free_func = free_func;
    }
    return array;
}

// Free array
void neoc_array_free(neoc_array_t* array) {
    if (!array) {
        return;
    }
    
    // Free items if free function is provided
    if (array->free_func) {
        for (size_t i = 0; i < array->count; i++) {
            if (array->items[i]) {
                array->free_func(array->items[i]);
            }
        }
    }
    
    neoc_free(array->items);
    neoc_free(array);
}

// Grow array capacity
static bool neoc_array_grow(neoc_array_t* array) {
    size_t new_capacity = array->capacity * 2;
    void** new_items = neoc_realloc(array->items, sizeof(void*) * new_capacity);
    
    if (!new_items) {
        return false;
    }
    
    array->items = new_items;
    array->capacity = new_capacity;
    return true;
}

// Add item to array
bool neoc_array_add(neoc_array_t* array, void* item) {
    if (!array) {
        return false;
    }
    
    // Grow array if needed
    if (array->count >= array->capacity) {
        if (!neoc_array_grow(array)) {
            return false;
        }
    }
    
    array->items[array->count++] = item;
    return true;
}

// Insert item at index
bool neoc_array_insert(neoc_array_t* array, size_t index, void* item) {
    if (!array || index > array->count) {
        return false;
    }
    
    // Grow array if needed
    if (array->count >= array->capacity) {
        if (!neoc_array_grow(array)) {
            return false;
        }
    }
    
    // Shift items to make room
    if (index < array->count) {
        memmove(&array->items[index + 1], 
                &array->items[index], 
                sizeof(void*) * (array->count - index));
    }
    
    array->items[index] = item;
    array->count++;
    return true;
}

// Remove item at index
void* neoc_array_remove(neoc_array_t* array, size_t index) {
    if (!array || index >= array->count) {
        return NULL;
    }
    
    void* item = array->items[index];
    
    // Shift items to fill gap
    if (index < array->count - 1) {
        memmove(&array->items[index], 
                &array->items[index + 1], 
                sizeof(void*) * (array->count - index - 1));
    }
    
    array->count--;
    return item;
}

// Remove item by value
bool neoc_array_remove_item(neoc_array_t* array, void* item) {
    int index = neoc_array_index_of(array, item);
    if (index >= 0) {
        neoc_array_remove(array, (size_t)index);
        return true;
    }
    return false;
}

// Get item at index
void* neoc_array_get(const neoc_array_t* array, size_t index) {
    if (!array || index >= array->count) {
        return NULL;
    }
    return array->items[index];
}

// Set item at index
bool neoc_array_set(neoc_array_t* array, size_t index, void* item) {
    if (!array || index >= array->count) {
        return false;
    }
    
    // Free old item if free function is provided
    if (array->free_func && array->items[index]) {
        array->free_func(array->items[index]);
    }
    
    array->items[index] = item;
    return true;
}

// Find item in array
int neoc_array_index_of(const neoc_array_t* array, void* item) {
    if (!array) {
        return -1;
    }
    
    for (size_t i = 0; i < array->count; i++) {
        if (array->items[i] == item) {
            return (int)i;
        }
    }
    
    return -1;
}

// Check if array contains item
bool neoc_array_contains(const neoc_array_t* array, void* item) {
    return neoc_array_index_of(array, item) >= 0;
}

// Clear array
void neoc_array_clear(neoc_array_t* array) {
    if (!array) {
        return;
    }
    
    // Free items if free function is provided
    if (array->free_func) {
        for (size_t i = 0; i < array->count; i++) {
            if (array->items[i]) {
                array->free_func(array->items[i]);
            }
        }
    }
    
    array->count = 0;
}

// Get array size
size_t neoc_array_size(const neoc_array_t* array) {
    return array ? array->count : 0;
}

// Check if array is empty
bool neoc_array_is_empty(const neoc_array_t* array) {
    return !array || array->count == 0;
}

// Sort array with custom comparator
void neoc_array_sort(neoc_array_t* array, int (*compare)(const void*, const void*)) {
    if (!array || !compare || array->count < 2) {
        return;
    }
    
    qsort(array->items, array->count, sizeof(void*), compare);
}

// Reverse array
void neoc_array_reverse(neoc_array_t* array) {
    if (!array || array->count < 2) {
        return;
    }
    
    size_t left = 0;
    size_t right = array->count - 1;
    
    while (left < right) {
        void* temp = array->items[left];
        array->items[left] = array->items[right];
        array->items[right] = temp;
        left++;
        right--;
    }
}

// Map function over array
neoc_array_t* neoc_array_map(const neoc_array_t* array, void* (*map_func)(void*)) {
    if (!array || !map_func) {
        return NULL;
    }
    
    neoc_array_t* result = neoc_array_create_with_capacity(array->capacity);
    if (!result) {
        return NULL;
    }
    
    for (size_t i = 0; i < array->count; i++) {
        void* mapped = map_func(array->items[i]);
        if (!neoc_array_add(result, mapped)) {
            neoc_array_free(result);
            return NULL;
        }
    }
    
    return result;
}

// Filter array
neoc_array_t* neoc_array_filter(const neoc_array_t* array, bool (*filter_func)(void*)) {
    if (!array || !filter_func) {
        return NULL;
    }
    
    neoc_array_t* result = neoc_array_create();
    if (!result) {
        return NULL;
    }
    
    for (size_t i = 0; i < array->count; i++) {
        if (filter_func(array->items[i])) {
            if (!neoc_array_add(result, array->items[i])) {
                neoc_array_free(result);
                return NULL;
            }
        }
    }
    
    return result;
}

// Reduce array to single value
void* neoc_array_reduce(const neoc_array_t* array, 
                        void* (*reduce_func)(void*, void*), 
                        void* initial) {
    if (!array || !reduce_func) {
        return initial;
    }
    
    void* result = initial;
    for (size_t i = 0; i < array->count; i++) {
        result = reduce_func(result, array->items[i]);
    }
    
    return result;
}

// For each item in array
void neoc_array_foreach(const neoc_array_t* array, void (*func)(void*)) {
    if (!array || !func) {
        return;
    }
    
    for (size_t i = 0; i < array->count; i++) {
        func(array->items[i]);
    }
}

// Clone array (shallow copy)
neoc_array_t* neoc_array_clone(const neoc_array_t* array) {
    if (!array) {
        return NULL;
    }
    
    neoc_array_t* clone = neoc_array_create_with_capacity(array->capacity);
    if (!clone) {
        return NULL;
    }
    
    clone->free_func = array->free_func;
    
    for (size_t i = 0; i < array->count; i++) {
        if (!neoc_array_add(clone, array->items[i])) {
            neoc_array_free(clone);
            return NULL;
        }
    }
    
    return clone;
}

// Clone array (deep copy)
neoc_array_t* neoc_array_deep_clone(const neoc_array_t* array, void* (*clone_func)(void*)) {
    if (!array || !clone_func) {
        return NULL;
    }
    
    neoc_array_t* clone = neoc_array_create_with_capacity(array->capacity);
    if (!clone) {
        return NULL;
    }
    
    clone->free_func = array->free_func;
    
    for (size_t i = 0; i < array->count; i++) {
        void* cloned_item = clone_func(array->items[i]);
        if (!neoc_array_add(clone, cloned_item)) {
            // Clean up already cloned items
            if (clone->free_func) {
                for (size_t j = 0; j < clone->count; j++) {
                    clone->free_func(clone->items[j]);
                }
            }
            neoc_array_free(clone);
            return NULL;
        }
    }
    
    return clone;
}

// Convert array to C array
void** neoc_array_to_c_array(const neoc_array_t* array) {
    if (!array || array->count == 0) {
        return NULL;
    }
    
    void** c_array = neoc_malloc(sizeof(void*) * array->count);
    if (!c_array) {
        return NULL;
    }
    
    memcpy(c_array, array->items, sizeof(void*) * array->count);
    return c_array;
}

// Create array from C array
neoc_array_t* neoc_array_from_c_array(void** items, size_t count) {
    if (!items || count == 0) {
        return NULL;
    }
    
    neoc_array_t* array = neoc_array_create_with_capacity(count);
    if (!array) {
        return NULL;
    }
    
    for (size_t i = 0; i < count; i++) {
        if (!neoc_array_add(array, items[i])) {
            neoc_array_free(array);
            return NULL;
        }
    }
    
    return array;
}

// Byte array utilities
uint8_t* neoc_byte_array_concat(const uint8_t* a, size_t a_len, 
                                const uint8_t* b, size_t b_len) {
    if ((!a && a_len > 0) || (!b && b_len > 0)) {
        return NULL;
    }
    
    size_t total_len = a_len + b_len;
    if (total_len == 0) {
        return NULL;
    }
    
    uint8_t* result = neoc_malloc(total_len);
    if (!result) {
        return NULL;
    }
    
    if (a_len > 0) {
        memcpy(result, a, a_len);
    }
    if (b_len > 0) {
        memcpy(result + a_len, b, b_len);
    }
    
    return result;
}

uint8_t* neoc_byte_array_slice(const uint8_t* array, size_t start, size_t end) {
    if (!array || start > end) {
        return NULL;
    }
    
    size_t len = end - start;
    if (len == 0) {
        return NULL;
    }
    
    uint8_t* result = neoc_malloc(len);
    if (!result) {
        return NULL;
    }
    
    memcpy(result, array + start, len);
    return result;
}

bool neoc_byte_array_equals(const uint8_t* a, size_t a_len,
                           const uint8_t* b, size_t b_len) {
    if (a_len != b_len) {
        return false;
    }
    
    if (a_len == 0) {
        return true;
    }
    
    if (!a || !b) {
        return false;
    }
    
    return memcmp(a, b, a_len) == 0;
}

void neoc_byte_array_reverse(uint8_t* array, size_t len) {
    if (!array || len < 2) {
        return;
    }
    
    size_t left = 0;
    size_t right = len - 1;
    
    while (left < right) {
        uint8_t temp = array[left];
        array[left] = array[right];
        array[right] = temp;
        left++;
        right--;
    }
}
