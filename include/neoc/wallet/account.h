#ifndef NEOC_ACCOUNT_H
#define NEOC_ACCOUNT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/scrypt_params.h"
#include "neoc/script/verification_script.h"
#include "neoc/transaction/witness.h"
#include "neoc/wallet/nep6/nep6_account.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NEOC_PP_OVERLOAD
#define NEOC_PP_CONCAT(a, b) NEOC_PP_CONCAT_IMPL(a, b)
#define NEOC_PP_CONCAT_IMPL(a, b) a##b
#define NEOC_PP_NARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define NEOC_PP_NARGS(...) NEOC_PP_NARGS_IMPL(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)
#define NEOC_PP_OVERLOAD(prefix, ...) \
    NEOC_PP_CONCAT(prefix, NEOC_PP_NARGS(__VA_ARGS__))(__VA_ARGS__)
#endif

/**
 * @brief Maximum length of a Neo address string
 */
#define NEOC_ADDRESS_LENGTH 64

/**
 * @brief Additional metadata stored alongside an account.
 */
typedef enum {
    NEOC_ACCOUNT_EXTRA_NONE = 0,
    NEOC_ACCOUNT_EXTRA_MULTISIG = 1,
    NEOC_ACCOUNT_EXTRA_MULTISIG_INFO = 2
} neoc_account_extra_type_t;

/**
 * @brief Core account structure for the NeoC SDK.
 */
typedef struct {
    char *label;
    char *address;
    neoc_hash160_t script_hash;
    neoc_ec_key_pair_t *key_pair;
    bool is_locked;
    bool is_default;
    uint8_t *encrypted_key;
    size_t encrypted_key_len;
    uint8_t *verification_script;
    size_t verification_script_len;
    neoc_account_extra_type_t extra_type;
    void *extra;
} neoc_account_t;

/* --------------------------------------------------------------------- */
/* Account creation                                                      */
/* --------------------------------------------------------------------- */

neoc_error_t neoc_account_create_with_label(const char *label, neoc_account_t **account);
neoc_error_t neoc_account_create_random(neoc_account_t **account);
neoc_error_t neoc_account_create_from_key_pair_with_label(const char *label,
                                                          const neoc_ec_key_pair_t *key_pair,
                                                          neoc_account_t **account);
neoc_error_t neoc_account_create_from_wif_with_label(const char *label,
                                                     const char *wif,
                                                     neoc_account_t **account);
neoc_error_t neoc_account_create_from_nep2_with_label(const char *label,
                                                      const char *encrypted_key,
                                                      const char *passphrase,
                                                      neoc_account_t **account);
neoc_error_t neoc_account_create_from_verification_script(const uint8_t *script,
                                                          size_t script_len,
                                                          neoc_account_t **account);
neoc_error_t neoc_account_create_from_public_key(neoc_ec_public_key_t *public_key,
                                                 neoc_account_t **account);
neoc_error_t neoc_account_create_from_address(const char *address,
                                              neoc_account_t **account);
neoc_error_t neoc_account_create_from_nep6(const neoc_nep6_account_t *nep6_account,
                                           neoc_account_t **account);

#ifndef NEOC_ACCOUNT_DISABLE_OVERLOADS
#define NEOC_ACCOUNT_CREATE_1(account_out) \
    neoc_account_create_with_label(NULL, account_out)
#define NEOC_ACCOUNT_CREATE_2(label, account_out) \
    neoc_account_create_with_label(label, account_out)
#define neoc_account_create(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_CREATE_, __VA_ARGS__)

#define NEOC_ACCOUNT_CREATE_FROM_KEY_PAIR_2(key_pair, account_out) \
    neoc_account_create_from_key_pair_with_label(NULL, key_pair, account_out)
#define NEOC_ACCOUNT_CREATE_FROM_KEY_PAIR_3(label, key_pair, account_out) \
    neoc_account_create_from_key_pair_with_label(label, key_pair, account_out)
#define neoc_account_create_from_key_pair(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_CREATE_FROM_KEY_PAIR_, __VA_ARGS__)

#define NEOC_ACCOUNT_CREATE_FROM_WIF_2(wif, account_out) \
    neoc_account_create_from_wif_with_label(NULL, wif, account_out)
#define NEOC_ACCOUNT_CREATE_FROM_WIF_3(label, wif, account_out) \
    neoc_account_create_from_wif_with_label(label, wif, account_out)
#define neoc_account_create_from_wif(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_CREATE_FROM_WIF_, __VA_ARGS__)

