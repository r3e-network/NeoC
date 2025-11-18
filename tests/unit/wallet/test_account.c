/**
 * @file test_account.c
 * @brief Modernized account unit tests derived from AccountTests.swift.
 */

#include "unity.h"
#include <string.h>

#include "neoc/neoc.h"
#include "neoc/neoc_memory.h"
#include "neoc/neo_constants.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/script/verification_script.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/hex.h"
#include "neoc/wallet/account.h"

static const char *DEFAULT_ACCOUNT_ADDRESS = "NM7Aky765FG8NhhwtxjXRx7jEL1cnw7PBP";
static const char *DEFAULT_ACCOUNT_VERIFICATION_SCRIPT =
    "0c21033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b4156e7b327";
static const char *DEFAULT_ACCOUNT_PUBLIC_KEY =
    "033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b";
static const char *DEFAULT_ACCOUNT_PRIVATE_KEY =
    "84180ac9d6eb6fba207ea4ef9d2200102d1ebeb4b9c07e2c6a738a42742e27a5";
static const char *DEFAULT_MULTISIG_SCRIPT_HASH = "05859de95ccbbd5668e0f055b208273634d4657f";

static void expect_hash160_eq(const neoc_hash160_t *hash, const char *expected_hex) {
    char buffer[NEOC_HASH160_STRING_LENGTH];
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_to_string(hash, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING(expected_hex, buffer);
}

static neoc_ec_public_key_t *public_key_from_hex(const char *hex) {
    uint8_t bytes[NEOC_PUBLIC_KEY_SIZE_UNCOMPRESSED];
    size_t len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hex_decode(hex, bytes, sizeof(bytes), &len));
    neoc_ec_public_key_t *key = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_ec_public_key_from_bytes(bytes, len, &key));
    return key;
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

static void test_create_generic_account(void) {
    neoc_account_t *account = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_create(&account));
    TEST_ASSERT_NOT_NULL(account);

    char *address = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_address(account, &address));
    TEST_ASSERT(address && strlen(address) > 0);

    uint8_t *script_bytes = NULL;
    size_t script_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_account_get_verification_script(account, &script_bytes, &script_len));
    TEST_ASSERT(script_bytes && script_len > 0);

    char *label = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_label(account, &label));
    TEST_ASSERT(label && strlen(label) > 0);

    TEST_ASSERT_NULL(neoc_account_get_encrypted_private_key(account));
    TEST_ASSERT_FALSE(neoc_account_is_locked(account));
    TEST_ASSERT_FALSE(neoc_account_is_default(account));
    TEST_ASSERT_NOT_NULL(neoc_account_get_key_pair(account));

    neoc_free(script_bytes);
    neoc_free(label);
    neoc_free(address);
    neoc_account_free(account);
}

static void test_init_account_from_key_pair(void) {
    uint8_t priv[32];
    size_t priv_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hex_decode(DEFAULT_ACCOUNT_PRIVATE_KEY, priv, sizeof(priv), &priv_len));

    neoc_ec_key_pair_t *key_pair = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_ec_key_pair_create_from_private_key(priv, &key_pair));

    neoc_account_t *account = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_create_from_key_pair(key_pair, &account));
    TEST_ASSERT_NOT_NULL(account);

    char *address = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_address(account, &address));
    TEST_ASSERT_EQUAL_STRING(DEFAULT_ACCOUNT_ADDRESS, address);

    char *label = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_label(account, &label));
    TEST_ASSERT_EQUAL_STRING(DEFAULT_ACCOUNT_ADDRESS, label);

    uint8_t *script_bytes = NULL;
    size_t script_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_account_get_verification_script(account, &script_bytes, &script_len));

    uint8_t expected[256];
    size_t expected_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_hex_decode(DEFAULT_ACCOUNT_VERIFICATION_SCRIPT, expected, sizeof(expected),
                                          &expected_len));
    TEST_ASSERT_EQUAL_UINT(expected_len, script_len);
    TEST_ASSERT_EQUAL_MEMORY(expected, script_bytes, script_len);

    neoc_hash160_t script_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_script_hash(account, &script_hash));
    TEST_ASSERT_TRUE(neoc_hash160_equal(&script_hash, &script_hash)); // ensure callable

    neoc_free(script_bytes);
    neoc_free(label);
    neoc_free(address);
    neoc_account_free(account);
    neoc_ec_key_pair_free(key_pair);
}

static void test_account_from_verification_script(void) {
    const char *hex_script =
        "0c2102163946a133e3d2e0d987fb90cb01b060ed1780f1718e2da28edf13b965fd2b600b4195440d78";
    uint8_t script[256];
    size_t script_len = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hex_decode(hex_script, script, sizeof(script), &script_len));

    neoc_account_t *account = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_account_create_from_verification_script(script, script_len, &account));
    TEST_ASSERT_NOT_NULL(account);

    char *address = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_address(account, &address));
    TEST_ASSERT_EQUAL_STRING("NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj", address);

    neoc_free(address);
    neoc_account_free(account);
}

static void test_account_from_public_key(void) {
    neoc_ec_public_key_t *public_key = public_key_from_hex(DEFAULT_ACCOUNT_PUBLIC_KEY);

    neoc_account_t *account = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_from_public_key(public_key, &account));
    TEST_ASSERT_NOT_NULL(account);

    char *address = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_address(account, &address));
    TEST_ASSERT_EQUAL_STRING(DEFAULT_ACCOUNT_ADDRESS, address);

    neoc_hash160_t script_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_script_hash(account, &script_hash));

    neoc_free(address);
    neoc_account_free(account);
    neoc_ec_public_key_free(public_key);
}

static void test_create_multisig_account(void) {
    neoc_ec_public_key_t *public_key = public_key_from_hex(DEFAULT_ACCOUNT_PUBLIC_KEY);
    neoc_ec_public_key_t *keys[1] = { public_key };

    neoc_account_t *account = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_create_multisig(1, keys, 1, &account));
    TEST_ASSERT_NOT_NULL(account);
    TEST_ASSERT_TRUE(neoc_account_is_multi_sig(account));

    neoc_hash160_t script_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_account_get_script_hash(account, &script_hash));
    expect_hash160_eq(&script_hash, DEFAULT_MULTISIG_SCRIPT_HASH);

    neoc_account_free(account);
    neoc_ec_public_key_free(public_key);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_generic_account);
    RUN_TEST(test_init_account_from_key_pair);
    RUN_TEST(test_account_from_verification_script);
    RUN_TEST(test_account_from_public_key);
    RUN_TEST(test_create_multisig_account);
    UNITY_END();
}
