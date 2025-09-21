/**
 * @file nep2_error.h
 * @brief NEP2 (Neo Enhancement Proposal 2) error handling
 * 
 * Provides error handling for NEP2 encrypted private key operations.
 * Based on the Swift NEP2Error implementation.
 */

#ifndef NEOC_CRYPTO_ERRORS_NEP2_ERROR_H
#define NEOC_CRYPTO_ERRORS_NEP2_ERROR_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NEP2 error types
 */
typedef enum {
    NEOC_NEP2_ERROR_INVALID_PASSPHRASE, /**< Invalid passphrase provided */
    NEOC_NEP2_ERROR_INVALID_FORMAT      /**< Invalid NEP2 format */
} neoc_nep2_error_type_t;

/**
 * @brief Create a NEP2 invalid passphrase error
 * @param message The error message
 * @return Error code with message
 */
neoc_error_t neoc_nep2_error_invalid_passphrase(const char *message);

/**
 * @brief Create a NEP2 invalid format error
 * @param message The error message
 * @return Error code with message
 */
neoc_error_t neoc_nep2_error_invalid_format(const char *message);

/**
 * @brief Get error description for NEP2 error type
 * @param error_type The NEP2 error type
 * @param message The specific error message
 * @param description Buffer to store the description
 * @param description_size Size of the description buffer
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_nep2_error_get_description(neoc_nep2_error_type_t error_type, 
                                             const char *message, 
                                             char *description, 
                                             size_t description_size);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_ERRORS_NEP2_ERROR_H */
