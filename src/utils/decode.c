/**
 * @file decode.c
 * @brief Decoding utilities implementation
 *
 * Implements helper functions that mirror the behaviour of Swift's
 * utils/Decode.swift helpers (StringDecodable/SafeDecode) for the parts
 * that the C SDK relies on. The implementation focuses on robust
 * string-to-value conversions with proper error handling and fallback
 * helpers, as well as small JSON field extraction helpers built on cJSON.
 */

#include "neoc/utils/decode.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"

#include <cjson/cJSON.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utility ---------------------------------------------------------------- */

static void trim_whitespace(const char *input, const char **start_out, const char **end_out) {
    const char *start = input;
    const char *end = input + strlen(input);

    while (start < end && isspace((unsigned char)*start)) {
        start++;
    }
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }

    *start_out = start;
    *end_out = end;
}

static neoc_error_t parse_long_long(const char *string,
                                    long long min_value,
                                    long long max_value,
                                    long long *value_out) {
    if (!string || !value_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument to parse_long_long");
    }

    const char *start;
    const char *end;
    trim_whitespace(string, &start, &end);

    if (start == end) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "String is empty");
    }

    errno = 0;
    char *parse_end = NULL;
    long long value = strtoll(start, &parse_end, 10);

    if (errno == ERANGE || value < min_value || value > max_value) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Numeric value out of range");
    }

    if (parse_end == start) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "No numeric data in string");
    }

    while (parse_end < end && isspace((unsigned char)*parse_end)) {
        parse_end++;
    }

    if (parse_end != end) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected characters after number");
    }

    *value_out = value;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_double(const char *string, double *value_out) {
    if (!string || !value_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument to parse_double");
    }

    const char *start;
    const char *end;
    trim_whitespace(string, &start, &end);

    if (start == end) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "String is empty");
    }

    errno = 0;
    char *parse_end = NULL;
    double value = strtod(start, &parse_end);
    if (errno == ERANGE || !isfinite(value)) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Double value out of range");
    }

    if (parse_end == start) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "No numeric data in string");
    }

    while (parse_end < end && isspace((unsigned char)*parse_end)) {
        parse_end++;
    }

    if (parse_end != end) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected characters after number");
    }

    *value_out = value;
    return NEOC_SUCCESS;
}

static neoc_error_t allocate_string_from_format(char **out, const char *format, ...) {
    if (!out || !format) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument to allocate_string_from_format");
    }

    va_list args;
    va_start(args, format);
    int required = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (required < 0) {
        return neoc_error_set(NEOC_ERROR_INTERNAL, "Failed to format string");
    }

    size_t size = (size_t)required + 1;
    char *buffer = (char *)neoc_malloc(size);
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate string");
    }

    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);

    *out = buffer;
    return NEOC_SUCCESS;
}

static void json_free_if_not_null(cJSON *json) {
    if (json) {
        cJSON_Delete(json);
    }
}

/* Conversion helpers ------------------------------------------------------ */

neoc_error_t neoc_decode_int_from_string(const char *string, int *value) {
    long long temp;
    neoc_error_t err = parse_long_long(string, INT_MIN, INT_MAX, &temp);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    *value = (int)temp;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_decode_int64_from_string(const char *string, int64_t *value) {
    long long temp;
    neoc_error_t err = parse_long_long(string, LLONG_MIN, LLONG_MAX, &temp);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    *value = (int64_t)temp;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_decode_bool_from_string(const char *string, bool *value) {
    if (!string || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument to neoc_decode_bool_from_string");
    }

    const char *start;
    const char *end;
    trim_whitespace(string, &start, &end);
    size_t length = (size_t)(end - start);
    if (length == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Empty boolean string");
    }

    char buffer[8];
    if (length >= sizeof(buffer)) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Boolean string too long");
    }

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = (char)tolower((unsigned char)start[i]);
    }
    buffer[length] = '\0';

    if (strcmp(buffer, "true") == 0 || strcmp(buffer, "1") == 0) {
        *value = true;
        return NEOC_SUCCESS;
    }
    if (strcmp(buffer, "false") == 0 || strcmp(buffer, "0") == 0) {
        *value = false;
        return NEOC_SUCCESS;
    }

    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid boolean string");
}

