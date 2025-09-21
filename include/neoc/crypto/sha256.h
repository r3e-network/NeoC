#ifndef NEOC_SHA256_H
#define NEOC_SHA256_H

#include <stdint.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate SHA-256 hash of data
 * 
 * @param data Input data
 * @param data_len Length of input data
 * @param hash Output hash (32 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_sha256(const uint8_t *data, size_t data_len, uint8_t hash[32]);

/**
 * @brief Calculate double SHA-256 hash of data
 * 
 * @param data Input data
 * @param data_len Length of input data
 * @param hash Output hash (32 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_sha256_double(const uint8_t *data, size_t data_len, uint8_t hash[32]);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SHA256_H