#define NEOC_ACCOUNT_CREATE_FROM_NEP2_3(encrypted_key, passphrase, account_out) \
    neoc_account_create_from_nep2_with_label(NULL, encrypted_key, passphrase, account_out)
#define NEOC_ACCOUNT_CREATE_FROM_NEP2_4(label, encrypted_key, passphrase, account_out) \
    neoc_account_create_from_nep2_with_label(label, encrypted_key, passphrase, account_out)
#define neoc_account_create_from_nep2(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_CREATE_FROM_NEP2_, __VA_ARGS__)
#endif /* NEOC_ACCOUNT_DISABLE_OVERLOADS */

/* Convenience aliases */
#define neoc_account_from_address(address, account_out) \
    neoc_account_create_from_address(address, account_out)
#define neoc_account_from_public_key(public_key, account_out) \
    neoc_account_create_from_public_key(public_key, account_out)
#define neoc_account_from_wif(wif, account_out) \
    neoc_account_create_from_wif_with_label(NULL, wif, account_out)
#define neoc_account_import_encrypted(encrypted_key, passphrase, account_out) \
    neoc_account_create_from_nep2_with_label(NULL, encrypted_key, passphrase, account_out)

/* --------------------------------------------------------------------- */
/* Multi-signature creation                                              */
/* --------------------------------------------------------------------- */

neoc_error_t neoc_account_create_multisig_from_public_keys(neoc_ec_public_key_t **public_keys,
                                                           size_t key_count,
                                                           int threshold,
                                                           neoc_account_t **account);
neoc_error_t neoc_account_create_multisig_with_address(const char *address,
                                                       int threshold,
                                                       int nr_participants,
                                                       neoc_account_t **account);
neoc_error_t neoc_account_create_multisig(int threshold,
                                          neoc_ec_public_key_t **public_keys,
                                          size_t key_count,
                                          neoc_account_t **account);
neoc_error_t neoc_account_create_multi_sig(neoc_ec_public_key_t **public_keys,
                                           size_t key_count,
                                           int threshold,
                                           neoc_account_t **account);

/* --------------------------------------------------------------------- */
/* Accessors                                                             */
/* --------------------------------------------------------------------- */

neoc_ec_public_key_t* neoc_account_get_public_key(const neoc_account_t *account);
neoc_ec_key_pair_t* neoc_account_get_key_pair_ptr(const neoc_account_t *account);
neoc_error_t neoc_account_get_key_pair_copy(const neoc_account_t *account,
                                            neoc_ec_key_pair_t **key_pair);

#ifndef NEOC_ACCOUNT_DISABLE_OVERLOADS
#define NEOC_ACCOUNT_GET_KEY_PAIR_1(account) \
    neoc_account_get_key_pair_ptr(account)
#define NEOC_ACCOUNT_GET_KEY_PAIR_2(account, key_pair_out) \
    neoc_account_get_key_pair_copy(account, key_pair_out)
#define neoc_account_get_key_pair(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_GET_KEY_PAIR_, __VA_ARGS__)
#endif

const char* neoc_account_get_address_ptr(const neoc_account_t *account);
neoc_error_t neoc_account_get_address_copy(const neoc_account_t *account, char **address);

const neoc_hash160_t* neoc_account_get_script_hash_ptr(const neoc_account_t *account);
neoc_error_t neoc_account_get_script_hash_copy(const neoc_account_t *account,
                                               neoc_hash160_t *hash);

const char* neoc_account_get_label_ptr(const neoc_account_t *account);
neoc_error_t neoc_account_get_label_copy(const neoc_account_t *account, char **label);

neoc_error_t neoc_account_get_verification_script(const neoc_account_t *account,
                                                  uint8_t **script,
                                                  size_t *script_len);
neoc_verification_script_t* neoc_account_get_verification_script_ptr(const neoc_account_t *account);
neoc_error_t neoc_account_get_verification_script_object(const neoc_account_t *account,
                                                         neoc_verification_script_t **script);

#ifndef NEOC_ACCOUNT_DISABLE_OVERLOADS
#define NEOC_ACCOUNT_GET_ADDRESS_1(account) \
    neoc_account_get_address_ptr(account)
#define NEOC_ACCOUNT_GET_ADDRESS_2(account, address_out) \
    neoc_account_get_address_copy(account, address_out)
#define neoc_account_get_address(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_GET_ADDRESS_, __VA_ARGS__)

#define NEOC_ACCOUNT_GET_SCRIPT_HASH_1(account) \
    neoc_account_get_script_hash_ptr(account)
