#include "neoc/protocol/core/record_type.h"
#include "neoc/neoc_memory.h"
#include <string.h>

const char *neoc_record_type_to_string(neoc_record_type_t value) {
    switch (value) {
        case NEOC_RECORD_TYPE_A: return "A";
        case NEOC_RECORD_TYPE_CNAME: return "CNAME";
        case NEOC_RECORD_TYPE_TXT: return "TXT";
        case NEOC_RECORD_TYPE_AAAA: return "AAAA";
        default: return NULL;
    }
}

neoc_error_t neoc_record_type_from_string(const char *str,
                                          neoc_record_type_t *type) {
    if (!str || !type) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid record type string");
    }

    if (strcmp(str, "A") == 0) {
        *type = NEOC_RECORD_TYPE_A;
    } else if (strcmp(str, "CNAME") == 0) {
        *type = NEOC_RECORD_TYPE_CNAME;
    } else if (strcmp(str, "TXT") == 0) {
        *type = NEOC_RECORD_TYPE_TXT;
    } else if (strcmp(str, "AAAA") == 0) {
        *type = NEOC_RECORD_TYPE_AAAA;
    } else {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unsupported record type");
    }
    return NEOC_SUCCESS;
}

uint8_t neoc_record_type_to_byte(neoc_record_type_t type) {
    if (!neoc_record_type_is_valid(type)) {
        return 0;
    }
    return (uint8_t)type;
}

neoc_error_t neoc_record_type_from_byte(uint8_t byte,
                                        neoc_record_type_t *type) {
    if (!type) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid type pointer");
    }
    switch (byte) {
        case NEOC_RECORD_TYPE_A:
        case NEOC_RECORD_TYPE_CNAME:
        case NEOC_RECORD_TYPE_TXT:
        case NEOC_RECORD_TYPE_AAAA:
            *type = (neoc_record_type_t)byte;
            return NEOC_SUCCESS;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unsupported record type byte");
    }
}

bool neoc_record_type_is_valid(neoc_record_type_t type) {
    return type == NEOC_RECORD_TYPE_A ||
           type == NEOC_RECORD_TYPE_CNAME ||
           type == NEOC_RECORD_TYPE_TXT ||
           type == NEOC_RECORD_TYPE_AAAA;
}

neoc_error_t neoc_record_type_get_all_supported(neoc_record_type_t **types,
                                                size_t *count) {
    if (!types || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid output parameters");
    }
    *types = neoc_malloc(4 * sizeof(neoc_record_type_t));
    if (!*types) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate record type list");
    }
    (*types)[0] = NEOC_RECORD_TYPE_A;
    (*types)[1] = NEOC_RECORD_TYPE_CNAME;
    (*types)[2] = NEOC_RECORD_TYPE_TXT;
    (*types)[3] = NEOC_RECORD_TYPE_AAAA;
    *count = 4;
    return NEOC_SUCCESS;
}
