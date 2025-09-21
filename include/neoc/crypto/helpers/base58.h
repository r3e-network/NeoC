/**
 * @file base58.h
 * @brief Base58 encoding/decoding
 */

#ifndef NEOC_BASE58_H
#define NEOC_BASE58_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode data to Base58
 */
neoc_error_t neoc_base58_encode(const uint8_t *data, size_t len, char **encoded);

/**
 * Decode Base58 string
 */
neoc_error_t neoc_base58_decode(const char *encoded, uint8_t **data, size_t *len);

/**
 * Encode with checksum (Base58Check)
 */
neoc_error_t neoc_base58_encode_check(const uint8_t *data, size_t len, char **encoded);

/**
 * Decode with checksum verification
 */
neoc_error_t neoc_base58_decode_check(const char *encoded, uint8_t **data, size_t *len);

/**
 * Validate Base58 string
 */
bool neoc_base58_is_valid(const char *str);

#ifdef __cplusplus
}
#endif

#endif // NEOC_BASE58_H
