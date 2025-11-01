#include <unity.h>
#include <neoc/crypto/bip39.h>
#include <neoc/crypto/neoc_hash.h>
#include <string.h>
#include <stdlib.h>

void setUp(void) {
    // Initialize crypto system for BIP-39 operations
    neoc_crypto_init();
}

void tearDown(void) {
    // Cleanup code
}

void test_bip39_word_count(void) {
    TEST_ASSERT_EQUAL_INT(12, neoc_bip39_get_word_count(NEOC_BIP39_STRENGTH_128));
    TEST_ASSERT_EQUAL_INT(15, neoc_bip39_get_word_count(NEOC_BIP39_STRENGTH_160));
    TEST_ASSERT_EQUAL_INT(18, neoc_bip39_get_word_count(NEOC_BIP39_STRENGTH_192));
    TEST_ASSERT_EQUAL_INT(21, neoc_bip39_get_word_count(NEOC_BIP39_STRENGTH_224));
    TEST_ASSERT_EQUAL_INT(24, neoc_bip39_get_word_count(NEOC_BIP39_STRENGTH_256));
}

void test_bip39_get_word(void) {
    // Test first word
    const char *word = neoc_bip39_get_word(NEOC_BIP39_LANG_ENGLISH, 0);
    TEST_ASSERT_NOT_NULL(word);
    TEST_ASSERT_EQUAL_STRING("abandon", word);
    
    // Test last word (index 2047 for 2048 words)
    word = neoc_bip39_get_word(NEOC_BIP39_LANG_ENGLISH, 2047);
    TEST_ASSERT_NOT_NULL(word);
    TEST_ASSERT_EQUAL_STRING("zoo", word);  // Verified against official BIP-39 wordlist
    
    // Test middle word
    word = neoc_bip39_get_word(NEOC_BIP39_LANG_ENGLISH, 1000);
    TEST_ASSERT_NOT_NULL(word);
    
    // Test invalid index
    word = neoc_bip39_get_word(NEOC_BIP39_LANG_ENGLISH, 2048);
    TEST_ASSERT_NULL(word);
}

void test_bip39_find_word(void) {
    // Test finding first word
    int index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "abandon");
    TEST_ASSERT_EQUAL_INT(0, index);
    
    // Test finding last word
    index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "zoo");
    TEST_ASSERT_EQUAL_INT(2047, index);
    
    // Verify additional BIP-39 words at known positions
    index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "able");
    TEST_ASSERT_EQUAL_INT(2, index);
    
    index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "year");
    TEST_ASSERT_EQUAL_INT(2039, index);
    
    index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "work");
    TEST_ASSERT_EQUAL_INT(2028, index);
    
    // Test finding middle words
    index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "ability");
    TEST_ASSERT_EQUAL_INT(1, index);
    
    // Test invalid word
    index = neoc_bip39_find_word(NEOC_BIP39_LANG_ENGLISH, "notaword");
    TEST_ASSERT_EQUAL_INT(-1, index);
}

