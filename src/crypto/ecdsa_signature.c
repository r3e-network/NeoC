#include "neoc/crypto/ecdsa_signature.h"
#include "neoc/neoc_error.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>

// secp256r1 half curve order for canonical signatures
static const uint8_t SECP256R1_HALF_CURVE_ORDER[32] = {
    0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x5D, 0x57, 0x6E, 0x73, 0x57, 0xA4, 0x50, 0x1D,
    0xDF, 0xE9, 0x2F, 0x46, 0x68, 0x10, 0x00, 0x00
};

neoc_error_t neoc_ecdsa_signature_create(const uint8_t *r, const uint8_t *s,
                                          neoc_ecdsa_signature_t **signature) {
    if (!r || !s || !signature) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *signature = calloc(1, sizeof(neoc_ecdsa_signature_t));
    if (!*signature) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signature");
    }
    
    memcpy((*signature)->r, r, 32);
    memcpy((*signature)->s, s, 32);
    (*signature)->v = 0;  // Will be set when creating recoverable signature
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_ecdsa_signature_from_bytes(const uint8_t *bytes,
                                               neoc_ecdsa_signature_t **signature) {
    if (!bytes || !signature) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_ecdsa_signature_create(bytes, bytes + 32, signature);
}

neoc_error_t neoc_ecdsa_signature_to_bytes(const neoc_ecdsa_signature_t *signature,
                                             uint8_t **bytes) {
    if (!signature || !bytes) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *bytes = malloc(64);
    if (!*bytes) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate memory");
    }
    
    memcpy(*bytes, signature->r, 32);
    memcpy(*bytes + 32, signature->s, 32);
    
    return NEOC_SUCCESS;
}

bool neoc_ecdsa_signature_is_canonical(const neoc_ecdsa_signature_t *signature) {
    if (!signature) {
        return false;
    }
    
    // Compare S component with half curve order
    for (int i = 0; i < 32; i++) {
        if (signature->s[i] < SECP256R1_HALF_CURVE_ORDER[i]) {
            return true;  // S is less than half curve order
        }
        if (signature->s[i] > SECP256R1_HALF_CURVE_ORDER[i]) {
            return false;  // S is greater than half curve order
        }
    }
    
    return true;  // S equals half curve order (edge case)
}

void neoc_ecdsa_signature_free(neoc_ecdsa_signature_t *signature) {
    if (signature) {
        // Clear sensitive data
        memset(signature->r, 0, 32);
        memset(signature->s, 0, 32);
        free(signature);
    }
}

neoc_error_t neoc_signature_data_create(uint8_t v, const uint8_t *r, const uint8_t *s,
                                         neoc_signature_data_t **sig_data) {
    if (!r || !s || !sig_data) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Validate recovery ID range
    if (v < 27 || v > 34) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid recovery ID");
    }
    
    *sig_data = calloc(1, sizeof(neoc_signature_data_t));
    if (!*sig_data) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signature data");
    }
    
    (*sig_data)->v = v;
    memcpy((*sig_data)->r, r, 32);
    memcpy((*sig_data)->s, s, 32);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_signature_data_from_bytes(uint8_t v, const uint8_t *signature,
                                              neoc_signature_data_t **sig_data) {
    if (!signature || !sig_data) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_signature_data_create(v, signature, signature + 32, sig_data);
}

neoc_error_t neoc_signature_data_to_bytes(const neoc_signature_data_t *sig_data,
                                           uint8_t **bytes) {
    if (!sig_data || !bytes) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *bytes = malloc(64);
    if (!*bytes) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate memory");
    }
    
    memcpy(*bytes, sig_data->r, 32);
    memcpy(*bytes + 32, sig_data->s, 32);
    
    return NEOC_SUCCESS;
}

void neoc_signature_data_free(neoc_signature_data_t *sig_data) {
    if (sig_data) {
        // Clear sensitive data
        memset(sig_data->r, 0, 32);
        memset(sig_data->s, 0, 32);
        sig_data->v = 0;
        free(sig_data);
    }
}
