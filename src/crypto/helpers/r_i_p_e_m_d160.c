/**
 * @file r_i_p_e_m_d160.c
 * @brief RIPEMD160 hash implementation
 */

#include "neoc/crypto/helpers/r_i_p_e_m_d160.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <openssl/ripemd.h>

neoc_error_t neoc_ripemd160(const uint8_t *data,
                             size_t data_len,
                             uint8_t *hash) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!RIPEMD160(data, data_len, hash)) {
        return neoc_error_set(NEOC_ERROR_CRYPTO, "RIPEMD160 hash failed");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ripemd160_string(const char *str, uint8_t *hash) {
    if (!str || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_ripemd160((const uint8_t *)str, strlen(str), hash);
}
