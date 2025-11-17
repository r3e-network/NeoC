/**
 * Complete Crypto Module Test Suite
 * Comprehensive unit tests matching Swift test coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "neoc/crypto/crypto.h"
#include "neoc/crypto/ecdsa.h"
#include "neoc/crypto/sha256.h"
#include "neoc/crypto/ripemd160.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/crypto/base64.h"
#include "neoc/crypto/aes.h"
#include "neoc/crypto/scrypt.h"
#include "neoc/utils/utils.h"

// Test framework
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return 1; \
        } \
    } while(0)

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Run a test and track results
#define RUN_TEST(test_func) \
    do { \
        printf("Running: %s... ", #test_func); \
        tests_run++; \
        if (test_func() == TEST_SUCCESS) { \
            printf("✓ PASSED\n"); \
            tests_passed++; \
        } else { \
            printf("✗ FAILED\n"); \
            tests_failed++; \
        } \
    } while(0)

// ECDSA Tests
int test_ecdsa_key_generation() {
    neoc_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ecdsa_generate_key_pair(&key_pair);
    
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(key_pair != NULL);
    TEST_ASSERT(key_pair->private_key != NULL);
    TEST_ASSERT(key_pair->public_key != NULL);
    TEST_ASSERT(key_pair->private_key_len == 32);
    TEST_ASSERT(key_pair->public_key_len == 33 || key_pair->public_key_len == 65);
    
    neoc_key_pair_free(key_pair);
    return TEST_SUCCESS;
}

int test_ecdsa_signature() {
    neoc_key_pair_t *key_pair = NULL;
    neoc_ecdsa_generate_key_pair(&key_pair);
    
    uint8_t message[] = "Test message for signing";
    uint8_t signature[72];
    size_t sig_len = sizeof(signature);
    
    neoc_error_t err = neoc_ecdsa_sign(key_pair, message, sizeof(message), signature, &sig_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(sig_len > 0);
    
    // Verify signature
    bool valid = false;
    err = neoc_ecdsa_verify(key_pair->public_key, key_pair->public_key_len,
                            message, sizeof(message), signature, sig_len, &valid);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(valid == true);
    
    // Verify with wrong message fails
    uint8_t wrong_message[] = "Wrong message";
    err = neoc_ecdsa_verify(key_pair->public_key, key_pair->public_key_len,
                            wrong_message, sizeof(wrong_message), signature, sig_len, &valid);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(valid == false);
    
    neoc_key_pair_free(key_pair);
    return TEST_SUCCESS;
}

int test_ecdsa_public_key_recovery() {
    neoc_key_pair_t *key_pair = NULL;
    neoc_ecdsa_generate_key_pair(&key_pair);
    
    // Get public key from private key
    uint8_t recovered_public[65];
    size_t pub_len = sizeof(recovered_public);
    
    neoc_error_t err = neoc_ecdsa_get_public_key(key_pair->private_key, 
                                                  key_pair->private_key_len,
                                                  recovered_public, &pub_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(pub_len == key_pair->public_key_len);
    
    // For compressed keys, compare directly
    if (key_pair->public_key_len == 33) {
        TEST_ASSERT(memcmp(recovered_public, key_pair->public_key, 33) == 0);
    }
    
    neoc_key_pair_free(key_pair);
    return TEST_SUCCESS;
}

int test_ecdsa_deterministic_signatures() {
    neoc_key_pair_t *key_pair = NULL;
    neoc_ecdsa_generate_key_pair(&key_pair);
    
    uint8_t message[] = "Deterministic signature test";
    uint8_t sig1[72], sig2[72];
    size_t sig1_len = sizeof(sig1);
    size_t sig2_len = sizeof(sig2);
    
    // Sign same message twice
    neoc_ecdsa_sign(key_pair, message, sizeof(message), sig1, &sig1_len);
    neoc_ecdsa_sign(key_pair, message, sizeof(message), sig2, &sig2_len);
    
    // RFC 6979 deterministic signatures should be identical
    TEST_ASSERT(sig1_len == sig2_len);
    TEST_ASSERT(memcmp(sig1, sig2, sig1_len) == 0);
    
    neoc_key_pair_free(key_pair);
    return TEST_SUCCESS;
}

// SHA256 Tests
int test_sha256_basic() {
    uint8_t data[] = "Hello, World!";
    uint8_t hash[32];
    
    neoc_error_t err = neoc_sha256(data, sizeof(data) - 1, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Known hash value
    uint8_t expected[] = {
        0xdf, 0xfd, 0x60, 0x21, 0xbb, 0x2b, 0xd5, 0xb0,
        0xaf, 0x67, 0x62, 0x90, 0x80, 0x9e, 0xc3, 0xa5,
        0x31, 0x91, 0xdd, 0x81, 0xc7, 0xf7, 0x0a, 0x4b,
        0x28, 0x68, 0x8a, 0x36, 0x21, 0x82, 0x98, 0x6f
    };
    
    TEST_ASSERT(memcmp(hash, expected, 32) == 0);
    return TEST_SUCCESS;
}

int test_sha256_double() {
    uint8_t data[] = "Bitcoin";
    uint8_t hash[32];
    
    neoc_error_t err = neoc_sha256_double(data, sizeof(data) - 1, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Verify double hash is different from single
    uint8_t single_hash[32];
    neoc_sha256(data, sizeof(data) - 1, single_hash);
    TEST_ASSERT(memcmp(hash, single_hash, 32) != 0);
    
    return TEST_SUCCESS;
}

int test_sha256_empty() {
    uint8_t hash[32];
    
    neoc_error_t err = neoc_sha256(NULL, 0, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // SHA256 of empty string
    uint8_t expected[] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
    };
    
    TEST_ASSERT(memcmp(hash, expected, 32) == 0);
    return TEST_SUCCESS;
}

int test_sha256_large_data() {
    // Test with 1MB of data
    size_t data_size = 1024 * 1024;
    uint8_t *data = malloc(data_size);
    TEST_ASSERT(data != NULL);
    
    // Fill with pattern
    for (size_t i = 0; i < data_size; i++) {
        data[i] = (uint8_t)(i & 0xFF);
    }
    
    uint8_t hash[32];
    neoc_error_t err = neoc_sha256(data, data_size, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Hash should be deterministic
    uint8_t hash2[32];
    err = neoc_sha256(data, data_size, hash2);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(memcmp(hash, hash2, 32) == 0);
    
    free(data);
    return TEST_SUCCESS;
}

// RIPEMD160 Tests
int test_ripemd160_basic() {
    uint8_t data[] = "The quick brown fox jumps over the lazy dog";
    uint8_t hash[20];
    
    neoc_error_t err = neoc_ripemd160(data, sizeof(data) - 1, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Known hash value
    uint8_t expected[] = {
        0x37, 0xf3, 0x32, 0xf6, 0x8d, 0xb7, 0x7b, 0xd9,
        0xd7, 0xed, 0xd4, 0x96, 0x95, 0x71, 0xad, 0x67,
        0x1c, 0xf9, 0xdd, 0x3b
    };
    
    TEST_ASSERT(memcmp(hash, expected, 20) == 0);
    return TEST_SUCCESS;
}

int test_ripemd160_empty() {
    uint8_t hash[20];
    
    neoc_error_t err = neoc_ripemd160(NULL, 0, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // RIPEMD160 of empty string
    uint8_t expected[] = {
        0x9c, 0x11, 0x85, 0xa5, 0xc5, 0xe9, 0xfc, 0x54,
        0x61, 0x28, 0x08, 0x97, 0x7e, 0xe8, 0xf5, 0x48,
        0xb2, 0x25, 0x8d, 0x31
    };
    
    TEST_ASSERT(memcmp(hash, expected, 20) == 0);
    return TEST_SUCCESS;
}

// Base58 Tests
int test_base58_encode_decode() {
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    char encoded[256];
    
    neoc_error_t err = neoc_base58_encode(data, sizeof(data), encoded, sizeof(encoded));
    TEST_ASSERT(err == NEOC_SUCCESS);
    size_t encoded_len = strlen(encoded);
    TEST_ASSERT(encoded_len > 0);
    
    // Decode back
    uint8_t decoded[256];
    size_t decoded_len = 0;
    err = neoc_base58_decode(encoded, decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(decoded_len == sizeof(data));
    TEST_ASSERT(memcmp(data, decoded, sizeof(data)) == 0);
    
    return TEST_SUCCESS;
}

int test_base58_check_encode_decode() {
    uint8_t data[] = {0x00, 0x14, 0x75, 0x1e, 0x76, 0xe8, 0x19, 0x91, 0x96,
                      0xd4, 0x54, 0x94, 0x1c, 0x45, 0xd1, 0xb3, 0xa3, 0x23};
    char encoded[256];
    
    neoc_error_t err = neoc_base58_check_encode(data, sizeof(data), encoded, sizeof(encoded));
    TEST_ASSERT(err == NEOC_SUCCESS);
    size_t encoded_len = strlen(encoded);
    
    // Should produce Bitcoin address format
    TEST_ASSERT(encoded[0] == '1');
    
    // Decode back
    uint8_t decoded[256];
    size_t decoded_len = 0;
    err = neoc_base58_check_decode(encoded, decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(decoded_len == sizeof(data));
    TEST_ASSERT(memcmp(data, decoded, sizeof(data)) == 0);
    
    return TEST_SUCCESS;
}

int test_base58_invalid_decode() {
    // Invalid characters
    char invalid[] = "InvalidBase58String!@#";
    uint8_t decoded[256];
    size_t decoded_len = 0;
    
    neoc_error_t err = neoc_base58_decode(invalid, decoded, sizeof(decoded), &decoded_len);
    TEST_ASSERT(err != NEOC_SUCCESS);
    
    return TEST_SUCCESS;
}

// Base64 Tests  
int test_base64_encode_decode() {
    uint8_t data[] = "Hello, World!";
    char encoded[256];
    size_t encoded_len = sizeof(encoded);
    
    neoc_error_t err = neoc_base64_encode(data, sizeof(data) - 1, encoded, &encoded_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(strcmp(encoded, "SGVsbG8sIFdvcmxkIQ==") == 0);
    
    // Decode back
    uint8_t decoded[256];
    size_t decoded_len = sizeof(decoded);
    err = neoc_base64_decode(encoded, decoded, &decoded_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(decoded_len == sizeof(data) - 1);
    TEST_ASSERT(memcmp(data, decoded, sizeof(data) - 1) == 0);
    
    return TEST_SUCCESS;
}

int test_base64_padding() {
    // Test different padding scenarios
    uint8_t data1[] = "f";
    uint8_t data2[] = "fo";
    uint8_t data3[] = "foo";
    
    char encoded[256];
    size_t encoded_len;
    
    // One byte - two padding chars
    encoded_len = sizeof(encoded);
    neoc_base64_encode(data1, 1, encoded, &encoded_len);
    TEST_ASSERT(strcmp(encoded, "Zg==") == 0);
    
    // Two bytes - one padding char
    encoded_len = sizeof(encoded);
    neoc_base64_encode(data2, 2, encoded, &encoded_len);
    TEST_ASSERT(strcmp(encoded, "Zm8=") == 0);
    
    // Three bytes - no padding
    encoded_len = sizeof(encoded);
    neoc_base64_encode(data3, 3, encoded, &encoded_len);
    TEST_ASSERT(strcmp(encoded, "Zm9v") == 0);
    
    return TEST_SUCCESS;
}

// AES Tests
int test_aes_128_cbc() {
    uint8_t key[16] = "1234567890123456";
    uint8_t iv[16] = "abcdefghijklmnop";
    uint8_t plaintext[] = "The quick brown fox jumps over the lazy dog";
    
    uint8_t ciphertext[256];
    size_t cipher_len = sizeof(ciphertext);
    
    neoc_error_t err = neoc_aes_encrypt_cbc(plaintext, sizeof(plaintext),
                                            key, 16, iv, ciphertext, &cipher_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(cipher_len > sizeof(plaintext)); // Due to padding
    
    // Decrypt back
    uint8_t decrypted[256];
    size_t decrypted_len = sizeof(decrypted);
    err = neoc_aes_decrypt_cbc(ciphertext, cipher_len,
                               key, 16, iv, decrypted, &decrypted_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(decrypted_len == sizeof(plaintext));
    TEST_ASSERT(memcmp(plaintext, decrypted, sizeof(plaintext)) == 0);
    
    return TEST_SUCCESS;
}

int test_aes_256_cbc() {
    uint8_t key[32] = "12345678901234567890123456789012";
    uint8_t iv[16] = "abcdefghijklmnop";
    uint8_t plaintext[] = "AES-256 encryption test";
    
    uint8_t ciphertext[256];
    size_t cipher_len = sizeof(ciphertext);
    
    neoc_error_t err = neoc_aes_encrypt_cbc(plaintext, sizeof(plaintext),
                                            key, 32, iv, ciphertext, &cipher_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Decrypt back
    uint8_t decrypted[256];
    size_t decrypted_len = sizeof(decrypted);
    err = neoc_aes_decrypt_cbc(ciphertext, cipher_len,
                               key, 32, iv, decrypted, &decrypted_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(memcmp(plaintext, decrypted, sizeof(plaintext)) == 0);
    
    return TEST_SUCCESS;
}

// Scrypt Tests
int test_scrypt_basic() {
    uint8_t password[] = "password";
    uint8_t salt[] = "NaCl";
    uint8_t derived[64];
    
    neoc_error_t err = neoc_scrypt(password, sizeof(password) - 1,
                                   salt, sizeof(salt) - 1,
                                   16384, 8, 1, derived, 64);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Verify deterministic output
    uint8_t derived2[64];
    err = neoc_scrypt(password, sizeof(password) - 1,
                     salt, sizeof(salt) - 1,
                     16384, 8, 1, derived2, 64);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(memcmp(derived, derived2, 64) == 0);
    
    return TEST_SUCCESS;
}

int test_scrypt_different_params() {
    uint8_t password[] = "testpass";
    uint8_t salt[] = "saltsalt";
    uint8_t derived1[32], derived2[32];
    
    // Different N parameter
    neoc_scrypt(password, sizeof(password) - 1, salt, sizeof(salt) - 1,
                16384, 8, 1, derived1, 32);
    neoc_scrypt(password, sizeof(password) - 1, salt, sizeof(salt) - 1,
                32768, 8, 1, derived2, 32);
    
    TEST_ASSERT(memcmp(derived1, derived2, 32) != 0);
    
    // Different r parameter
    neoc_scrypt(password, sizeof(password) - 1, salt, sizeof(salt) - 1,
                16384, 8, 1, derived1, 32);
    neoc_scrypt(password, sizeof(password) - 1, salt, sizeof(salt) - 1,
                16384, 16, 1, derived2, 32);
    
    TEST_ASSERT(memcmp(derived1, derived2, 32) != 0);
    
    return TEST_SUCCESS;
}

// Random number generation tests
int test_random_bytes() {
    uint8_t random1[32];
    uint8_t random2[32];
    
    neoc_error_t err = neoc_random_bytes(random1, 32);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    err = neoc_random_bytes(random2, 32);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Should be different (extremely high probability)
    TEST_ASSERT(memcmp(random1, random2, 32) != 0);
    
    // Should not be all zeros
    int all_zeros = 1;
    for (int i = 0; i < 32; i++) {
        if (random1[i] != 0) {
            all_zeros = 0;
            break;
        }
    }
    TEST_ASSERT(!all_zeros);
    
    return TEST_SUCCESS;
}

// Hash combination tests
int test_hash160() {
    uint8_t data[] = "Bitcoin";
    uint8_t hash[20];
    
    neoc_error_t err = neoc_hash160(data, sizeof(data) - 1, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Hash160 is SHA256 followed by RIPEMD160
    uint8_t sha256_hash[32];
    uint8_t ripemd_hash[20];
    
    neoc_sha256(data, sizeof(data) - 1, sha256_hash);
    neoc_ripemd160(sha256_hash, 32, ripemd_hash);
    
    TEST_ASSERT(memcmp(hash, ripemd_hash, 20) == 0);
    
    return TEST_SUCCESS;
}

int test_hash256() {
    uint8_t data[] = "Double SHA256";
    uint8_t hash[32];
    
    neoc_error_t err = neoc_hash256(data, sizeof(data) - 1, hash);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Hash256 is double SHA256
    uint8_t sha1[32], sha2[32];
    neoc_sha256(data, sizeof(data) - 1, sha1);
    neoc_sha256(sha1, 32, sha2);
    
    TEST_ASSERT(memcmp(hash, sha2, 32) == 0);
    
    return TEST_SUCCESS;
}

// Key derivation tests
int test_pbkdf2() {
    uint8_t password[] = "password";
    uint8_t salt[] = "salt";
    uint8_t derived[32];
    
    neoc_error_t err = neoc_pbkdf2_hmac_sha256(password, sizeof(password) - 1,
                                               salt, sizeof(salt) - 1,
                                               4096, derived, 32);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Known test vector for PBKDF2-HMAC-SHA256
    uint8_t expected[] = {
        0xc5, 0xe4, 0x78, 0xd5, 0x92, 0x88, 0xc8, 0x41,
        0xaa, 0x53, 0x0d, 0xb6, 0x84, 0x5c, 0x4c, 0x8d,
        0x96, 0x28, 0x93, 0xa0, 0x01, 0xce, 0x4e, 0x11,
        0xa4, 0x96, 0x38, 0x73, 0xaa, 0x98, 0x13, 0x4a
    };
    
    TEST_ASSERT(memcmp(derived, expected, 32) == 0);
    
    return TEST_SUCCESS;
}

// WIF (Wallet Import Format) tests
int test_wif_encode_decode() {
    uint8_t private_key[32];
    memset(private_key, 0x01, 32); // Test key
    
    char wif[128];
    size_t wif_len = sizeof(wif);
    
    neoc_error_t err = neoc_wif_encode(private_key, 32, 0x80, true, wif, &wif_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(wif[0] == 'K' || wif[0] == 'L'); // Compressed key prefix
    
    // Decode back
    uint8_t decoded[32];
    uint8_t version;
    bool compressed;
    
    err = neoc_wif_decode(wif, decoded, &version, &compressed);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(version == 0x80);
    TEST_ASSERT(compressed == true);
    TEST_ASSERT(memcmp(private_key, decoded, 32) == 0);
    
    return TEST_SUCCESS;
}

// Address generation tests
int test_address_from_public_key() {
    neoc_key_pair_t *key_pair = NULL;
    neoc_ecdsa_generate_key_pair(&key_pair);
    
    char address[128];
    size_t addr_len = sizeof(address);
    
    neoc_error_t err = neoc_address_from_public_key(key_pair->public_key,
                                                     key_pair->public_key_len,
                                                     0x17, // Neo address version
                                                     address, &addr_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(address[0] == 'A'); // Neo addresses start with 'A'
    
    neoc_key_pair_free(key_pair);
    return TEST_SUCCESS;
}

// Performance tests
int test_crypto_performance() {
    printf("\n  Performance Metrics:\n");
    
    // SHA256 performance
    clock_t start = clock();
    for (int i = 0; i < 10000; i++) {
        uint8_t data[64];
        uint8_t hash[32];
        neoc_sha256(data, 64, hash);
    }
    clock_t end = clock();
    double sha256_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("    SHA256: %.0f ops/sec\n", 10000.0 / sha256_time);
    
    // ECDSA sign performance
    neoc_key_pair_t *key_pair = NULL;
    neoc_ecdsa_generate_key_pair(&key_pair);
    
    start = clock();
    for (int i = 0; i < 100; i++) {
        uint8_t data[32];
        uint8_t sig[72];
        size_t sig_len = 72;
        neoc_ecdsa_sign(key_pair, data, 32, sig, &sig_len);
    }
    end = clock();
    double sign_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("    ECDSA Sign: %.0f ops/sec\n", 100.0 / sign_time);
    
    neoc_key_pair_free(key_pair);
    
    return TEST_SUCCESS;
}

// Main test runner
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("    Complete Crypto Module Test Suite   \n");
    printf("========================================\n\n");
    
    // ECDSA Tests
    printf("ECDSA Tests:\n");
    RUN_TEST(test_ecdsa_key_generation);
    RUN_TEST(test_ecdsa_signature);
    RUN_TEST(test_ecdsa_public_key_recovery);
    RUN_TEST(test_ecdsa_deterministic_signatures);
    
    // SHA256 Tests
    printf("\nSHA256 Tests:\n");
    RUN_TEST(test_sha256_basic);
    RUN_TEST(test_sha256_double);
    RUN_TEST(test_sha256_empty);
    RUN_TEST(test_sha256_large_data);
    
    // RIPEMD160 Tests
    printf("\nRIPEMD160 Tests:\n");
    RUN_TEST(test_ripemd160_basic);
    RUN_TEST(test_ripemd160_empty);
    
    // Base58 Tests
    printf("\nBase58 Tests:\n");
    RUN_TEST(test_base58_encode_decode);
    RUN_TEST(test_base58_check_encode_decode);
    RUN_TEST(test_base58_invalid_decode);
    
    // Base64 Tests
    printf("\nBase64 Tests:\n");
    RUN_TEST(test_base64_encode_decode);
    RUN_TEST(test_base64_padding);
    
    // AES Tests
    printf("\nAES Tests:\n");
    RUN_TEST(test_aes_128_cbc);
    RUN_TEST(test_aes_256_cbc);
    
    // Scrypt Tests
    printf("\nScrypt Tests:\n");
    RUN_TEST(test_scrypt_basic);
    RUN_TEST(test_scrypt_different_params);
    
    // Random Tests
    printf("\nRandom Generation Tests:\n");
    RUN_TEST(test_random_bytes);
    
    // Hash Combination Tests
    printf("\nHash Combination Tests:\n");
    RUN_TEST(test_hash160);
    RUN_TEST(test_hash256);
    
    // Key Derivation Tests
    printf("\nKey Derivation Tests:\n");
    RUN_TEST(test_pbkdf2);
    
    // Format Tests
    printf("\nFormat Tests:\n");
    RUN_TEST(test_wif_encode_decode);
    RUN_TEST(test_address_from_public_key);
    
    // Performance Tests
    printf("\nPerformance Tests:\n");
    RUN_TEST(test_crypto_performance);
    
    // Summary
    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✅ ALL TESTS PASSED!\n");
    } else {
        printf("\n❌ SOME TESTS FAILED\n");
    }
    printf("========================================\n");
    
    return tests_failed > 0 ? 1 : 0;
}