#define NEOC_ACCOUNT_GET_SCRIPT_HASH_2(account, hash_out) \
    neoc_account_get_script_hash_copy(account, hash_out)
#define neoc_account_get_script_hash(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_GET_SCRIPT_HASH_, __VA_ARGS__)

#define NEOC_ACCOUNT_GET_LABEL_1(account) \
    neoc_account_get_label_ptr(account)
#define NEOC_ACCOUNT_GET_LABEL_2(account, label_out) \
    neoc_account_get_label_copy(account, label_out)
#define neoc_account_get_label(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_GET_LABEL_, __VA_ARGS__)

#define NEOC_ACCOUNT_GET_VERIFICATION_SCRIPT_1(account) \
    neoc_account_get_verification_script_ptr(account)
#define NEOC_ACCOUNT_GET_VERIFICATION_SCRIPT_3(account, script_out, len_out) \
    neoc_account_get_verification_script(account, script_out, len_out)
#define neoc_account_get_verification_script(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_GET_VERIFICATION_SCRIPT_, __VA_ARGS__)
#endif /* NEOC_ACCOUNT_DISABLE_OVERLOADS */

/* --------------------------------------------------------------------- */
/* State helpers                                                         */
/* --------------------------------------------------------------------- */

neoc_error_t neoc_account_set_label(neoc_account_t *account, const char *label);
neoc_error_t neoc_account_set_default(neoc_account_t *account, bool is_default);
bool neoc_account_is_default_value(const neoc_account_t *account);
neoc_error_t neoc_account_is_default_out(const neoc_account_t *account, bool *is_default);

bool neoc_account_is_multi_sig(const neoc_account_t *account);
neoc_error_t neoc_account_is_multisig(const neoc_account_t *account, bool *is_multisig);
neoc_error_t neoc_account_get_signing_threshold(const neoc_account_t *account, int *threshold);
neoc_error_t neoc_account_get_nr_participants(const neoc_account_t *account, int *nr_participants);

neoc_error_t neoc_account_lock_internal(neoc_account_t *account);
neoc_error_t neoc_account_unlock_internal(neoc_account_t *account);

bool neoc_account_has_encrypted_private_key_value(const neoc_account_t *account);
neoc_error_t neoc_account_has_encrypted_private_key_out(const neoc_account_t *account,
                                                        bool *has_encrypted_key);

const char* neoc_account_get_encrypted_private_key_ptr(const neoc_account_t *account);
neoc_error_t neoc_account_get_encrypted_private_key_copy(const neoc_account_t *account,
                                                         char **encrypted_key);
neoc_error_t neoc_account_get_encrypted_private_key(const neoc_account_t *account,
                                                    uint8_t **encrypted_key,
                                                    size_t *encrypted_key_len);
neoc_error_t neoc_account_set_encrypted_private_key(neoc_account_t *account,
                                                    const uint8_t *encrypted_key,
                                                    size_t encrypted_key_len);

bool neoc_account_is_locked_value(const neoc_account_t *account);
neoc_error_t neoc_account_is_locked_out(const neoc_account_t *account, bool *is_locked);

#ifndef NEOC_ACCOUNT_DISABLE_OVERLOADS
#define NEOC_ACCOUNT_IS_DEFAULT_1(account) \
    neoc_account_is_default_value(account)
#define NEOC_ACCOUNT_IS_DEFAULT_2(account, out) \
    neoc_account_is_default_out(account, out)
#define neoc_account_is_default(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_IS_DEFAULT_, __VA_ARGS__)

#define NEOC_ACCOUNT_IS_MULTISIG_1(account) \
    neoc_account_is_multi_sig(account)
#define NEOC_ACCOUNT_IS_MULTISIG_2(account, out) \
    neoc_account_is_multisig(account, out)
#define neoc_account_is_multisig(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_IS_MULTISIG_, __VA_ARGS__)

#define NEOC_ACCOUNT_LOCK_1(account) \
    neoc_account_lock_internal(account)
#define NEOC_ACCOUNT_LOCK_2(account, password) \
    neoc_account_encrypt(account, password)
#define neoc_account_lock(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_LOCK_, __VA_ARGS__)

#define NEOC_ACCOUNT_UNLOCK_1(account) \
    neoc_account_unlock_internal(account)
#define NEOC_ACCOUNT_UNLOCK_2(account, password) \
    neoc_account_decrypt(account, password)
#define neoc_account_unlock(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_UNLOCK_, __VA_ARGS__)

#define NEOC_ACCOUNT_HAS_ENCRYPTED_KEY_1(account) \
    neoc_account_has_encrypted_private_key_value(account)
