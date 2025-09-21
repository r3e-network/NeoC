/**
 * @file n_e_p2_error.h
 * @brief Header converted from NEP2Error.swift
 */

#ifndef NEOC_N_E_P2_ERROR_H
#define NEOC_N_E_P2_ERROR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NEP-2 error codes
 */
typedef enum {
    NEOC_NEP2_ERROR_NONE = 0,
    NEOC_NEP2_ERROR_INVALID_FORMAT,
    NEOC_NEP2_ERROR_INVALID_PASSWORD,
    NEOC_NEP2_ERROR_DECRYPTION_FAILED,
    NEOC_NEP2_ERROR_ENCRYPTION_FAILED,
    NEOC_NEP2_ERROR_INVALID_KEY,
    NEOC_NEP2_ERROR_CHECKSUM_MISMATCH
} neoc_nep2_error_t;

/**
 * Get NEP-2 error message
 * @param error Error code
 * @return Error message string
 */
const char* neoc_nep2_error_message(neoc_nep2_error_t error);

#ifdef __cplusplus
}
#endif

#endif // NEOC_N_E_P2_ERROR_H
