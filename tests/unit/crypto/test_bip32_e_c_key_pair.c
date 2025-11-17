#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/crypto/bip32.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/utils/neoc_base58.h"

static const uint8_t TEST_SEED[32] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
};

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

static void derive_and_roundtrip(const neoc_bip32_key_t *key) {
    char buffer[128] = {0};

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_serialize(key, buffer, sizeof(buffer)));
    neoc_bip32_key_t decoded = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_deserialize(buffer, &decoded));
    TEST_ASSERT_EQUAL_MEMORY(key, &decoded, sizeof(neoc_bip32_key_t));

    neoc_bip32_key_t public_key;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_get_public_key(key, &public_key));
    memset(buffer, 0, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_serialize(&public_key, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_deserialize(buffer, &decoded));
    TEST_ASSERT_EQUAL_MEMORY(&public_key, &decoded, sizeof(neoc_bip32_key_t));
}

void test_bip32_master_and_child_derivation(void) {
    neoc_bip32_key_t master = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_from_seed(TEST_SEED, sizeof(TEST_SEED), &master));
    derive_and_roundtrip(&master);

    neoc_bip32_key_t child = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_derive_child(&master,
                                                  BIP32_HARDENED_KEY_START,
                                                  &child));
    derive_and_roundtrip(&child);
}

void test_bip32_parse_path_and_public_conversion(void) {
    const char *path = "m/44'/888'/0'/0/5";
    uint32_t indices[8];
    size_t count = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_parse_path(path, indices, 8, &count));
    TEST_ASSERT_EQUAL_UINT(5, count);
    TEST_ASSERT_EQUAL_UINT(BIP32_HARDENED_KEY_START + 44, indices[0]);
    TEST_ASSERT_EQUAL_UINT(5, indices[4]);

    neoc_bip32_key_t master = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_from_seed(TEST_SEED, sizeof(TEST_SEED), &master));

    neoc_bip32_key_t derived = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_derive_path_indices(&master, indices, count, &derived));

    neoc_bip32_key_t public_only = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_get_public_key(&derived, &public_only));
    TEST_ASSERT_FALSE(public_only.is_private);
}

void test_bip32_to_ec_key_pair(void) {
    neoc_bip32_key_t master = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_from_seed(TEST_SEED, sizeof(TEST_SEED), &master));

    neoc_ec_key_pair_t *pair = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bip32_to_ec_key_pair(&master, &pair));
    TEST_ASSERT_NOT_NULL(pair);

    char *wif = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_ec_key_pair_export_as_wif(pair, &wif));
    TEST_ASSERT_NOT_NULL(wif);
    neoc_free(wif);

    neoc_ec_key_pair_free(pair);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_bip32_master_and_child_derivation);
    RUN_TEST(test_bip32_parse_path_and_public_conversion);
    RUN_TEST(test_bip32_to_ec_key_pair);
    UNITY_END();
}
