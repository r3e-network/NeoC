/**
 * @file nep2.c
 * @brief NEP-2 password-protected private key encryption implementation
 */

#include "neoc/crypto/nep2.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/neoc_memory.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/aes.h>
#include <string.h>

// NEP-2 constants
#define NEP2_PREFIX_1 0x01
#define NEP2_PREFIX_2 0x42
#define NEP2_FLAG_UNCOMPRESSED 0xC0
#define NEP2_FLAG_COMPRESSED 0xE0
#define NEP2_ENCRYPTED_SIZE 39  // Total size of encrypted data
#define NEP2_ENCODED_SIZE 58    // Base58Check encoded size

// Default scrypt parameters
const neoc_nep2_params_t NEOC_NEP2_DEFAULT_PARAMS = {
    .n = 16384,
    .r = 8,
    .p = 8
};

// Light scrypt parameters (for testing/development)
const neoc_nep2_params_t NEOC_NEP2_LIGHT_PARAMS = {
    .n = 1024,
    .r = 1,
    .p = 1
};

// Helper function to perform scrypt key derivation
static neoc_error_t derive_key(const char *password, const uint8_t *salt, size_t salt_len,
                                const neoc_nep2_params_t *params, uint8_t *derived_key) {
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_SCRYPT, NULL);
    if (!pctx) {
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to create scrypt context");
    }
    
    if (EVP_PKEY_derive_init(pctx) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to init scrypt");
    }
    
    // Use provided params or defaults
    const neoc_nep2_params_t *p = params ? params : &NEOC_NEP2_DEFAULT_PARAMS;
    
    if (EVP_PKEY_CTX_set1_pbe_pass(pctx, password, strlen(password)) <= 0 ||
        EVP_PKEY_CTX_set1_scrypt_salt(pctx, salt, salt_len) <= 0 ||
        EVP_PKEY_CTX_set_scrypt_N(pctx, p->n) <= 0 ||
        EVP_PKEY_CTX_set_scrypt_r(pctx, p->r) <= 0 ||
        EVP_PKEY_CTX_set_scrypt_p(pctx, p->p) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to set scrypt parameters");
    }
    
    size_t keylen = 64;
    if (EVP_PKEY_derive(pctx, derived_key, &keylen) <= 0 || keylen != 64) {
        EVP_PKEY_CTX_free(pctx);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to derive key");
    }
    
    EVP_PKEY_CTX_free(pctx);
    return NEOC_SUCCESS;
}

// Helper function to XOR two byte arrays
static void xor_bytes(uint8_t *dest, const uint8_t *src, size_t len) {
    for (size_t i = 0; i < len; i++) {
        dest[i] ^= src[i];
    }
}

neoc_error_t neoc_nep2_encrypt(const uint8_t *private_key,
                                const char *password,
                                const neoc_nep2_params_t *params,
                                char *encrypted_key,
                                size_t encrypted_key_len) {
    if (!private_key || !password || !encrypted_key) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (encrypted_key_len < NEP2_ENCODED_SIZE) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    // Create EC key pair from private key
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Get public key and address
    uint8_t public_key[65];
    size_t public_key_len = sizeof(public_key);
    err = neoc_ec_key_pair_get_public_key(key_pair, public_key, &public_key_len);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    // Get address hash
    neoc_hash160_t address_hash;
    // Generate proper verification script from public key for address calculation
    uint8_t compressed_pub[33];
    if (public_key_len == 33) {
        memcpy(compressed_pub, public_key, 33);
    } else if (public_key_len == 65 && public_key[0] == 0x04) {
        // Convert uncompressed key to compressed form
        memcpy(&compressed_pub[1], &public_key[1], 32);
        compressed_pub[0] = (public_key[64] & 0x01) ? 0x03 : 0x02;
    } else {
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unsupported public key format");
    }

    // Build standard verification script: PUSHBYTES33 <pubkey> CHECKSIG
    size_t script_len = 35;
    uint8_t *verification_script = neoc_malloc(script_len);
    if (!verification_script) {
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate verification script");
    }
    verification_script[0] = 0x21; // PUSHBYTES33
    memcpy(&verification_script[1], compressed_pub, 33);
    verification_script[34] = 0xAC; // CHECKSIG

    err = neoc_hash160_from_script(&address_hash, verification_script, script_len);
    neoc_free(verification_script);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    // Get first 4 bytes of address hash as salt
    uint8_t salt[4];
    uint8_t address_bytes[20];
    neoc_hash160_to_bytes(&address_hash, address_bytes, sizeof(address_bytes));
    memcpy(salt, address_bytes, 4);
    
    // Derive key using scrypt
    uint8_t derived_key[64];
    err = derive_key(password, salt, 4, params, derived_key);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    // XOR private key with first 32 bytes of derived key
    uint8_t xored[32];
    memcpy(xored, private_key, 32);
    xor_bytes(xored, derived_key, 32);
    
    // Perform AES encryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create cipher context");
    }
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, derived_key + 32, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to init AES");
    }
    
    EVP_CIPHER_CTX_set_padding(ctx, 0); // Disable padding for NEP-2
    
    int len;
    // int ciphertext_len; // Not needed - we know output is 32 bytes
    uint8_t ciphertext[48]; // Enough for padding
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, xored, 32) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to encrypt");
    }
    // ciphertext_len = len; // Not used
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to finalize encryption");
    }
    // ciphertext_len += len; // Not used - we know total is 32 bytes
    
    EVP_CIPHER_CTX_free(ctx);
    
    // Build NEP-2 structure
    uint8_t nep2_data[NEP2_ENCRYPTED_SIZE];
    nep2_data[0] = NEP2_PREFIX_1;
    nep2_data[1] = NEP2_PREFIX_2;
    nep2_data[2] = NEP2_FLAG_COMPRESSED; // Assume compressed
    memcpy(nep2_data + 3, salt, 4);
    memcpy(nep2_data + 7, ciphertext, 32);
    
    // Base58Check encode
    err = neoc_base58_check_encode(nep2_data, NEP2_ENCRYPTED_SIZE, encrypted_key, encrypted_key_len);
    
    neoc_ec_key_pair_free(key_pair);
    return err;
}

