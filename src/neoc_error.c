/**
 * @file neoc_error.c
 * @brief Error handling implementation
 */

#include "neoc/neoc_error.h"
#include <string.h>
#include <stdio.h>

static __thread neoc_error_t last_error = NEOC_SUCCESS;
static __thread char error_message[256] = {0};

neoc_error_t neoc_error_set(neoc_error_t error, const char *message) {
    last_error = error;
    if (message) {
        strncpy(error_message, message, sizeof(error_message) - 1);
        error_message[sizeof(error_message) - 1] = '\0';
    } else {
        error_message[0] = '\0';
    }
    return error;
}

neoc_error_t neoc_error_get(void) {
    return last_error;
}

const char* neoc_error_message(void) {
    if (error_message[0] != '\0') {
        return error_message;
    }
    
    switch (last_error) {
        case NEOC_SUCCESS:
            return "Success";
        case NEOC_ERROR_MEMORY:
            return "Memory allocation failed";
        case NEOC_ERROR_INVALID_ARGUMENT:
            return "Invalid argument";
        case NEOC_ERROR_INVALID_FORMAT:
            return "Invalid format";
        case NEOC_ERROR_NOT_FOUND:
            return "Not found";
        case NEOC_ERROR_CRYPTO:
            return "Cryptographic error";
        case NEOC_ERROR_IO:
            return "I/O error";
        case NEOC_ERROR_NETWORK:
            return "Network error";
        case NEOC_ERROR_TIMEOUT:
            return "Operation timed out";
        case NEOC_ERROR_NOT_IMPLEMENTED:
            return "Not implemented";
        case NEOC_ERROR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case NEOC_ERROR_INVALID_STATE:
            return "Invalid state";
        case NEOC_ERROR_NOT_SUPPORTED:
            return "Operation not supported";
        default:
            return "Unknown error";
    }
}

void neoc_error_clear(void) {
    last_error = NEOC_SUCCESS;
    error_message[0] = '\0';
}

bool neoc_error_is_success(neoc_error_t error) {
    return error == NEOC_SUCCESS;
}

bool neoc_error_is_failure(neoc_error_t error) {
    return error != NEOC_SUCCESS;
}
