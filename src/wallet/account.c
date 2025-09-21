/**
 * @file account.c
 * @brief NEO account implementation
 */

#include "neoc/wallet/account.h"
#include "neoc/wallet/multi_sig.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/crypto/sign.h"
#include "neoc/script/script_helper.h"
#include "neoc/transaction/witness.h"
#include <string.h>
#include <stdlib.h>

// Helper function for internal use
static bool is_multisig_internal(const neoc_account_t *account) {
    if (!account || !account->extra) {
        return false;
    }
    
    // Check if extra field contains multi-sig data
    neoc_multi_sig_account_t *multisig = (neoc_multi_sig_account_t *)account->extra;
    
    // Validate that it's actually a multi-sig structure by checking reasonable threshold values
    if (multisig->threshold > 0 && multisig->threshold <= multisig->public_key_count && 
        multisig->public_key_count > 1 && multisig->public_key_count <= 255) {
        return true;
    }
    
    // Also check for address-only multi-sig info
    typedef struct {
        int threshold;
        int nr_participants;
        bool is_address_only;
    } multisig_info_t;
    
    multisig_info_t *info = (multisig_info_t *)account->extra;
    if (info->threshold > 0 && info->nr_participants > 1 && info->threshold <= info->nr_participants) {
        return true;
    }
    
    return false;
}

// Account implementation uses the public struct from account.h

