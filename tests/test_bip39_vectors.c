/**
 * @file test_bip39_vectors.c
 * @brief Official BIP-39 test vectors
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include "unity.h"
#include "neoc/neoc.h"
#include "neoc/crypto/bip39.h"

void setUp(void) {
    neoc_error_t result = neoc_init();
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, result);
}

void tearDown(void) {
    neoc_cleanup();
}

// Official BIP-39 test vectors from:
// https://github.com/trezor/python-mnemonic/blob/master/vectors.json

void test_vector_1(void) {
    // Test vector #1
    const char *mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    const char *passphrase = "TREZOR";
    
    uint8_t seed[64];
    neoc_error_t err = neoc_bip39_mnemonic_to_seed(mnemonic, passphrase, seed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Expected seed
    uint8_t expected[64] = {
        0xc5, 0x52, 0x57, 0xc3, 0x60, 0xc0, 0x7c, 0x72,
        0x02, 0x9a, 0xeb, 0xc1, 0xb5, 0x3c, 0x05, 0xed,
        0x03, 0x62, 0xad, 0xa3, 0x8e, 0xad, 0x3e, 0x3e,
        0x9e, 0xfa, 0x37, 0x08, 0xe5, 0x34, 0x95, 0x53,
        0x1f, 0x09, 0xa6, 0x98, 0x75, 0x99, 0xd1, 0x82,
        0x64, 0xc1, 0xe1, 0xc9, 0x2f, 0x2c, 0xf1, 0x41,
        0x63, 0x0c, 0x7a, 0x3c, 0x4a, 0xb7, 0xc8, 0x1b,
        0x2f, 0x00, 0x16, 0x98, 0xe7, 0x46, 0x3b, 0x04
    };
    
    // Print actual vs expected for debugging
    printf("\nTest Vector 1 (with passphrase 'TREZOR'):\n");
    printf("Actual:   ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", seed[i]);
    }
    printf("\nExpected: ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", expected[i]);
    }
    printf("\n");
    
    TEST_ASSERT_EQUAL_MEMORY(expected, seed, sizeof(expected));
}

void test_vector_2(void) {
    // Test vector #2 (no passphrase)
    const char *mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    
    uint8_t seed[64];
    neoc_error_t err = neoc_bip39_mnemonic_to_seed(mnemonic, "", seed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Expected seed (no passphrase)
    uint8_t expected[64] = {
        0x5e, 0xb0, 0x0b, 0xbd, 0xdc, 0xf0, 0x69, 0x08,
        0x48, 0x89, 0xa8, 0xab, 0x91, 0x55, 0x56, 0x81,
        0x65, 0xf5, 0xc4, 0x53, 0xcc, 0xb8, 0x5e, 0x70,
        0x81, 0x1a, 0xae, 0xd6, 0xf6, 0xda, 0x5f, 0xc1,
        0x9a, 0x5a, 0xc4, 0x0b, 0x38, 0x9c, 0xd3, 0x70,
        0xd0, 0x86, 0x20, 0x6d, 0xec, 0x8a, 0xa6, 0xc4,
        0x3d, 0xae, 0xa6, 0x69, 0x0f, 0x20, 0xad, 0x3d,
        0x8d, 0x48, 0xb2, 0xd2, 0xce, 0x9e, 0x38, 0xe4
    };
    
    printf("\nTest Vector 2 (no passphrase):\n");
    printf("Actual:   ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", seed[i]);
    }
    printf("\nExpected: ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", expected[i]);
    }
    printf("\n");

    TEST_ASSERT_EQUAL_MEMORY(expected, seed, sizeof(expected));
}

void test_vector_3(void) {
    // Test vector #3
    const char *mnemonic = "legal winner thank year wave sausage worth useful legal winner thank yellow";
    
    uint8_t seed[64];
    neoc_error_t err = neoc_bip39_mnemonic_to_seed(mnemonic, "TREZOR", seed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Expected seed
    uint8_t expected[64] = {
        0x2e, 0x89, 0x05, 0x81, 0x9b, 0x87, 0x23, 0xfe,
        0x2c, 0x1d, 0x16, 0x18, 0x60, 0xe5, 0xee, 0x18,
        0x30, 0x31, 0x8d, 0xbf, 0x49, 0xa8, 0x3b, 0xd4,
        0x51, 0xcf, 0xb8, 0x44, 0x0c, 0x28, 0xbd, 0x6f,
        0xa4, 0x57, 0xfe, 0x12, 0x96, 0x10, 0x65, 0x59,
        0xa3, 0xc8, 0x09, 0x37, 0xa1, 0xc1, 0x06, 0x9b,
        0xe3, 0xa3, 0xa5, 0xbd, 0x38, 0x1e, 0xe6, 0x26,
        0x0e, 0x8d, 0x97, 0x39, 0xfc, 0xe1, 0xf6, 0x07
    };
    
    printf("\nTest Vector 3:\n");
    printf("Actual:   ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", seed[i]);
    }
    printf("\nExpected: ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", expected[i]);
    }
    printf("\n");

    TEST_ASSERT_EQUAL_MEMORY(expected, seed, sizeof(expected));
}

void test_direct_pbkdf2(void) {
    // Test PBKDF2 directly to isolate the issue
    const char *password = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    const char *salt = "mnemonicTREZOR";
    
    uint8_t seed[64];
    
    int result = PKCS5_PBKDF2_HMAC(
        password, strlen(password),
        (unsigned char*)salt, strlen(salt),
        2048,  // iterations
        EVP_sha512(),
        64,    // key length
        seed
    );
    
    TEST_ASSERT_EQUAL_INT(1, result);
    
    printf("\nDirect PBKDF2 test:\n");
    printf("Password: %s\n", password);
    printf("Salt: %s\n", salt);
    printf("Result: ");
    for (int i = 0; i < 64; i++) {
        printf("%02x", seed[i]);
    }
    printf("\n");

    // Expected matches test vector #1 seed (mnemonic + TREZOR)
    uint8_t expected[64] = {
        0xc5, 0x52, 0x57, 0xc3, 0x60, 0xc0, 0x7c, 0x72,
        0x02, 0x9a, 0xeb, 0xc1, 0xb5, 0x3c, 0x05, 0xed,
        0x03, 0x62, 0xad, 0xa3, 0x8e, 0xad, 0x3e, 0x3e,
        0x9e, 0xfa, 0x37, 0x08, 0xe5, 0x34, 0x95, 0x53,
        0x1f, 0x09, 0xa6, 0x98, 0x75, 0x99, 0xd1, 0x82,
        0x64, 0xc1, 0xe1, 0xc9, 0x2f, 0x2c, 0xf1, 0x41,
        0x63, 0x0c, 0x7a, 0x3c, 0x4a, 0xb7, 0xc8, 0x1b,
        0x2f, 0x00, 0x16, 0x98, 0xe7, 0x46, 0x3b, 0x04
    };

    TEST_ASSERT_EQUAL_MEMORY(expected, seed, sizeof(expected));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_vector_1);
    RUN_TEST(test_vector_2);
    RUN_TEST(test_vector_3);
    RUN_TEST(test_direct_pbkdf2);
    
    UNITY_END();
}
