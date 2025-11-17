#include "neoc/utils/numeric.h"
#include "neoc/utils/neoc_hex.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

static bool neoc_numeric_is_little_endian(void) {
    const uint16_t value = 1;
    return *((const uint8_t *)&value) == 1;
}

int neoc_numeric_power(int base, int power) {
    if (power < 0) {
        return 0;
    }
    int result = 1;
    for (int i = 0; i < power; i++) {
        result *= base;
    }
    return result;
}

size_t neoc_numeric_var_size(uint64_t value) {
    if (value < 0xFD) {
        return 1;
    } else if (value <= 0xFFFF) {
        return 3;
    } else if (value <= 0xFFFFFFFF) {
        return 5;
    }
    return 9;
}

neoc_error_t neoc_numeric_encode_var_size(uint64_t value,
                                           uint8_t *buffer,
                                           size_t buffer_size,
                                           size_t *encoded_size) {
    if (!buffer || !encoded_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "var size encode: invalid arguments");
    }

    size_t needed = neoc_numeric_var_size(value);
    if (buffer_size < needed) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "var size encode: buffer too small");
    }

    if (value < 0xFD) {
        buffer[0] = (uint8_t)value;
    } else if (value <= 0xFFFF) {
        buffer[0] = 0xFD;
        buffer[1] = (uint8_t)(value & 0xFF);
        buffer[2] = (uint8_t)((value >> 8) & 0xFF);
    } else if (value <= 0xFFFFFFFF) {
        buffer[0] = 0xFE;
        for (size_t i = 0; i < 4; i++) {
            buffer[1 + i] = (uint8_t)((value >> (8 * i)) & 0xFF);
        }
    } else {
        buffer[0] = 0xFF;
        for (size_t i = 0; i < 8; i++) {
            buffer[1 + i] = (uint8_t)((value >> (8 * i)) & 0xFF);
        }
    }

    *encoded_size = needed;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_numeric_decode_var_size(const uint8_t *buffer,
                                           size_t buffer_size,
                                           uint64_t *value,
                                           size_t *bytes_consumed) {
    if (!buffer || !value || !bytes_consumed) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "var size decode: invalid arguments");
    }
    if (buffer_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "var size decode: empty buffer");
    }

    uint8_t prefix = buffer[0];
    if (prefix < 0xFD) {
        *value = prefix;
        *bytes_consumed = 1;
        return NEOC_SUCCESS;
    }

    if (prefix == 0xFD) {
        if (buffer_size < 3) {
            return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "var size decode: insufficient bytes");
        }
        *value = (uint64_t)buffer[1] | ((uint64_t)buffer[2] << 8);
        *bytes_consumed = 3;
    } else if (prefix == 0xFE) {
        if (buffer_size < 5) {
            return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "var size decode: insufficient bytes");
        }
        uint64_t result = 0;
        for (size_t i = 0; i < 4; i++) {
            result |= ((uint64_t)buffer[1 + i]) << (8 * i);
        }
        *value = result;
        *bytes_consumed = 5;
    } else {
        if (buffer_size < 9) {
            return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "var size decode: insufficient bytes");
        }
        uint64_t result = 0;
        for (size_t i = 0; i < 8; i++) {
            result |= ((uint64_t)buffer[1 + i]) << (8 * i);
        }
        *value = result;
        *bytes_consumed = 9;
    }

    return NEOC_SUCCESS;
}

uint32_t neoc_numeric_to_unsigned(int32_t value) {
    return (uint32_t)value;
}

static neoc_error_t neoc_numeric_copy_bytes(const void *value,
                                            size_t value_size,
                                            bool little_endian_target,
                                            uint8_t **bytes,
                                            size_t *bytes_len) {
    if (!value || !bytes || !bytes_len || value_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "numeric copy bytes: invalid arguments");
    }

    uint8_t *output = neoc_malloc(value_size);
    if (!output) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "numeric copy bytes: allocation failed");
    }

    const uint8_t *source = (const uint8_t *)value;
    bool little_endian_host = neoc_numeric_is_little_endian();

    if (little_endian_host == little_endian_target) {
        memcpy(output, source, value_size);
    } else {
        for (size_t i = 0; i < value_size; i++) {
            output[i] = source[value_size - 1 - i];
        }
    }

    *bytes = output;
    *bytes_len = value_size;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_numeric_to_bytes_le(const void *value,
                                       size_t value_size,
                                       uint8_t **bytes,
                                       size_t *bytes_len) {
    return neoc_numeric_copy_bytes(value, value_size, true, bytes, bytes_len);
}