neoc_error_t neoc_account_create(const char *label, neoc_account_t **account) {
    if (!account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid account pointer");
    }
    
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Generate new key pair
    neoc_error_t err = neoc_ec_key_pair_create_random(&(*account)->key_pair);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Generate verification script from public key
    uint8_t public_key[65];  // Buffer for public key
    size_t public_key_len = sizeof(public_key);
    err = neoc_ec_key_pair_get_public_key((*account)->key_pair, public_key, &public_key_len);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    uint8_t *verification_script = NULL;
    size_t verification_script_len = 0;
    err = neoc_script_create_single_sig_verification(public_key, public_key_len,
                                                     &verification_script,
                                                     &verification_script_len);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    // Calculate script hash from verification script (value type, not pointer)
    err = neoc_hash160_from_script(&(*account)->script_hash,
                                   verification_script,
                                   verification_script_len);
    neoc_free(verification_script);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    // Generate address from script hash
    (*account)->address = neoc_malloc(NEOC_ADDRESS_LENGTH);
    if (!(*account)->address) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    err = neoc_hash160_to_address(&(*account)->script_hash, (*account)->address, NEOC_ADDRESS_LENGTH);
    if (err != NEOC_SUCCESS) {
        neoc_free((*account)->address);
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    // Set label
    if (label) {
        (*account)->label = strdup(label);
    } else {
        (*account)->label = strdup("Account");
    }
    
    (*account)->is_default = false;
    (*account)->is_locked = false;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_create_from_key_pair(const char *label,
                                               const neoc_ec_key_pair_t *key_pair,
                                               neoc_account_t **account) {
    if (!key_pair || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Get private key from source key pair
    uint8_t private_key[32];
    size_t private_key_len = sizeof(private_key);
    neoc_error_t err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &private_key_len);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Create new key pair from private key
    err = neoc_ec_key_pair_create_from_private_key(private_key, &(*account)->key_pair);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Generate verification script from public key
    uint8_t public_key[65];  // Buffer for public key
    size_t public_key_len = sizeof(public_key);
    err = neoc_ec_key_pair_get_public_key((*account)->key_pair, public_key, &public_key_len);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    uint8_t *verification_script = NULL;
    size_t verification_script_len = 0;
    err = neoc_script_create_single_sig_verification(public_key, public_key_len,
                                                     &verification_script,
                                                     &verification_script_len);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    // Calculate script hash (value type, not pointer)
    err = neoc_hash160_from_script(&(*account)->script_hash,
                                   verification_script,
                                   verification_script_len);
    neoc_free(verification_script);
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    // Generate address
    (*account)->address = neoc_malloc(NEOC_ADDRESS_LENGTH);
    if (!(*account)->address) {
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    err = neoc_hash160_to_address(&(*account)->script_hash, (*account)->address, NEOC_ADDRESS_LENGTH);
    if (err != NEOC_SUCCESS) {
        neoc_free((*account)->address);
        neoc_ec_key_pair_free((*account)->key_pair);
        neoc_free(*account);
        return err;
    }
    
    // Set label
    if (label) {
        (*account)->label = strdup(label);
    } else {
        (*account)->label = strdup("Imported Account");
    }
    
    (*account)->is_default = false;
    (*account)->is_locked = false;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_create_from_wif(const char *label,
                                           const char *wif,
                                           neoc_account_t **account) {
    if (!wif || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Import key pair from WIF
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_import_from_wif(wif, &key_pair);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create account from key pair
    err = neoc_account_create_from_key_pair(label, key_pair, account);
    neoc_ec_key_pair_free(key_pair);
    
    return err;
}

neoc_error_t neoc_account_create_from_nep2(const char *label,
                                            const char *encrypted_key,
                                            const char *passphrase,
                                            neoc_account_t **account) {
    (void)label;  // Suppress unused parameter warning
    if (!encrypted_key || !passphrase || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Decrypt NEP-2 encrypted private key
    uint8_t private_key[32];
    neoc_error_t err = neoc_nep2_decrypt(encrypted_key, passphrase, private_key);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create EC key pair from private key
    neoc_ec_key_pair_t *key_pair = NULL;
    err = neoc_ec_key_pair_from_private_key(private_key, 32, &key_pair);
    
    // Clear private key from memory
    memset(private_key, 0, sizeof(private_key));
    
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create account from key pair
    return neoc_account_from_key_pair(key_pair, label, account);
}



neoc_error_t neoc_account_get_public_key(const neoc_account_t *account,
                                         bool compressed,
                                         uint8_t **public_key,
                                         size_t *key_len) {
    if (!account || !public_key || !key_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!account->key_pair) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no key pair");
    }
    
    // Allocate buffer for public key (compressed=33, uncompressed=65)
    size_t buffer_size = compressed ? 33 : 65;
    uint8_t *buffer = neoc_malloc(buffer_size);
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate public key buffer");
    }
    
    size_t actual_len = buffer_size;
    neoc_error_t err = neoc_ec_key_pair_get_public_key(account->key_pair, buffer, &actual_len);
    if (err != NEOC_SUCCESS) {
        neoc_free(buffer);
        return err;
    }
    
    *public_key = buffer;
    *key_len = actual_len;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_export_wif(const neoc_account_t *account, char **wif) {
    if (!account || !wif) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (account->is_locked) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account is locked");
    }
    
    if (!account->key_pair) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no key pair");
    }
    
    return neoc_ec_key_pair_export_as_wif(account->key_pair, wif);
}

neoc_error_t neoc_account_export_nep2(const neoc_account_t *account,
                                      const char *password,
                                      char **nep2_key) {
    if (!account || !password || !nep2_key) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!account->key_pair) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no key pair to encrypt");
    }
    
    // Get private key from account
    uint8_t private_key[32];
    neoc_error_t err = neoc_ec_key_pair_get_private_key(account->key_pair, private_key, sizeof(private_key));
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create NEP-2 encrypted key
    err = neoc_nep2_create_from_private_key(private_key, password, nep2_key);
    
    // Clear private key from memory
    memset(private_key, 0, sizeof(private_key));
    
    return err;
}

neoc_error_t neoc_account_sign_data(const neoc_account_t *account,
                                    const uint8_t *data,
                                    size_t data_len,
                                    uint8_t **signature,
                                    size_t *signature_len) {
    (void)data_len;  // Suppress unused parameter warning
    if (!account || !data || !signature || !signature_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!account->key_pair) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no key pair for signing");
    }
    
    // Sign data with account's key pair
    return neoc_ec_key_pair_sign(account->key_pair, data, data_len, signature, signature_len);
}

