/**
 * @file n_e_p2_error.c
 * @brief Error handling for n_e_p2
 */

#include "neoc/crypto/errors/n_e_p2_error.h"
#include <string.h>
#include <stdio.h>

static const char* n_e_p2_error_messages[] = {
    "No error",
    "Invalid argument",
    "Memory allocation failed",
    "Invalid state",
    "Operation failed",
    "Not found",
    "Invalid format",
    "Timeout"
};

const char* neoc_n_e_p2_error_string(int error_code) {
    if (error_code < 0 || (size_t)error_code >= sizeof(n_e_p2_error_messages)/sizeof(char*)) {
        return "Unknown error";
    }
    return n_e_p2_error_messages[error_code];
}

void neoc_n_e_p2_error_clear(void) {
    // Clear any error state
}
