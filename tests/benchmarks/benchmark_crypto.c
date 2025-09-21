/**
 * @file benchmark_crypto.c
 * @brief Performance benchmarks for cryptographic operations
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/crypto/ecdsa.h"
#include "neoc/crypto/base58.h"
#include "neoc/crypto/base64.h"
#include "neoc/crypto/scrypt.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/wallet/account.h"

#define ITERATIONS 1000
#define WARMUP_ITERATIONS 100

// Timing utilities
typedef struct {
    const char *name;
    clock_t start;
    clock_t end;
    int iterations;
} benchmark_t;

static void benchmark_start(benchmark_t *bench, const char *name, int iterations) {
    bench->name = name;
    bench->iterations = iterations;
    bench->start = clock();
}

static void benchmark_end(benchmark_t *bench) {
    bench->end = clock();
    double cpu_time_used = ((double)(bench->end - bench->start)) / CLOCKS_PER_SEC;
    double ops_per_sec = bench->iterations / cpu_time_used;
    double us_per_op = (cpu_time_used * 1000000) / bench->iterations;
    
    printf("%-30s: %8.2f ops/sec, %8.2f μs/op (%d iterations in %.3fs)\n",
           bench->name, ops_per_sec, us_per_op, bench->iterations, cpu_time_used);
}

// Benchmark key generation
static void benchmark_key_generation(void) {
    printf("\n=== Key Generation Benchmarks ===\n");
    benchmark_t bench;
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        neoc_ec_key_pair_t *key_pair = NULL;
        neoc_ec_key_pair_create(&key_pair);
        neoc_ec_key_pair_free(key_pair);
    }
    
    // Benchmark EC key pair generation
    benchmark_start(&bench, "EC Key Pair Generation", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_ec_key_pair_t *key_pair = NULL;
        neoc_error_t err = neoc_ec_key_pair_create(&key_pair);
        assert(err == NEOC_SUCCESS);
        neoc_ec_key_pair_free(key_pair);
    }
    benchmark_end(&bench);
    
    // Benchmark account creation
    benchmark_start(&bench, "Account Creation", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_account_t *account = NULL;
        neoc_error_t err = neoc_account_create(&account);
        assert(err == NEOC_SUCCESS);
        neoc_account_free(account);
    }
    benchmark_end(&bench);
}

// Benchmark signing operations
static void benchmark_signing(void) {
    printf("\n=== Signing Benchmarks ===\n");
    benchmark_t bench;
    
    // Create a key pair for signing
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create(&key_pair);
    assert(err == NEOC_SUCCESS);
    
    // Test data
    uint8_t message[32];
    for (int i = 0; i < 32; i++) {
        message[i] = i;
    }
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        uint8_t signature[64];
        size_t sig_len = sizeof(signature);
        neoc_ecdsa_sign(key_pair, message, sizeof(message), signature, &sig_len);
    }
    
    // Benchmark signing
    benchmark_start(&bench, "ECDSA Sign", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        uint8_t signature[64];
        size_t sig_len = sizeof(signature);
        err = neoc_ecdsa_sign(key_pair, message, sizeof(message), signature, &sig_len);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Create a signature for verification
    uint8_t signature[64];
    size_t sig_len = sizeof(signature);
    err = neoc_ecdsa_sign(key_pair, message, sizeof(message), signature, &sig_len);
    assert(err == NEOC_SUCCESS);
    
    // Get public key
    neoc_ec_public_key_t *pub_key = neoc_ec_key_pair_get_public_key(key_pair);
    assert(pub_key != NULL);
    
    // Benchmark verification
    benchmark_start(&bench, "ECDSA Verify", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        bool valid = false;
        err = neoc_ecdsa_verify(pub_key, message, sizeof(message), signature, sig_len, &valid);
        assert(err == NEOC_SUCCESS);
        assert(valid == true);
    }
    benchmark_end(&bench);
    
    neoc_ec_key_pair_free(key_pair);
}

// Benchmark encoding operations
static void benchmark_encoding(void) {
    printf("\n=== Encoding Benchmarks ===\n");
    benchmark_t bench;
    
    // Test data
    uint8_t binary_data[256];
    for (int i = 0; i < sizeof(binary_data); i++) {
        binary_data[i] = i;
    }
    
    char encoded[512];
    uint8_t decoded[256];
    size_t decoded_len;
    
    // Benchmark Base58 encoding
    benchmark_start(&bench, "Base58 Encode", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_error_t err = neoc_base58_encode(binary_data, sizeof(binary_data), encoded, sizeof(encoded));
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Get a Base58 string for decoding
    neoc_base58_encode(binary_data, sizeof(binary_data), encoded, sizeof(encoded));
    
    // Benchmark Base58 decoding
    benchmark_start(&bench, "Base58 Decode", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        decoded_len = sizeof(decoded);
        neoc_error_t err = neoc_base58_decode(encoded, decoded, &decoded_len);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Benchmark Base64 encoding
    benchmark_start(&bench, "Base64 Encode", ITERATIONS * 10);
    for (int i = 0; i < ITERATIONS * 10; i++) {
        neoc_error_t err = neoc_base64_encode(binary_data, sizeof(binary_data), encoded, sizeof(encoded));
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Get a Base64 string for decoding
    neoc_base64_encode(binary_data, sizeof(binary_data), encoded, sizeof(encoded));
    
    // Benchmark Base64 decoding
    benchmark_start(&bench, "Base64 Decode", ITERATIONS * 10);
    for (int i = 0; i < ITERATIONS * 10; i++) {
        decoded_len = sizeof(decoded);
        neoc_error_t err = neoc_base64_decode(encoded, decoded, &decoded_len);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
}

// Benchmark hashing operations
static void benchmark_hashing(void) {
    printf("\n=== Hashing Benchmarks ===\n");
    benchmark_t bench;
    
    // Test data
    uint8_t data[1024];
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = i & 0xFF;
    }
    
    uint8_t hash[32];
    
    // Benchmark SHA256 (small data)
    benchmark_start(&bench, "SHA256 (32 bytes)", ITERATIONS * 10);
    for (int i = 0; i < ITERATIONS * 10; i++) {
        neoc_error_t err = neoc_sha256(data, 32, hash);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Benchmark SHA256 (1KB data)
    benchmark_start(&bench, "SHA256 (1KB)", ITERATIONS * 5);
    for (int i = 0; i < ITERATIONS * 5; i++) {
        neoc_error_t err = neoc_sha256(data, sizeof(data), hash);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Benchmark RIPEMD160
    uint8_t ripemd_hash[20];
    benchmark_start(&bench, "RIPEMD160 (32 bytes)", ITERATIONS * 10);
    for (int i = 0; i < ITERATIONS * 10; i++) {
        neoc_error_t err = neoc_ripemd160(data, 32, ripemd_hash);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Benchmark double SHA256
    benchmark_start(&bench, "Double SHA256 (32 bytes)", ITERATIONS * 5);
    for (int i = 0; i < ITERATIONS * 5; i++) {
        uint8_t first_hash[32];
        neoc_error_t err = neoc_sha256(data, 32, first_hash);
        assert(err == NEOC_SUCCESS);
        err = neoc_sha256(first_hash, 32, hash);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
}

// Benchmark key derivation
static void benchmark_key_derivation(void) {
    printf("\n=== Key Derivation Benchmarks ===\n");
    benchmark_t bench;
    
    const char *password = "TestPassword123!";
    uint8_t salt[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t derived_key[32];
    
    // Benchmark Scrypt (low parameters for speed)
    neoc_scrypt_params_t params = {
        .n = 256,    // Low N for benchmarking
        .r = 1,
        .p = 1
    };
    
    benchmark_start(&bench, "Scrypt (N=256)", 10);  // Fewer iterations for slow operation
    for (int i = 0; i < 10; i++) {
        neoc_error_t err = neoc_scrypt(
            (uint8_t*)password, strlen(password),
            salt, sizeof(salt),
            params.n, params.r, params.p,
            derived_key, sizeof(derived_key)
        );
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Benchmark with higher parameters (more realistic)
    params.n = 16384;  // Standard N
    
    benchmark_start(&bench, "Scrypt (N=16384)", 3);  // Very few iterations
    for (int i = 0; i < 3; i++) {
        neoc_error_t err = neoc_scrypt(
            (uint8_t*)password, strlen(password),
            salt, sizeof(salt),
            params.n, params.r, params.p,
            derived_key, sizeof(derived_key)
        );
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
}

int main(void) {
    printf("=================================================\n");
    printf("         NeoC SDK Crypto Benchmarks\n");
    printf("=================================================\n");
    printf("CPU: Performance measured in operations per second\n");
    printf("All times are CPU time, not wall clock time\n");
    
    // Initialize NeoC
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
    
    // Run benchmarks
    benchmark_key_generation();
    benchmark_signing();
    benchmark_encoding();
    benchmark_hashing();
    benchmark_key_derivation();
    
    // Cleanup
    neoc_cleanup();
    
    printf("\n=================================================\n");
    printf("               Benchmarks Complete\n");
    printf("=================================================\n");
    
    return 0;
}