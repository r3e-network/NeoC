/**
 * @file scrypt_params.h
 * @brief Scrypt key derivation parameters
 */

#ifndef NEOC_SCRYPT_PARAMS_H
#define NEOC_SCRYPT_PARAMS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Scrypt parameters structure
 */
typedef struct neoc_scrypt_params {
    uint32_t n;         // CPU/memory cost parameter
    uint32_t r;         // Block size parameter
    uint32_t p;         // Parallelization parameter
    uint32_t dk_len;    // Derived key length
} neoc_scrypt_params_t;

// Standard parameter sets
extern const neoc_scrypt_params_t NEOC_SCRYPT_PARAMS_DEFAULT;
extern const neoc_scrypt_params_t NEOC_SCRYPT_PARAMS_LIGHT;
extern const neoc_scrypt_params_t NEOC_SCRYPT_PARAMS_STANDARD;

/**
 * Create Scrypt parameters
 * @param n CPU/memory cost (must be power of 2)
 * @param r Block size
 * @param p Parallelization factor
 * @param dk_len Derived key length
 * @param params Output parameters
 * @return Error code
 */
neoc_error_t neoc_scrypt_params_create(uint32_t n,
                                        uint32_t r,
                                        uint32_t p,
                                        uint32_t dk_len,
                                        neoc_scrypt_params_t **params);

/**
 * Validate Scrypt parameters
 * @param params Parameters to validate
 * @return True if valid
 */
bool neoc_scrypt_params_is_valid(neoc_scrypt_params_t *params);

/**
 * Free Scrypt parameters
 * @param params Parameters to free
 */
void neoc_scrypt_params_free(neoc_scrypt_params_t *params);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SCRYPT_PARAMS_H
