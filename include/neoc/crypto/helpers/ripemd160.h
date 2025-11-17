/**
 * @file ripemd160.h
 * @brief RIPEMD-160 hashing helper (pure C implementation)
 */

#ifndef NEOC_CRYPTO_HELPERS_RIPEMD160_H
#define NEOC_CRYPTO_HELPERS_RIPEMD160_H

#include <stdint.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute RIPEMD-160 digest of input data.
 *
 * @param data Input bytes
 * @param len Length of input
 * @param hash Output 20-byte digest
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ripemd160_hash(const uint8_t *data, size_t len, uint8_t hash[20]);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_HELPERS_RIPEMD160_H */