neoc_error_t neoc_nep2_decrypt(const char *encrypted_key,
                                const char *password,
                                const neoc_nep2_params_t *params,
                                uint8_t *private_key,
                                size_t private_key_len) {
    if (!encrypted_key || !password || !private_key) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (private_key_len < 32) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    // Base58Check decode
    uint8_t nep2_data[NEP2_ENCRYPTED_SIZE];
    size_t decoded_len = NEP2_ENCRYPTED_SIZE;
    neoc_error_t err = neoc_base58_check_decode(encrypted_key, nep2_data, NEP2_ENCRYPTED_SIZE, &decoded_len);
    if (err != NEOC_SUCCESS || decoded_len != NEP2_ENCRYPTED_SIZE) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid NEP-2 format");
    }
    
    // Verify prefix
    if (nep2_data[0] != NEP2_PREFIX_1 || nep2_data[1] != NEP2_PREFIX_2) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid NEP-2 prefix");
    }
    
    // Extract salt and encrypted data
    uint8_t salt[4];
    uint8_t encrypted[32];
    memcpy(salt, nep2_data + 3, 4);
    memcpy(encrypted, nep2_data + 7, 32);
    
    // Derive key using scrypt
    uint8_t derived_key[64];
    err = derive_key(password, salt, 4, params, derived_key);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Decrypt using AES
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create cipher context");
    }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, derived_key + 32, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to init AES");
    }
    
    EVP_CIPHER_CTX_set_padding(ctx, 0); // Disable padding for NEP-2
    
    int len;
    // int plaintext_len; // Not needed - we know output is 32 bytes
    uint8_t decrypted[48];
    
    if (EVP_DecryptUpdate(ctx, decrypted, &len, encrypted, 32) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to decrypt");
    }
    // plaintext_len = len; // Not used
    
    if (EVP_DecryptFinal_ex(ctx, decrypted + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return neoc_error_set(NEOC_ERROR_CRYPTO, "Failed to finalize decryption");
    }
    // plaintext_len += len; // Not used - we know total is 32 bytes
    
    EVP_CIPHER_CTX_free(ctx);
    
    // XOR with first 32 bytes of derived key to get private key
    memcpy(private_key, decrypted, 32);
    xor_bytes(private_key, derived_key, 32);
    
    // Verify by checking address hash
    neoc_ec_key_pair_t *key_pair = NULL;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    if (err != NEOC_SUCCESS) {
        return neoc_error_set(NEOC_ERROR_INVALID_PASSWORD, "Invalid password");
    }
    
    // Get public key and verify address matches salt
    uint8_t public_key[65];
    size_t public_key_len = sizeof(public_key);
    err = neoc_ec_key_pair_get_public_key(key_pair, public_key, &public_key_len);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    // Calculate address hash
    neoc_hash160_t address_hash;
    uint8_t script[35];
    script[0] = 0x21; // PUSH21
    memcpy(script + 1, public_key, 33);
    script[34] = 0xAC; // CHECKSIG
    err = neoc_hash160_from_script(&address_hash, script, 35);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    // Verify salt matches first 4 bytes of address hash
    uint8_t address_bytes[20];
    neoc_hash160_to_bytes(&address_hash, address_bytes, sizeof(address_bytes));
    if (memcmp(salt, address_bytes, 4) != 0) {
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_INVALID_PASSWORD, "Invalid password");
    }
    
    neoc_ec_key_pair_free(key_pair);
    return NEOC_SUCCESS;
}

bool neoc_nep2_verify_password(const char *encrypted_key,
                                const char *password,
                                const neoc_nep2_params_t *params) {
    uint8_t private_key[32];
    neoc_error_t err = neoc_nep2_decrypt(encrypted_key, password, params, private_key, sizeof(private_key));
    
    // Clear private key from memory
    memset(private_key, 0, sizeof(private_key));
    
    return err == NEOC_SUCCESS;
}

bool neoc_nep2_is_valid_format(const char *encrypted_key) {
    if (!encrypted_key) {
        return false;
    }
    
    // Check length (should be 58 characters for Base58Check encoded)
    size_t len = strlen(encrypted_key);
    if (len != NEP2_ENCODED_SIZE) {
        return false;
    }
    
    // Try to decode
    uint8_t decoded[NEP2_ENCRYPTED_SIZE];
    size_t decoded_len = NEP2_ENCRYPTED_SIZE;
    neoc_error_t err = neoc_base58_check_decode(encrypted_key, decoded, sizeof(decoded), &decoded_len);
    if (err != NEOC_SUCCESS || decoded_len != NEP2_ENCRYPTED_SIZE) {
        return false;
    }
    
    // Check prefix
    return decoded[0] == NEP2_PREFIX_1 && decoded[1] == NEP2_PREFIX_2;
}
