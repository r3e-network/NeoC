/**
 * @file hash.c
 * @brief Cryptographic hash implementation
 */

#include "neoc/crypto/hash.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/hmac.h>
#include <string.h>

neoc_error_t neoc_hash_sha256(const uint8_t *data, size_t len, uint8_t *hash) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    SHA256(data, len, hash);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash_sha256_sha256(const uint8_t *data, size_t len, uint8_t *hash) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    uint8_t temp[32];
    SHA256(data, len, temp);
    SHA256(temp, 32, hash);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash_ripemd160(const uint8_t *data, size_t len, uint8_t *hash) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    RIPEMD160(data, len, hash);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash_hash160(const uint8_t *data, size_t len, uint8_t *hash) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters");
    }
    
    uint8_t sha256_hash[32];
    SHA256(data, len, sha256_hash);
    RIPEMD160(sha256_hash, 32, hash);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash_hash256(const uint8_t *data, size_t len, uint8_t *hash) {
    return neoc_hash_sha256_sha256(data, len, hash);
}

uint32_t neoc_hash_murmur32(const uint8_t *data, size_t len, uint32_t seed) {
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;
    
    uint32_t hash = seed;
    const int nblocks = len / 4;
    const uint32_t *blocks = (const uint32_t *)data;
    
    for (int i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        
        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }
    
    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
    uint32_t k1 = 0;
    
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16; /* fall through */
        case 2: k1 ^= tail[1] << 8;  /* fall through */
        case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = (k1 << r1) | (k1 >> (32 - r1));
            k1 *= c2;
            hash ^= k1;
    }
    
    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);
    
    return hash;
}

uint32_t neoc_hash_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    
    return ~crc;
}

neoc_error_t neoc_hash_hmac_sha512(const uint8_t *data, size_t data_len,
                                   const uint8_t *key, size_t key_len,
                                   uint8_t *hash) {
    if (!data || !key || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameters for HMAC SHA-512");
    }
    
    unsigned int len = 64;
    if (!HMAC(EVP_sha512(), key, key_len, data, data_len, hash, &len)) {
        return neoc_error_set(NEOC_ERROR_CRYPTO_HASH, "HMAC SHA-512 computation failed");
    }
    
    if (len != 64) {
        return neoc_error_set(NEOC_ERROR_CRYPTO_HASH, "Unexpected HMAC SHA-512 output length");
    }
    
    return NEOC_SUCCESS;
}

bool neoc_hash_verify(const uint8_t *data, size_t data_len,
                      const uint8_t *hash, size_t hash_len) {
    if (!data || !hash) return false;
    
    uint8_t computed[32];
    
    if (hash_len == 20) {
        neoc_hash_hash160(data, data_len, computed);
        return memcmp(computed, hash, 20) == 0;
    } else if (hash_len == 32) {
        neoc_hash_sha256(data, data_len, computed);
        return memcmp(computed, hash, 32) == 0;
    }
    
    return false;
}
