#ifndef NEOC_WIF_H
#define NEOC_WIF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert private key to WIF (Wallet Import Format)
 * 
 * @param private_key 32-byte private key
 * @param wif Output WIF string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_private_key_to_wif(const uint8_t *private_key, char **wif);

/**
 * @brief Convert WIF to private key
 * 
 * @param wif WIF string
 * @param private_key Output 32-byte private key (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wif_to_private_key(const char *wif, uint8_t **private_key);

/**
 * @brief Validate a WIF string
 * 
 * @param wif WIF string to validate
 * @return true if valid, false otherwise
 */
bool neoc_wif_is_valid(const char *wif);

#ifdef __cplusplus
}
#endif

#endif // NEOC_WIF_H
