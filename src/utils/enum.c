#include "neoc/utils/enum.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

static neoc_error_t duplicate_error(const char *message) {
    return neoc_error_set(NEOC_ERROR_INVALID_STATE, message);
}

neoc_error_t neoc_enum_find_by_byte(const neoc_byte_enum_def_t *enum_def,
                                     uint8_t byte_value,
                                     const neoc_byte_enum_entry_t **entry) {
    if (!enum_def || !entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum arguments");
    }

    for (size_t i = 0; i < enum_def->count; i++) {
        if (enum_def->entries[i].byte_value == byte_value) {
            *entry = &enum_def->entries[i];
            return NEOC_SUCCESS;
        }
    }

    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Enum byte value not found");
}

neoc_error_t neoc_enum_throwing_value_of(const neoc_byte_enum_def_t *enum_def,
                                          uint8_t byte_value,
                                          const neoc_byte_enum_entry_t **entry) {
    return neoc_enum_find_by_byte(enum_def, byte_value, entry);
}

neoc_error_t neoc_enum_find_by_json_value(const neoc_byte_enum_def_t *enum_def,
                                           const char *json_value,
                                           const neoc_byte_enum_entry_t **entry) {
    if (!enum_def || !json_value || !entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum arguments");
    }

    for (size_t i = 0; i < enum_def->count; i++) {
        const char *candidate = enum_def->entries[i].json_value;
        if (candidate && strcmp(candidate, json_value) == 0) {
            *entry = &enum_def->entries[i];
            return NEOC_SUCCESS;
        }
    }

    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Enum JSON value not found");
}

neoc_error_t neoc_enum_find_by_name(const neoc_byte_enum_def_t *enum_def,
                                     const char *name,
                                     const neoc_byte_enum_entry_t **entry) {
    if (!enum_def || !name || !entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum arguments");
    }

    for (size_t i = 0; i < enum_def->count; i++) {
        const char *candidate = enum_def->entries[i].name;
        if (candidate && strcmp(candidate, name) == 0) {
            *entry = &enum_def->entries[i];
            return NEOC_SUCCESS;
        }
    }

    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Enum name not found");
}

neoc_error_t neoc_enum_parse_json(const neoc_byte_enum_def_t *enum_def,
                                   const char *json_str,
                                   const neoc_byte_enum_entry_t **entry) {
    if (!enum_def || !json_str || !entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum arguments");
    }

    const char *start = NULL;
    const char *end = NULL;
    trim_whitespace(json_str, &start, &end);
    size_t length = (size_t)(end - start);

    if (length == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Empty enum JSON string");
    }

    if (*start == '"' && length >= 2 && *(end - 1) == '"') {
        size_t value_len = length - 2;
        char *value = neoc_malloc(value_len + 1);
        if (!value) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate enum json value");
        }
        memcpy(value, start + 1, value_len);
        value[value_len] = '\0';

        neoc_error_t err = neoc_enum_find_by_json_value(enum_def, value, entry);
        neoc_free(value);
        return err;
    }

    char *number_buffer = neoc_malloc(length + 1);
    if (!number_buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate enum numeric buffer");
    }

    memcpy(number_buffer, start, length);
    number_buffer[length] = '\0';

    char *endptr = NULL;
    unsigned long value = strtoul(number_buffer, &endptr, 0);
    neoc_free(number_buffer);

    if (endptr == number_buffer || value > UINT8_MAX) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum numeric JSON value");
    }

    return neoc_enum_find_by_byte(enum_def, (uint8_t)value, entry);
}

neoc_error_t neoc_enum_to_json(const neoc_byte_enum_entry_t *entry,
                                char **json_str) {
    if (!entry || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum arguments");
    }

    const char *value = entry->json_value;
    if (value) {
        size_t len = strlen(value);
        char *buffer = neoc_malloc(len + 3);
        if (!buffer) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate enum json");
        }
        buffer[0] = '"';
        memcpy(buffer + 1, value, len);
        buffer[len + 1] = '"';
        buffer[len + 2] = '\0';
        *json_str = buffer;
        return NEOC_SUCCESS;
    }

    char temp[32];
    int written = snprintf(temp, sizeof(temp), "%u", (unsigned)entry->byte_value);
    if (written < 0 || (size_t)written >= sizeof(temp)) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Enum numeric conversion failed");
    }

    char *buffer = neoc_malloc((size_t)written + 1);
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate enum json");
    }
    memcpy(buffer, temp, (size_t)written + 1);
    *json_str = buffer;
    return NEOC_SUCCESS;
}

uint32_t neoc_enum_hash(const neoc_byte_enum_entry_t *entry) {
    if (!entry) {
        return 0;
    }

    uint32_t hash = entry->byte_value;
    if (entry->json_value) {
        hash ^= (uint32_t)strlen(entry->json_value) << 8;
    }
    if (entry->name) {
        hash ^= (uint32_t)strlen(entry->name) << 16;
    }
    return hash;
}

bool neoc_enum_equals(const neoc_byte_enum_entry_t *entry1,
                       const neoc_byte_enum_entry_t *entry2) {
    if (entry1 == entry2) {
        return true;
    }
    if (!entry1 || !entry2) {
        return false;
    }
    return entry1->byte_value == entry2->byte_value;
}

neoc_error_t neoc_enum_validate_definition(const neoc_byte_enum_def_t *enum_def) {
    if (!enum_def || !enum_def->entries || enum_def->count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum definition");
    }

    for (size_t i = 0; i < enum_def->count; i++) {
        const neoc_byte_enum_entry_t *entry_i = &enum_def->entries[i];
        for (size_t j = i + 1; j < enum_def->count; j++) {
            const neoc_byte_enum_entry_t *entry_j = &enum_def->entries[j];

            if (entry_i->byte_value == entry_j->byte_value) {
                return duplicate_error("Duplicate enum byte value");
            }
            if (entry_i->json_value && entry_j->json_value &&
                strcmp(entry_i->json_value, entry_j->json_value) == 0) {
                return duplicate_error("Duplicate enum JSON value");
            }
            if (entry_i->name && entry_j->name &&
                strcmp(entry_i->name, entry_j->name) == 0) {
                return duplicate_error("Duplicate enum name");
            }
        }
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_enum_get_all_cases(const neoc_byte_enum_def_t *enum_def,
                                      const neoc_byte_enum_entry_t **entries,
                                      size_t *count) {
    if (!enum_def || !entries || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid enum arguments");
    }

    *entries = enum_def->entries;
    *count = enum_def->count;
    return NEOC_SUCCESS;
}

bool neoc_enum_contains_byte(const neoc_byte_enum_def_t *enum_def, uint8_t byte_value) {
    const neoc_byte_enum_entry_t *entry = NULL;
    return neoc_enum_find_by_byte(enum_def, byte_value, &entry) == NEOC_SUCCESS;
}

bool neoc_enum_contains_json_value(const neoc_byte_enum_def_t *enum_def, const char *json_value) {
    const neoc_byte_enum_entry_t *entry = NULL;
    return neoc_enum_find_by_json_value(enum_def, json_value, &entry) == NEOC_SUCCESS;
}

const char *neoc_enum_get_type_name(const neoc_byte_enum_def_t *enum_def) {
    return enum_def ? enum_def->type_name : NULL;
}

size_t neoc_enum_get_case_count(const neoc_byte_enum_def_t *enum_def) {
    return enum_def ? enum_def->count : 0;
}
