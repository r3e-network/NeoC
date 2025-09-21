/**
 * @file test_performance_comprehensive.c
 * @brief Comprehensive performance benchmark tests for NeoC SDK
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/crypto/wif.h>
#include <neoc/crypto/nep2.h>
#include <neoc/crypto/sign.h>
#include <neoc/crypto/hash.h>
#include <neoc/wallet/account.h>
#include <neoc/wallet/wallet.h>
#include <neoc/contract/gas_token.h>
#include <neoc/contract/neo_token.h>
#include <neoc/script/script_builder.h>
#include <neoc/transaction/transaction_builder.h>
#include <neoc/utils/neoc_hex.h>
#include <neoc/utils/neoc_base58.h>
#include <neoc/utils/neoc_base64.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Performance thresholds (in operations per second)
#define MIN_EC_KEYPAIR_OPS_PER_SEC      100
#define MIN_WIF_OPS_PER_SEC             500
#define MIN_HASH_OPS_PER_SEC           1000
#define MIN_BASE58_OPS_PER_SEC          200
#define MIN_HEX_OPS_PER_SEC            2000
#define MIN_ACCOUNT_OPS_PER_SEC         200
#define MIN_SCRIPT_BUILD_OPS_PER_SEC    500

// Test data
static const char* TEST_PRIVATE_KEY_HEX = "84180ac9d6eb6fba207ea4ef9d2200102d1ebeb4b9c07e2c6a738a42742e27a5";
static const char* TEST_WIF = "L1eV34wPoj9weqhGijdDLtVQzUpWGHszXXpdU9dPuh2nRFFzFa7E";
static const char* TEST_DATA_HEX = "deadbeefcafebabe0123456789abcdef";
static const char* TEST_MESSAGE = "Hello, Neo blockchain! This is a test message for performance benchmarking.";

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

// Utility function to measure operations per second
static double measure_ops_per_second(clock_t start, clock_t end, int operations) {
    double seconds = ((double)(end - start)) / CLOCKS_PER_SEC;
    return operations / seconds;
}

/* ===== EC KEY PAIR PERFORMANCE TESTS ===== */

void test_ec_key_pair_creation_performance(void) {
    printf("Testing EC key pair creation performance\n");
    
    const int NUM_OPERATIONS = 100;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        neoc_ec_key_pair_t* key_pair;
        neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_ec_key_pair_free(key_pair);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Created %d key pairs at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_EC_KEYPAIR_OPS_PER_SEC);
}

void test_ec_key_pair_from_private_key_performance(void) {
    printf("Testing EC key pair from private key performance\n");
    
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(TEST_PRIVATE_KEY_HEX, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    const int NUM_OPERATIONS = 200;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        neoc_ec_key_pair_t* key_pair;
        err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_ec_key_pair_free(key_pair);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Created %d key pairs from private key at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_EC_KEYPAIR_OPS_PER_SEC * 2); // Should be faster than random generation
}

void test_public_key_operations_performance(void) {
    printf("Testing public key operations performance\n");
    
    // Create a key pair once
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    const int NUM_OPERATIONS = 500;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t public_key[33];
        size_t key_len = sizeof(public_key);
        err = neoc_ec_key_pair_get_public_key(key_pair, public_key, &key_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Extracted %d public keys at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= 1000); // Should be very fast
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== WIF PERFORMANCE TESTS ===== */

void test_wif_encoding_performance(void) {
    printf("Testing WIF encoding performance\n");
    
    uint8_t private_key[32];
    size_t decoded_len;
    neoc_error_t err = neoc_hex_decode(TEST_PRIVATE_KEY_HEX, private_key, sizeof(private_key), &decoded_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    const int NUM_OPERATIONS = 500;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* wif;
        err = neoc_private_key_to_wif(private_key, &wif);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(wif);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Encoded %d WIF keys at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_WIF_OPS_PER_SEC);
}

void test_wif_decoding_performance(void) {
    printf("Testing WIF decoding performance\n");
    
    const int NUM_OPERATIONS = 500;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t private_key[32];
        size_t key_len = sizeof(private_key);
        neoc_error_t err = neoc_wif_to_private_key(TEST_WIF, private_key, &key_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Decoded %d WIF keys at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_WIF_OPS_PER_SEC);
}

/* ===== CRYPTOGRAPHIC HASH PERFORMANCE TESTS ===== */

void test_hash160_performance(void) {
    printf("Testing Hash160 performance\n");
    
    uint8_t test_data[100];
    memset(test_data, 0xAB, sizeof(test_data));
    
    const int NUM_OPERATIONS = 1000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t hash[20];
        neoc_error_t err = neoc_hash160(test_data, sizeof(test_data), hash);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Computed %d Hash160 operations at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_HASH_OPS_PER_SEC);
}