neoc_error_t neoc_account_sign_hash(const neoc_account_t *account,
                                    const neoc_hash256_t *hash,
                                    neoc_witness_t **witness) {
    if (!account || !hash || !witness) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!account->key_pair) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no key pair for signing");
    }
    
    // Sign hash
    uint8_t *signature = NULL;
    size_t signature_len = 0;
    neoc_error_t err = neoc_ec_key_pair_sign(account->key_pair, hash->data, 32, &signature, &signature_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create invocation script from signature
    uint8_t *invocation_script = NULL;
    size_t invocation_len = 0;
    err = neoc_script_create_invocation_from_signature(signature, signature_len, &invocation_script, &invocation_len);
    neoc_free(signature);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Get verification script
    uint8_t *verification_script = NULL;
    size_t verification_len = 0;
    err = neoc_account_get_verification_script(account, &verification_script, &verification_len);
    if (err != NEOC_SUCCESS) {
        neoc_free(invocation_script);
        return err;
    }
    
    // Create witness
    err = neoc_witness_create(invocation_script, invocation_len, verification_script, verification_len, witness);
    
    neoc_free(invocation_script);
    neoc_free(verification_script);
    
    return err;
}



bool neoc_account_is_default(const neoc_account_t *account) {
    return account ? account->is_default : false;
}

void neoc_account_set_default(neoc_account_t *account, bool is_default) {
    if (account) {
        account->is_default = is_default;
    }
}



neoc_error_t neoc_account_set_label(neoc_account_t *account, const char *label) {
    if (!account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid account");
    }
    
    if (account->label) {
        free(account->label);
    }
    account->label = label ? strdup(label) : NULL;
    return NEOC_SUCCESS;
}

// Multi-signature account functions

