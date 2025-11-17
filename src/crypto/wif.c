#include "neoc/crypto/wif.h"
#include "neoc/crypto/sha256.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/neoc_memory.h"
#include <string.h>

#define WIF_VERSION 0x80
#define WIF_COMPRESSED_FLAG 0x01
#define WIF_MAX_LENGTH 64
#define WIF_DECODED_MAX_LENGTH 40

neoc_error_t neoc_private_key_to_wif_len(const uint8_t *private_key,
                                         size_t key_len,
                                         char **wif) {
    if (!private_key || !wif) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    if (key_len != 32) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Private key must be 32 bytes");
    }

    uint8_t payload[34];  // version + key + compressed flag
    payload[0] = WIF_VERSION;
    memcpy(payload + 1, private_key, 32);
    payload[33] = WIF_COMPRESSED_FLAG;

    char *buffer = neoc_malloc(WIF_MAX_LENGTH);
    if (!buffer) {
        memset(payload, 0, sizeof(payload));
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate WIF string");
    }

    neoc_error_t err = neoc_base58_check_encode(payload, sizeof(payload), buffer, WIF_MAX_LENGTH);
    memset(payload, 0, sizeof(payload));
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return err;
    }

    *wif = buffer;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_private_key_to_wif(const uint8_t *private_key, char **wif) {
    return neoc_private_key_to_wif_len(private_key, 32, wif);
}

neoc_error_t neoc_wif_to_private_key(const char *wif, uint8_t **private_key) {
    if (!wif || !private_key) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    uint8_t decoded[WIF_DECODED_MAX_LENGTH] = {0};
    size_t decoded_len = 0;
    neoc_error_t err = neoc_base58_check_decode(wif, decoded, sizeof(decoded), &decoded_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    // After removing checksum, payload should be 34 bytes: version + key + compressed flag
    if (decoded_len != 34) {
        memset(decoded, 0, sizeof(decoded));
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid WIF length - expected compressed key payload");
    }

    if (decoded[0] != WIF_VERSION) {
        memset(decoded, 0, sizeof(decoded));
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid WIF version");
    }

    if (decoded[33] != WIF_COMPRESSED_FLAG) {
        memset(decoded, 0, sizeof(decoded));
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NEO only supports compressed WIF format");
    }

    uint8_t *key = neoc_malloc(32);
    if (!key) {
        memset(decoded, 0, sizeof(decoded));
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate private key");
    }

    memcpy(key, decoded + 1, 32);
    memset(decoded, 0, sizeof(decoded));

    *private_key = key;
    return NEOC_SUCCESS;
}

bool neoc_wif_is_valid(const char *wif) {
    if (!wif) {
        return false;
    }
    
    uint8_t *private_key = NULL;
    neoc_error_t err = neoc_wif_to_private_key(wif, &private_key);
    
    if (err == NEOC_SUCCESS && private_key) {
        memset(private_key, 0, 32);
        neoc_free(private_key);
        return true;
    }
    
    return false;
}
