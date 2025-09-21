/**
 * @file call_flags.c
 * @brief Neo contract call flags implementation
 */

#include "neoc/types/call_flags.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>

uint8_t neoc_call_flags_to_byte(neoc_call_flags_t flags) {
    return (uint8_t)flags;
}

neoc_error_t neoc_call_flags_from_byte(uint8_t value, neoc_call_flags_t *flags) {
    if (!flags) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Flags pointer is null");
    }
    
    // Validate that the value matches one of the defined flags
    switch (value) {
        case 0x00: *flags = NEOC_CALL_FLAG_NONE; break;
        case 0x01: *flags = NEOC_CALL_FLAG_READ_STATES; break;
        case 0x02: *flags = NEOC_CALL_FLAG_WRITE_STATES; break;
        case 0x04: *flags = NEOC_CALL_FLAG_ALLOW_CALL; break;
        case 0x08: *flags = NEOC_CALL_FLAG_ALLOW_NOTIFY; break;
        case 0x03: *flags = NEOC_CALL_FLAG_STATES; break;
        case 0x05: *flags = NEOC_CALL_FLAG_READ_ONLY; break;
        case 0x0F: *flags = NEOC_CALL_FLAG_ALL; break;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, 
                                 "Invalid call flags byte value");
    }
    
    return NEOC_SUCCESS;
}

char* neoc_call_flags_to_string(neoc_call_flags_t flags) {
    const char* name;
    
    switch (flags) {
        case NEOC_CALL_FLAG_NONE:         name = "none"; break;
        case NEOC_CALL_FLAG_READ_STATES:  name = "readStates"; break;
        case NEOC_CALL_FLAG_WRITE_STATES: name = "writeStates"; break;
        case NEOC_CALL_FLAG_ALLOW_CALL:   name = "allowCall"; break;
        case NEOC_CALL_FLAG_ALLOW_NOTIFY: name = "allowNotify"; break;
        case NEOC_CALL_FLAG_STATES:       name = "states"; break;
        case NEOC_CALL_FLAG_READ_ONLY:    name = "readOnly"; break;
        case NEOC_CALL_FLAG_ALL:          name = "all"; break;
        default:                          name = "unknown"; break;
    }
    
    size_t len = strlen(name) + 1;
    char* result = neoc_malloc(len);
    if (result) {
        strcpy(result, name);
    }
    return result;
}

neoc_error_t neoc_call_flags_from_string(const char *str, neoc_call_flags_t *flags) {
    if (!str || !flags) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (strcmp(str, "none") == 0) {
        *flags = NEOC_CALL_FLAG_NONE;
    } else if (strcmp(str, "readStates") == 0) {
        *flags = NEOC_CALL_FLAG_READ_STATES;
    } else if (strcmp(str, "writeStates") == 0) {
        *flags = NEOC_CALL_FLAG_WRITE_STATES;
    } else if (strcmp(str, "allowCall") == 0) {
        *flags = NEOC_CALL_FLAG_ALLOW_CALL;
    } else if (strcmp(str, "allowNotify") == 0) {
        *flags = NEOC_CALL_FLAG_ALLOW_NOTIFY;
    } else if (strcmp(str, "states") == 0) {
        *flags = NEOC_CALL_FLAG_STATES;
    } else if (strcmp(str, "readOnly") == 0) {
        *flags = NEOC_CALL_FLAG_READ_ONLY;
    } else if (strcmp(str, "all") == 0) {
        *flags = NEOC_CALL_FLAG_ALL;
    } else {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, 
                             "Unknown call flags string");
    }
    
    return NEOC_SUCCESS;
}

bool neoc_call_flags_has_flag(neoc_call_flags_t flags, neoc_call_flags_t flag) {
    return (flags & flag) == flag;
}

neoc_call_flags_t neoc_call_flags_combine(neoc_call_flags_t a, neoc_call_flags_t b) {
    return (neoc_call_flags_t)(a | b);
}