neoc_error_t neoc_decode_double_from_string(const char *string, double *value) {
    return parse_double(string, value);
}

neoc_error_t neoc_decode_int_to_string(int value, char **string) {
    return allocate_string_from_format(string, "%d", value);
}

neoc_error_t neoc_decode_int64_to_string(int64_t value, char **string) {
    return allocate_string_from_format(string, "%lld", (long long)value);
}

neoc_error_t neoc_decode_bool_to_string(bool value, char **string) {
    return allocate_string_from_format(string, "%s", value ? "true" : "false");
}

neoc_error_t neoc_decode_double_to_string(double value, char **string) {
    /* Use %.17g to mirror Swift Double default stringification */
    return allocate_string_from_format(string, "%.17g", value);
}

neoc_error_t neoc_decode_safe_int_from_string(const char *string,
                                              int default_value,
                                              int *value) {
    if (!value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument to neoc_decode_safe_int_from_string");
    }
    if (neoc_decode_int_from_string(string, value) != NEOC_SUCCESS) {
        *value = default_value;
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_decode_safe_bool_from_string(const char *string,
                                               bool default_value,
                                               bool *value) {
    if (!value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument to neoc_decode_safe_bool_from_string");
    }
    if (neoc_decode_bool_from_string(string, value) != NEOC_SUCCESS) {
        *value = default_value;
    }
    return NEOC_SUCCESS;
}

/* JSON helpers ------------------------------------------------------------ */

static neoc_error_t ensure_json_object(const char *json_str, cJSON **out_root) {
    if (!json_str || !out_root) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid JSON argument");
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON payload");
    }

    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "JSON root is not an object");
    }

    *out_root = root;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_decode_json_string_field(const char *json_str,
                                           const char *field_name,
                                           char **value) {
    if (!field_name || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_decode_json_string_field");
    }

    cJSON *root = NULL;
    neoc_error_t err = ensure_json_object(json_str, &root);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    cJSON *field = cJSON_GetObjectItemCaseSensitive(root, field_name);
    if (!field) {
        json_free_if_not_null(root);
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "JSON field not found");
    }

    if (!cJSON_IsString(field) || field->valuestring == NULL) {
        json_free_if_not_null(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "JSON field is not a string");
    }

    *value = neoc_strdup(field->valuestring);
    if (!*value) {
        json_free_if_not_null(root);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to duplicate string");
    }

    json_free_if_not_null(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_decode_json_int_field(const char *json_str,
                                        const char *field_name,
                                        int *value) {
    if (!field_name || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_decode_json_int_field");
    }

    cJSON *root = NULL;
    neoc_error_t err = ensure_json_object(json_str, &root);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    cJSON *field = cJSON_GetObjectItemCaseSensitive(root, field_name);
    if (!field) {
        json_free_if_not_null(root);
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "JSON field not found");
    }

    if (cJSON_IsNumber(field)) {
        double number = field->valuedouble;
        if (number < INT_MIN || number > INT_MAX) {
            json_free_if_not_null(root);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "JSON number out of int range");
        }
        *value = (int)number;
        json_free_if_not_null(root);
        return NEOC_SUCCESS;
    }

    if (cJSON_IsString(field) && field->valuestring) {
        err = neoc_decode_int_from_string(field->valuestring, value);
        json_free_if_not_null(root);
        return err;
    }

    json_free_if_not_null(root);
    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "JSON field cannot be parsed as int");
}

neoc_error_t neoc_decode_json_bool_field(const char *json_str,
                                         const char *field_name,
                                         bool *value) {
    if (!field_name || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_decode_json_bool_field");
    }

    cJSON *root = NULL;
    neoc_error_t err = ensure_json_object(json_str, &root);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    cJSON *field = cJSON_GetObjectItemCaseSensitive(root, field_name);
    if (!field) {
        json_free_if_not_null(root);
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "JSON field not found");
    }

    if (cJSON_IsBool(field)) {
        *value = cJSON_IsTrue(field);
        json_free_if_not_null(root);
        return NEOC_SUCCESS;
    }

    if (cJSON_IsString(field) && field->valuestring) {
        err = neoc_decode_bool_from_string(field->valuestring, value);
        json_free_if_not_null(root);
        return err;
    }

    json_free_if_not_null(root);
    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "JSON field cannot be parsed as bool");
}
