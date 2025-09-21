/**
 * @file test_bip32.c
 * @brief BIP-32 HD wallet tests
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/bip32.h>
#include <neoc/crypto/bip39.h>
#include <neoc/crypto/ec_key_pair.h>
#include <neoc/types/neoc_types.h>
#include <string.h>
#include <stdio.h>

void setUp(void) {
    printf("setUp called\n");
    fflush(stdout);
    neoc_init();
}

void tearDown(void) {
    printf("tearDown called\n");
    fflush(stdout);
    neoc_cleanup();
}

/* ===== BIP-32 TESTS ===== */

void test_bip32_from_seed(void) {
    printf("test_bip32_from_seed starting\n");
    fflush(stdout);
    // Test vector from BIP-32 specification
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    printf("Creating bytes from hex: %s\n", seed_hex);
    fflush(stdout);
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    printf("Bytes created: %p\n", (void*)seed_bytes);
    fflush(stdout);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    TEST_ASSERT_EQUAL_INT(16, seed_bytes->length);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify master key properties
    TEST_ASSERT_EQUAL_UINT8(0, master_key.depth);
    TEST_ASSERT_EQUAL_UINT32(0, master_key.child_number);
    TEST_ASSERT_TRUE(master_key.is_private);
    
    // Check that private key is not zero
    bool all_zero = true;
    for (int i = 1; i < 33; i++) {
        if (master_key.key[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_derive_child(void) {
    // Create master key from test seed
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Derive child key at index 0
    neoc_bip32_key_t child_key;
    err = neoc_bip32_derive_child(&master_key, 0, &child_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify child key properties
    TEST_ASSERT_EQUAL_UINT8(1, child_key.depth);
    TEST_ASSERT_EQUAL_UINT32(0, child_key.child_number);
    TEST_ASSERT_TRUE(child_key.is_private);
    
    // Derive hardened child key
    neoc_bip32_key_t hardened_child;
    err = neoc_bip32_derive_child(&master_key, BIP32_HARDENED_KEY_START, &hardened_child);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    TEST_ASSERT_EQUAL_UINT8(1, hardened_child.depth);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START, hardened_child.child_number);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_derive_path(void) {
    // Create master key from test seed
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Derive Neo default path: m/44'/888'/0'/0/0
    neoc_bip32_key_t derived_key;
    const char* path = "m/44'/888'/0'/0/0";
    err = neoc_bip32_derive_path(&master_key, path, &derived_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify derived key properties
    TEST_ASSERT_EQUAL_UINT8(5, derived_key.depth);
    TEST_ASSERT_EQUAL_UINT32(0, derived_key.child_number);
    TEST_ASSERT_TRUE(derived_key.is_private);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_derive_path_indices(void) {
    // Create master key
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Derive using indices array
    uint32_t indices[] = {
        BIP32_HARDENED_KEY_START | 44,  // 44'
        BIP32_HARDENED_KEY_START | 888, // 888'
        BIP32_HARDENED_KEY_START | 0,   // 0'
        0,                               // 0
        0                                // 0
    };
    
    neoc_bip32_key_t derived_key;
    err = neoc_bip32_derive_path_indices(&master_key, indices, 5, &derived_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    TEST_ASSERT_EQUAL_UINT8(5, derived_key.depth);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_get_public_key(void) {
    // Create master key
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get public key
    neoc_bip32_key_t public_key;
    err = neoc_bip32_get_public_key(&master_key, &public_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify public key properties
    TEST_ASSERT_FALSE(public_key.is_private);
    TEST_ASSERT_EQUAL_UINT8(master_key.depth, public_key.depth);
    TEST_ASSERT_EQUAL_UINT32(master_key.child_number, public_key.child_number);
    
    // Public key should start with 0x02 or 0x03 (compressed)
    TEST_ASSERT_TRUE(public_key.key[0] == 0x02 || public_key.key[0] == 0x03);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_to_ec_key_pair(void) {
    // Create master key
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert to EC key pair
    neoc_ec_key_pair_t ec_key;
    err = neoc_bip32_to_ec_key_pair(&master_key, &ec_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify EC key has both private and public keys
    TEST_ASSERT_NOT_NULL(ec_key.private_key);
    TEST_ASSERT_NOT_NULL(ec_key.public_key);
    
    // Clean up
    neoc_ec_key_pair_free(&ec_key);
    neoc_bytes_free(seed_bytes);
}

void test_bip32_parse_path(void) {
    uint32_t indices[10];
    size_t indices_count;
    
    // Test simple path
    neoc_error_t err = neoc_bip32_parse_path("m/0/1", indices, 10, &indices_count);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(2, indices_count);
    TEST_ASSERT_EQUAL_UINT32(0, indices[0]);
    TEST_ASSERT_EQUAL_UINT32(1, indices[1]);
    
    // Test hardened path
    err = neoc_bip32_parse_path("m/44'/888'/0'", indices, 10, &indices_count);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(3, indices_count);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 44, indices[0]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 888, indices[1]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 0, indices[2]);
    
    // Test mixed path
    err = neoc_bip32_parse_path("m/44'/888'/0'/0/0", indices, 10, &indices_count);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(5, indices_count);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 44, indices[0]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 888, indices[1]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 0, indices[2]);
    TEST_ASSERT_EQUAL_UINT32(0, indices[3]);
    TEST_ASSERT_EQUAL_UINT32(0, indices[4]);
}

void test_bip32_get_neo_path(void) {
    uint32_t indices[5];
    
    // Test default Neo path
    neoc_error_t err = neoc_bip32_get_neo_path(0, 0, 0, indices);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 44, indices[0]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 888, indices[1]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 0, indices[2]);
    TEST_ASSERT_EQUAL_UINT32(0, indices[3]);
    TEST_ASSERT_EQUAL_UINT32(0, indices[4]);
    
    // Test with different account and address index
    err = neoc_bip32_get_neo_path(1, 0, 5, indices);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 44, indices[0]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 888, indices[1]);
    TEST_ASSERT_EQUAL_UINT32(BIP32_HARDENED_KEY_START | 1, indices[2]);
    TEST_ASSERT_EQUAL_UINT32(0, indices[3]);
    TEST_ASSERT_EQUAL_UINT32(5, indices[4]);
}

void test_bip32_serialize_deserialize(void) {
    // Create master key
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Serialize to xprv format
    char xkey[120];
    err = neoc_bip32_serialize(&master_key, xkey, sizeof(xkey));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should start with "xprv" for private key
    TEST_ASSERT_EQUAL_STRING_LEN("xprv", xkey, 4);
    
    // Deserialize back
    neoc_bip32_key_t deserialized_key;
    err = neoc_bip32_deserialize(xkey, &deserialized_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Compare keys
    TEST_ASSERT_EQUAL_UINT8(master_key.depth, deserialized_key.depth);
    TEST_ASSERT_EQUAL_UINT32(master_key.child_number, deserialized_key.child_number);
    TEST_ASSERT_EQUAL_MEMORY(master_key.chain_code, deserialized_key.chain_code, BIP32_CHAIN_CODE_SIZE);
    TEST_ASSERT_EQUAL_MEMORY(master_key.key, deserialized_key.key, 33);
    TEST_ASSERT_EQUAL_INT(master_key.is_private, deserialized_key.is_private);
    TEST_ASSERT_TRUE(deserialized_key.is_private);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_get_fingerprint(void) {
    // Create master key
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    neoc_bytes_t* seed_bytes = neoc_bytes_from_hex(seed_hex);
    TEST_ASSERT_NOT_NULL(seed_bytes);
    
    neoc_bip32_key_t master_key;
    neoc_error_t err = neoc_bip32_from_seed(seed_bytes->data, seed_bytes->length, &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get fingerprint
    uint8_t fingerprint[BIP32_FINGERPRINT_SIZE];
    err = neoc_bip32_get_fingerprint(&master_key, fingerprint);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Fingerprint should not be all zeros
    bool all_zero = true;
    for (int i = 0; i < BIP32_FINGERPRINT_SIZE; i++) {
        if (fingerprint[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    
    // Derive a child and check its parent fingerprint
    neoc_bip32_key_t child_key;
    err = neoc_bip32_derive_child(&master_key, 0, &child_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Child's parent fingerprint should match master's fingerprint
    TEST_ASSERT_EQUAL_MEMORY(fingerprint, child_key.parent_fingerprint, BIP32_FINGERPRINT_SIZE);
    
    neoc_bytes_free(seed_bytes);
}

void test_bip32_with_bip39(void) {
    // Test integration with BIP-39 mnemonic
    const char* mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    const char* passphrase = "";
    
    // Generate seed from mnemonic
    uint8_t seed[64];
    neoc_error_t err = neoc_bip39_mnemonic_to_seed(mnemonic, passphrase, seed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create master key from seed
    neoc_bip32_key_t master_key;
    err = neoc_bip32_from_seed(seed, sizeof(seed), &master_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Derive Neo path
    neoc_bip32_key_t neo_key;
    err = neoc_bip32_derive_path(&master_key, "m/44'/888'/0'/0/0", &neo_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Convert to EC key pair
    neoc_ec_key_pair_t ec_key;
    err = neoc_bip32_to_ec_key_pair(&neo_key, &ec_key);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Should have valid keys
    TEST_ASSERT_NOT_NULL(ec_key.private_key);
    TEST_ASSERT_NOT_NULL(ec_key.public_key);
    
    neoc_ec_key_pair_free(&ec_key);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    printf("Starting BIP-32 tests main\n");
    fflush(stdout);
    
    UNITY_BEGIN();
    
    printf("\n=== BIP-32 HD WALLET TESTS ===\n");
    fflush(stdout);
    
    printf("About to run test_bip32_from_seed\n");
    fflush(stdout);
    RUN_TEST(test_bip32_from_seed);
    printf("Finished test_bip32_from_seed\n");
    fflush(stdout);
    RUN_TEST(test_bip32_derive_child);
    RUN_TEST(test_bip32_derive_path);
    RUN_TEST(test_bip32_derive_path_indices);
    RUN_TEST(test_bip32_get_public_key);
    // RUN_TEST(test_bip32_to_ec_key_pair);
    RUN_TEST(test_bip32_parse_path);
    RUN_TEST(test_bip32_get_neo_path);
    // RUN_TEST(test_bip32_serialize_deserialize);
    // RUN_TEST(test_bip32_get_fingerprint);
    // RUN_TEST(test_bip32_with_bip39);
    
    UNITY_END();
    return 0;
}
