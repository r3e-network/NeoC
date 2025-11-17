/**
 * @file bip32.h
 * @brief BIP-32 Hierarchical Deterministic (HD) wallet implementation
 */

#ifndef NEOC_BIP32_H
#define NEOC_BIP32_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ec_key_pair.h"

#ifdef __cplusplus
extern "C" {
#endif

// BIP-32 constants
#define BIP32_SEED_KEY "Neo seed"
#define BIP32_HARDENED_KEY_START 0x80000000
#define BIP32_EXTENDED_KEY_SIZE 64
#define BIP32_CHAIN_CODE_SIZE 32
#define BIP32_FINGERPRINT_SIZE 4
#define BIP32_SERIALIZED_SIZE 82

// Default Neo derivation path: m/44'/888'/0'/0/0
#define BIP32_NEO_PURPOSE 44
#define BIP32_NEO_COIN_TYPE 888
#define BIP32_NEO_ACCOUNT 0
#define BIP32_NEO_CHANGE 0

/**
 * @brief BIP-32 extended key structure
 */
typedef struct neoc_bip32_key {
    uint8_t version[4];           // Version bytes (mainnet/testnet, public/private)
    uint8_t depth;                // Depth in derivation tree
    uint8_t parent_fingerprint[BIP32_FINGERPRINT_SIZE]; // Parent key fingerprint
    uint32_t child_number;        // Child key index
    uint8_t chain_code[BIP32_CHAIN_CODE_SIZE];  // Chain code for derivation
    uint8_t key[33];              // Private key (0x00 prefix) or public key
    bool is_private;              // Whether this is a private key
} neoc_bip32_key_t;

/**
 * @brief Generate master key from seed
 * 
 * @param seed Seed bytes (from BIP-39 mnemonic)
 * @param seed_len Seed length (typically 64 bytes)
 * @param master_key Output master key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_from_seed_raw(const uint8_t *seed,
                                      size_t seed_len,
                                      neoc_bip32_key_t *master_key);
neoc_error_t neoc_bip32_from_seed_alloc(const uint8_t *seed,
                                        size_t seed_len,
                                        neoc_bip32_key_t **master_key);

/**
 * @brief Derive child key from parent
 * 
 * @param parent Parent key
 * @param index Child index (use BIP32_HARDENED_KEY_START | index for hardened)
 * @param child Output child key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_derive_child(const neoc_bip32_key_t *parent,
                                      uint32_t index,
                                      neoc_bip32_key_t *child);

/**
 * @brief Derive key from path
 * 
 * @param master Master key
 * @param path Derivation path (e.g., "m/44'/888'/0'/0/0")
 * @param derived Output derived key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_derive_path_raw(const neoc_bip32_key_t *master,
                                        const char *path,
                                        neoc_bip32_key_t *derived);

/**
 * @brief Derive key from path components
 * 
 * @param master Master key
 * @param indices Array of derivation indices
 * @param indices_count Number of indices
 * @param derived Output derived key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_derive_path_indices(const neoc_bip32_key_t *master,
                                             const uint32_t *indices,
                                             size_t indices_count,
                                             neoc_bip32_key_t *derived);
neoc_error_t neoc_bip32_derive_path_indices_alloc(const neoc_bip32_key_t *master,
                                                  const uint32_t *indices,
                                                  size_t indices_count,
                                                  neoc_bip32_key_t **derived);

/**
 * @brief Get public key from extended key
 * 
 * @param key Extended key (private or public)
 * @param public_key Output public key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_get_public_key(const neoc_bip32_key_t *key,
                                        neoc_bip32_key_t *public_key);

/**
 * @brief Convert BIP-32 key to EC key pair
 * 
 * @param bip32_key BIP-32 extended key
 * @param ec_key Output EC key pair
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_to_ec_key_pair_raw(const neoc_bip32_key_t *bip32_key,
                                           neoc_ec_key_pair_t *ec_key);
neoc_error_t neoc_bip32_to_ec_key_pair_alloc(const neoc_bip32_key_t *bip32_key,
                                             neoc_ec_key_pair_t **ec_key);

/**
 * @brief Serialize extended key to base58
 * 
 * @param key Extended key
 * @param xkey Output base58 string (xprv/xpub format)
 * @param xkey_size Size of output buffer (min 112 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_serialize(const neoc_bip32_key_t *key,
                                   char *xkey,
                                   size_t xkey_size);

/**
 * @brief Deserialize extended key from base58
 * 
 * @param xkey Base58 string (xprv/xpub format)
 * @param key Output extended key
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_deserialize(const char *xkey,
                                     neoc_bip32_key_t *key);

/**
 * @brief Get key fingerprint
 * 
 * @param key Extended key
 * @param fingerprint Output fingerprint (4 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_get_fingerprint(const neoc_bip32_key_t *key,
                                         uint8_t fingerprint[BIP32_FINGERPRINT_SIZE]);

/**
 * @brief Parse derivation path string
 * 
 * @param path Path string (e.g., "m/44'/888'/0'/0/0")
 * @param indices Output array of indices
 * @param max_indices Maximum indices array size
 * @param indices_count Output number of indices parsed
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_parse_path(const char *path,
                                    uint32_t *indices,
                                    size_t max_indices,
                                    size_t *indices_count);

void neoc_bip32_key_free(neoc_bip32_key_t *key);

#ifndef NEOC_PP_OVERLOAD
#define NEOC_PP_CONCAT(a,b) NEOC_PP_CONCAT_IMPL(a,b)
#define NEOC_PP_CONCAT_IMPL(a,b) a##b
#define NEOC_PP_NARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define NEOC_PP_NARGS(...) NEOC_PP_NARGS_IMPL(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
#define NEOC_PP_OVERLOAD(prefix, ...) NEOC_PP_CONCAT(prefix, NEOC_PP_NARGS(__VA_ARGS__))(__VA_ARGS__)
#endif

static inline neoc_error_t neoc_bip32_from_seed_wrapper(const uint8_t *seed,
                                                        size_t seed_len,
                                                        void *out,
                                                        size_t elem_size) {
    if (elem_size == sizeof(neoc_bip32_key_t)) {
        return neoc_bip32_from_seed_raw(seed, seed_len, (neoc_bip32_key_t *)out);
    }
    return neoc_bip32_from_seed_alloc(seed, seed_len, (neoc_bip32_key_t **)out);
}

#define neoc_bip32_from_seed(seed, len, out) \
    neoc_bip32_from_seed_wrapper((seed), (len), (void *)(out), sizeof(*(out)))

#define NEOC_BIP32_DERIVE_PATH_3(master, path, out) \
    neoc_bip32_derive_path_raw(master, path, out)
#define NEOC_BIP32_DERIVE_PATH_4(master, indices, count, out) \
    neoc_bip32_derive_path_indices_wrapper((master), (indices), (count), (void *)(out), sizeof(*(out)))

#define neoc_bip32_derive_path(...) \
    NEOC_PP_OVERLOAD(NEOC_BIP32_DERIVE_PATH_, __VA_ARGS__)

static inline neoc_error_t neoc_bip32_derive_path_indices_wrapper(const neoc_bip32_key_t *master,
                                                                  const uint32_t *indices,
                                                                  size_t count,
                                                                  void *out,
                                                                  size_t elem_size) {
    if (elem_size == sizeof(neoc_bip32_key_t)) {
        return neoc_bip32_derive_path_indices(master, indices, count, (neoc_bip32_key_t *)out);
    }
    return neoc_bip32_derive_path_indices_alloc(master, indices, count, (neoc_bip32_key_t **)out);
}

static inline neoc_error_t neoc_bip32_to_ec_key_pair_wrapper(const neoc_bip32_key_t *bip32_key,
                                                             void *out,
                                                             size_t elem_size) {
    if (elem_size == sizeof(neoc_ec_key_pair_t)) {
        return neoc_bip32_to_ec_key_pair_raw(bip32_key, (neoc_ec_key_pair_t *)out);
    }
    return neoc_bip32_to_ec_key_pair_alloc(bip32_key, (neoc_ec_key_pair_t **)out);
}

#define neoc_bip32_to_ec_key_pair(bip32_key, out) \
    neoc_bip32_to_ec_key_pair_wrapper((bip32_key), (void *)(out), sizeof(*(out)))

/**
 * @brief Get default Neo derivation path indices
 * 
 * Returns m/44'/888'/account'/change/address_index
 * 
 * @param account Account index (usually 0)
 * @param change Change index (0 for external, 1 for internal)
 * @param address_index Address index
 * @param indices Output array (must have space for 5 elements)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_bip32_get_neo_path(uint32_t account,
                                      uint32_t change,
                                      uint32_t address_index,
                                      uint32_t indices[5]);

#ifdef __cplusplus
}
#endif

#endif // NEOC_BIP32_H
