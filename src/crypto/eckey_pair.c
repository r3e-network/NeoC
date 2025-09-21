/**
 * @file eckey_pair.c
 * @brief EC Key Pair implementation
 * 
 * Based on Swift source: crypto/ECKeyPair.swift
 * Implements SECP256R1 key pair operations for Neo blockchain
 */

#include "../../include/neoc/neoc_error.h"
#include "../../include/neoc/neoc_memory.h"
#include "../../include/neoc/crypto/ec_key_pair.h"
#include "../../include/neoc/crypto/ecdsa_signature.h"
#include "../../include/neoc/crypto/neoc_hash.h"
#include "../../include/neoc/crypto/sha256.h"
#include "../../include/neoc/utils/neoc_base58.h"
#include "../../include/neoc/script/script_builder.h"
#include "../../include/neoc/types/neoc_hash160.h"
#include "../../include/neoc/crypto/wif.h"
#include <string.h>
#include <stdio.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bn.h>

// SECP256R1 constants
#define EC_CURVE_NAME NID_X9_62_prime256v1
#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_COMPRESSED_SIZE 33
#define PUBLIC_KEY_UNCOMPRESSED_SIZE 65

// Additional error codes for crypto operations
#define NEOC_ERROR_CRYPTO_OPERATION NEOC_ERROR_INVALID_PARAM
#define NEOC_ERROR_CRYPTO_INIT NEOC_ERROR_INVALID_PARAM
#define NEOC_ERROR_INVALID_POINT NEOC_ERROR_INVALID_PARAM

/**
 * @brief Get the SECP256R1 EC_GROUP
 */
static EC_GROUP* get_secp256r1_group(void) {
    static EC_GROUP *group = NULL;
    if (!group) {
        group = EC_GROUP_new_by_curve_name(EC_CURVE_NAME);
    }
    return group;
}

/**
 * @brief Create a new EC key pair from a private key
 */
