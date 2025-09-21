#include "neoc/crypto/sign.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

static neoc_error_t neoc_sign_stub_error(const char *fn) {
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          fn ? fn : "Signature operation not implemented");
}

neoc_error_t neoc_sign_message(const uint8_t *message, size_t message_len,
                                const neoc_ec_key_pair_t *key_pair,
                                neoc_signature_data_t **sig_data) {
    (void)message;
    (void)message_len;
    (void)key_pair;
    if (sig_data) {
        *sig_data = NULL;
    }
    return neoc_sign_stub_error("neoc_sign_message");
}

neoc_error_t neoc_sign_hex_message(const char *hex_message,
                                    const neoc_ec_key_pair_t *key_pair,
                                    neoc_signature_data_t **sig_data) {
    (void)hex_message;
    (void)key_pair;
    if (sig_data) {
        *sig_data = NULL;
    }
    return neoc_sign_stub_error("neoc_sign_hex_message");
}

neoc_error_t neoc_recover_from_signature(int rec_id,
                                          const neoc_ecdsa_signature_t *signature,
                                          const uint8_t *message_hash,
                                          neoc_ec_public_key_t **public_key) {
    (void)rec_id;
    (void)signature;
    (void)message_hash;
    if (public_key) {
        *public_key = NULL;
    }
    return neoc_sign_stub_error("neoc_recover_from_signature");
}

neoc_error_t neoc_signed_message_to_key(const uint8_t *message, size_t message_len,
                                         const neoc_signature_data_t *sig_data,
                                         neoc_ec_public_key_t **public_key) {
    (void)message;
    (void)message_len;
    (void)sig_data;
    if (public_key) {
        *public_key = NULL;
    }
    return neoc_sign_stub_error("neoc_signed_message_to_key");
}

neoc_error_t neoc_recover_signing_script_hash(const uint8_t *message, size_t message_len,
                                               const neoc_signature_data_t *sig_data,
                                               neoc_hash160_t *script_hash) {
    (void)message;
    (void)message_len;
    (void)sig_data;
    if (script_hash) {
        memset(script_hash, 0, sizeof(*script_hash));
    }
    return neoc_sign_stub_error("neoc_recover_signing_script_hash");
}

bool neoc_verify_signature(const uint8_t *message, size_t message_len,
                            const neoc_signature_data_t *sig_data,
                            const neoc_ec_public_key_t *public_key) {
    (void)message;
    (void)message_len;
    (void)sig_data;
    (void)public_key;
    return false;
}

neoc_error_t neoc_public_key_from_private_key(const neoc_ec_private_key_t *private_key,
                                               neoc_ec_public_key_t **public_key) {
    (void)private_key;
    if (public_key) {
        *public_key = NULL;
    }
    return neoc_sign_stub_error("neoc_public_key_from_private_key");
}

neoc_error_t neoc_verify_message(const uint8_t *message, size_t message_len,
                                  const neoc_signature_data_t *signature,
                                  const neoc_ec_key_pair_t *key_pair,
                                  bool *verified) {
    (void)message;
    (void)message_len;
    (void)signature;
    (void)key_pair;
    if (verified) {
        *verified = false;
    }
    return neoc_sign_stub_error("neoc_verify_message");
}
