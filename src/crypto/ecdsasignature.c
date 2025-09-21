/**
 * @file ecdsasignature.c
 * @brief ECDSA signature implementation for Neo C SDK
 * 
 * Based on Swift ECDSASignature.swift
 */

#include "neoc/crypto/ecdsasignature.h"
#include "neoc/neo_constants.h"
#include "neoc/neoc_memory.h"
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <string.h>

// SECP256R1 half curve order for canonical signature check
static const uint8_t SECP256R1_HALF_CURVE_ORDER[32] = {
    0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

neoc_error_t neoc_ecdsa_signature_create(const uint8_t *r, const uint8_t *s, 
                                          neoc_ecdsa_signature_t **signature) {
    if (!r || !s || !signature) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *signature = neoc_malloc(sizeof(neoc_ecdsa_signature_t));
    if (!*signature) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signature");
    }
    
    memcpy((*signature)->r, r, 32);
    memcpy((*signature)->s, s, 32);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ecdsa_signature_from_der(const uint8_t *der_bytes, size_t der_len,
                                            neoc_ecdsa_signature_t **signature) {
    if (!der_bytes || der_len == 0 || !signature) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    ECDSA_SIG *sig = d2i_ECDSA_SIG(NULL, &der_bytes, (long)der_len);
    if (!sig) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid DER signature");
    }
    
    const BIGNUM *r_bn, *s_bn;
    ECDSA_SIG_get0(sig, &r_bn, &s_bn);
    
    *signature = neoc_malloc(sizeof(neoc_ecdsa_signature_t));
    if (!*signature) {
        ECDSA_SIG_free(sig);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signature");
    }
    
    // Convert bignums to 32-byte arrays
    memset((*signature)->r, 0, 32);
    memset((*signature)->s, 0, 32);
    
    int r_len = BN_num_bytes(r_bn);
    int s_len = BN_num_bytes(s_bn);
    
    if (r_len <= 32 && s_len <= 32) {
        BN_bn2bin(r_bn, (*signature)->r + (32 - r_len));
        BN_bn2bin(s_bn, (*signature)->s + (32 - s_len));
    } else {
        ECDSA_SIG_free(sig);
        neoc_free(*signature);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid signature component size");
    }
    
    ECDSA_SIG_free(sig);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ecdsa_signature_get_r(const neoc_ecdsa_signature_t *signature, uint8_t *r) {
    if (!signature || !r) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(r, signature->r, 32);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ecdsa_signature_get_s(const neoc_ecdsa_signature_t *signature, uint8_t *s) {
    if (!signature || !s) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(s, signature->s, 32);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ecdsa_signature_is_canonical(const neoc_ecdsa_signature_t *signature, 
                                                bool *is_canonical) {
    if (!signature || !is_canonical) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Compare S component with half curve order
    int cmp = memcmp(signature->s, SECP256R1_HALF_CURVE_ORDER, 32);
    *is_canonical = (cmp <= 0);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ecdsa_signature_to_der(const neoc_ecdsa_signature_t *signature,
                                          uint8_t **der_bytes, size_t *der_len) {
    if (!signature || !der_bytes || !der_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    ECDSA_SIG *sig = ECDSA_SIG_new();
    if (!sig) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create ECDSA_SIG");
    }
    
    BIGNUM *r_bn = BN_bin2bn(signature->r, 32, NULL);
    BIGNUM *s_bn = BN_bin2bn(signature->s, 32, NULL);
    
    if (!r_bn || !s_bn) {
        ECDSA_SIG_free(sig);
        BN_free(r_bn);
        BN_free(s_bn);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create bignums");
    }
    
    ECDSA_SIG_set0(sig, r_bn, s_bn);
    
    int len = i2d_ECDSA_SIG(sig, NULL);
    if (len <= 0) {
        ECDSA_SIG_free(sig);
        return neoc_error_set(NEOC_ERROR_INTERNAL, "Failed to encode signature");
    }
    
    *der_bytes = neoc_malloc(len);
    if (!*der_bytes) {
        ECDSA_SIG_free(sig);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate DER buffer");
    }
    
    uint8_t *ptr = *der_bytes;
    int actual_len = i2d_ECDSA_SIG(sig, &ptr);
    
    if (actual_len != len) {
        neoc_free(*der_bytes);
        ECDSA_SIG_free(sig);
        return neoc_error_set(NEOC_ERROR_INTERNAL, "DER encoding length mismatch");
    }
    
    *der_len = (size_t)len;
    ECDSA_SIG_free(sig);
    return NEOC_SUCCESS;
}

void neoc_ecdsa_signature_free(neoc_ecdsa_signature_t *signature) {
    if (signature) {
        // Clear sensitive data
        memset(signature, 0, sizeof(neoc_ecdsa_signature_t));
        neoc_free(signature);
    }
}

