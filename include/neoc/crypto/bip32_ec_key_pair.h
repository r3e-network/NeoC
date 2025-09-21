/**
 * @file bip32_e_c_key_pair.h
 * @brief BIP-32 hierarchical deterministic key pairs
 */

#ifndef NEOC_BIP32_E_C_KEY_PAIR_H
#define NEOC_BIP32_E_C_KEY_PAIR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ec_key_pair.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEOC_BIP32_SEED_SIZE 64
#define NEOC_BIP32_CHAIN_CODE_SIZE 32
#define NEOC_BIP32_HARDENED_BIT 0x80000000

/**
 * BIP-32 extended key pair structure
 */
typedef struct neoc_bip32_key_pair {
    neoc_ec_key_pair_t *key_pair;      // Base key pair
    uint8_t chain_code[32];            // Chain code
    uint32_t depth;                     // Derivation depth
    uint32_t child_number;              // Child key index
    uint8_t parent_fingerprint[4];      // Parent key fingerprint
} neoc_bip32_key_pair_t;

/**
 * Create master key pair from seed
 * @param seed Seed bytes
 * @param seed_len Seed length
 * @param key_pair Output master key pair
 * @return Error code
 */
neoc_error_t neoc_bip32_create_master(const uint8_t *seed,
                                       size_t seed_len,
                                       neoc_bip32_key_pair_t **key_pair);

/**
 * Create key pair from mnemonic phrase
 * @param mnemonic BIP-39 mnemonic phrase
 * @param passphrase Optional passphrase
 * @param key_pair Output key pair
 * @return Error code
 */
neoc_error_t neoc_bip32_from_mnemonic(const char *mnemonic,
                                       const char *passphrase,
                                       neoc_bip32_key_pair_t **key_pair);

/**
 * Derive child key pair
 * @param parent Parent key pair
 * @param index Child index (set bit 31 for hardened)
 * @param child Output child key pair
 * @return Error code
 */
neoc_error_t neoc_bip32_derive_child(neoc_bip32_key_pair_t *parent,
                                      uint32_t index,
                                      neoc_bip32_key_pair_t **child);

/**
 * Derive key pair from path
 * @param master Master key pair
 * @param path Derivation path (e.g., "m/44'/888'/0'/0/0")
 * @param derived Output derived key pair
 * @return Error code
 */
neoc_error_t neoc_bip32_derive_path(neoc_bip32_key_pair_t *master,
                                     const char *path,
                                     neoc_bip32_key_pair_t **derived);

/**
 * Get extended public key
 * @param key_pair The key pair
 * @param xpub Output extended public key string
 * @return Error code
 */
neoc_error_t neoc_bip32_get_extended_public_key(neoc_bip32_key_pair_t *key_pair,
                                                 char **xpub);

/**
 * Get extended private key
 * @param key_pair The key pair
 * @param xprv Output extended private key string
 * @return Error code
 */
neoc_error_t neoc_bip32_get_extended_private_key(neoc_bip32_key_pair_t *key_pair,
                                                  char **xprv);

/**
 * Free BIP-32 key pair
 * @param key_pair The key pair to free
 */
void neoc_bip32_key_pair_free(neoc_bip32_key_pair_t *key_pair);

#ifdef __cplusplus
}
#endif

#endif // NEOC_BIP32_E_C_KEY_PAIR_H
