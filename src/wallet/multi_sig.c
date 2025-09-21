/**
 * @file multi_sig.c
 * @brief Multi-signature account implementation
 */

#include "neoc/wallet/multi_sig.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include "neoc/script/opcode.h"
#include <string.h>
#include <stdlib.h>

// Compare public keys for sorting
static int compare_public_keys(const void *a, const void *b) {
    neoc_ec_public_key_t *key_a = *(neoc_ec_public_key_t **)a;
    neoc_ec_public_key_t *key_b = *(neoc_ec_public_key_t **)b;
    
    uint8_t *bytes_a = NULL, *bytes_b = NULL;
    size_t size_a = 0, size_b = 0;
    
    neoc_ec_public_key_get_encoded(key_a, true, &bytes_a, &size_a);
    neoc_ec_public_key_get_encoded(key_b, true, &bytes_b, &size_b);
    
    int result = memcmp(bytes_a, bytes_b, (size_a < size_b) ? size_a : size_b);
    
    neoc_free(bytes_a);
    neoc_free(bytes_b);
    
    return result;
}

neoc_error_t neoc_multi_sig_create(uint8_t threshold,
                                    neoc_ec_public_key_t **public_keys,
                                    uint8_t key_count,
                                    neoc_multi_sig_account_t **account) {
    if (!public_keys || !account || key_count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (threshold == 0 || threshold > key_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid threshold");
    }
    
    if (key_count > 255) {  // Changed from 1024 to 255 to fix comparison warning
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Too many public keys");
    }
    
    // Allocate account structure
    *account = neoc_calloc(1, sizeof(neoc_multi_sig_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate multi-sig account");
    }
    
    (*account)->threshold = threshold;
    (*account)->public_key_count = key_count;
    
    // Copy and sort public keys
    (*account)->public_keys = neoc_calloc(key_count, sizeof(neoc_ec_public_key_t *));
    if (!(*account)->public_keys) {
        neoc_free(*account);
        *account = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate public keys array");
    }
    
    // Copy public keys
    for (uint8_t i = 0; i < key_count; i++) {
        (*account)->public_keys[i] = public_keys[i];
    }
    
    // Sort public keys lexicographically
    qsort((*account)->public_keys, key_count, sizeof(neoc_ec_public_key_t *), compare_public_keys);
    
    // Build verification script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        neoc_free((*account)->public_keys);
        neoc_free(*account);
        *account = NULL;
        return err;
    }
    
    // Push threshold (M)
    err = neoc_script_builder_push_integer(builder, threshold);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Push all public keys
    for (uint8_t i = 0; i < key_count; i++) {
        uint8_t *pubkey_bytes = NULL;
        size_t pubkey_size = 0;
        
        err = neoc_ec_public_key_get_encoded((*account)->public_keys[i], true, &pubkey_bytes, &pubkey_size);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
        
        err = neoc_script_builder_push_data(builder, pubkey_bytes, pubkey_size);
        neoc_free(pubkey_bytes);
        if (err != NEOC_SUCCESS) {
            goto cleanup;
        }
    }
    
    // Push key count (N)
    err = neoc_script_builder_push_integer(builder, key_count);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Add CHECKMULTISIG syscall
    err = neoc_script_builder_emit_syscall(builder, NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    // Get the script
    (*account)->script_size = neoc_script_builder_get_size(builder);
    (*account)->verification_script = neoc_malloc((*account)->script_size);
    if (!(*account)->verification_script) {
        err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate verification script");
        goto cleanup;
    }
    
    uint8_t *script_data = NULL;
    size_t actual_size = 0;
    err = neoc_script_builder_to_array(builder, &script_data, &actual_size);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    if (actual_size != (*account)->script_size) {
        neoc_free(script_data);
        err = neoc_error_set(NEOC_ERROR_INTERNAL, "Script size mismatch");
        goto cleanup;
    }
    
    memcpy((*account)->verification_script, script_data, (*account)->script_size);
    neoc_free(script_data);
    
    // Calculate script hash
    err = neoc_hash160_from_script(&(*account)->script_hash, 
                                    (*account)->verification_script, 
                                    (*account)->script_size);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
    
cleanup:
    neoc_script_builder_free(builder);
    if ((*account)->verification_script) {
        neoc_free((*account)->verification_script);
    }
    neoc_free((*account)->public_keys);
    neoc_free(*account);
    *account = NULL;
    return err;
}

neoc_error_t neoc_multi_sig_get_verification_script(const neoc_multi_sig_account_t *account,
                                                     uint8_t *script,
                                                     size_t script_size,
                                                     size_t *actual_size) {
    if (!account || !script || !actual_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *actual_size = account->script_size;
    
    if (script_size < account->script_size) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Script buffer too small");
    }
    
    memcpy(script, account->verification_script, account->script_size);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_multi_sig_get_script_hash(const neoc_multi_sig_account_t *account,
                                             neoc_hash160_t *hash) {
    if (!account || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(hash, &account->script_hash, sizeof(neoc_hash160_t));
    return NEOC_SUCCESS;
}

neoc_error_t neoc_multi_sig_create_witness(const neoc_multi_sig_account_t *account,
                                            const uint8_t **signatures,
                                            uint8_t sig_count,
                                            uint8_t *invocation_script,
                                            size_t invocation_size,
                                            size_t *actual_size) {
    if (!account || !signatures || !invocation_script || !actual_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (sig_count < account->threshold) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Not enough signatures");
    }
    
    // Build invocation script
    neoc_script_builder_t *builder = NULL;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push each signature
    for (uint8_t i = 0; i < sig_count && i < account->threshold; i++) {
        if (!signatures[i]) {
            neoc_script_builder_free(builder);
            return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Null signature");
        }
        
        // Signatures are typically 64 bytes (r + s)
        err = neoc_script_builder_push_data(builder, signatures[i], 64);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            return err;
        }
    }
    
    // Get the invocation script
    size_t script_len = neoc_script_builder_get_size(builder);
    *actual_size = script_len;
    
    if (invocation_size < script_len) {
        neoc_script_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Invocation script buffer too small");
    }
    
    uint8_t *script_data = NULL;
    size_t actual_len = 0;
    err = neoc_script_builder_to_array(builder, &script_data, &actual_len);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    if (actual_len != script_len) {
        neoc_free(script_data);
        neoc_script_builder_free(builder);
        return neoc_error_set(NEOC_ERROR_INTERNAL, "Script size mismatch");
    }
    
    memcpy(invocation_script, script_data, script_len);
    neoc_free(script_data);
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

bool neoc_multi_sig_contains_key(const neoc_multi_sig_account_t *account,
                                  const neoc_ec_public_key_t *public_key) {
    if (!account || !public_key) {
        return false;
    }
    
    uint8_t *key_bytes = NULL;
    size_t key_size = 0;
    
    if (neoc_ec_public_key_get_encoded(public_key, true, &key_bytes, &key_size) != NEOC_SUCCESS) {
        return false;
    }
    
    for (uint8_t i = 0; i < account->public_key_count; i++) {
        uint8_t *account_key_bytes = NULL;
        size_t account_key_size = 0;
        
        if (neoc_ec_public_key_get_encoded(account->public_keys[i], 
                                           true,
                                           &account_key_bytes, 
                                           &account_key_size) == NEOC_SUCCESS) {
            bool match = (key_size == account_key_size) && 
                        (memcmp(key_bytes, account_key_bytes, key_size) == 0);
            neoc_free(account_key_bytes);
            if (match) {
                neoc_free(key_bytes);
                return true;
            }
        }
    }
    
    neoc_free(key_bytes);
    return false;
}

void neoc_multi_sig_free(neoc_multi_sig_account_t *account) {
    if (!account) {
        return;
    }
    
    if (account->public_keys) {
        neoc_free(account->public_keys);
    }
    
    if (account->verification_script) {
        neoc_free(account->verification_script);
    }
    
    neoc_free(account);
}
