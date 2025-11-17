#include "neoc/wallet/bip39_account.h"

#include "neoc/crypto/bip39.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/neoc_error.h"
#include "neoc/wallet/account.h"
#include "neoc/utils/neoc_hex.h"
#include <string.h>

static neoc_bip39_strength_t strength_from_word_count(int word_count) {
    switch (word_count) {
        case NEOC_BIP39_WORD_COUNT_12: return NEOC_BIP39_STRENGTH_128;
        case NEOC_BIP39_WORD_COUNT_15: return NEOC_BIP39_STRENGTH_160;
        case NEOC_BIP39_WORD_COUNT_18: return NEOC_BIP39_STRENGTH_192;
        case NEOC_BIP39_WORD_COUNT_21: return NEOC_BIP39_STRENGTH_224;
        case NEOC_BIP39_WORD_COUNT_24: return NEOC_BIP39_STRENGTH_256;
        default: return 0;
    }
}

static int count_words(const char *mnemonic) {
    if (!mnemonic || *mnemonic == '\0') return 0;
    int count = 1;
    for (const char *p = mnemonic; *p; ++p) {
        if (*p == ' ') count++;
    }
    return count;
}

static void bip39_account_free_internal(neoc_bip39_account_t *account) {
    if (!account) return;
    if (account->mnemonic) {
        neoc_free(account->mnemonic);
    }
    if (account->password) {
        neoc_secure_memzero(account->password, strlen(account->password));
        neoc_free(account->password);
    }
    if (account->base_account) {
        neoc_account_free(account->base_account);
    }
    neoc_free(account);
}

static neoc_error_t build_base_account_from_seed(const char *password,
                                                 const char *mnemonic,
                                                 int word_count,
                                                 bool password_protected,
                                                 neoc_bip39_account_t **out) {
    if (!mnemonic || !out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    if (!neoc_bip39_validate_mnemonic(mnemonic, NEOC_BIP39_LANG_ENGLISH)) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid BIP-39 mnemonic");
    }

    int derived_word_count = count_words(mnemonic);
    if (word_count == 0) {
        word_count = derived_word_count;
    }

    uint8_t seed[64];
    size_t seed_len = sizeof(seed);
    neoc_error_t err = neoc_bip39_mnemonic_to_seed_len(mnemonic,
                                                       password ? password : "",
                                                       seed,
                                                       seed_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint8_t private_key[NEOC_SHA256_DIGEST_LENGTH];
    err = neoc_sha256(seed, seed_len, private_key);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_ec_key_pair_t *key_pair = NULL;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &key_pair);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_account_t *base_account = NULL;
    err = neoc_account_create_from_key_pair(key_pair, &base_account);
    neoc_ec_key_pair_free(key_pair);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_bip39_account_t *bip39 = neoc_calloc(1, sizeof(neoc_bip39_account_t));
    if (!bip39) {
        neoc_account_free(base_account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate bip39 account");
    }

    bip39->base_account = base_account;
    bip39->word_count = word_count;
    bip39->is_password_protected = password_protected;

    bip39->mnemonic_length = strlen(mnemonic);
    bip39->mnemonic = neoc_strdup(mnemonic);
    if (!bip39->mnemonic) {
        bip39_account_free_internal(bip39);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy mnemonic");
    }

    if (password && password[0] != '\0') {
        bip39->password = neoc_strdup(password);
        if (!bip39->password) {
            bip39_account_free_internal(bip39);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy password");
        }
    }

    *out = bip39;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bip39_account_create(const char *password,
                                       int word_count,
                                       neoc_bip39_account_t **bip39_account) {
    if (!bip39_account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "bip39_account is NULL");
    }
    *bip39_account = NULL;

    neoc_bip39_strength_t strength = strength_from_word_count(word_count);
    if (strength == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unsupported word count");
    }

    char *mnemonic = NULL;
    neoc_error_t err = neoc_bip39_generate_mnemonic(strength,
                                                    NEOC_BIP39_LANG_ENGLISH,
                                                    &mnemonic);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = build_base_account_from_seed(password, mnemonic, word_count,
                                       password && password[0] != '\0',
                                       bip39_account);
    neoc_free(mnemonic);
    return err;
}

neoc_error_t neoc_bip39_account_create_random(int word_count,
                                              neoc_bip39_account_t **bip39_account) {
    return neoc_bip39_account_create(NULL, word_count, bip39_account);
}

neoc_error_t neoc_bip39_account_from_mnemonic(const char *password,
                                              const char *mnemonic,
                                              neoc_bip39_account_t **bip39_account) {
    if (!bip39_account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "bip39_account is NULL");
    }
    *bip39_account = NULL;
    return build_base_account_from_seed(password, mnemonic, 0,
                                        password && password[0] != '\0',
                                        bip39_account);
}

neoc_error_t neoc_bip39_account_from_mnemonic_no_password(const char *mnemonic,
                                                          neoc_bip39_account_t **bip39_account) {
    return neoc_bip39_account_from_mnemonic("", mnemonic, bip39_account);
}

const char *neoc_bip39_account_get_mnemonic(const neoc_bip39_account_t *bip39_account) {
    return bip39_account ? bip39_account->mnemonic : NULL;
}

const neoc_account_t *neoc_bip39_account_get_base_account(const neoc_bip39_account_t *bip39_account) {
    return bip39_account ? bip39_account->base_account : NULL;
}