void test_bip39_mnemonic_from_entropy(void) {
    // Test vector from BIP-39 spec
    uint8_t entropy[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    char *mnemonic = NULL;
    neoc_error_t err = neoc_bip39_mnemonic_from_entropy(entropy, 16, 
                                                         NEOC_BIP39_LANG_ENGLISH, 
                                                         &mnemonic);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(mnemonic);
    
    // Expected mnemonic for all zeros entropy
    const char *expected = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    TEST_ASSERT_EQUAL_STRING(expected, mnemonic);
    
    free(mnemonic);
}

void test_bip39_generate_mnemonic(void) {
    char *mnemonic = NULL;
    
    // Test 128-bit strength (12 words)
    neoc_error_t err = neoc_bip39_generate_mnemonic(NEOC_BIP39_STRENGTH_128,
                                                     NEOC_BIP39_LANG_ENGLISH,
                                                     &mnemonic);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(mnemonic);
    
    // Count words
    int word_count = 1;
    for (char *p = mnemonic; *p; p++) {
        if (*p == ' ') word_count++;
    }
    TEST_ASSERT_EQUAL_INT(12, word_count);
    
    free(mnemonic);
    
    // Test 256-bit strength (24 words)
    err = neoc_bip39_generate_mnemonic(NEOC_BIP39_STRENGTH_256,
                                        NEOC_BIP39_LANG_ENGLISH,
                                        &mnemonic);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(mnemonic);
    
    word_count = 1;
    for (char *p = mnemonic; *p; p++) {
        if (*p == ' ') word_count++;
    }
    TEST_ASSERT_EQUAL_INT(24, word_count);
    
    free(mnemonic);
}

void test_bip39_mnemonic_to_seed(void) {
    // Test vector from BIP-39 spec
    const char *mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    const char *passphrase = "TREZOR";
    
    uint8_t seed[64];
    neoc_error_t err = neoc_bip39_mnemonic_to_seed(mnemonic, passphrase, seed);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Expected seed from BIP-39 test vectors
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
    
    TEST_ASSERT_EQUAL_MEMORY(expected, seed, 64);
}

void test_bip39_mnemonic_to_entropy(void) {
    // Test with known mnemonic
    const char *mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    
    uint8_t *entropy = NULL;
    size_t entropy_len = 0;
    
    neoc_error_t err = neoc_bip39_mnemonic_to_entropy(mnemonic, 
                                                       NEOC_BIP39_LANG_ENGLISH,
                                                       &entropy, &entropy_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(entropy);
    TEST_ASSERT_EQUAL_INT(16, entropy_len);
    
    // Expected entropy (all zeros)
    uint8_t expected[16] = {0};
    TEST_ASSERT_EQUAL_MEMORY(expected, entropy, 16);
    
    free(entropy);
}

void test_bip39_validate_mnemonic(void) {
    // Valid mnemonic
    const char *valid = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    TEST_ASSERT_TRUE(neoc_bip39_validate_mnemonic(valid, NEOC_BIP39_LANG_ENGLISH));
    
    // Invalid checksum
    const char *invalid_checksum = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";
    TEST_ASSERT_FALSE(neoc_bip39_validate_mnemonic(invalid_checksum, NEOC_BIP39_LANG_ENGLISH));
    
    // Invalid word
    const char *invalid_word = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon notaword";
    TEST_ASSERT_FALSE(neoc_bip39_validate_mnemonic(invalid_word, NEOC_BIP39_LANG_ENGLISH));
    
    // Invalid word count
    const char *invalid_count = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";
    TEST_ASSERT_FALSE(neoc_bip39_validate_mnemonic(invalid_count, NEOC_BIP39_LANG_ENGLISH));
}

void test_bip39_entropy_roundtrip(void) {
    // Generate random mnemonic
    char *mnemonic = NULL;
    neoc_error_t err = neoc_bip39_generate_mnemonic(NEOC_BIP39_STRENGTH_128,
                                                     NEOC_BIP39_LANG_ENGLISH,
                                                     &mnemonic);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(mnemonic);
    
    // Convert to entropy
    uint8_t *entropy = NULL;
    size_t entropy_len = 0;
    err = neoc_bip39_mnemonic_to_entropy(mnemonic, NEOC_BIP39_LANG_ENGLISH,
                                          &entropy, &entropy_len);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(entropy);
    TEST_ASSERT_EQUAL_INT(16, entropy_len);
    
    // Convert back to mnemonic
    char *mnemonic2 = NULL;
    err = neoc_bip39_mnemonic_from_entropy(entropy, entropy_len,
                                            NEOC_BIP39_LANG_ENGLISH,
                                            &mnemonic2);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(mnemonic2);
    
    // Should match original
    TEST_ASSERT_EQUAL_STRING(mnemonic, mnemonic2);
    
    free(mnemonic);
    free(mnemonic2);
    free(entropy);
}

void test_bip39_entropy_roundtrip_all_strengths(void) {
    const size_t lengths[] = {16, 20, 24, 28, 32};

    for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
        size_t len = lengths[i];
        uint8_t entropy[32];
        for (size_t j = 0; j < len; ++j) {
            entropy[j] = (uint8_t)((j * 37U + i) & 0xFFU);
        }

        char *mnemonic = NULL;
        neoc_error_t err = neoc_bip39_mnemonic_from_entropy(entropy, len,
                                                             NEOC_BIP39_LANG_ENGLISH,
                                                             &mnemonic);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(mnemonic);

        uint8_t *roundtrip_entropy = NULL;
        size_t roundtrip_len = 0;
        err = neoc_bip39_mnemonic_to_entropy(mnemonic,
                                             NEOC_BIP39_LANG_ENGLISH,
                                             &roundtrip_entropy,
                                             &roundtrip_len);
        TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
        TEST_ASSERT_NOT_NULL(roundtrip_entropy);
        TEST_ASSERT_EQUAL_UINT(len, roundtrip_len);
        TEST_ASSERT_EQUAL_MEMORY(entropy, roundtrip_entropy, len);

        free(mnemonic);
        free(roundtrip_entropy);
    }
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bip39_word_count);
    RUN_TEST(test_bip39_get_word);
    RUN_TEST(test_bip39_find_word);
    RUN_TEST(test_bip39_mnemonic_from_entropy);
    RUN_TEST(test_bip39_generate_mnemonic);
    RUN_TEST(test_bip39_mnemonic_to_seed);
    RUN_TEST(test_bip39_mnemonic_to_entropy);
    RUN_TEST(test_bip39_validate_mnemonic);
    RUN_TEST(test_bip39_entropy_roundtrip);
    RUN_TEST(test_bip39_entropy_roundtrip_all_strengths);
    
    return UnityEnd();
}
