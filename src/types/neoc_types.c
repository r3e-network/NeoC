/**
 * @file neoc_types.c
 * @brief Implementation of basic type definitions for NeoC SDK
 */

#include "neoc/types/neoc_types.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/neoc_hex.h"
#include <string.h>

neoc_bytes_t* neoc_bytes_create(size_t length) {
    neoc_bytes_t* bytes = (neoc_bytes_t*)neoc_malloc(sizeof(neoc_bytes_t));
    if (!bytes) {
        return NULL;
    }
    
    bytes->length = length;
    bytes->capacity = length;
    
    if (length > 0) {
        bytes->data = (neoc_byte_t*)neoc_malloc(length);
        if (!bytes->data) {
            neoc_free(bytes);
            return NULL;
        }
        memset(bytes->data, 0, length);
    } else {
        bytes->data = NULL;
    }
    
    return bytes;
}

neoc_bytes_t* neoc_bytes_from_data(const neoc_byte_t* data, size_t length) {
    if (!data && length > 0) {
        return NULL;
    }
    
    neoc_bytes_t* bytes = neoc_bytes_create(length);
    if (!bytes) {
        return NULL;
    }
    
    if (length > 0) {
        memcpy(bytes->data, data, length);
    }
    
    return bytes;
}

neoc_bytes_t* neoc_bytes_from_hex(const char* hex_string) {
    if (!hex_string) {
        return NULL;
    }
    
    size_t decoded_length;
    uint8_t* decoded = neoc_hex_decode_alloc(hex_string, &decoded_length);
    if (!decoded) {
        return NULL;
    }
    
    neoc_bytes_t* bytes = neoc_bytes_from_data(decoded, decoded_length);
    neoc_free(decoded);
    
    return bytes;
}

neoc_bytes_t* neoc_bytes_duplicate(const neoc_bytes_t* bytes) {
    if (!bytes) {
        return NULL;
    }
    
    return neoc_bytes_from_data(bytes->data, bytes->length);
}

neoc_error_t neoc_bytes_resize(neoc_bytes_t* bytes, size_t new_length) {
    if (!bytes) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (new_length == bytes->length) {
        return NEOC_SUCCESS;
    }
    
    if (new_length == 0) {
        /* Resize to zero - free data */
        neoc_free(bytes->data);
        bytes->data = NULL;
        bytes->length = 0;
        bytes->capacity = 0;
        return NEOC_SUCCESS;
    }
    
    if (new_length <= bytes->capacity) {
        /* Shrinking within existing capacity */
        bytes->length = new_length;
        return NEOC_SUCCESS;
    }
    
    /* Need to reallocate */
    neoc_byte_t* new_data = (neoc_byte_t*)neoc_realloc(bytes->data, new_length);
    if (!new_data) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    /* Zero out new bytes */
    if (new_length > bytes->length) {
        memset(new_data + bytes->length, 0, new_length - bytes->length);
    }
    
    bytes->data = new_data;
    bytes->length = new_length;
    bytes->capacity = new_length;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_append(neoc_bytes_t* bytes, const neoc_byte_t* data, size_t length) {
    if (!bytes) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!data && length > 0) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length == 0) {
        return NEOC_SUCCESS;
    }
    
    size_t old_length = bytes->length;
    neoc_error_t result = neoc_bytes_resize(bytes, old_length + length);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    memcpy(bytes->data + old_length, data, length);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_reverse(neoc_bytes_t* bytes) {
    if (!bytes) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (bytes->length <= 1) {
        return NEOC_SUCCESS;
    }
    
    neoc_byte_t* data = bytes->data;
    size_t left = 0;
    size_t right = bytes->length - 1;
    
    while (left < right) {
        neoc_byte_t temp = data[left];
        data[left] = data[right];
        data[right] = temp;
        left++;
        right--;
    }
    
    return NEOC_SUCCESS;
}

neoc_bytes_t* neoc_bytes_reversed(const neoc_bytes_t* bytes) {
    if (!bytes) {
        return NULL;
    }
    
    neoc_bytes_t* reversed = neoc_bytes_duplicate(bytes);
    if (!reversed) {
        return NULL;
    }
    
    if (neoc_bytes_reverse(reversed) != NEOC_SUCCESS) {
        neoc_bytes_free(reversed);
        return NULL;
    }
    
    return reversed;
}

int neoc_bytes_compare(const neoc_bytes_t* a, const neoc_bytes_t* b) {
    if (!a || !b) {
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    
    size_t min_len = (a->length < b->length) ? a->length : b->length;
    
    int result = memcmp(a->data, b->data, min_len);
    if (result != 0) {
        return result;
    }
    
    /* If common parts are equal, shorter array is "less" */
    if (a->length < b->length) return -1;
    if (a->length > b->length) return 1;
    
    return 0;
}

bool neoc_bytes_equal(const neoc_bytes_t* a, const neoc_bytes_t* b) {
    return neoc_bytes_compare(a, b) == 0;
}

char* neoc_bytes_to_hex(const neoc_bytes_t* bytes, bool uppercase) {
    if (!bytes) {
        return NULL;
    }
    
    return neoc_hex_encode_alloc(bytes->data, bytes->length, uppercase, false);
}

void neoc_bytes_clear(neoc_bytes_t* bytes) {
    if (bytes && bytes->data && bytes->length > 0) {
        neoc_secure_memzero(bytes->data, bytes->length);
    }
}

void neoc_bytes_free(neoc_bytes_t* bytes) {
    if (bytes) {
        neoc_free(bytes->data);
        neoc_free(bytes);
    }
}

void neoc_bytes_secure_free(neoc_bytes_t* bytes) {
    if (bytes) {
        if (bytes->data && bytes->length > 0) {
            neoc_secure_memzero(bytes->data, bytes->length);
        }
        neoc_free(bytes->data);
        neoc_free(bytes);
    }
}