neoc_error_t neoc_account_create_multisig_from_public_keys(neoc_ec_public_key_t **public_keys,
                                                           size_t key_count,
                                                           int threshold,
                                                           neoc_account_t **account) {
    if (!public_keys || !account || key_count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (threshold <= 0 || threshold > (int)key_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid threshold");
    }
    
    if (key_count > 1024) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Too many public keys");
    }
    
    // Create multi-sig account using the multi_sig module
    neoc_multi_sig_account_t *multisig = NULL;
    neoc_error_t err = neoc_multi_sig_create((uint8_t)threshold, public_keys, (uint8_t)key_count, &multisig);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Create account structure
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        neoc_multi_sig_free(multisig);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Get address from script hash
    (*account)->address = neoc_malloc(NEOC_ADDRESS_LENGTH);
    if (!(*account)->address) {
        neoc_multi_sig_free(multisig);
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    err = neoc_hash160_to_address(&multisig->script_hash, (*account)->address, NEOC_ADDRESS_LENGTH);
    if (err != NEOC_SUCCESS) {
        neoc_free((*account)->address);
        neoc_multi_sig_free(multisig);
        neoc_free(*account);
        return err;
    }
    
    // Copy script hash
    memcpy(&(*account)->script_hash, &multisig->script_hash, sizeof(neoc_hash160_t));
    
    // Set up multi-sig specific fields
    (*account)->label = strdup((*account)->address);
    (*account)->key_pair = NULL; // Multi-sig accounts don't have a single key pair
    (*account)->is_default = false;
    (*account)->is_locked = false;
    (*account)->encrypted_key = NULL;
    (*account)->encrypted_key_len = 0;
    (*account)->extra = multisig; // Store the multi-sig data in extra field
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_create_multisig_with_address(const char *address,
                                                      int threshold,
                                                      int nr_participants,
                                                      neoc_account_t **account) {
    if (!address || !account || threshold <= 0 || nr_participants <= 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (threshold > nr_participants) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Threshold cannot exceed number of participants");
    }
    
    // Create account structure
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Set address
    (*account)->address = strdup(address);
    if (!(*account)->address) {
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    // Calculate script hash from address
    neoc_error_t err = neoc_hash160_from_address(&(*account)->script_hash, address);
    if (err != NEOC_SUCCESS) {
        free((*account)->address);
        neoc_free(*account);
        return err;
    }
    
    // Create a simple multi-sig info structure to store threshold and participants
    typedef struct {
        int threshold;
        int nr_participants;
        bool is_address_only;
    } multisig_info_t;
    
    multisig_info_t *info = neoc_malloc(sizeof(multisig_info_t));
    if (!info) {
        free((*account)->address);
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate multi-sig info");
    }
    
    info->threshold = threshold;
    info->nr_participants = nr_participants;
    info->is_address_only = true;
    
    // Set up account fields
    (*account)->label = strdup(address);
    (*account)->key_pair = NULL;
    (*account)->is_default = false;
    (*account)->is_locked = false;
    (*account)->encrypted_key = NULL;
    (*account)->encrypted_key_len = 0;
    (*account)->extra = info;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_create_from_verification_script(const uint8_t *script,
                                                          size_t script_len,
                                                          neoc_account_t **account) {
    if (!script || script_len == 0 || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create account structure
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Calculate script hash from verification script
    neoc_error_t err = neoc_hash160_from_script(&(*account)->script_hash, script, script_len);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Generate address from script hash
    (*account)->address = neoc_malloc(NEOC_ADDRESS_LENGTH);
    if (!(*account)->address) {
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    err = neoc_hash160_to_address(&(*account)->script_hash, (*account)->address, NEOC_ADDRESS_LENGTH);
    if (err != NEOC_SUCCESS) {
        neoc_free((*account)->address);
        neoc_free(*account);
        return err;
    }
    
    // Analyze script to determine if it's multi-sig and extract threshold/participants
    if (script_len >= 4) {
        size_t pos = 0;
        
        // Check for multi-sig pattern: PUSH<m> ... PUSH<n> SYSCALL
        // First byte should be PUSH opcode for threshold (m)
        uint8_t threshold = 0;
        if (script[pos] >= 0x11 && script[pos] <= 0x20) { // PUSH1 to PUSH16
            threshold = script[pos] - 0x10;
            pos++;
        } else if (script[pos] == 0x00) { // PUSHINT8
            pos++;
            if (pos < script_len) {
                threshold = script[pos++];
            }
        }
        
        // Count public keys (33-byte compressed or 65-byte uncompressed)
        int pubkey_count = 0;
        while (pos < script_len - 2) {
            if (script[pos] == 0x21) { // 33-byte push (compressed pubkey)
                pos += 34; // Skip opcode + 33 bytes
                pubkey_count++;
            } else if (script[pos] == 0x41) { // 65-byte push (uncompressed pubkey)  
                pos += 66; // Skip opcode + 65 bytes
                pubkey_count++;
            } else if (script[pos] == 0x0C && pos + 1 < script_len) { // PUSHDATA1
                uint8_t len = script[pos + 1];
                if (len == 33 || len == 65) {
                    pos += 2 + len;
                    pubkey_count++;
                } else {
                    break;
                }
            } else {
                break; // Not a public key push
            }
        }
        
        // Check for participant count push
        uint8_t participants = 0;
        if (pos < script_len && script[pos] >= 0x11 && script[pos] <= 0x20) { // PUSH1 to PUSH16
            participants = script[pos] - 0x10;
            pos++;
        } else if (pos < script_len && script[pos] == 0x00) { // PUSHINT8
            pos++;
            if (pos < script_len) {
                participants = script[pos++];
            }
        }
        
        // Check for CHECKMULTISIG syscall (0x41 followed by 4-byte hash)
        bool is_multisig = false;
        if (pos + 4 < script_len && script[pos] == 0x41) { // SYSCALL
            // Check for System.Crypto.CheckMultiSig hash: 0xbb3b1f6f
            if (script[pos + 1] == 0xbb && script[pos + 2] == 0x3b &&
                script[pos + 3] == 0x1f && script[pos + 4] == 0x6f) {
                is_multisig = true;
            }
        }
        
        // If it's a multi-sig script, store the info
        if (is_multisig && threshold > 0 && participants > 0 && 
            participants == pubkey_count && threshold <= participants) {
            // Allocate multisig info structure
            typedef struct {
                int threshold;
                int nr_participants;
                bool is_address_only;
            } multisig_info_t;
            
            multisig_info_t* info = neoc_malloc(sizeof(multisig_info_t));
            if (info) {
                info->threshold = threshold;
                info->nr_participants = participants;
                info->is_address_only = true; // We don't have the public keys stored
                (*account)->extra = info;
            }
        }
    }
    
    // Set basic fields
    (*account)->label = strdup((*account)->address);
    (*account)->key_pair = NULL;
    (*account)->is_default = false;
    (*account)->is_locked = false;
    (*account)->encrypted_key = NULL;
    (*account)->encrypted_key_len = 0;
    (*account)->extra = NULL;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_create_from_public_key(neoc_ec_public_key_t *public_key,
                                                  neoc_account_t **account) {
    if (!public_key || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create account structure
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Get encoded public key
    uint8_t *pubkey_bytes = NULL;
    size_t pubkey_size = 0;
    neoc_error_t err = neoc_ec_public_key_get_encoded(public_key, true, &pubkey_bytes, &pubkey_size);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Generate verification script from public key
    uint8_t *verification_script = NULL;
    size_t verification_script_len = 0;
    err = neoc_script_create_single_sig_verification(pubkey_bytes, pubkey_size,
                                                     &verification_script,
                                                     &verification_script_len);
    neoc_free(pubkey_bytes);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Calculate script hash from verification script
    err = neoc_hash160_from_script(&(*account)->script_hash, verification_script, verification_script_len);
    neoc_free(verification_script);
    if (err != NEOC_SUCCESS) {
        neoc_free(*account);
        return err;
    }
    
    // Generate address from script hash
    (*account)->address = neoc_malloc(NEOC_ADDRESS_LENGTH);
    if (!(*account)->address) {
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    err = neoc_hash160_to_address(&(*account)->script_hash, (*account)->address, NEOC_ADDRESS_LENGTH);
    if (err != NEOC_SUCCESS) {
        neoc_free((*account)->address);
        neoc_free(*account);
        return err;
    }
    
    // Set up account fields
    (*account)->label = strdup((*account)->address);
    (*account)->key_pair = NULL; // We don't have the private key, only public key
    (*account)->is_default = false;
    (*account)->is_locked = false;
    (*account)->encrypted_key = NULL;
    (*account)->encrypted_key_len = 0;
    (*account)->extra = NULL;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_create_from_address(const char *address, neoc_account_t **account) {
    if (!address || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create account structure
    *account = neoc_calloc(1, sizeof(neoc_account_t));
    if (!*account) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    // Set address
    (*account)->address = strdup(address);
    if (!(*account)->address) {
        neoc_free(*account);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    // Calculate script hash from address
    neoc_error_t err = neoc_hash160_from_address(&(*account)->script_hash, address);
    if (err != NEOC_SUCCESS) {
        free((*account)->address);
        neoc_free(*account);
        return err;
    }
    
    // Set up account fields
    (*account)->label = strdup(address);
    (*account)->key_pair = NULL;
    (*account)->is_default = false;
    (*account)->is_locked = false;
    (*account)->encrypted_key = NULL;
    (*account)->encrypted_key_len = 0;
    (*account)->extra = NULL;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_get_address(const neoc_account_t *account, char **address) {
    if (!account || !address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!account->address) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no address");
    }
    
    *address = strdup(account->address);
    if (!*address) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate address");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_get_script_hash(const neoc_account_t *account, neoc_hash160_t *hash) {
    if (!account || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(hash, &account->script_hash, sizeof(neoc_hash160_t));
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_get_label(const neoc_account_t *account, char **label) {
    if (!account || !label) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!account->label) {
        *label = NULL;
        return NEOC_SUCCESS;
    }
    
    *label = strdup(account->label);
    if (!*label) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate label");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_get_verification_script(const neoc_account_t *account,
                                                   uint8_t **script,
                                                   size_t *script_len) {
    if (!account || !script || !script_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Check if this is a multi-sig account with verification script
    if (is_multisig_internal(account)) {
        neoc_multi_sig_account_t *multisig = (neoc_multi_sig_account_t *)account->extra;
        if (multisig && multisig->verification_script) {
            *script = neoc_malloc(multisig->script_size);
            if (!*script) {
                return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate script");
            }
            memcpy(*script, multisig->verification_script, multisig->script_size);
            *script_len = multisig->script_size;
            return NEOC_SUCCESS;
        }
    }
    
    // For single-sig accounts, generate verification script from public key
    if (account->key_pair) {
        uint8_t public_key[65];
        size_t public_key_len = sizeof(public_key);
        neoc_error_t err = neoc_ec_key_pair_get_public_key(account->key_pair, public_key, &public_key_len);
        if (err != NEOC_SUCCESS) {
            return err;
        }
        
        return neoc_script_create_single_sig_verification(public_key, public_key_len, script, script_len);
    }
    
    // No verification script available
    *script = NULL;
    *script_len = 0;
    return neoc_error_set(NEOC_ERROR_INVALID_STATE, "No verification script available");
}

neoc_error_t neoc_account_is_multisig(const neoc_account_t *account, bool *is_multisig) {
    if (!account || !is_multisig) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *is_multisig = is_multisig_internal(account);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_get_signing_threshold(const neoc_account_t *account, int *threshold) {
    if (!account || !threshold) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!is_multisig_internal(account)) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account is not multi-signature");
    }
    
    if (account->extra) {
        // Try multi-sig account structure first
        neoc_multi_sig_account_t *multisig = (neoc_multi_sig_account_t *)account->extra;
        if (multisig->threshold > 0 && multisig->threshold <= multisig->public_key_count) {
            *threshold = multisig->threshold;
            return NEOC_SUCCESS;
        }
        
        // Try address-only multi-sig info
        typedef struct {
            int threshold;
            int nr_participants;
            bool is_address_only;
        } multisig_info_t;
        
        multisig_info_t *info = (multisig_info_t *)account->extra;
        if (info->threshold > 0) {
            *threshold = info->threshold;
            return NEOC_SUCCESS;
        }
    }
    
    return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Cannot determine signing threshold");
}

neoc_error_t neoc_account_get_nr_participants(const neoc_account_t *account, int *nr_participants) {
    if (!account || !nr_participants) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!is_multisig_internal(account)) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account is not multi-signature");
    }
    
    if (account->extra) {
        // Try multi-sig account structure first
        neoc_multi_sig_account_t *multisig = (neoc_multi_sig_account_t *)account->extra;
        if (multisig->public_key_count > 0) {
            *nr_participants = multisig->public_key_count;
            return NEOC_SUCCESS;
        }
        
        // Try address-only multi-sig info
        typedef struct {
            int threshold;
            int nr_participants;
            bool is_address_only;
        } multisig_info_t;
        
        multisig_info_t *info = (multisig_info_t *)account->extra;
        if (info->nr_participants > 0) {
            *nr_participants = info->nr_participants;
            return NEOC_SUCCESS;
        }
    }
    
    return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Cannot determine number of participants");
}

neoc_error_t neoc_account_lock(neoc_account_t *account) {
    if (!account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    account->is_locked = true;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_unlock(neoc_account_t *account) {
    if (!account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    account->is_locked = false;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_account_is_locked(const neoc_account_t *account, bool *is_locked) {
    if (!account || !is_locked) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *is_locked = account->is_locked;
    return NEOC_SUCCESS;
}

void neoc_account_free(neoc_account_t *account) {
    if (!account) return;
    
    if (account->key_pair) {
        neoc_ec_key_pair_free(account->key_pair);
    }
    
    if (account->address) {
        free(account->address);
    }
    
    if (account->label) {
        free(account->label);
    }
    
    if (account->encrypted_key) {
        // Clear encrypted key from memory before freeing
        size_t len = account->encrypted_key_len;
        memset(account->encrypted_key, 0, len);
        free(account->encrypted_key);
    }
    
    // Free multi-sig data if present
    if (account->extra) {
        // Check if it's a full multi-sig account or just info
        if (is_multisig_internal(account)) {
            neoc_multi_sig_account_t *multisig = (neoc_multi_sig_account_t *)account->extra;
            if (multisig->public_key_count > 0 && multisig->verification_script) {
                // It's a full multi-sig account
                neoc_multi_sig_free(multisig);
            } else {
                // It's just multi-sig info
                neoc_free(account->extra);
            }
        } else {
            neoc_free(account->extra);
        }
    }
    
    neoc_free(account);
}
