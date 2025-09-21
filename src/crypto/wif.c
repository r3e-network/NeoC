#include "neoc/crypto/wif.h"
#include "neoc/crypto/sha256.h"
#include "neoc/utils/neoc_base58.h"
#include <string.h>
#include <stdlib.h>

#define WIF_VERSION 0x80
#define WIF_COMPRESSED_FLAG 0x01
#define WIF_MAX_LENGTH 64
#define WIF_DECODED_MAX_LENGTH 40

neoc_error_t neoc_private_key_to_wif(const uint8_t *private_key, char **wif) {
    if (!private_key || !wif) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create WIF bytes: version + private_key + compressed_flag + checksum
    uint8_t wif_bytes[38];  // 1 + 32 + 1 + 4
    wif_bytes[0] = WIF_VERSION;
    memcpy(wif_bytes + 1, private_key, 32);
    wif_bytes[33] = WIF_COMPRESSED_FLAG;
    
    // Calculate checksum (double SHA-256)
    uint8_t hash1[32];
    uint8_t hash2[32];
    
    neoc_error_t err = neoc_sha256(wif_bytes, 34, hash1);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    err = neoc_sha256(hash1, 32, hash2);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Append first 4 bytes of hash as checksum
    memcpy(wif_bytes + 34, hash2, 4);
    
    // Allocate buffer for Base58 encoded string
    *wif = malloc(WIF_MAX_LENGTH);
    if (!*wif) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate WIF string");
    }
    
    // Encode to Base58
    err = neoc_base58_encode(wif_bytes, 38, *wif, WIF_MAX_LENGTH);
    if (err != NEOC_SUCCESS) {
        free(*wif);
        *wif = NULL;
    }
    
    // Clear sensitive data
    memset(wif_bytes, 0, sizeof(wif_bytes));
    memset(hash1, 0, sizeof(hash1));
    memset(hash2, 0, sizeof(hash2));
    
    return err;
}

neoc_error_t neoc_wif_to_private_key(const char *wif, uint8_t **private_key) {
    if (!wif || !private_key) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Decode from Base58
    uint8_t decoded[64];  // Max WIF decoded size
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base58_decode(wif, decoded, sizeof(decoded), &decoded_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // WIF for NEO should be 38 bytes (compressed only, no uncompressed support)
    // Format: version(1) + private_key(32) + compressed_flag(1) + checksum(4) = 38
    if (decoded_len != 38) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid WIF length - NEO only supports compressed keys");
    }
    
    // Check version byte
    if (decoded[0] != WIF_VERSION) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid WIF version");
    }
    
    // Check compression flag for NEO (must be compressed)
    if (decoded[33] != WIF_COMPRESSED_FLAG) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NEO only supports compressed WIF format");
    }
    
    // Verify checksum
    size_t data_len = decoded_len - 4;  // Exclude checksum
    uint8_t hash1[32];
    uint8_t hash2[32];
    
    err = neoc_sha256(decoded, data_len, hash1);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    err = neoc_sha256(hash1, 32, hash2);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Compare checksum
    if (memcmp(decoded + data_len, hash2, 4) != 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid WIF checksum");
    }
    
    // Allocate private key
    *private_key = malloc(32);
    if (!*private_key) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate private key");
    }
    
    // Copy private key (skip version byte and compression flag if present)
    memcpy(*private_key, decoded + 1, 32);
    
    // Clear sensitive data
    memset(decoded, 0, sizeof(decoded));
    memset(hash1, 0, sizeof(hash1));
    memset(hash2, 0, sizeof(hash2));
    
    return NEOC_SUCCESS;
}

bool neoc_wif_is_valid(const char *wif) {
    if (!wif) {
        return false;
    }
    
    uint8_t *private_key = NULL;
    neoc_error_t err = neoc_wif_to_private_key(wif, &private_key);
    
    if (err == NEOC_SUCCESS && private_key) {
        // Clear and free private key
        memset(private_key, 0, 32);
        free(private_key);
        return true;
    }
    
    return false;
}
