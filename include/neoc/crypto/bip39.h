/**
 * @file bip39.h
 * @brief BIP-39 Mnemonic code for deterministic key generation
 */

#ifndef NEOC_BIP39_H
#define NEOC_BIP39_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Mnemonic strength levels (entropy bits)
 */
typedef enum {
    NEOC_BIP39_STRENGTH_128 = 128,  // 12 words
    NEOC_BIP39_STRENGTH_160 = 160,  // 15 words
    NEOC_BIP39_STRENGTH_192 = 192,  // 18 words
    NEOC_BIP39_STRENGTH_224 = 224,  // 21 words
    NEOC_BIP39_STRENGTH_256 = 256   // 24 words
} neoc_bip39_strength_t;

/**
 * @brief Language codes for mnemonic wordlists
 */
typedef enum {
    NEOC_BIP39_LANG_ENGLISH = 0,
    NEOC_BIP39_LANG_JAPANESE,
    NEOC_BIP39_LANG_KOREAN,
    NEOC_BIP39_LANG_SPANISH,
    NEOC_BIP39_LANG_CHINESE_SIMPLIFIED,
    NEOC_BIP39_LANG_CHINESE_TRADITIONAL,
    NEOC_BIP39_LANG_FRENCH,
    NEOC_BIP39_LANG_ITALIAN,
    NEOC_BIP39_LANG_CZECH,
    NEOC_BIP39_LANG_PORTUGUESE
} neoc_bip39_language_t;

/**
 * @brief Generate a random mnemonic phrase
 * 
 * @param strength Entropy strength in bits (128-256)
 * @param language Language for wordlist
 * @param mnemonic Output mnemonic phrase (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_generate_mnemonic(neoc_bip39_strength_t strength,
                                           neoc_bip39_language_t language,
                                           char **mnemonic);

/**
 * @brief Generate mnemonic from entropy
 * 
 * @param entropy Entropy bytes
 * @param entropy_len Length of entropy (16, 20, 24, 28, or 32 bytes)
 * @param language Language for wordlist
 * @param mnemonic Output mnemonic phrase (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_mnemonic_from_entropy(const uint8_t *entropy,
                                               size_t entropy_len,
                                               neoc_bip39_language_t language,
                                               char **mnemonic);

/**
 * @brief Derive entropy from mnemonic phrase
 * 
 * @param mnemonic Mnemonic phrase
 * @param language Language of mnemonic
 * @param entropy Output entropy bytes (caller must free)
 * @param entropy_len Output entropy length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_mnemonic_to_entropy(const char *mnemonic,
                                             neoc_bip39_language_t language,
                                             uint8_t **entropy,
                                             size_t *entropy_len);

/**
 * @brief Generate seed from mnemonic and passphrase
 * 
 * Uses PBKDF2 with 2048 iterations and HMAC-SHA512
 * 
 * @param mnemonic Mnemonic phrase
 * @param passphrase Optional passphrase (can be NULL or empty)
 * @param seed Output seed (64 bytes, caller must allocate)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip39_mnemonic_to_seed(const char *mnemonic,
                                          const char *passphrase,
                                          uint8_t seed[64]);

/**
 * @brief Validate a mnemonic phrase
 * 
 * @param mnemonic Mnemonic phrase to validate
 * @param language Language of mnemonic
 * @return true if valid, false otherwise
 */
bool neoc_bip39_validate_mnemonic(const char *mnemonic,
                                   neoc_bip39_language_t language);

/**
 * @brief Get word count for a given entropy strength
 * 
 * @param strength Entropy strength in bits
 * @return Number of words in mnemonic
 */
size_t neoc_bip39_get_word_count(neoc_bip39_strength_t strength);

/**
 * @brief Get the wordlist for a language
 * 
 * @param language Language code
 * @return Pointer to wordlist array (2048 words), or NULL if invalid
 */
const char* const* neoc_bip39_get_wordlist(neoc_bip39_language_t language);

/**
 * @brief Get word from wordlist by index
 * 
 * @param language Language code
 * @param index Word index (0-2047)
 * @return Word string, or NULL if invalid index
 */
const char* neoc_bip39_get_word(neoc_bip39_language_t language, uint16_t index);

/**
 * @brief Find word index in wordlist
 * 
 * @param language Language code
 * @param word Word to find
 * @return Word index (0-2047), or -1 if not found
 */
int neoc_bip39_find_word(neoc_bip39_language_t language, const char *word);

#ifdef __cplusplus
}
#endif

#endif // NEOC_BIP39_H
