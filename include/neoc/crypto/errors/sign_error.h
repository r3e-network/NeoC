/**
 * @file sign_error.h
 * @brief Header converted from SignError.swift
 */

#ifndef NEOC_SIGN_ERROR_H
#define NEOC_SIGN_ERROR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sign error codes
 */
typedef enum {
    NEOC_SIGN_ERROR_NONE = 0,
    NEOC_SIGN_ERROR_INVALID_KEY,
    NEOC_SIGN_ERROR_INVALID_DATA,
    NEOC_SIGN_ERROR_SIGNATURE_FAILED,
    NEOC_SIGN_ERROR_VERIFICATION_FAILED,
    NEOC_SIGN_ERROR_UNSUPPORTED_ALGORITHM
} neoc_sign_error_t;

/**
 * Get sign error message
 * @param error Error code
 * @return Error message string
 */
const char* neoc_sign_error_message(neoc_sign_error_t error);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SIGN_ERROR_H
