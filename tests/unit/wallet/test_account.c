/**
 * @file test_account.c
 * @brief Unit tests converted from AccountTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/wallet/account.h"
#include "neoc/wallet/nep6_account.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/nep2.h"
#include "neoc/script/verification_script.h"
#include "neoc/types/hash160.h"
#include "neoc/utils/hex.h"

// Test data from TestProperties.swift
static const char *DEFAULT_ACCOUNT_ADDRESS = "NM7Aky765FG8NhhwtxjXRx7jEL1cnw7PBP";
static const char *DEFAULT_ACCOUNT_SCRIPT_HASH = "69ecca587293047be4c59159bf8bc399985c160d";
static const char *DEFAULT_ACCOUNT_VERIFICATION_SCRIPT = "0c21033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b4156e7b327";
static const char *DEFAULT_ACCOUNT_PUBLIC_KEY = "033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b";
static const char *DEFAULT_ACCOUNT_PRIVATE_KEY = "84180ac9d6eb6fba207ea4ef9d2200102d1ebeb4b9c07e2c6a738a42742e27a5";
static const char *DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY = "6PYM7jHL4GmS8Aw2iEFpuaHTCUKjhT4mwVqdoozGU6sUE25BjV4ePXDdLz";
static const char *DEFAULT_ACCOUNT_WIF = "L1eV34wPoj9weqhGijdDLtVQzUpWGHszXXpdU9dPuh2nRFFzFa7E";
static const char *DEFAULT_ACCOUNT_PASSWORD = "neo";

static const char *COMMITTEE_ACCOUNT_ADDRESS = "NXXazKH39yNFWWZF5MJ8tEN98VYHwzn7g3";
static const char *COMMITTEE_ACCOUNT_SCRIPT_HASH = "05859de95ccbbd5668e0f055b208273634d4657f";
static const char *COMMITTEE_ACCOUNT_VERIFICATION_SCRIPT = "110c21033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b11419ed0dc3a";

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Test create generic account
static void test_create_generic_account(void) {
    printf("Testing create generic account...\n");
    
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Check account properties
    const char *address = neoc_account_get_address(account);
    assert(address != NULL);
    assert(strlen(address) > 0);
    
    neoc_verification_script_t *script = neoc_account_get_verification_script(account);
    assert(script != NULL);
    
    neoc_ec_key_pair_t *key_pair = neoc_account_get_key_pair(account);
    assert(key_pair != NULL);
    
    const char *label = neoc_account_get_label(account);
    assert(label != NULL);
    
    const char *encrypted = neoc_account_get_encrypted_private_key(account);
    assert(encrypted == NULL);
    
    assert(neoc_account_is_locked(account) == false);
    assert(neoc_account_is_default(account) == false);
    
    neoc_account_free(account);
    printf("  ✅ Create generic account test passed\n");
}

// Test init account from existing key pair
static void test_init_account_from_key_pair(void) {
    printf("Testing init account from existing key pair...\n");
    
    // Create key pair from private key
    uint8_t private_key[32];
    size_t private_key_len = 0;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, private_key, sizeof(private_key), &private_key_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_key_pair_t *key_pair = NULL;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    assert(err == NEOC_SUCCESS);
    
    // Create account from key pair
    neoc_account_t *account = NULL;
    err = neoc_account_create_from_key_pair(key_pair, &account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Verify account properties
    assert(neoc_account_is_multi_sig(account) == false);
    
    const char *address = neoc_account_get_address(account);
    assert(strcmp(address, DEFAULT_ACCOUNT_ADDRESS) == 0);
    
    const char *label = neoc_account_get_label(account);
    assert(strcmp(label, DEFAULT_ACCOUNT_ADDRESS) == 0);
    
    neoc_verification_script_t *script = neoc_account_get_verification_script(account);
    assert(script != NULL);
    
    uint8_t expected_script[256];
    size_t expected_script_len = 0;
    err = neoc_hex_decode(DEFAULT_ACCOUNT_VERIFICATION_SCRIPT, expected_script, sizeof(expected_script), &expected_script_len);
    assert(err == NEOC_SUCCESS);
    
    size_t script_len = 0;
    const uint8_t *script_bytes = neoc_verification_script_get_script(script, &script_len);
    assert(script_len == expected_script_len);
    assert(memcmp(script_bytes, expected_script, script_len) == 0);
    
    neoc_ec_key_pair_free(key_pair);
    neoc_account_free(account);
    printf("  ✅ Init account from key pair test passed\n");
}

// Test account from verification script
static void test_from_verification_script(void) {
    printf("Testing account from verification script...\n");
    
    // Create verification script
    const char *hex_script = "0c2102163946a133e3d2e0d987fb90cb01b060ed1780f1718e2da28edf13b965fd2b600b4195440d78";
    uint8_t script_bytes[256];
    size_t script_len = 0;
    neoc_error_t err = neoc_hex_decode(hex_script, script_bytes, sizeof(script_bytes), &script_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_verification_script_t *script = NULL;
    err = neoc_verification_script_create(script_bytes, script_len, &script);
    assert(err == NEOC_SUCCESS);
    
    // Create account from verification script
    neoc_account_t *account = NULL;
    err = neoc_account_from_verification_script(script, &account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Verify address
    const char *address = neoc_account_get_address(account);
    assert(strcmp(address, "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj") == 0);
    
    neoc_verification_script_free(script);
    neoc_account_free(account);
    printf("  ✅ Account from verification script test passed\n");
}

// Test account from public key
static void test_from_public_key(void) {
    printf("Testing account from public key...\n");
    
    // Create public key
    uint8_t public_key_bytes[33];
    size_t public_key_len = 0;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PUBLIC_KEY, public_key_bytes, sizeof(public_key_bytes), &public_key_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_public_key_t *public_key = NULL;
    err = neoc_ec_public_key_from_bytes(public_key_bytes, public_key_len, &public_key);
    assert(err == NEOC_SUCCESS);
    
    // Create account from public key
    neoc_account_t *account = NULL;
    err = neoc_account_from_public_key(public_key, &account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Verify address
    const char *address = neoc_account_get_address(account);
    assert(strcmp(address, DEFAULT_ACCOUNT_ADDRESS) == 0);
    
    neoc_ec_public_key_free(public_key);
    neoc_account_free(account);
    printf("  ✅ Account from public key test passed\n");
}

// Test create multi-sig account from public keys
static void test_create_multi_sig_account_from_public_keys(void) {
    printf("Testing create multi-sig account from public keys...\n");
    
    // Create public key
    uint8_t public_key_bytes[33];
    size_t public_key_len = 0;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PUBLIC_KEY, public_key_bytes, sizeof(public_key_bytes), &public_key_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_public_key_t *public_key = NULL;
    err = neoc_ec_public_key_from_bytes(public_key_bytes, public_key_len, &public_key);
    assert(err == NEOC_SUCCESS);
    
    // Create multi-sig account
    neoc_ec_public_key_t *keys[1] = {public_key};
    neoc_account_t *account = NULL;
    err = neoc_account_create_multi_sig(keys, 1, 1, &account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Verify multi-sig properties
    assert(neoc_account_is_multi_sig(account) == true);
    
    const char *address = neoc_account_get_address(account);
    assert(strcmp(address, COMMITTEE_ACCOUNT_ADDRESS) == 0);
    
    neoc_ec_public_key_free(public_key);
    neoc_account_free(account);
    printf("  ✅ Create multi-sig account from public keys test passed\n");
}

// Test encrypt private key
static void test_encrypt_private_key(void) {
    printf("Testing encrypt private key...\n");
    
    // Create key pair
    uint8_t private_key[32];
    size_t private_key_len = 0;
    neoc_error_t err = neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, private_key, sizeof(private_key), &private_key_len);
    assert(err == NEOC_SUCCESS);
    
    neoc_ec_key_pair_t *key_pair = NULL;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    assert(err == NEOC_SUCCESS);
    
    // Create account
    neoc_account_t *account = NULL;
    err = neoc_account_create_from_key_pair(key_pair, &account);
    assert(err == NEOC_SUCCESS);
    
    // Encrypt private key
    err = neoc_account_encrypt_private_key(account, DEFAULT_ACCOUNT_PASSWORD);
    assert(err == NEOC_SUCCESS);
    
    // Verify encrypted key
    const char *encrypted = neoc_account_get_encrypted_private_key(account);
    assert(encrypted != NULL);
    assert(strcmp(encrypted, DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY) == 0);
    
    neoc_ec_key_pair_free(key_pair);
    neoc_account_free(account);
    printf("  ✅ Encrypt private key test passed\n");
}

// Test fail encrypt account without private key
static void test_fail_encrypt_without_private_key(void) {
    printf("Testing fail encrypt account without private key...\n");
    
    // Create account from address only
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_from_address(DEFAULT_ACCOUNT_ADDRESS, &account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Try to encrypt - should fail
    err = neoc_account_encrypt_private_key(account, "password");
    assert(err != NEOC_SUCCESS);
    
    neoc_account_free(account);
    printf("  ✅ Fail encrypt without private key test passed\n");
}

// Test decrypt private key
static void test_decrypt_private_key(void) {
    printf("Testing decrypt private key...\n");
    
    // Create NEP6 account with encrypted key
    neoc_nep6_account_t *nep6 = NULL;
    neoc_error_t err = neoc_nep6_account_create(&nep6);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_nep6_account_set_address(nep6, DEFAULT_ACCOUNT_ADDRESS);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_nep6_account_set_key(nep6, DEFAULT_ACCOUNT_ENCRYPTED_PRIVATE_KEY);
    assert(err == NEOC_SUCCESS);
    
    // Create account from NEP6
    neoc_account_t *account = NULL;
    err = neoc_account_from_nep6(nep6, &account);
    assert(err == NEOC_SUCCESS);
    
    // Decrypt private key
    err = neoc_account_decrypt_private_key(account, DEFAULT_ACCOUNT_PASSWORD);
    assert(err == NEOC_SUCCESS);
    
    // Verify decrypted key
    neoc_ec_key_pair_t *key_pair = neoc_account_get_key_pair(account);
    assert(key_pair != NULL);
    
    neoc_nep6_account_free(nep6);
    neoc_account_free(account);
    printf("  ✅ Decrypt private key test passed\n");
}

int main(void) {
    printf("\n=== AccountTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_create_generic_account();
    test_init_account_from_key_pair();
    test_from_verification_script();
    test_from_public_key();
    test_create_multi_sig_account_from_public_keys();
    test_encrypt_private_key();
    test_fail_encrypt_without_private_key();
    test_decrypt_private_key();
    
    tearDown();
    
    printf("\n✅ All AccountTests tests passed!\n\n");
    return 0;
}
