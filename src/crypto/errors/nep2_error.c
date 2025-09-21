/**
 * @file nep2_error.c
 * @brief NEP2 (Neo Enhancement Proposal 2) error handling implementation
 * 
 * This implementation provides error handling for NEP2 encrypted private key operations
 * based on the Swift NEP2Error source file.
 */

#include "neoc/neoc.h"
#include "neoc/crypto/errors/nep2_error.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Create a NEP2 invalid passphrase error
 * @param message The error message
 * @return Error code with message
 */
neoc_error_t neoc_nep2_error_invalid_passphrase(const char *message) {
    return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, 
                         message ? message : "Invalid NEP2 passphrase");
}

/**
 * @brief Create a NEP2 invalid format error
 * @param message The error message
 * @return Error code with message
 */
neoc_error_t neoc_nep2_error_invalid_format(const char *message) {
    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, 
                         message ? message : "Invalid NEP2 format");
}

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
                                             size_t description_size) {
    if (!description || description_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid description buffer");
    }
    
    const char *base_message = "";
    switch (error_type) {
        case NEOC_NEP2_ERROR_INVALID_PASSPHRASE:
            base_message = "Invalid NEP2 passphrase";
            break;
        case NEOC_NEP2_ERROR_INVALID_FORMAT:
            base_message = "Invalid NEP2 format";
            break;
        default:
            base_message = "Unknown NEP2 error";
            break;
    }
    
    if (message && strlen(message) > 0) {
        snprintf(description, description_size, "%s: %s", base_message, message);
    } else {
        snprintf(description, description_size, "%s", base_message);
    }
    
    return NEOC_SUCCESS;
}