neoc_error_t neoc_numeric_to_bytes_be(const void *value,
                                       size_t value_size,
                                       uint8_t **bytes,
                                       size_t *bytes_len) {
    return neoc_numeric_copy_bytes(value, value_size, false, bytes, bytes_len);
}

static neoc_error_t neoc_numeric_from_bytes(const uint8_t *bytes,
                                            size_t bytes_len,
                                            void *value,
                                            size_t value_size,
                                            bool little_endian_source) {
    if (!bytes || !value || bytes_len == 0 || value_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "numeric from bytes: invalid arguments");
    }

    memset(value, 0, value_size);

    size_t copy_len = bytes_len < value_size ? bytes_len : value_size;
    uint8_t *destination = (uint8_t *)value;
    bool little_endian_host = neoc_numeric_is_little_endian();

    if (little_endian_source == little_endian_host) {
        memcpy(destination, bytes, copy_len);
    } else {
        for (size_t i = 0; i < copy_len; i++) {
            destination[value_size - 1 - i] = bytes[i];
        }
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_numeric_from_bytes_le(const uint8_t *bytes,
                                         size_t bytes_len,
                                         void *value,
                                         size_t value_size) {
    return neoc_numeric_from_bytes(bytes, bytes_len, value, value_size, true);
}

neoc_error_t neoc_numeric_from_bytes_be(const uint8_t *bytes,
                                         size_t bytes_len,
                                         void *value,
                                         size_t value_size) {
    return neoc_numeric_from_bytes(bytes, bytes_len, value, value_size, false);
}

neoc_error_t neoc_numeric_bigint_to_padded_bytes(const char *big_int_hex,
                                                  size_t target_length,
                                                  uint8_t **bytes,
                                                  size_t *bytes_len) {
    if (!big_int_hex || !bytes || !bytes_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "bigint to bytes: invalid arguments");
    }
    if (target_length == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "bigint to bytes: zero target length");
    }

    size_t decoded_len = 0;
    uint8_t *decoded = neoc_hex_decode_alloc(big_int_hex, &decoded_len);
    if (!decoded) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "bigint to bytes: invalid hex");
    }

    // Trim leading zeros if necessary
    size_t useful_start = 0;
    while (useful_start < decoded_len && decoded[useful_start] == 0) {
        useful_start++;
    }
    size_t useful_len = decoded_len - useful_start;

    if (useful_len > target_length) {
        neoc_free(decoded);
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "bigint to bytes: exceeds target length");
    }

    uint8_t *output = neoc_calloc(target_length, 1);
    if (!output) {
        neoc_free(decoded);
        return neoc_error_set(NEOC_ERROR_MEMORY, "bigint to bytes: allocation failed");
    }

    if (useful_len > 0) {
        memcpy(output + (target_length - useful_len), decoded + useful_start, useful_len);
    }

    neoc_free(decoded);
    *bytes = output;
    *bytes_len = target_length;
    return NEOC_SUCCESS;
}

int64_t neoc_numeric_current_time_millis(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    const uint64_t WINDOWS_TO_UNIX_EPOCH = 116444736000000000ULL;
    if (li.QuadPart < WINDOWS_TO_UNIX_EPOCH) {
        return 0;
    }
    uint64_t unix_time_100ns = li.QuadPart - WINDOWS_TO_UNIX_EPOCH;
    return (int64_t)(unix_time_100ns / 10000ULL);
#else
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        return 0;
    }
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

int neoc_numeric_get_decimal_scale(const char *decimal_str) {
    if (!decimal_str) {
        return 0;
    }

    const char *dot = strchr(decimal_str, '.');
    if (!dot) {
        return 0;
    }

    const char *ptr = dot + 1;
    int scale = 0;
    while (*ptr) {
        if (!isdigit((unsigned char)*ptr)) {
            break;
        }
        scale++;
        ptr++;
    }
    return scale;
}