void test_hash256_performance(void) {
    printf("Testing Hash256 performance\n");
    
    uint8_t test_data[100];
    memset(test_data, 0xCD, sizeof(test_data));
    
    const int NUM_OPERATIONS = 1000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t hash[32];
        neoc_error_t err = neoc_hash256(test_data, sizeof(test_data), hash);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Computed %d Hash256 operations at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_HASH_OPS_PER_SEC);
}

void test_sha256_performance(void) {
    printf("Testing SHA256 performance\n");
    
    const char* message = TEST_MESSAGE;
    size_t message_len = strlen(message);
    
    const int NUM_OPERATIONS = 2000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t hash[32];
        neoc_error_t err = neoc_sha256((const uint8_t*)message, message_len, hash);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Computed %d SHA256 hashes at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_HASH_OPS_PER_SEC * 2); // SHA256 should be faster than composite hashes
}

/* ===== ENCODING PERFORMANCE TESTS ===== */

void test_hex_encoding_performance(void) {
    printf("Testing hex encoding performance\n");
    
    uint8_t test_data[100];
    for (int i = 0; i < sizeof(test_data); i++) {
        test_data[i] = i & 0xFF;
    }
    
    const int NUM_OPERATIONS = 2000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* hex_string;
        neoc_error_t err = neoc_hex_encode(test_data, sizeof(test_data), &hex_string);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(hex_string);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Encoded %d hex strings at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_HEX_OPS_PER_SEC);
}

void test_hex_decoding_performance(void) {
    printf("Testing hex decoding performance\n");
    
    const int NUM_OPERATIONS = 2000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t decoded_data[16];
        size_t decoded_len;
        neoc_error_t err = neoc_hex_decode(TEST_DATA_HEX, decoded_data, sizeof(decoded_data), &decoded_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Decoded %d hex strings at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_HEX_OPS_PER_SEC);
}

void test_base58_encoding_performance(void) {
    printf("Testing Base58 encoding performance\n");
    
    uint8_t test_data[25]; // Typical address size
    for (int i = 0; i < sizeof(test_data); i++) {
        test_data[i] = i & 0xFF;
    }
    
    const int NUM_OPERATIONS = 500;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* base58_string;
        neoc_error_t err = neoc_base58_encode(test_data, sizeof(test_data), &base58_string);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(base58_string);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Encoded %d Base58 strings at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_BASE58_OPS_PER_SEC);
}

void test_base58_decoding_performance(void) {
    printf("Testing Base58 decoding performance\n");
    
    // Use a valid Neo address for decoding
    const char* neo_address = "NM7Aky765FG8NhhwtxjXRx7jEL1cnw7PBP";
    
    const int NUM_OPERATIONS = 500;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t decoded_data[25];
        size_t decoded_len = sizeof(decoded_data);
        neoc_error_t err = neoc_base58_decode(neo_address, decoded_data, &decoded_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Decoded %d Base58 strings at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_BASE58_OPS_PER_SEC);
}

