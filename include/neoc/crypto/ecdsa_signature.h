#ifndef NEOC_ECDSA_SIGNATURE_H
#define NEOC_ECDSA_SIGNATURE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECDSA Signature structure
 */
typedef struct neoc_ecdsa_signature_t {
    uint8_t r[32];  // R component (32 bytes)
    uint8_t s[32];  // S component (32 bytes)
    uint8_t v;      // Recovery ID (for recoverable signatures)
} neoc_ecdsa_signature_t;

/**
 * @brief Signature data structure (with recovery ID)
 */
typedef struct {
    uint8_t v;      // Recovery ID (27-34)
    uint8_t r[32];  // R component
    uint8_t s[32];  // S component
} neoc_signature_data_t;

/**
 * @brief Create a new ECDSA signature
 * 
 * @param r R component (32 bytes)
 * @param s S component (32 bytes)
 * @param signature Output signature (caller must free with neoc_ecdsa_signature_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_create(const uint8_t *r, const uint8_t *s,
                                          neoc_ecdsa_signature_t **signature);

/**
 * @brief Create signature from concatenated bytes
 * 
 * @param bytes 64-byte array (32 bytes R + 32 bytes S)
 * @param signature Output signature (caller must free with neoc_ecdsa_signature_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_from_bytes(const uint8_t *bytes,
                                               neoc_ecdsa_signature_t **signature);

/**
 * @brief Get concatenated signature bytes
 * 
 * @param signature The signature
 * @param bytes Output 64-byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_to_bytes(const neoc_ecdsa_signature_t *signature,
                                             uint8_t **bytes);

/**
 * @brief Check if signature is canonical (S component is low)
 * 
 * @param signature The signature
 * @return true if canonical, false otherwise
 */
bool neoc_ecdsa_signature_is_canonical(const neoc_ecdsa_signature_t *signature);

/**
 * @brief Free an ECDSA signature
 * 
 * @param signature The signature to free
 */
void neoc_ecdsa_signature_free(neoc_ecdsa_signature_t *signature);

/**
 * @brief Create signature data with recovery ID
 * 
 * @param v Recovery ID (27-34)
 * @param r R component (32 bytes)
 * @param s S component (32 bytes)
 * @param sig_data Output signature data (caller must free with neoc_signature_data_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signature_data_create(uint8_t v, const uint8_t *r, const uint8_t *s,
                                         neoc_signature_data_t **sig_data);

/**
 * @brief Create signature data from concatenated bytes
 * 
 * @param v Recovery ID
 * @param signature 64-byte signature (R + S)
 * @param sig_data Output signature data (caller must free with neoc_signature_data_free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signature_data_from_bytes(uint8_t v, const uint8_t *signature,
                                              neoc_signature_data_t **sig_data);

/**
 * @brief Get concatenated signature bytes from signature data
 * 
 * @param sig_data The signature data
 * @param bytes Output 64-byte array (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_signature_data_to_bytes(const neoc_signature_data_t *sig_data,
                                           uint8_t **bytes);

/**
 * @brief Free signature data
 * 
 * @param sig_data The signature data to free
 */
void neoc_signature_data_free(neoc_signature_data_t *sig_data);

#ifdef __cplusplus
}
#endif

#endif // NEOC_ECDSA_SIGNATURE_H
