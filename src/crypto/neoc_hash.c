/**
 * @file neoc_hash.c
 * @brief Implementation of cryptographic hash functions using OpenSSL
 */

#include "neoc/crypto/neoc_hash.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <string.h>

/* Track initialization state */
static int crypto_initialized = 0;

neoc_error_t neoc_crypto_init(void) {
    if (crypto_initialized) {
        return NEOC_SUCCESS;
    }
    
    /* Initialize OpenSSL */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    
    crypto_initialized = 1;
    return NEOC_SUCCESS;
}

void neoc_crypto_cleanup(void) {
    if (!crypto_initialized) {
        return;
    }
    
    /* Cleanup OpenSSL */
    EVP_cleanup();
    ERR_free_strings();
    
    crypto_initialized = 0;
}

bool neoc_crypto_is_initialized(void) {
    return crypto_initialized != 0;
}

neoc_error_t neoc_sha256(const uint8_t* data, size_t data_length, uint8_t digest[NEOC_SHA256_DIGEST_LENGTH]) {
    if (!data || !digest) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!crypto_initialized) {
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    SHA256_CTX ctx;
    if (!SHA256_Init(&ctx)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    if (!SHA256_Update(&ctx, data, data_length)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    if (!SHA256_Final(digest, &ctx)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_sha256_double(const uint8_t* data, size_t data_length, uint8_t digest[NEOC_SHA256_DIGEST_LENGTH]) {
    if (!data || !digest) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    uint8_t first_hash[NEOC_SHA256_DIGEST_LENGTH];
    
    /* First SHA-256 */
    neoc_error_t result = neoc_sha256(data, data_length, first_hash);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    /* Second SHA-256 */
    result = neoc_sha256(first_hash, NEOC_SHA256_DIGEST_LENGTH, digest);
    
    /* Clear intermediate result */
    memset(first_hash, 0, sizeof(first_hash));
    
    return result;
}

neoc_error_t neoc_ripemd160(const uint8_t* data, size_t data_length, uint8_t digest[NEOC_RIPEMD160_DIGEST_LENGTH]) {
    if (!data || !digest) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!crypto_initialized) {
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    RIPEMD160_CTX ctx;
    if (!RIPEMD160_Init(&ctx)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    if (!RIPEMD160_Update(&ctx, data, data_length)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    if (!RIPEMD160_Final(digest, &ctx)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160(const uint8_t* data, size_t data_length, uint8_t digest[NEOC_RIPEMD160_DIGEST_LENGTH]) {
    if (!data || !digest) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    uint8_t sha256_hash[NEOC_SHA256_DIGEST_LENGTH];
    
    /* First apply SHA-256 */
    neoc_error_t result = neoc_sha256(data, data_length, sha256_hash);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    /* Then apply RIPEMD-160 */
    result = neoc_ripemd160(sha256_hash, NEOC_SHA256_DIGEST_LENGTH, digest);
    
    /* Clear intermediate result */
    memset(sha256_hash, 0, sizeof(sha256_hash));
    
    return result;
}

neoc_error_t neoc_hash256(const uint8_t* data, size_t data_length, uint8_t digest[NEOC_SHA256_DIGEST_LENGTH]) {
    /* Hash256 is just SHA-256 for Neo */
    return neoc_sha256(data, data_length, digest);
}

neoc_error_t neoc_hmac_sha256(const uint8_t* key, size_t key_length,
                             const uint8_t* data, size_t data_length,
                             uint8_t digest[NEOC_SHA256_DIGEST_LENGTH]) {
    if (!key || !data || !digest) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!crypto_initialized) {
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    unsigned int digest_len = NEOC_SHA256_DIGEST_LENGTH;
    
    if (!HMAC(EVP_sha256(), key, (int)key_length, data, data_length, digest, &digest_len)) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    if (digest_len != NEOC_SHA256_DIGEST_LENGTH) {
        return NEOC_ERROR_CRYPTO_HASH;
    }
    
    return NEOC_SUCCESS;
}
