/**
 * @file neoc_hash256.c
 * @brief Implementation of Hash256 type for NeoC SDK
 */

#include "neoc/types/neoc_hash256.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/neoc_memory.h"
#include <string.h>

/* Constant zero Hash256 */
const neoc_hash256_t NEOC_HASH256_ZERO_VALUE = {{0}};

neoc_error_t neoc_hash256_init_zero(neoc_hash256_t* hash) {
    if (!hash) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memset(hash->data, 0, NEOC_HASH256_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_from_bytes(neoc_hash256_t* hash, const uint8_t data[NEOC_HASH256_SIZE]) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memcpy(hash->data, data, NEOC_HASH256_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_from_data(neoc_hash256_t* hash, const uint8_t* data, size_t length) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length != NEOC_HASH256_SIZE) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    memcpy(hash->data, data, NEOC_HASH256_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_from_hex(neoc_hash256_t* hash, const char* hex_string) {
    if (!hash || !hex_string) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!neoc_hex_is_valid_string(hex_string, true)) {
        return NEOC_ERROR_INVALID_HEX;
    }
    
    size_t decoded_length;
    neoc_error_t result = neoc_hex_decode(hex_string, hash->data, NEOC_HASH256_SIZE, &decoded_length);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    if (decoded_length != NEOC_HASH256_SIZE) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_from_data_hash(neoc_hash256_t* hash, const uint8_t* data, size_t data_length) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    return neoc_hash256(data, data_length, hash->data);
}

neoc_error_t neoc_hash256_from_data_double_hash(neoc_hash256_t* hash, const uint8_t* data, size_t data_length) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    return neoc_sha256_double(data, data_length, hash->data);
}

neoc_error_t neoc_hash256_copy(neoc_hash256_t* dest, const neoc_hash256_t* src) {
    if (!dest || !src) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memcpy(dest->data, src->data, NEOC_HASH256_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_to_bytes(const neoc_hash256_t* hash, uint8_t* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < NEOC_HASH256_SIZE) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    memcpy(buffer, hash->data, NEOC_HASH256_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_to_little_endian_bytes(const neoc_hash256_t* hash, uint8_t* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < NEOC_HASH256_SIZE) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    /* Reverse byte order */
    for (size_t i = 0; i < NEOC_HASH256_SIZE; i++) {
        buffer[i] = hash->data[NEOC_HASH256_SIZE - 1 - i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash256_to_hex(const neoc_hash256_t* hash, char* buffer, size_t buffer_size, bool uppercase) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t required_size = neoc_hex_encode_buffer_size(NEOC_HASH256_SIZE, false);
    if (buffer_size < required_size) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    return neoc_hex_encode(hash->data, NEOC_HASH256_SIZE, buffer, buffer_size, uppercase, false);
}

int neoc_hash256_compare(const neoc_hash256_t* a, const neoc_hash256_t* b) {
    if (!a || !b) {
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    
    return memcmp(a->data, b->data, NEOC_HASH256_SIZE);
}

bool neoc_hash256_equal(const neoc_hash256_t* a, const neoc_hash256_t* b) {
    return neoc_hash256_compare(a, b) == 0;
}

bool neoc_hash256_is_zero(const neoc_hash256_t* hash) {
    if (!hash) {
        return false;
    }
    
    return neoc_hash256_equal(hash, &NEOC_HASH256_ZERO_VALUE);
}

neoc_error_t neoc_hash256_serialize(const neoc_hash256_t* hash, neoc_binary_writer_t* writer) {
    if (!hash || !writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Write hash bytes directly */
    return neoc_binary_writer_write_bytes(writer, hash->data, NEOC_HASH256_SIZE);
}

neoc_error_t neoc_hash256_deserialize(neoc_hash256_t* hash, neoc_binary_reader_t* reader) {
    if (!hash || !reader) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Read hash bytes directly */
    return neoc_binary_reader_read_bytes(reader, hash->data, NEOC_HASH256_SIZE);
}

size_t neoc_hash256_serialized_size(void) {
    return NEOC_HASH256_SIZE;
}

neoc_error_t neoc_hash256_from_string(const char *str, neoc_hash256_t *hash) {
    if (!str || !hash) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    const char *hex_str = str;
    
    /* Skip '0x' prefix if present */
    if (strlen(str) >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        hex_str = str + 2;
    }
    
    /* Check if length is correct for Hash256 (64 hex characters) */
    size_t hex_len = strlen(hex_str);
    if (hex_len != 64) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return neoc_hash256_from_hex(hash, hex_str);
}

neoc_error_t neoc_hash256_to_string(const neoc_hash256_t *hash, char *buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Need at least 65 bytes for 64 hex chars + null terminator */
    if (buffer_size < 65) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    return neoc_hash256_to_hex(hash, buffer, buffer_size, false);
}