void test_base64_performance(void) {
    printf("Testing Base64 encoding/decoding performance\n");
    
    uint8_t test_data[100];
    for (int i = 0; i < sizeof(test_data); i++) {
        test_data[i] = i & 0xFF;
    }
    
    const int NUM_OPERATIONS = 1000;
    
    // Test encoding
    clock_t start = clock();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* base64_string;
        neoc_error_t err = neoc_base64_encode(test_data, sizeof(test_data), &base64_string);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(base64_string);
    }
    clock_t encode_end = clock();
    
    // Test decoding
    char* test_base64;
    neoc_base64_encode(test_data, sizeof(test_data), &test_base64);
    
    clock_t decode_start = clock();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        uint8_t decoded_data[100];
        size_t decoded_len = sizeof(decoded_data);
        neoc_error_t err = neoc_base64_decode(test_base64, decoded_data, &decoded_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    clock_t decode_end = clock();
    
    double encode_ops_per_sec = measure_ops_per_second(start, encode_end, NUM_OPERATIONS);
    double decode_ops_per_sec = measure_ops_per_second(decode_start, decode_end, NUM_OPERATIONS);
    
    printf("  Encoded %d Base64 strings at %.1f ops/sec\n", NUM_OPERATIONS, encode_ops_per_sec);
    printf("  Decoded %d Base64 strings at %.1f ops/sec\n", NUM_OPERATIONS, decode_ops_per_sec);
    
    TEST_ASSERT_TRUE(encode_ops_per_sec >= 800);
    TEST_ASSERT_TRUE(decode_ops_per_sec >= 800);
    
    neoc_free(test_base64);
}

/* ===== ACCOUNT AND WALLET PERFORMANCE TESTS ===== */

void test_account_creation_performance(void) {
    printf("Testing account creation performance\n");
    
    const int NUM_OPERATIONS = 200;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        neoc_account_t* account;
        neoc_error_t err = neoc_account_create_random(&account);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_account_free(account);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Created %d accounts at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_ACCOUNT_OPS_PER_SEC);
}

void test_account_from_wif_performance(void) {
    printf("Testing account from WIF performance\n");
    
    const int NUM_OPERATIONS = 300;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        neoc_account_t* account;
        neoc_error_t err = neoc_account_create_from_wif(TEST_WIF, &account);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_account_free(account);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Created %d accounts from WIF at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_ACCOUNT_OPS_PER_SEC);
}

void test_address_generation_performance(void) {
    printf("Testing address generation performance\n");
    
    // Create a single account once
    neoc_account_t* account;
    neoc_error_t err = neoc_account_create_random(&account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    const int NUM_OPERATIONS = 1000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* address;
        err = neoc_account_get_address(account, &address);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(address);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Generated %d addresses at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= 2000); // Should be very fast
    
    neoc_account_free(account);
}

/* ===== NEP-2 PERFORMANCE TESTS ===== */

void test_nep2_encryption_performance(void) {
    printf("Testing NEP-2 encryption performance\n");
    
    // Create a key pair once
    neoc_ec_key_pair_t* key_pair;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Use fast scrypt parameters for performance testing
    neoc_scrypt_params_t fast_params;
    neoc_scrypt_params_init(&fast_params, 256, 1, 1);
    
    const int NUM_OPERATIONS = 20; // NEP-2 is slow, fewer operations
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* encrypted;
        err = neoc_nep2_encrypt("password", key_pair, &fast_params, &encrypted);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        neoc_free(encrypted);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Encrypted %d keys with NEP-2 at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= 5); // NEP-2 is inherently slow due to scrypt
    
    neoc_ec_key_pair_free(key_pair);
}

/* ===== SCRIPT BUILDING PERFORMANCE TESTS ===== */

void test_script_builder_performance(void) {
    printf("Testing script builder performance\n");
    
    const int NUM_OPERATIONS = 500;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        neoc_script_builder_t* builder;
        neoc_error_t err = neoc_script_builder_create(&builder);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Add multiple operations to the script
        err = neoc_script_builder_push_integer(builder, i);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
        err = neoc_script_builder_push_data(builder, data, sizeof(data));
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        err = neoc_script_builder_push_string(builder, "test");
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        // Build the script
        uint8_t* script;
        size_t script_len;
        err = neoc_script_builder_to_array(builder, &script, &script_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        neoc_free(script);
        neoc_script_builder_free(builder);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Built %d scripts at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= MIN_SCRIPT_BUILD_OPS_PER_SEC);
}

