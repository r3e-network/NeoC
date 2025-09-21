/**
 * @file neoc_bytes_utils.c
 * @brief Implementation of extended byte manipulation utilities
 */

#include "neoc/utils/neoc_bytes_utils.h"
#include "neoc/utils/neoc_base64.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/neo_constants.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdlib.h>

/* Internal helper functions */
static size_t get_varint_size(size_t value);

neoc_error_t neoc_bytes_to_base64(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size) {
    if (!bytes || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0) {
        if (buffer_size > 0) {
            buffer[0] = '\0';
        }
        return NEOC_SUCCESS;
    }
    
    return neoc_base64_encode(bytes->data, bytes->length, buffer, buffer_size);
}

neoc_error_t neoc_bytes_to_base58(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size) {
    if (!bytes || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0) {
        if (buffer_size > 0) {
            buffer[0] = '\0';
        }
        return NEOC_SUCCESS;
    }
    
    return neoc_base58_encode(bytes->data, bytes->length, buffer, buffer_size);
}

neoc_error_t neoc_bytes_to_base58_check(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size) {
    if (!bytes || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0) {
        if (buffer_size > 0) {
            buffer[0] = '\0';
        }
        return NEOC_SUCCESS;
    }
    
    char* encoded = neoc_base58_check_encode_alloc(bytes->data, bytes->length);
    if (!encoded) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    size_t encoded_len = strlen(encoded);
    if (buffer_size <= encoded_len) {
        neoc_free(encoded);
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    strncpy(buffer, encoded, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    neoc_free(encoded);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_to_hex_no_prefix(const neoc_bytes_t* bytes, char* buffer, size_t buffer_size) {
    if (!bytes || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0) {
        if (buffer_size > 0) {
            buffer[0] = '\0';
        }
        return NEOC_SUCCESS;
    }
    
    /* Convert to hex with prefix first */
    char temp_buffer[NEOC_MAX_HEX_STRING_LENGTH];
    neoc_error_t result = neoc_hex_encode(bytes->data, bytes->length, temp_buffer, sizeof(temp_buffer), false, true);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    /* Remove 0x prefix if present */
    const char* hex_start = (strncmp(temp_buffer, "0x", 2) == 0) ? temp_buffer + 2 : temp_buffer;
    
    size_t hex_len = strlen(hex_start);
    if (hex_len + 1 > buffer_size) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    strncpy(buffer, hex_start, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return NEOC_SUCCESS;
}

size_t neoc_bytes_var_size(const neoc_bytes_t* bytes) {
    if (!bytes) {
        return 0;
    }
    
    return get_varint_size(bytes->length) + bytes->length;
}

neoc_error_t neoc_bytes_script_hash_to_address(const neoc_bytes_t* script_hash, char* address_buffer, size_t buffer_size) {
    if (!script_hash || !address_buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!script_hash->data || script_hash->length != NEOC_HASH160_SIZE) {
        return NEOC_ERROR_INVALID_LENGTH;
    }
    
    /* Create versioned script: version + reversed script hash */
    uint8_t versioned_script[NEOC_HASH160_SIZE + 1];
    versioned_script[0] = NEOC_ADDRESS_VERSION;
    
    /* Reverse the script hash */
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        versioned_script[i + 1] = script_hash->data[NEOC_HASH160_SIZE - 1 - i];
    }
    
    /* Calculate checksum (first 4 bytes of hash256) */
    uint8_t hash256_result[NEOC_HASH256_SIZE];
    neoc_error_t result = neoc_hash256(versioned_script, sizeof(versioned_script), hash256_result);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    /* Create final address data: versioned script + checksum */
    uint8_t address_data[NEOC_HASH160_SIZE + 1 + 4];
    memcpy(address_data, versioned_script, sizeof(versioned_script));
    memcpy(address_data + sizeof(versioned_script), hash256_result, 4);
    
    /* Encode to Base58 */
    return neoc_base58_encode(address_data, sizeof(address_data), address_buffer, buffer_size);
}

neoc_error_t neoc_bytes_pad(const neoc_bytes_t* bytes, size_t length, bool trailing, neoc_bytes_t** result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Check for leading zero and adjust source */
    size_t src_offset = (bytes->length > 0 && bytes->data[0] == 0) ? 1 : 0;
    size_t effective_length = bytes->length - src_offset;
    
    if (effective_length > length) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *result = neoc_bytes_create(length);
    if (!*result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    size_t padding_size = length - effective_length;
    
    if (trailing) {
        /* Copy data first, then pad with zeros */
        memcpy((*result)->data, bytes->data + src_offset, effective_length);
        memset((*result)->data + effective_length, 0, padding_size);
    } else {
        /* Pad with zeros first, then copy data */
        memset((*result)->data, 0, padding_size);
        memcpy((*result)->data + padding_size, bytes->data + src_offset, effective_length);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_trim_trailing(const neoc_bytes_t* bytes, uint8_t byte_value, neoc_bytes_t** result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0) {
        *result = neoc_bytes_create(0);
        return *result ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    /* Find the last non-matching byte */
    size_t new_length = bytes->length;
    while (new_length > 0 && bytes->data[new_length - 1] == byte_value) {
        new_length--;
    }
    
    *result = neoc_bytes_create(new_length);
    if (!*result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (new_length > 0) {
        memcpy((*result)->data, bytes->data, new_length);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_to_uint32(const neoc_bytes_t* bytes, bool little_endian, uint32_t* result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0 || bytes->length > sizeof(uint32_t)) {
        return NEOC_ERROR_INVALID_LENGTH;
    }
    
    *result = 0;
    
    if (little_endian) {
        for (size_t i = 0; i < bytes->length; i++) {
            *result |= ((uint32_t)bytes->data[i]) << (i * 8);
        }
    } else {
        for (size_t i = 0; i < bytes->length; i++) {
            *result = (*result << 8) | bytes->data[i];
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_to_uint64(const neoc_bytes_t* bytes, bool little_endian, uint64_t* result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!bytes->data || bytes->length == 0 || bytes->length > sizeof(uint64_t)) {
        return NEOC_ERROR_INVALID_LENGTH;
    }
    
    *result = 0;
    
    if (little_endian) {
        for (size_t i = 0; i < bytes->length; i++) {
            *result |= ((uint64_t)bytes->data[i]) << (i * 8);
        }
    } else {
        for (size_t i = 0; i < bytes->length; i++) {
            *result = (*result << 8) | bytes->data[i];
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_xor(const neoc_bytes_t* lhs, const neoc_bytes_t* rhs, neoc_bytes_t** result) {
    if (!lhs || !rhs || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!lhs->data || !rhs->data) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (lhs->length != rhs->length) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *result = neoc_bytes_create(lhs->length);
    if (!*result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < lhs->length; i++) {
        (*result)->data[i] = lhs->data[i] ^ rhs->data[i];
    }
    
    return NEOC_SUCCESS;
}

bool neoc_byte_is_between_opcodes(uint8_t byte_value, uint8_t opcode1, uint8_t opcode2) {
    return byte_value >= opcode1 && byte_value <= opcode2;
}

neoc_error_t neoc_bytes_reverse_inplace(neoc_bytes_t* bytes) {
    if (!bytes || !bytes->data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (bytes->length <= 1) {
        return NEOC_SUCCESS; /* Nothing to reverse */
    }
    
    for (size_t i = 0; i < bytes->length / 2; i++) {
        uint8_t temp = bytes->data[i];
        bytes->data[i] = bytes->data[bytes->length - 1 - i];
        bytes->data[bytes->length - 1 - i] = temp;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_reverse_copy(const neoc_bytes_t* bytes, neoc_bytes_t** result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    *result = neoc_bytes_create(bytes->length);
    if (!*result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (bytes->data && bytes->length > 0) {
        for (size_t i = 0; i < bytes->length; i++) {
            (*result)->data[i] = bytes->data[bytes->length - 1 - i];
        }
    }
    
    return NEOC_SUCCESS;
}

/* Internal helper functions */

static size_t get_varint_size(size_t value) {
    if (value < 0xFD) {
        return 1;
    } else if (value <= 0xFFFF) {
        return 3; /* 0xFD + 2 bytes */
    } else if (value <= 0xFFFFFFFF) {
        return 5; /* 0xFE + 4 bytes */
    } else {
        return 9; /* 0xFF + 8 bytes */
    }
}
