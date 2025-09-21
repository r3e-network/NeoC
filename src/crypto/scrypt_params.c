/**
 * @file scrypt_params.c
 * @brief Scrypt parameters implementation
 */

#include "neoc/crypto/scrypt_params.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <openssl/evp.h>

// Default NEP-2 parameters
#define DEFAULT_N 16384
#define DEFAULT_R 8
#define DEFAULT_P 8

// Standard parameter sets
const neoc_scrypt_params_t NEOC_SCRYPT_PARAMS_DEFAULT = {
    .n = 16384,
    .r = 8,
    .p = 8,
    .dk_len = 64
};

const neoc_scrypt_params_t NEOC_SCRYPT_PARAMS_LIGHT = {
    .n = 4096,
    .r = 8,
    .p = 2,
    .dk_len = 32
};

const neoc_scrypt_params_t NEOC_SCRYPT_PARAMS_STANDARD = {
    .n = 32768,
    .r = 8,
    .p = 16,
    .dk_len = 64
};

neoc_error_t neoc_scrypt_params_create(uint32_t n,
                                        uint32_t r,
                                        uint32_t p,
                                        uint32_t dk_len,
                                        neoc_scrypt_params_t **params) {
    if (!params) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid params pointer");
    }
    
    // Validate parameters
    if (n == 0 || (n & (n - 1)) != 0) {  // n must be power of 2
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "N must be power of 2");
    }
    
    if (r == 0 || p == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "R and P must be non-zero");
    }
    
    *params = neoc_calloc(1, sizeof(neoc_scrypt_params_t));
    if (!*params) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate params");
    }
    
    (*params)->n = n;
    (*params)->r = r;
    (*params)->p = p;
    (*params)->dk_len = dk_len;
    
    return NEOC_SUCCESS;
}

bool neoc_scrypt_params_is_valid(neoc_scrypt_params_t *params) {
    if (!params) {
        return false;
    }
    
    // Check n is power of 2
    if (params->n == 0 || (params->n & (params->n - 1)) != 0) {
        return false;
    }
    
    // Check other parameters are non-zero
    if (params->r == 0 || params->p == 0 || params->dk_len == 0) {
        return false;
    }
    
    return true;
}


void neoc_scrypt_params_free(neoc_scrypt_params_t *params) {
    if (!params) return;
    neoc_free(params);
}
