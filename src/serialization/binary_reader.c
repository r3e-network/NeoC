#include "neoc/serialization/binary_reader.h"
#include <stdlib.h>
#include <string.h>

neoc_error_t neoc_binary_reader_create(const uint8_t *data,
                                        size_t size,
                                        neoc_binary_reader_t **reader) {
    if (!data || !reader) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *reader = calloc(1, sizeof(neoc_binary_reader_t));
    if (!*reader) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate binary reader");
    }
    
    (*reader)->data = data;
    (*reader)->size = size;
    (*reader)->position = 0;
    (*reader)->marker = SIZE_MAX; // No marker set initially
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_byte(neoc_binary_reader_t *reader,
                                           uint8_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (reader->position >= reader->size) {
        return neoc_error_set(NEOC_ERROR_END_OF_STREAM, "End of stream reached");
    }
    
    *value = reader->data[reader->position++];
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_bytes(neoc_binary_reader_t *reader,
                                            uint8_t *buffer,
                                            size_t len) {
    if (!reader || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (len == 0) return NEOC_SUCCESS;
    
    if (reader->position + len > reader->size) {
        return neoc_error_set(NEOC_ERROR_END_OF_STREAM, "Not enough data to read");
    }
    
    memcpy(buffer, reader->data + reader->position, len);
    reader->position += len;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_bool(neoc_binary_reader_t *reader,
                                           bool *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t byte_val;
    neoc_error_t err = neoc_binary_reader_read_byte(reader, &byte_val);
    if (err != NEOC_SUCCESS) return err;
    
    *value = (byte_val != 0);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_uint16(neoc_binary_reader_t *reader,
                                             uint16_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (reader->position + 2 > reader->size) {
        return neoc_error_set(NEOC_ERROR_END_OF_STREAM, "Not enough data to read uint16");
    }
    
    // Little-endian
    *value = (uint16_t)reader->data[reader->position] |
             ((uint16_t)reader->data[reader->position + 1] << 8);
    reader->position += 2;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_uint32(neoc_binary_reader_t *reader,
                                             uint32_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (reader->position + 4 > reader->size) {
        return neoc_error_set(NEOC_ERROR_END_OF_STREAM, "Not enough data to read uint32");
    }
    
    // Little-endian
    *value = (uint32_t)reader->data[reader->position] |
             ((uint32_t)reader->data[reader->position + 1] << 8) |
             ((uint32_t)reader->data[reader->position + 2] << 16) |
             ((uint32_t)reader->data[reader->position + 3] << 24);
    reader->position += 4;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_uint64(neoc_binary_reader_t *reader,
                                             uint64_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (reader->position + 8 > reader->size) {
        return neoc_error_set(NEOC_ERROR_END_OF_STREAM, "Not enough data to read uint64");
    }
    
    // Little-endian
    *value = 0;
    for (int i = 0; i < 8; i++) {
        *value |= ((uint64_t)reader->data[reader->position + i] << (i * 8));
    }
    reader->position += 8;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_int16(neoc_binary_reader_t *reader,
                                            int16_t *value) {
    return neoc_binary_reader_read_uint16(reader, (uint16_t*)value);
}

neoc_error_t neoc_binary_reader_read_int32(neoc_binary_reader_t *reader,
                                            int32_t *value) {
    return neoc_binary_reader_read_uint32(reader, (uint32_t*)value);
}

neoc_error_t neoc_binary_reader_read_int64(neoc_binary_reader_t *reader,
                                            int64_t *value) {
    return neoc_binary_reader_read_uint64(reader, (uint64_t*)value);
}

neoc_error_t neoc_binary_reader_read_var_int(neoc_binary_reader_t *reader,
                                              uint64_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t prefix;
    neoc_error_t err = neoc_binary_reader_read_byte(reader, &prefix);
    if (err != NEOC_SUCCESS) return err;
    
    if (prefix < 0xFD) {
        *value = prefix;
        return NEOC_SUCCESS;
    } else if (prefix == 0xFD) {
        uint16_t val;
        err = neoc_binary_reader_read_uint16(reader, &val);
        if (err != NEOC_SUCCESS) return err;
        *value = val;
        return NEOC_SUCCESS;
    } else if (prefix == 0xFE) {
        uint32_t val;
        err = neoc_binary_reader_read_uint32(reader, &val);
        if (err != NEOC_SUCCESS) return err;
        *value = val;
        return NEOC_SUCCESS;
    } else { // 0xFF
        return neoc_binary_reader_read_uint64(reader, value);
    }
}

neoc_error_t neoc_binary_reader_read_var_bytes(neoc_binary_reader_t *reader,
                                                uint8_t **data,
                                                size_t *len) {
    if (!reader || !data || !len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint64_t length;
    neoc_error_t err = neoc_binary_reader_read_var_int(reader, &length);
    if (err != NEOC_SUCCESS) return err;
    
    if (length > SIZE_MAX) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Variable bytes length too large");
    }
    
    *len = (size_t)length;
    
    if (*len == 0) {
        *data = NULL;
        return NEOC_SUCCESS;
    }
    
    *data = malloc(*len);
    if (!*data) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate data buffer");
    }
    
    err = neoc_binary_reader_read_bytes(reader, *data, *len);
    if (err != NEOC_SUCCESS) {
        free(*data);
        *data = NULL;
        *len = 0;
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_var_string(neoc_binary_reader_t *reader,
                                                 char **str) {
    if (!reader || !str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t *data;
    size_t len;
    neoc_error_t err = neoc_binary_reader_read_var_bytes(reader, &data, &len);
    if (err != NEOC_SUCCESS) return err;
    
    if (len == 0) {
        *str = calloc(1, 1);  // Empty string
        if (!*str) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate empty string");
        }
        return NEOC_SUCCESS;
    }
    
    *str = malloc(len + 1);
    if (!*str) {
        free(data);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate string");
    }
    
    memcpy(*str, data, len);
    (*str)[len] = '\0';
    
    free(data);
    return NEOC_SUCCESS;
}

size_t neoc_binary_reader_get_position(const neoc_binary_reader_t *reader) {
    return reader ? reader->position : 0;
}

size_t neoc_binary_reader_get_remaining(const neoc_binary_reader_t *reader) {
    if (!reader || reader->position >= reader->size) {
        return 0;
    }
    return reader->size - reader->position;
}

bool neoc_binary_reader_is_at_end(const neoc_binary_reader_t *reader) {
    return !reader || reader->position >= reader->size;
}

neoc_error_t neoc_binary_reader_seek(neoc_binary_reader_t *reader,
                                      size_t position) {
    if (!reader) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid reader");
    }
    
    if (position > reader->size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Position beyond data size");
    }
    
    reader->position = position;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_skip(neoc_binary_reader_t *reader,
                                      size_t count) {
    if (!reader) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid reader");
    }
    
    if (reader->position + count > reader->size) {
        return neoc_error_set(NEOC_ERROR_END_OF_STREAM, "Not enough data to skip");
    }
    
    reader->position += count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_mark(neoc_binary_reader_t *reader) {
    if (!reader) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid reader");
    }
    
    reader->marker = reader->position;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_reset(neoc_binary_reader_t *reader) {
    if (!reader) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid reader");
    }
    
    if (reader->marker == SIZE_MAX) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "No marker set");
    }
    
    reader->position = reader->marker;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_encoded_ec_point(neoc_binary_reader_t *reader,
                                                      uint8_t **data,
                                                      size_t *len) {
    if (!reader || !data || !len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *data = NULL;
    *len = 0;
    
    uint8_t byte;
    neoc_error_t err = neoc_binary_reader_read_byte(reader, &byte);
    if (err != NEOC_SUCCESS) return err;
    
    if (byte == 0x02 || byte == 0x03) {
        // Compressed point: 1 byte prefix + 32 bytes
        *len = 33;
        *data = malloc(*len);
        if (!*data) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate EC point data");
        }
        
        (*data)[0] = byte;
        err = neoc_binary_reader_read_bytes(reader, *data + 1, 32);
        if (err != NEOC_SUCCESS) {
            free(*data);
            *data = NULL;
            *len = 0;
        }
        return err;
    }
    
    return neoc_error_set(NEOC_ERROR_INVALID_DATA, "Invalid EC point encoding");
}

neoc_error_t neoc_binary_reader_read_push_data(neoc_binary_reader_t *reader,
                                                uint8_t **data,
                                                size_t *len) {
    if (!reader || !data || !len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *data = NULL;
    *len = 0;
    
    uint8_t byte;
    neoc_error_t err = neoc_binary_reader_read_byte(reader, &byte);
    if (err != NEOC_SUCCESS) return err;
    
    size_t size;
    switch (byte) {
        case 0x4C: // PUSHDATA1
            {
                uint8_t size8;
                err = neoc_binary_reader_read_byte(reader, &size8);
                if (err != NEOC_SUCCESS) return err;
                size = size8;
            }
            break;
        case 0x4D: // PUSHDATA2
            {
                uint16_t size16;
                err = neoc_binary_reader_read_uint16(reader, &size16);
                if (err != NEOC_SUCCESS) return err;
                size = size16;
            }
            break;
        case 0x4E: // PUSHDATA4
            {
                uint32_t size32;
                err = neoc_binary_reader_read_uint32(reader, &size32);
                if (err != NEOC_SUCCESS) return err;
                size = size32;
            }
            break;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_DATA, "Not a PUSHDATA opcode");
    }
    
    if (size == 0) {
        *data = malloc(1); // Empty but valid pointer
        if (!*data) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate empty buffer");
        }
        *len = 0;
        return NEOC_SUCCESS;
    }
    
    *data = malloc(size);
    if (!*data) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    err = neoc_binary_reader_read_bytes(reader, *data, size);
    if (err != NEOC_SUCCESS) {
        free(*data);
        *data = NULL;
        return err;
    }
    
    *len = size;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_var_bytes_max(neoc_binary_reader_t *reader,
                                                    size_t max_length,
                                                    uint8_t **data,
                                                    size_t *len) {
    if (!reader || !data || !len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint64_t length;
    neoc_error_t err = neoc_binary_reader_read_var_int(reader, &length);
    if (err != NEOC_SUCCESS) return err;
    
    if (length > max_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_DATA, "Length exceeds maximum");
    }
    
    *len = (size_t)length;
    if (*len == 0) {
        *data = malloc(1); // Empty but valid pointer
        if (!*data) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate empty buffer");
        }
        return NEOC_SUCCESS;
    }
    
    *data = malloc(*len);
    if (!*data) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    err = neoc_binary_reader_read_bytes(reader, *data, *len);
    if (err != NEOC_SUCCESS) {
        free(*data);
        *data = NULL;
        *len = 0;
    }
    
    return err;
}

neoc_error_t neoc_binary_reader_read_var_int_max(neoc_binary_reader_t *reader,
                                                  uint64_t max_value,
                                                  uint64_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_error_t err = neoc_binary_reader_read_var_int(reader, value);
    if (err != NEOC_SUCCESS) return err;
    
    if (*value > max_value) {
        return neoc_error_set(NEOC_ERROR_INVALID_DATA, "Value exceeds maximum");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_push_string(neoc_binary_reader_t *reader,
                                                  char **str) {
    if (!reader || !str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t *data;
    size_t len;
    neoc_error_t err = neoc_binary_reader_read_push_data(reader, &data, &len);
    if (err != NEOC_SUCCESS) return err;
    
    *str = malloc(len + 1);
    if (!*str) {
        free(data);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate string");
    }
    
    if (len > 0) {
        memcpy(*str, data, len);
    }
    (*str)[len] = '\0';
    
    free(data);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_push_int(neoc_binary_reader_t *reader,
                                               int32_t *value) {
    if (!reader || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t *data;
    size_t len;
    bool is_negative;
    
    neoc_error_t err = neoc_binary_reader_read_push_big_int(reader, &data, &len, &is_negative);
    if (err != NEOC_SUCCESS) return err;
    
    if (len > 4) {
        free(data);
        return neoc_error_set(NEOC_ERROR_INVALID_DATA, "Integer too large for 32-bit");
    }
    
    uint32_t result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= (uint32_t)data[i] << (i * 8);
    }
    
    *value = is_negative ? -(int32_t)result : (int32_t)result;
    free(data);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_binary_reader_read_push_big_int(neoc_binary_reader_t *reader,
                                                   uint8_t **data,
                                                   size_t *len,
                                                   bool *is_negative) {
    if (!reader || !data || !len || !is_negative) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *data = NULL;
    *len = 0;
    *is_negative = false;
    
    uint8_t byte;
    neoc_error_t err = neoc_binary_reader_read_byte(reader, &byte);
    if (err != NEOC_SUCCESS) return err;
    
    // Handle simple push opcodes (PUSH1-PUSH16, PUSHM1)
    if (byte >= 0x51 && byte <= 0x60) { // PUSH1 - PUSH16
        int32_t val = byte - 0x50;
        *len = sizeof(int32_t);
        *data = malloc(*len);
        if (!*data) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate integer data");
        }
        *(int32_t*)*data = val;
        return NEOC_SUCCESS;
    }
    
    if (byte == 0x4F) { // PUSHM1
        *is_negative = true;
        *len = sizeof(int32_t);
        *data = malloc(*len);
        if (!*data) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate integer data");
        }
        *(int32_t*)*data = 1;
        return NEOC_SUCCESS;
    }
    
    // Handle PUSHINT opcodes
    int count = -1;
    switch (byte) {
        case 0x00: // PUSHINT8
            count = 1;
            break;
        case 0x01: // PUSHINT16
            count = 2;
            break;
        case 0x02: // PUSHINT32
            count = 4;
            break;
        case 0x03: // PUSHINT64
            count = 8;
            break;
        case 0x04: // PUSHINT128
            count = 16;
            break;
        case 0x05: // PUSHINT256
            count = 32;
            break;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_DATA, "Not a PUSHINT opcode");
    }
    
    *data = malloc(count);
    if (!*data) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate integer data");
    }
    
    err = neoc_binary_reader_read_bytes(reader, *data, count);
    if (err != NEOC_SUCCESS) {
        free(*data);
        *data = NULL;
        return err;
    }
    
    *len = count;
    
    // Check if the number is negative (most significant bit set)
    if (count > 0 && (*data)[count - 1] & 0x80) {
        *is_negative = true;
    }
    
    return NEOC_SUCCESS;
}

void neoc_binary_reader_free(neoc_binary_reader_t *reader) {
    if (reader) {
        free(reader);
    }
}
