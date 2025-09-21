/**
 * @file ecdsasignature.h
 * @brief ECDSA signature implementation for Neo C SDK
 * 
 * Based on Swift ECDSASignature.swift
 */

#ifndef NEOC_CRYPTO_ECDSASIGNATURE_H
#define NEOC_CRYPTO_ECDSASIGNATURE_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECDSA signature structure
 */
typedef struct {
    uint8_t r[32];  /**< R component (big endian) */
    uint8_t s[32];  /**< S component (big endian) */
} neoc_ecdsa_signature_t;

/**
 * @brief Create an ECDSA signature from R and S components
 * 
 * @param r R component (32 bytes, big endian)
 * @param s S component (32 bytes, big endian)
 * @param signature Output signature (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_create(const uint8_t *r, const uint8_t *s, 
                                          neoc_ecdsa_signature_t **signature);

/**
 * @brief Create an ECDSA signature from DER-encoded bytes
 * 
 * @param der_bytes DER-encoded signature bytes
 * @param der_len Length of DER bytes
 * @param signature Output signature (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_from_der(const uint8_t *der_bytes, size_t der_len,
                                            neoc_ecdsa_signature_t **signature);

/**
 * @brief Get the R component of the signature
 * 
 * @param signature The signature
 * @param r Output R component (32 bytes, caller must provide buffer)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_get_r(const neoc_ecdsa_signature_t *signature, uint8_t *r);

/**
 * @brief Get the S component of the signature
 * 
 * @param signature The signature
 * @param s Output S component (32 bytes, caller must provide buffer)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_get_s(const neoc_ecdsa_signature_t *signature, uint8_t *s);

/**
 * @brief Check if the signature is canonical (S component is "low")
 * 
 * A canonical signature has S <= SECP256R1_HALF_CURVE_ORDER
 * 
 * @param signature The signature
 * @param is_canonical Output canonical status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_is_canonical(const neoc_ecdsa_signature_t *signature, 
                                                bool *is_canonical);

/**
 * @brief Convert signature to DER encoding
 * 
 * @param signature The signature
 * @param der_bytes Output DER bytes (caller must free)
 * @param der_len Output DER length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ecdsa_signature_to_der(const neoc_ecdsa_signature_t *signature,
                                          uint8_t **der_bytes, size_t *der_len);

/**
 * @brief Free an ECDSA signature
 * 
 * @param signature The signature to free
 */
void neoc_ecdsa_signature_free(neoc_ecdsa_signature_t *signature);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_ECDSASIGNATURE_H */
