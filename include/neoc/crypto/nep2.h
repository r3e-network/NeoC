/**
 * @file nep2.h
 * @brief NEP-2 password-protected private key encryption
 */

#ifndef NEOC_NEP2_H
#define NEOC_NEP2_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NEP-2 encryption parameters
 */
typedef struct {
    uint32_t n;         // CPU/memory cost parameter (default: 16384)
    uint32_t r;         // Block size parameter (default: 8)
    uint32_t p;         // Parallelization parameter (default: 8)
} neoc_nep2_params_t;

/**
 * @brief Default NEP-2 scrypt parameters
 */
extern const neoc_nep2_params_t NEOC_NEP2_DEFAULT_PARAMS;

/**
 * @brief Light NEP-2 scrypt parameters (faster but less secure)
 */
extern const neoc_nep2_params_t NEOC_NEP2_LIGHT_PARAMS;

/**
 * @brief Encrypt a private key using NEP-2
 * 
 * @param private_key The 32-byte private key to encrypt
 * @param password The password to use for encryption
 * @param params Scrypt parameters (use NULL for defaults)
 * @param encrypted_key Output buffer for encrypted key (must be at least 58 bytes)
 * @param encrypted_key_len Size of output buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep2_encrypt(const uint8_t *private_key,
                                const char *password,
                                const neoc_nep2_params_t *params,
                                char *encrypted_key,
                                size_t encrypted_key_len);

/**
 * @brief Decrypt a NEP-2 encrypted private key
 * 
 * @param encrypted_key The NEP-2 encrypted key string
 * @param password The password to decrypt with
 * @param params Scrypt parameters (use NULL for defaults)
 * @param private_key Output buffer for 32-byte private key
 * @param private_key_len Size of output buffer (must be at least 32)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nep2_decrypt(const char *encrypted_key,
                                const char *password,
                                const neoc_nep2_params_t *params,
                                uint8_t *private_key,
                                size_t private_key_len);

/**
 * @brief Verify if a password is correct for an encrypted key
 * 
 * @param encrypted_key The NEP-2 encrypted key string
 * @param password The password to verify
 * @param params Scrypt parameters (use NULL for defaults)
 * @return true if password is correct, false otherwise
 */
bool neoc_nep2_verify_password(const char *encrypted_key,
                                const char *password,
                                const neoc_nep2_params_t *params);

/**
 * @brief Check if a string is a valid NEP-2 encrypted key
 * 
 * @param encrypted_key The string to check
 * @return true if valid NEP-2 format, false otherwise
 */
bool neoc_nep2_is_valid_format(const char *encrypted_key);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEP2_H