neoc_error_t neoc_ec_key_pair_create_from_private_key(const uint8_t *private_key_bytes, 
                                                       neoc_ec_key_pair_t **key_pair) {
    if (!private_key_bytes || !key_pair) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *key_pair = NULL;
    
    // Create the key pair structure
    neoc_ec_key_pair_t *new_pair = neoc_malloc(sizeof(neoc_ec_key_pair_t));
    if (!new_pair) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize to NULL for cleanup safety
    new_pair->private_key = NULL;
    new_pair->public_key = NULL;
    
    // Create private key structure
    new_pair->private_key = neoc_malloc(sizeof(neoc_ec_private_key_t));
    if (!new_pair->private_key) {
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Copy private key bytes
    memcpy(new_pair->private_key->bytes, private_key_bytes, PRIVATE_KEY_SIZE);
    
    // Create OpenSSL private key
    EC_GROUP *group = get_secp256r1_group();
    if (!group) {
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    new_pair->private_key->ec_key = EC_KEY_new();
    if (!new_pair->private_key->ec_key) {
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EC_KEY_set_group(new_pair->private_key->ec_key, group) != 1) {
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Set private key
    BIGNUM *priv_bn = BN_bin2bn(private_key_bytes, PRIVATE_KEY_SIZE, NULL);
    if (!priv_bn) {
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EC_KEY_set_private_key(new_pair->private_key->ec_key, priv_bn) != 1) {
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Derive public key from private key
    EC_POINT *pub_point = EC_POINT_new(group);
    if (!pub_point) {
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) {
        EC_POINT_free(pub_point);
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EC_POINT_mul(group, pub_point, priv_bn, NULL, NULL, ctx) != 1) {
        BN_CTX_free(ctx);
        EC_POINT_free(pub_point);
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    if (EC_KEY_set_public_key(new_pair->private_key->ec_key, pub_point) != 1) {
        BN_CTX_free(ctx);
        EC_POINT_free(pub_point);
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Create EVP_PKEY
    new_pair->private_key->pkey = EVP_PKEY_new();
    if (!new_pair->private_key->pkey) {
        BN_CTX_free(ctx);
        EC_POINT_free(pub_point);
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EVP_PKEY_set1_EC_KEY(new_pair->private_key->pkey, new_pair->private_key->ec_key) != 1) {
        BN_CTX_free(ctx);
        EC_POINT_free(pub_point);
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Create public key structure
    neoc_error_t error = neoc_ec_public_key_from_private(private_key_bytes, &new_pair->public_key);
    if (error != NEOC_SUCCESS) {
        BN_CTX_free(ctx);
        EC_POINT_free(pub_point);
        BN_free(priv_bn);
        neoc_ec_key_pair_free(new_pair);
        return error;
    }
    
    BN_CTX_free(ctx);
    EC_POINT_free(pub_point);
    BN_free(priv_bn);
    
    *key_pair = new_pair;
    return NEOC_SUCCESS;
}

/**
 * @brief Create a fresh secp256r1 EC key pair
 */
neoc_error_t neoc_ec_key_pair_create_random(neoc_ec_key_pair_t **key_pair) {
    if (!key_pair) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Generate random 32-byte private key
    uint8_t private_key[PRIVATE_KEY_SIZE];
    if (RAND_bytes(private_key, PRIVATE_KEY_SIZE) != 1) {
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    return neoc_ec_key_pair_create_from_private_key(private_key, key_pair);
}

/**
 * @brief Create EC key pair from private key bytes (compatibility alias)
 */
neoc_error_t neoc_ec_key_pair_from_private_key(const uint8_t *private_key_bytes,
                                                size_t key_size,
                                                neoc_ec_key_pair_t **key_pair) {
    if (key_size != PRIVATE_KEY_SIZE) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    return neoc_ec_key_pair_create_from_private_key(private_key_bytes, key_pair);
}

/**
 * @brief Get the NEO address from this key pair's public key
 */
neoc_error_t neoc_ec_key_pair_get_address(const neoc_ec_key_pair_t *key_pair, char **address) {
    if (!key_pair || !address) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *address = NULL;
    
    // Get script hash first
    neoc_hash160_t script_hash;
    neoc_error_t error = neoc_ec_key_pair_get_script_hash(key_pair, &script_hash);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Convert script hash to address
    char buffer[64]; // Temporary buffer for address
    neoc_error_t addr_error = neoc_hash160_to_address(&script_hash, buffer, sizeof(buffer));
    if (addr_error != NEOC_SUCCESS) {
        return addr_error;
    }
    
    // Allocate and copy address string
    size_t addr_len = strlen(buffer);
    *address = neoc_malloc(addr_len + 1);
    if (!*address) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    strcpy(*address, buffer);
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get the script hash from this key pair's public key
 */
neoc_error_t neoc_ec_key_pair_get_script_hash(const neoc_ec_key_pair_t *key_pair, 
                                               neoc_hash160_t *script_hash) {
    if (!key_pair || !script_hash) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Get compressed public key
    uint8_t *encoded = NULL;
    size_t encoded_len = 0;
    neoc_error_t error = neoc_ec_public_key_get_encoded(key_pair->public_key, true, &encoded, &encoded_len);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Build verification script
    uint8_t *script = NULL;
    size_t script_len = 0;
    error = neoc_script_builder_build_verification_script(encoded, encoded_len, &script, &script_len);
    if (error != NEOC_SUCCESS) {
        neoc_free(encoded);
        return error;
    }
    
    // Create script hash
    error = neoc_hash160_from_script(script_hash, script, script_len);
    
    neoc_free(encoded);
    neoc_free(script);
    return error;
}

/**
 * @brief Sign a message hash with the private key
 */
neoc_error_t neoc_ec_key_pair_sign(const neoc_ec_key_pair_t *key_pair,
                                    const uint8_t *message_hash,
                                    neoc_ecdsa_signature_t **signature) {
    if (!key_pair || !message_hash || !signature) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *signature = NULL;
    
    if (!key_pair->private_key || !key_pair->private_key->pkey) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Create signature context
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EVP_DigestSignInit(ctx, NULL, NULL, NULL, key_pair->private_key->pkey) != 1) {
        EVP_MD_CTX_free(ctx);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Sign the hash
    size_t sig_len = 0;
    if (EVP_DigestSign(ctx, NULL, &sig_len, message_hash, 32) != 1) {
        EVP_MD_CTX_free(ctx);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    uint8_t *sig_der = neoc_malloc(sig_len);
    if (!sig_der) {
        EVP_MD_CTX_free(ctx);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EVP_DigestSign(ctx, sig_der, &sig_len, message_hash, 32) != 1) {
        neoc_free(sig_der);
        EVP_MD_CTX_free(ctx);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    EVP_MD_CTX_free(ctx);
    
    // Parse DER signature and create ECDSA signature
    neoc_error_t error = neoc_ecdsa_signature_from_bytes(sig_der, signature);
    neoc_free(sig_der);
    
    return error;
}

/**
 * @brief Export the private key as WIF (Wallet Import Format)
 */
neoc_error_t neoc_ec_key_pair_export_as_wif(const neoc_ec_key_pair_t *key_pair, char **wif) {
    if (!key_pair || !wif || !key_pair->private_key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Export private key as WIF (Wallet Import Format)
    // WIF format: [version][private_key][compressed_flag][checksum]
    uint8_t wif_data[38]; // 1 (version) + 32 (key) + 1 (compressed) + 4 (checksum)
    size_t wif_len = 0;
    
    // Version byte (0x80 for mainnet)
    wif_data[wif_len++] = 0x80;
    
    // Private key (32 bytes)
    uint8_t private_key_bytes[32];
    size_t key_len = 32;
    neoc_error_t err = neoc_ec_key_pair_get_private_key(key_pair, private_key_bytes, &key_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    memcpy(wif_data + wif_len, private_key_bytes, 32);
    wif_len += 32;
    
    // Compressed flag (0x01 for compressed public keys)
    wif_data[wif_len++] = 0x01;
    
    // Calculate checksum (double SHA256)
    uint8_t hash1[32], hash2[32];
    neoc_sha256(wif_data, wif_len, hash1);
    neoc_sha256(hash1, 32, hash2);
    
    // Append first 4 bytes of checksum
    memcpy(wif_data + wif_len, hash2, 4);
    wif_len += 4;
    
    // Base58 encode
    *wif = neoc_malloc(64);
    if (!*wif) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    err = neoc_base58_encode(wif_data, wif_len, *wif, 64);
    if (err != NEOC_SUCCESS) {
        neoc_free(*wif);
        *wif = NULL;
        return err;
    }
    
    // Clear sensitive data
    memset(private_key_bytes, 0, sizeof(private_key_bytes));
    memset(wif_data, 0, sizeof(wif_data));
    
    return NEOC_SUCCESS;
}

/**
 * @brief Import a key pair from WIF (Wallet Import Format)
 */
neoc_error_t neoc_ec_key_pair_import_from_wif(const char *wif, neoc_ec_key_pair_t **key_pair) {
    if (!wif || !key_pair) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Import private key from WIF (Wallet Import Format)
    if (strlen(wif) < 51 || strlen(wif) > 52) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Base58 decode
    uint8_t wif_data[38];
    size_t decoded_len = sizeof(wif_data);
    neoc_error_t err = neoc_base58_decode(wif, wif_data, sizeof(wif_data), &decoded_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Check version byte (0x80 for mainnet)
    if (wif_data[0] != 0x80) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Verify checksum
    size_t data_len = decoded_len - 4;
    uint8_t hash1[32], hash2[32];
    neoc_sha256(wif_data, data_len, hash1);
    neoc_sha256(hash1, 32, hash2);
    
    if (memcmp(wif_data + data_len, hash2, 4) != 0) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Extract private key (32 bytes after version byte)
    uint8_t private_key[32];
    memcpy(private_key, wif_data + 1, 32);
    
    // Create key pair from private key
    err = neoc_ec_key_pair_create_from_private_key(private_key, key_pair);
    
    // Clear sensitive data
    memset(private_key, 0, sizeof(private_key));
    memset(wif_data, 0, sizeof(wif_data));
    
    return err;
}

/**
 * @brief Free an EC key pair
 */
void neoc_ec_key_pair_free(neoc_ec_key_pair_t *key_pair) {
    if (!key_pair) {
        return;
    }
    
    if (key_pair->private_key) {
        neoc_ec_private_key_free(key_pair->private_key);
    }
    
    if (key_pair->public_key) {
        neoc_ec_public_key_free(key_pair->public_key);
    }
    
    neoc_free(key_pair);
}

/**
 * @brief Get public key from private key
 */
neoc_error_t neoc_ec_public_key_from_private(const uint8_t *private_key_bytes,
                                              neoc_ec_public_key_t **public_key) {
    if (!private_key_bytes || !public_key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *public_key = NULL;
    
    // Create public key structure
    neoc_ec_public_key_t *new_key = neoc_malloc(sizeof(neoc_ec_public_key_t));
    if (!new_key) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Get group
    new_key->group = get_secp256r1_group();
    if (!new_key->group) {
        neoc_free(new_key);
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    // Create point
    new_key->point = EC_POINT_new(new_key->group);
    if (!new_key->point) {
        neoc_free(new_key);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Convert private key to BIGNUM
    BIGNUM *priv_bn = BN_bin2bn(private_key_bytes, PRIVATE_KEY_SIZE, NULL);
    if (!priv_bn) {
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Compute public key point
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) {
        BN_free(priv_bn);
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EC_POINT_mul(new_key->group, new_key->point, priv_bn, NULL, NULL, ctx) != 1) {
        BN_CTX_free(ctx);
        BN_free(priv_bn);
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Encode compressed and uncompressed forms
    size_t comp_len = EC_POINT_point2oct(new_key->group, new_key->point, 
                                          POINT_CONVERSION_COMPRESSED, 
                                          new_key->compressed, 
                                          PUBLIC_KEY_COMPRESSED_SIZE, ctx);
    
    size_t uncomp_len = EC_POINT_point2oct(new_key->group, new_key->point, 
                                            POINT_CONVERSION_UNCOMPRESSED, 
                                            new_key->uncompressed, 
                                            PUBLIC_KEY_UNCOMPRESSED_SIZE, ctx);
    
    BN_CTX_free(ctx);
    BN_free(priv_bn);
    
    if (comp_len != PUBLIC_KEY_COMPRESSED_SIZE || uncomp_len != PUBLIC_KEY_UNCOMPRESSED_SIZE) {
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    new_key->is_compressed = true; // Default to compressed
    
    *public_key = new_key;
    return NEOC_SUCCESS;
}

/**
 * @brief Create public key from encoded bytes
 */
neoc_error_t neoc_ec_public_key_from_bytes(const uint8_t *encoded, size_t encoded_len,
                                            neoc_ec_public_key_t **public_key) {
    if (!encoded || !public_key || 
        (encoded_len != PUBLIC_KEY_COMPRESSED_SIZE && encoded_len != PUBLIC_KEY_UNCOMPRESSED_SIZE)) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *public_key = NULL;
    
    neoc_ec_public_key_t *new_key = neoc_malloc(sizeof(neoc_ec_public_key_t));
    if (!new_key) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_key->group = get_secp256r1_group();
    if (!new_key->group) {
        neoc_free(new_key);
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    new_key->point = EC_POINT_new(new_key->group);
    if (!new_key->point) {
        neoc_free(new_key);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) {
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (EC_POINT_oct2point(new_key->group, new_key->point, encoded, encoded_len, ctx) != 1) {
        BN_CTX_free(ctx);
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_INVALID_POINT;
    }
    
    // Store both formats
    size_t comp_len = EC_POINT_point2oct(new_key->group, new_key->point, 
                                          POINT_CONVERSION_COMPRESSED, 
                                          new_key->compressed, 
                                          PUBLIC_KEY_COMPRESSED_SIZE, ctx);
    
    size_t uncomp_len = EC_POINT_point2oct(new_key->group, new_key->point, 
                                            POINT_CONVERSION_UNCOMPRESSED, 
                                            new_key->uncompressed, 
                                            PUBLIC_KEY_UNCOMPRESSED_SIZE, ctx);
    
    BN_CTX_free(ctx);
    
    if (comp_len != PUBLIC_KEY_COMPRESSED_SIZE || uncomp_len != PUBLIC_KEY_UNCOMPRESSED_SIZE) {
        EC_POINT_free(new_key->point);
        neoc_free(new_key);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    new_key->is_compressed = (encoded_len == PUBLIC_KEY_COMPRESSED_SIZE);
    
    *public_key = new_key;
    return NEOC_SUCCESS;
}

/**
 * @brief Get encoded public key
 */
neoc_error_t neoc_ec_public_key_get_encoded(const neoc_ec_public_key_t *public_key,
                                             bool compressed,
                                             uint8_t **encoded,
                                             size_t *encoded_len) {
    if (!public_key || !encoded || !encoded_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (compressed) {
        *encoded_len = PUBLIC_KEY_COMPRESSED_SIZE;
        *encoded = neoc_malloc(*encoded_len);
        if (!*encoded) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(*encoded, public_key->compressed, *encoded_len);
    } else {
        *encoded_len = PUBLIC_KEY_UNCOMPRESSED_SIZE;
        *encoded = neoc_malloc(*encoded_len);
        if (!*encoded) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(*encoded, public_key->uncompressed, *encoded_len);
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Free a public key
 */
void neoc_ec_public_key_free(neoc_ec_public_key_t *public_key) {
    if (!public_key) {
        return;
    }
    
    if (public_key->point) {
        EC_POINT_free(public_key->point);
    }
    
    // Note: group is static, don't free
    neoc_free(public_key);
}

/**
 * @brief Free a private key
 */
void neoc_ec_private_key_free(neoc_ec_private_key_t *private_key) {
    if (!private_key) {
        return;
    }
    
    if (private_key->pkey) {
        EVP_PKEY_free(private_key->pkey);
    }
    
    if (private_key->ec_key) {
        EC_KEY_free(private_key->ec_key);
    }
    
    // Clear private key bytes for security
    memset(private_key->bytes, 0, PRIVATE_KEY_SIZE);
    
    neoc_free(private_key);
}

/**
 * @brief Get the private key from a key pair
 */
neoc_error_t neoc_ec_key_pair_get_private_key(const neoc_ec_key_pair_t *key_pair,
                                               uint8_t *private_key,
                                               size_t *key_len) {
    if (!key_pair || !private_key || !key_len || !key_pair->private_key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (*key_len < PRIVATE_KEY_SIZE) {
        *key_len = PRIVATE_KEY_SIZE;
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    memcpy(private_key, key_pair->private_key->bytes, PRIVATE_KEY_SIZE);
    *key_len = PRIVATE_KEY_SIZE;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get the public key from a key pair
 */
neoc_error_t neoc_ec_key_pair_get_public_key(const neoc_ec_key_pair_t *key_pair,
                                              uint8_t *public_key,
                                              size_t *key_len) {
    if (!key_pair || !public_key || !key_len || !key_pair->public_key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    bool compressed = (*key_len == PUBLIC_KEY_COMPRESSED_SIZE);
    size_t required_len = compressed ? PUBLIC_KEY_COMPRESSED_SIZE : PUBLIC_KEY_UNCOMPRESSED_SIZE;
    
    if (*key_len < required_len) {
        *key_len = required_len;
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    if (compressed) {
        memcpy(public_key, key_pair->public_key->compressed, PUBLIC_KEY_COMPRESSED_SIZE);
        *key_len = PUBLIC_KEY_COMPRESSED_SIZE;
    } else {
        memcpy(public_key, key_pair->public_key->uncompressed, PUBLIC_KEY_UNCOMPRESSED_SIZE);
        *key_len = PUBLIC_KEY_UNCOMPRESSED_SIZE;
    }
    
    return NEOC_SUCCESS;
}