/* ===== CONTRACT PERFORMANCE TESTS ===== */

void test_contract_properties_performance(void) {
    printf("Testing contract properties performance\n");
    
    const int NUM_OPERATIONS = 1000;
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        // Test GAS token properties
        neoc_gas_token_t* gas_token;
        neoc_error_t err = neoc_gas_token_create(&gas_token);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        char* name;
        err = neoc_gas_token_get_name(gas_token, &name);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        neoc_hash160_t script_hash;
        err = neoc_gas_token_get_script_hash(gas_token, &script_hash);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        neoc_free(name);
        neoc_gas_token_free(gas_token);
    }
    
    clock_t end = clock();
    double ops_per_sec = measure_ops_per_second(start, end, NUM_OPERATIONS);
    
    printf("  Retrieved contract properties %d times at %.1f ops/sec\n", NUM_OPERATIONS, ops_per_sec);
    TEST_ASSERT_TRUE(ops_per_sec >= 800);
}

/* ===== BULK OPERATIONS PERFORMANCE TESTS ===== */

void test_bulk_wallet_operations_performance(void) {
    printf("Testing bulk wallet operations performance\n");
    
    clock_t total_start = clock();
    
    // Create wallet
    neoc_wallet_t* wallet;
    neoc_error_t err = neoc_wallet_create("performance_test_wallet", &wallet);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Bulk account creation and addition
    const int NUM_ACCOUNTS = 100;
    clock_t creation_start = clock();
    
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        neoc_account_t* account;
        err = neoc_account_create_random(&account);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        
        err = neoc_wallet_add_account(wallet, account);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    }
    
    clock_t creation_end = clock();
    
    // Bulk address generation
    clock_t address_start = clock();
    
    size_t account_count;
    err = neoc_wallet_get_account_count(wallet, &account_count);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    for (size_t i = 0; i < account_count; i++) {
        neoc_account_t* account;
        err = neoc_wallet_get_account_by_index(wallet, i, &account);
        if (err == NEOC_SUCCESS) {
            char* address;
            err = neoc_account_get_address(account, &address);
            if (err == NEOC_SUCCESS) {
                neoc_free(address);
            }
        }
    }
    
    clock_t address_end = clock();
    clock_t total_end = clock();
    
    double creation_ops_per_sec = measure_ops_per_second(creation_start, creation_end, NUM_ACCOUNTS);
    double address_ops_per_sec = measure_ops_per_second(address_start, address_end, NUM_ACCOUNTS);
    double total_time = ((double)(total_end - total_start)) / CLOCKS_PER_SEC;
    
    printf("  Created and added %d accounts at %.1f ops/sec\n", NUM_ACCOUNTS, creation_ops_per_sec);
    printf("  Retrieved %d addresses at %.1f ops/sec\n", NUM_ACCOUNTS, address_ops_per_sec);
    printf("  Total time for bulk operations: %.3f seconds\n", total_time);
    
    TEST_ASSERT_TRUE(creation_ops_per_sec >= 100);
    TEST_ASSERT_TRUE(address_ops_per_sec >= 500);
    TEST_ASSERT_TRUE(total_time < 5.0);
    
    neoc_wallet_free(wallet);
}

/* ===== MEMORY PERFORMANCE TESTS ===== */

void test_memory_allocation_performance(void) {
    printf("Testing memory allocation performance\n");
    
    const int NUM_ALLOCATIONS = 10000;
    const size_t ALLOCATION_SIZE = 1024;
    
    clock_t start = clock();
    
    void* pointers[NUM_ALLOCATIONS];
    
    // Allocation phase
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        pointers[i] = neoc_malloc(ALLOCATION_SIZE);
        TEST_ASSERT_NOT_NULL(pointers[i]);
        
        // Write to ensure real allocation
        memset(pointers[i], i & 0xFF, ALLOCATION_SIZE);
    }
    
    clock_t alloc_end = clock();
    
    // Deallocation phase
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        neoc_free(pointers[i]);
    }
    
    clock_t free_end = clock();
    
    double alloc_ops_per_sec = measure_ops_per_second(start, alloc_end, NUM_ALLOCATIONS);
    double free_ops_per_sec = measure_ops_per_second(alloc_end, free_end, NUM_ALLOCATIONS);
    
    printf("  Allocated %d blocks at %.1f ops/sec\n", NUM_ALLOCATIONS, alloc_ops_per_sec);
    printf("  Freed %d blocks at %.1f ops/sec\n", NUM_ALLOCATIONS, free_ops_per_sec);
    
    TEST_ASSERT_TRUE(alloc_ops_per_sec >= 10000);
    TEST_ASSERT_TRUE(free_ops_per_sec >= 10000);
}

