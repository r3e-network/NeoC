/**
 * @file ripemd160.h
 * @brief RIPEMD160 hash algorithm
 * 
 * Converted from Swift source: crypto/helpers/RIPEMD160.swift
 */

#ifndef NEOC_CRYPTO_HELPERS_RIPEMD160_H
#define NEOC_CRYPTO_HELPERS_RIPEMD160_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate RIPEMD160 hash
 * @param data Input data to hash
 * @param len Length of input data
 * @param hash Output buffer for 20-byte hash
 * @return Error code
 */
neoc_error_t neoc_ripemd160_hash(const uint8_t *data, size_t len, uint8_t hash[20]);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_HELPERS_RIPEMD160_H */
