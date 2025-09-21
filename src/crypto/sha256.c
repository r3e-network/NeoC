#include "neoc/crypto/sha256.h"
#include <openssl/sha.h>
#include <string.h>

neoc_error_t neoc_sha256(const uint8_t *data, size_t data_len, uint8_t hash[32]) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    SHA256_CTX ctx;
    if (!SHA256_Init(&ctx)) {
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to initialize SHA256");
    }
    
    if (!SHA256_Update(&ctx, data, data_len)) {
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to update SHA256");
    }
    
    if (!SHA256_Final(hash, &ctx)) {
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to finalize SHA256");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_sha256_double(const uint8_t *data, size_t data_len, uint8_t hash[32]) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t first_hash[32];
    
    // First SHA-256
    neoc_error_t err = neoc_sha256(data, data_len, first_hash);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Second SHA-256
    err = neoc_sha256(first_hash, 32, hash);
    
    // Clear intermediate hash
    memset(first_hash, 0, sizeof(first_hash));
    
    return err;
}