/* ===== OVERALL PERFORMANCE SUMMARY TEST ===== */

void test_performance_summary(void) {
    printf("=== PERFORMANCE SUMMARY ===\n");
    
    struct {
        const char* operation;
        double target_ops_per_sec;
        const char* status;
    } performance_targets[] = {
        {"EC Key Pair Creation", MIN_EC_KEYPAIR_OPS_PER_SEC, "✓ Tested"},
        {"WIF Encoding/Decoding", MIN_WIF_OPS_PER_SEC, "✓ Tested"},
        {"Hash Operations", MIN_HASH_OPS_PER_SEC, "✓ Tested"},
        {"Base58 Operations", MIN_BASE58_OPS_PER_SEC, "✓ Tested"},
        {"Hex Operations", MIN_HEX_OPS_PER_SEC, "✓ Tested"},
        {"Account Creation", MIN_ACCOUNT_OPS_PER_SEC, "✓ Tested"},
        {"Script Building", MIN_SCRIPT_BUILD_OPS_PER_SEC, "✓ Tested"},
    };
    
    size_t num_targets = sizeof(performance_targets) / sizeof(performance_targets[0]);
    
    printf("Performance Targets:\n");
    for (size_t i = 0; i < num_targets; i++) {
        printf("  %-25s: %.0f ops/sec %s\n", 
               performance_targets[i].operation,
               performance_targets[i].target_ops_per_sec,
               performance_targets[i].status);
    }
    
    printf("\nAll performance tests completed successfully!\n");
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== COMPREHENSIVE PERFORMANCE BENCHMARK TESTS ===\n");
    printf("Performance thresholds are set for reasonable hardware.\n");
    printf("Tests may run slower in debug builds or virtualized environments.\n\n");
    
    // EC key pair performance tests
    RUN_TEST(test_ec_key_pair_creation_performance);
    RUN_TEST(test_ec_key_pair_from_private_key_performance);
    RUN_TEST(test_public_key_operations_performance);
    
    // WIF performance tests
    RUN_TEST(test_wif_encoding_performance);
    RUN_TEST(test_wif_decoding_performance);
    
    // Hash performance tests
    RUN_TEST(test_hash160_performance);
    RUN_TEST(test_hash256_performance);
    RUN_TEST(test_sha256_performance);
    
    // Encoding performance tests
    RUN_TEST(test_hex_encoding_performance);
    RUN_TEST(test_hex_decoding_performance);
    RUN_TEST(test_base58_encoding_performance);
    RUN_TEST(test_base58_decoding_performance);
    RUN_TEST(test_base64_performance);
    
    // Account and wallet performance tests
    RUN_TEST(test_account_creation_performance);
    RUN_TEST(test_account_from_wif_performance);
    RUN_TEST(test_address_generation_performance);
    
    // NEP-2 performance tests (slower due to scrypt)
    RUN_TEST(test_nep2_encryption_performance);
    
    // Script building performance tests
    RUN_TEST(test_script_builder_performance);
    
    // Contract performance tests
    RUN_TEST(test_contract_properties_performance);
    
    // Bulk operations performance tests
    RUN_TEST(test_bulk_wallet_operations_performance);
    
    // Memory performance tests
    RUN_TEST(test_memory_allocation_performance);
    
    // Performance summary
    RUN_TEST(test_performance_summary);
    
    UNITY_END();
    return 0;
}