/**
 * @file bip32_eckey_pair.h
 * @brief BIP-32 Hierarchical Deterministic EC Key Pair implementation
 * 
 * Based on Swift source: crypto/Bip32ECKeyPair.swift
 * Provides BIP-32 HD wallet functionality for Neo blockchain
 */

#ifndef NEOC_CRYPTO_BIP32_ECKEY_PAIR_H
#define NEOC_CRYPTO_BIP32_ECKEY_PAIR_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/ec_key_pair.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// BIP32 Constants
#define NEOC_BIP32_SEED_SIZE 64
#define NEOC_BIP32_CHAIN_CODE_SIZE 32
#define NEOC_BIP32_HARDENED_BIT 0x80000000
#define NEOC_BIP32_FINGERPRINT_SIZE 4

// Neo specific BIP32 derivation path constants
#define NEOC_BIP32_NEO_PURPOSE 44
#define NEOC_BIP32_NEO_COIN_TYPE 888
#define NEOC_BIP32_NEO_ACCOUNT 0
#define NEOC_BIP32_NEO_CHANGE 0

/**
 * @brief BIP-32 Extended Key Pair structure
 * 
 * Combines EC key pair with BIP-32 derivation information
 */
typedef struct {
    neoc_ec_key_pair_t *key_pair;           /**< Base EC key pair */
    uint8_t chain_code[NEOC_BIP32_CHAIN_CODE_SIZE]; /**< Chain code for key derivation */
    uint32_t depth;                         /**< Depth in derivation tree (0 = master) */
    uint32_t child_number;                  /**< Child key index */
    uint8_t parent_fingerprint[NEOC_BIP32_FINGERPRINT_SIZE]; /**< Parent key fingerprint */
} neoc_bip32_eckey_pair_t;

/**
 * @brief Generate master BIP-32 key pair from seed
 * 
 * @param seed Seed bytes (typically from BIP-39 mnemonic)
 * @param seed_len Seed length (must be >= 16 and <= 64 bytes)
 * @param key_pair Output master key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_from_seed(const uint8_t *seed,
                                              size_t seed_len,
                                              neoc_bip32_eckey_pair_t **key_pair);

/**
 * @brief Generate master BIP-32 key pair from mnemonic phrase
 * 
 * @param mnemonic BIP-39 mnemonic phrase
 * @param passphrase Optional passphrase (can be NULL)
 * @param key_pair Output master key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_from_mnemonic(const char *mnemonic,
                                                  const char *passphrase,
                                                  neoc_bip32_eckey_pair_t **key_pair);

/**
 * @brief Derive child key from parent using index
 * 
 * @param parent Parent BIP-32 key pair
 * @param index Child index (use NEOC_BIP32_HARDENED_BIT | index for hardened)
 * @param child Output child key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_derive_child(const neoc_bip32_eckey_pair_t *parent,
                                                 uint32_t index,
                                                 neoc_bip32_eckey_pair_t **child);

/**
 * @brief Derive key from BIP-32 derivation path
 * 
 * @param master Master key pair
 * @param path Derivation path string (e.g., "m/44'/888'/0'/0/0")
 * @param derived Output derived key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_derive_path(const neoc_bip32_eckey_pair_t *master,
                                                const char *path,
                                                neoc_bip32_eckey_pair_t **derived);

/**
 * @brief Derive key from array of indices
 * 
 * @param master Master key pair
 * @param indices Array of child indices
 * @param indices_count Number of indices
 * @param derived Output derived key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_derive_path_indices(const neoc_bip32_eckey_pair_t *master,
                                                        const uint32_t *indices,
                                                        size_t indices_count,
                                                        neoc_bip32_eckey_pair_t **derived);

/**
 * @brief Get the standard Neo derivation path: m/44'/888'/0'/0/0
 * 
 * @param master Master key pair
 * @param account_index Account index (default: 0)
 * @param address_index Address index (default: 0)
 * @param derived Output derived key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_derive_neo_path(const neoc_bip32_eckey_pair_t *master,
                                                    uint32_t account_index,
                                                    uint32_t address_index,
                                                    neoc_bip32_eckey_pair_t **derived);

/**
 * @brief Get the base EC key pair from BIP-32 key pair
 * 
 * @param bip32_key BIP-32 key pair
 * @return Pointer to internal EC key pair (do not free separately)
 */
const neoc_ec_key_pair_t *neoc_bip32_eckey_pair_get_ec_key_pair(const neoc_bip32_eckey_pair_t *bip32_key);

/**
 * @brief Get the chain code from BIP-32 key pair
 * 
 * @param bip32_key BIP-32 key pair
 * @param chain_code Output buffer for chain code (32 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_get_chain_code(const neoc_bip32_eckey_pair_t *bip32_key,
                                                   uint8_t *chain_code);

/**
 * @brief Get the depth of the key in derivation tree
 * 
 * @param bip32_key BIP-32 key pair
 * @return Depth (0 = master key)
 */
uint32_t neoc_bip32_eckey_pair_get_depth(const neoc_bip32_eckey_pair_t *bip32_key);

/**
 * @brief Get the child number/index of the key
 * 
 * @param bip32_key BIP-32 key pair
 * @return Child number
 */
uint32_t neoc_bip32_eckey_pair_get_child_number(const neoc_bip32_eckey_pair_t *bip32_key);

/**
 * @brief Get the parent fingerprint
 * 
 * @param bip32_key BIP-32 key pair
 * @param fingerprint Output buffer for fingerprint (4 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_get_parent_fingerprint(const neoc_bip32_eckey_pair_t *bip32_key,
                                                           uint8_t *fingerprint);

/**
 * @brief Check if the key uses hardened derivation
 * 
 * @param bip32_key BIP-32 key pair
 * @return True if hardened derivation was used
 */
bool neoc_bip32_eckey_pair_is_hardened(const neoc_bip32_eckey_pair_t *bip32_key);

/**
 * @brief Export BIP-32 key as extended private key (xprv)
 * 
 * @param bip32_key BIP-32 key pair
 * @param xprv Output extended private key string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_export_xprv(const neoc_bip32_eckey_pair_t *bip32_key,
                                                char **xprv);

/**
 * @brief Export BIP-32 key as extended public key (xpub)
 * 
 * @param bip32_key BIP-32 key pair
 * @param xpub Output extended public key string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_export_xpub(const neoc_bip32_eckey_pair_t *bip32_key,
                                                char **xpub);

/**
 * @brief Import BIP-32 key from extended private key (xprv)
 * 
 * @param xprv Extended private key string
 * @param bip32_key Output BIP-32 key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_import_xprv(const char *xprv,
                                                neoc_bip32_eckey_pair_t **bip32_key);

/**
 * @brief Import BIP-32 key from extended public key (xpub)
 * 
 * @param xpub Extended public key string
 * @param bip32_key Output BIP-32 key pair (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_eckey_pair_import_xpub(const char *xpub,
                                                neoc_bip32_eckey_pair_t **bip32_key);

/**
 * @brief Free a BIP-32 EC key pair
 * 
 * @param bip32_key The BIP-32 key pair to free
 */
void neoc_bip32_eckey_pair_free(neoc_bip32_eckey_pair_t *bip32_key);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_BIP32_ECKEY_PAIR_H */