#define NEOC_ACCOUNT_HAS_ENCRYPTED_KEY_2(account, out) \
    neoc_account_has_encrypted_private_key_out(account, out)
#define neoc_account_has_encrypted_private_key(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_HAS_ENCRYPTED_KEY_, __VA_ARGS__)

#define NEOC_ACCOUNT_GET_ENCRYPTED_PRIVATE_KEY_1(account) \
    neoc_account_get_encrypted_private_key_ptr(account)
#define NEOC_ACCOUNT_GET_ENCRYPTED_PRIVATE_KEY_2(account, encrypted_out) \
    neoc_account_get_encrypted_private_key_copy(account, encrypted_out)
#define NEOC_ACCOUNT_GET_ENCRYPTED_PRIVATE_KEY_3(account, data_out, len_out) \
    neoc_account_get_encrypted_private_key(account, data_out, len_out)
#define neoc_account_get_encrypted_private_key(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_GET_ENCRYPTED_PRIVATE_KEY_, __VA_ARGS__)

#define NEOC_ACCOUNT_IS_LOCKED_1(account) \
    neoc_account_is_locked_value(account)
#define NEOC_ACCOUNT_IS_LOCKED_2(account, out) \
    neoc_account_is_locked_out(account, out)
#define neoc_account_is_locked(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_IS_LOCKED_, __VA_ARGS__)
#endif /* NEOC_ACCOUNT_DISABLE_OVERLOADS */

/* --------------------------------------------------------------------- */
/* Serialization and NEP-6 helpers                                       */
/* --------------------------------------------------------------------- */

neoc_error_t neoc_account_to_nep6(const neoc_account_t *account,
                                  neoc_nep6_account_t **nep6_account);
neoc_error_t neoc_account_from_nep6(const neoc_nep6_account_t *nep6_account,
                                    neoc_account_t **account);

/* --------------------------------------------------------------------- */
/* Crypto utilities                                                      */
/* --------------------------------------------------------------------- */

neoc_error_t neoc_account_export_wif(const neoc_account_t *account, char **wif);
neoc_error_t neoc_account_export_nep2(const neoc_account_t *account,
                                      const char *password,
                                      char **encrypted_key);

#define neoc_account_export_encrypted(account, password, nep2_out) \
    neoc_account_export_nep2(account, password, nep2_out)

neoc_error_t neoc_account_sign(const neoc_account_t *account,
                               const uint8_t *data,
                               size_t data_len,
                               uint8_t **signature,
                               size_t *signature_len);
neoc_error_t neoc_account_sign_hash(const neoc_account_t *account,
                                    const neoc_hash256_t *hash,
                                    neoc_witness_t **witness);

neoc_error_t neoc_account_encrypt_private_key_with_params(neoc_account_t *account,
                                                          const char *password,
                                                          const neoc_scrypt_params_t *params);
neoc_error_t neoc_account_decrypt_private_key_with_params(neoc_account_t *account,
                                                          const char *password,
                                                          const neoc_scrypt_params_t *params);
neoc_error_t neoc_account_encrypt(neoc_account_t *account, const char *password);
neoc_error_t neoc_account_decrypt(neoc_account_t *account, const char *password);

#ifndef NEOC_ACCOUNT_DISABLE_OVERLOADS
#define NEOC_ACCOUNT_ENCRYPT_PRIVATE_KEY_2(account, password) \
    neoc_account_encrypt_private_key_with_params(account, password, NULL)
#define NEOC_ACCOUNT_ENCRYPT_PRIVATE_KEY_3(account, password, params) \
    neoc_account_encrypt_private_key_with_params(account, password, params)
#define neoc_account_encrypt_private_key(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_ENCRYPT_PRIVATE_KEY_, __VA_ARGS__)

#define NEOC_ACCOUNT_DECRYPT_PRIVATE_KEY_2(account, password) \
    neoc_account_decrypt_private_key_with_params(account, password, NULL)
#define NEOC_ACCOUNT_DECRYPT_PRIVATE_KEY_3(account, password, params) \
    neoc_account_decrypt_private_key_with_params(account, password, params)
#define neoc_account_decrypt_private_key(...) \
    NEOC_PP_OVERLOAD(NEOC_ACCOUNT_DECRYPT_PRIVATE_KEY_, __VA_ARGS__)
#endif /* NEOC_ACCOUNT_DISABLE_OVERLOADS */

/* --------------------------------------------------------------------- */
/* Memory management                                                     */
/* --------------------------------------------------------------------- */

void neoc_account_free(neoc_account_t *account);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_ACCOUNT_H */