int neoc_bip39_account_get_word_count(const neoc_bip39_account_t *bip39_account) {
    return bip39_account ? bip39_account->word_count : 0;
}

bool neoc_bip39_account_is_password_protected(const neoc_bip39_account_t *bip39_account) {
    return bip39_account ? bip39_account->is_password_protected : false;
}

bool neoc_bip39_account_validate_mnemonic(const char *mnemonic) {
    return neoc_bip39_validate_mnemonic(mnemonic, NEOC_BIP39_LANG_ENGLISH);
}

neoc_error_t neoc_bip39_account_generate_mnemonic(int word_count, char **mnemonic) {
    if (!mnemonic) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "mnemonic is NULL");
    }
    neoc_bip39_strength_t strength = strength_from_word_count(word_count);
    if (strength == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Unsupported word count");
    }
    return neoc_bip39_generate_mnemonic(strength, NEOC_BIP39_LANG_ENGLISH, mnemonic);
}

neoc_error_t neoc_bip39_account_mnemonic_to_seed(const char *mnemonic,
                                                 const char *password,
                                                 uint8_t **seed,
                                                 size_t *seed_length) {
    if (!mnemonic || !seed || !seed_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    uint8_t *buffer = neoc_calloc(1, 64);
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate seed buffer");
    }
    neoc_error_t err = neoc_bip39_mnemonic_to_seed_len(mnemonic,
                                                       password ? password : "",
                                                       buffer,
                                                       64);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return err;
    }
    *seed = buffer;
    *seed_length = 64;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_bip39_account_get_private_key(const neoc_bip39_account_t *bip39_account,
                                                uint8_t *private_key,
                                                size_t *key_length) {
    if (!bip39_account || !private_key || !key_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "private key buffer invalid");
    }
    if (*key_length < NEOC_PRIVATE_KEY_SIZE) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    size_t len = NEOC_PRIVATE_KEY_SIZE;
    neoc_error_t err = neoc_ec_key_pair_get_private_key(
        bip39_account->base_account->key_pair, private_key, &len);
    if (err == NEOC_SUCCESS) {
        *key_length = len;
    }
    return err;
}

neoc_error_t neoc_bip39_account_get_public_key(const neoc_bip39_account_t *bip39_account,
                                               bool compressed,
                                               uint8_t **public_key,
                                               size_t *key_length) {
    if (!bip39_account || !public_key || !key_length) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "public key output invalid");
    }
    *public_key = NULL;
    *key_length = 0;

    const neoc_ec_public_key_t *pub = neoc_account_get_public_key(bip39_account->base_account);
    if (!pub) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no public key");
    }
    uint8_t *encoded = NULL;
    size_t encoded_len = 0;
    neoc_error_t err = neoc_ec_public_key_get_encoded((neoc_ec_public_key_t *)pub,
                                                      compressed,
                                                      &encoded,
                                                      &encoded_len);
    if (err == NEOC_SUCCESS) {
        *public_key = encoded;
        *key_length = encoded_len;
    }
    return err;
}

neoc_error_t neoc_bip39_account_get_address(const neoc_bip39_account_t *bip39_account,
                                            char **address) {
    if (!bip39_account || !address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "address is NULL");
    }
    return neoc_account_get_address_copy(bip39_account->base_account, address);
}

neoc_error_t neoc_bip39_account_export_wif(const neoc_bip39_account_t *bip39_account,
                                           char **wif) {
    if (!bip39_account || !wif) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "wif is NULL");
    }
    return neoc_account_export_wif(bip39_account->base_account, wif);
}

neoc_error_t neoc_bip39_account_export_nep2(const neoc_bip39_account_t *bip39_account,
                                            const char *password,
                                            char **nep2_key) {
    if (!bip39_account || !nep2_key || !password) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    return neoc_account_export_nep2(bip39_account->base_account, password, nep2_key);
}

neoc_error_t neoc_bip39_account_copy(const neoc_bip39_account_t *source,
                                     neoc_bip39_account_t **copy) {
    if (!source || !copy) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    *copy = NULL;
    const char *pwd = source->password ? source->password : "";
    return build_base_account_from_seed(pwd,
                                        source->mnemonic,
                                        source->word_count,
                                        source->is_password_protected,
                                        copy);
}

bool neoc_bip39_account_equals(const neoc_bip39_account_t *account1,
                                const neoc_bip39_account_t *account2) {
    if (!account1 || !account2) {
        return false;
    }
    if (account1->word_count != account2->word_count) {
        return false;
    }
    if ((account1->mnemonic == NULL) != (account2->mnemonic == NULL)) {
        return false;
    }
    if (account1->mnemonic && strcmp(account1->mnemonic, account2->mnemonic) != 0) {
        return false;
    }
    char *addr1 = NULL;
    char *addr2 = NULL;
    neoc_error_t err1 = neoc_bip39_account_get_address(account1, &addr1);
    neoc_error_t err2 = neoc_bip39_account_get_address(account2, &addr2);
    bool equal = (err1 == NEOC_SUCCESS && err2 == NEOC_SUCCESS &&
                  addr1 && addr2 && strcmp(addr1, addr2) == 0);
    if (addr1) neoc_free(addr1);
    if (addr2) neoc_free(addr2);
    return equal;
}

void neoc_bip39_account_free(neoc_bip39_account_t *bip39_account) {
    bip39_account_free_internal(bip39_account);
}
