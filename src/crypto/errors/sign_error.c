/**
 * @file sign_error.c
 * @brief Error handling for sign
 */

#include "neoc/crypto/errors/sign_error.h"
#include <string.h>
#include <stdio.h>

static const char* sign_error_messages[] = {
    "No error",
    "Invalid argument",
    "Memory allocation failed",
    "Invalid state",
    "Operation failed",
    "Not found",
    "Invalid format",
    "Timeout"
};

const char* neoc_sign_error_string(int error_code) {
    if (error_code < 0 || (size_t)error_code >= sizeof(sign_error_messages)/sizeof(char*)) {
        return "Unknown error";
    }
    return sign_error_messages[error_code];
}

void neoc_sign_error_clear(void) {
    // Clear any error state
}
