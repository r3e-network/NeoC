/**
 * @file neoc_bytes.c
 * @brief Implementation of byte manipulation utilities
 */

#include "neoc/utils/neoc_bytes.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/neoc_memory.h"
#include <string.h>

neoc_error_t neoc_bytes_reverse(uint8_t* data, size_t length) {
    if (!data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length <= 1) {
        return NEOC_SUCCESS; // Nothing to reverse
    }
    
    size_t start = 0;
    size_t end = length - 1;
    
    while (start < end) {
        uint8_t temp = data[start];
        data[start] = data[end];
        data[end] = temp;
        start++;
        end--;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_reverse_copy(const uint8_t* src, size_t src_len,
                                     uint8_t* dst, size_t dst_len) {
    if (!src || !dst) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (dst_len < src_len) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    for (size_t i = 0; i < src_len; i++) {
        dst[i] = src[src_len - 1 - i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_pad(const uint8_t* src, size_t src_len,
                           size_t target_len, bool trailing,
                           uint8_t** result, size_t* result_len) {
    if (!src || !result || !result_len) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    // Skip leading zero if present
    size_t src_offset = (src_len > 0 && src[0] == 0) ? 1 : 0;
    size_t effective_len = src_len - src_offset;
    
    if (effective_len > target_len) {
        return NEOC_ERROR_INVALID_ARGUMENT; // Input too large
    }
    
    *result = neoc_malloc(target_len);
    if (!*result) {
        return NEOC_ERROR_MEMORY;
    }
    
    *result_len = target_len;
    
    if (trailing) {
        // Copy data to beginning, pad with zeros at end
        memcpy(*result, src + src_offset, effective_len);
        memset(*result + effective_len, 0, target_len - effective_len);
    } else {
        // Pad with zeros at beginning, copy data to end
        memset(*result, 0, target_len - effective_len);
        memcpy(*result + (target_len - effective_len), src + src_offset, effective_len);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_trim_trailing(const uint8_t* src, size_t src_len,
                                      uint8_t byte_value,
                                      uint8_t** result, size_t* result_len) {
    if (!src || !result || !result_len) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    // Find effective length by trimming trailing bytes
    size_t effective_len = src_len;
    while (effective_len > 0 && src[effective_len - 1] == byte_value) {
        effective_len--;
    }
    
    if (effective_len == 0) {
        *result = NULL;
        *result_len = 0;
        return NEOC_SUCCESS;
    }
    
    *result = neoc_malloc(effective_len);
    if (!*result) {
        return NEOC_ERROR_MEMORY;
    }
    
    memcpy(*result, src, effective_len);
    *result_len = effective_len;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_xor(const uint8_t* lhs, const uint8_t* rhs,
                            size_t length, uint8_t* result) {
    if (!lhs || !rhs || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    for (size_t i = 0; i < length; i++) {
        result[i] = lhs[i] ^ rhs[i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_script_hash_to_address(const uint8_t* script_hash,
                                               uint8_t address_version,
                                               char* address, size_t address_size) {
    if (!script_hash || !address) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    // Build the script: version + reversed_hash
    uint8_t script[21];
    script[0] = address_version;
    
    // Reverse the script hash
    for (int i = 0; i < 20; i++) {
        script[1 + i] = script_hash[19 - i];
    }
    
    // Calculate checksum (first 4 bytes of double SHA256)
    uint8_t hash[32];
    neoc_error_t err = neoc_sha256_double(script, 21, hash);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Build final payload: script + checksum
    uint8_t payload[25];
    memcpy(payload, script, 21);
    memcpy(payload + 21, hash, 4);
    
    // Base58 encode
    return neoc_base58_encode(payload, 25, address, address_size);
}

size_t neoc_bytes_var_size(size_t value) {
    if (value < 0xfd) {
        return 1;
    } else if (value <= 0xffff) {
        return 3;
    } else if (value <= 0xffffffff) {
        return 5;
    } else {
        return 9;
    }
}

neoc_error_t neoc_bytes_encode_var_size(size_t value, uint8_t* buffer,
                                        size_t buffer_size, size_t* bytes_written) {
    if (!buffer || !bytes_written) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (value < 0xfd) {
        if (buffer_size < 1) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        buffer[0] = (uint8_t)value;
        *bytes_written = 1;
    } else if (value <= 0xffff) {
        if (buffer_size < 3) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        buffer[0] = 0xfd;
        buffer[1] = (uint8_t)(value & 0xff);
        buffer[2] = (uint8_t)((value >> 8) & 0xff);
        *bytes_written = 3;
    } else if (value <= 0xffffffff) {
        if (buffer_size < 5) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        buffer[0] = 0xfe;
        buffer[1] = (uint8_t)(value & 0xff);
        buffer[2] = (uint8_t)((value >> 8) & 0xff);
        buffer[3] = (uint8_t)((value >> 16) & 0xff);
        buffer[4] = (uint8_t)((value >> 24) & 0xff);
        *bytes_written = 5;
    } else {
        if (buffer_size < 9) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        buffer[0] = 0xff;
        for (int i = 0; i < 8; i++) {
            buffer[1 + i] = (uint8_t)((value >> (i * 8)) & 0xff);
        }
        *bytes_written = 9;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_decode_var_size(const uint8_t* buffer, size_t buffer_size,
                                        size_t* value, size_t* bytes_read) {
    if (!buffer || !value || !bytes_read) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < 1) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    uint8_t first_byte = buffer[0];
    
    if (first_byte < 0xfd) {
        *value = first_byte;
        *bytes_read = 1;
    } else if (first_byte == 0xfd) {
        if (buffer_size < 3) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        *value = buffer[1] | (buffer[2] << 8);
        *bytes_read = 3;
    } else if (first_byte == 0xfe) {
        if (buffer_size < 5) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        *value = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
        *bytes_read = 5;
    } else { // 0xff
        if (buffer_size < 9) {
            return NEOC_ERROR_BUFFER_TOO_SMALL;
        }
        *value = 0;
        for (int i = 0; i < 8; i++) {
            *value |= ((size_t)buffer[1 + i]) << (i * 8);
        }
        *bytes_read = 9;
    }
    
    return NEOC_SUCCESS;
}

bool neoc_byte_is_between_opcodes(uint8_t byte, uint8_t op1, uint8_t op2) {
    return byte >= op1 && byte <= op2;
}

neoc_error_t neoc_bytes_to_uint64_le(const uint8_t* bytes, size_t length,
                                     uint64_t* result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length > 8) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *result = 0;
    for (size_t i = 0; i < length; i++) {
        *result |= ((uint64_t)bytes[i]) << (i * 8);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bytes_to_uint64_be(const uint8_t* bytes, size_t length,
                                     uint64_t* result) {
    if (!bytes || !result) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length > 8) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    *result = 0;
    for (size_t i = 0; i < length; i++) {
        *result = (*result << 8) | bytes[i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_uint64_to_bytes_le(uint64_t value, uint8_t* bytes, size_t length) {
    if (!bytes) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length > 8) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    for (size_t i = 0; i < length; i++) {
        bytes[i] = (uint8_t)((value >> (i * 8)) & 0xff);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_uint64_to_bytes_be(uint64_t value, uint8_t* bytes, size_t length) {
    if (!bytes) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length > 8) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    for (size_t i = 0; i < length; i++) {
        bytes[i] = (uint8_t)((value >> ((length - 1 - i) * 8)) & 0xff);
    }
    
    return NEOC_SUCCESS;
}
