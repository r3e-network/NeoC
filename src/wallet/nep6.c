#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

/**
 * @file nep6.c
 * @brief NEP-6 wallet file format implementation
 */

#include "neoc/wallet/nep6.h"
#include "neoc/crypto/nep2.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// NEP-6 account structure
struct neoc_nep6_account_t {
    char *address;
    char *label;
    bool is_default;
    bool lock;
    char *key;  // NEP-2 encrypted key
    neoc_nep6_contract_t *contract;
    void *extra;  // Extra data
};

// NEP-6 contract structure
struct neoc_nep6_contract_t {
    char *script;
    neoc_nep6_parameter_t *parameters;
    size_t parameter_count;
    bool is_deployed;
};

// NEP-6 wallet structure
struct neoc_nep6_wallet_t {
    char *name;
    char *version;
    neoc_nep6_scrypt_params_t scrypt;
    neoc_nep6_account_t **accounts;
    size_t account_count;
    size_t account_capacity;
    void *extra;  // Extra data
};

// Helper to duplicate string
static char* str_dup(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = neoc_malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

neoc_error_t neoc_nep6_wallet_create(const char *name,
                                      const char *version,
                                      neoc_nep6_wallet_t **wallet) {
    if (!wallet) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid wallet pointer");
    }
    
    *wallet = neoc_calloc(1, sizeof(neoc_nep6_wallet_t));
    if (!*wallet) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate wallet");
    }
    
    (*wallet)->name = str_dup(name ? name : "NeoC Wallet");
    (*wallet)->version = str_dup(version ? version : "1.0");
    
    // Set default scrypt parameters
    (*wallet)->scrypt.n = 16384;
    (*wallet)->scrypt.r = 8;
    (*wallet)->scrypt.p = 8;
    
    // Initialize account array
    (*wallet)->account_capacity = 10;
    (*wallet)->accounts = neoc_calloc((*wallet)->account_capacity, sizeof(neoc_nep6_account_t*));
    if (!(*wallet)->accounts) {
        neoc_free((*wallet)->name);
        neoc_free((*wallet)->version);
        neoc_free(*wallet);
        *wallet = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate accounts");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep6_wallet_add_account(neoc_nep6_wallet_t *wallet,
                                           const uint8_t *private_key,
                                           const char *password,
                                           const char *label,
                                           bool is_default) {
    if (!wallet || !private_key || !password) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create EC key pair
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_from_private_key(private_key, 32, &key_pair);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Get address
    char *address_ptr = NULL;
    err = neoc_ec_key_pair_get_address(key_pair, &address_ptr);
    if (err != NEOC_SUCCESS || !address_ptr) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    char address[64];
    strncpy(address, address_ptr, sizeof(address) - 1);
    address[sizeof(address) - 1] = '\0';
    neoc_free(address_ptr);
    
    // Encrypt private key using NEP-2
    char encrypted_key[64];
    neoc_nep2_params_t nep2_params = {
        .n = wallet->scrypt.n,
        .r = wallet->scrypt.r,
        .p = wallet->scrypt.p
    };
    err = neoc_nep2_encrypt(private_key, password, &nep2_params, encrypted_key, sizeof(encrypted_key));
    if (err != NEOC_SUCCESS) {
        neoc_ec_key_pair_free(key_pair);
        return err;
    }
    
    // Create account
    neoc_nep6_account_t *account = neoc_calloc(1, sizeof(neoc_nep6_account_t));
    if (!account) {
        neoc_ec_key_pair_free(key_pair);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate account");
    }
    
    account->address = str_dup(address);
    account->label = str_dup(label);
    account->is_default = is_default;
    account->lock = false;
    account->key = str_dup(encrypted_key);
    
    // Create default contract (single signature)
    account->contract = neoc_calloc(1, sizeof(neoc_nep6_contract_t));
    if (account->contract) {
        // Get public key
        uint8_t public_key[65];
        size_t public_key_len = sizeof(public_key);
        neoc_ec_key_pair_get_public_key(key_pair, public_key, &public_key_len);
        
        // Create verification script (simplified)
        uint8_t script[35];
        script[0] = 0x21; // PUSH21
        memcpy(script + 1, public_key, 33);
        script[34] = 0xAC; // CHECKSIG
        
        // Convert to hex
        char hex_script[71];
        for (int i = 0; i < 35; i++) {
            sprintf(hex_script + i * 2, "%02x", script[i]);
        }
        account->contract->script = str_dup(hex_script);
        
        // Set parameter
        account->contract->parameter_count = 1;
        account->contract->parameters = neoc_malloc(sizeof(neoc_nep6_parameter_t));
        if (account->contract->parameters) {
            account->contract->parameters[0].name = str_dup("signature");
            account->contract->parameters[0].type = NEOC_PARAM_TYPE_SIGNATURE;
        }
        account->contract->is_deployed = false;
    }
    
    neoc_ec_key_pair_free(key_pair);
    
    // Add to wallet
    if (wallet->account_count >= wallet->account_capacity) {
        // Resize array
        size_t new_capacity = wallet->account_capacity * 2;
        neoc_nep6_account_t **new_accounts = neoc_realloc(wallet->accounts,
                                                           new_capacity * sizeof(neoc_nep6_account_t*));
        if (!new_accounts) {
            neoc_nep6_account_free(account);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize accounts");
        }
        wallet->accounts = new_accounts;
        wallet->account_capacity = new_capacity;
    }
    
    // If this is the default account, clear default flag on others
    if (is_default) {
        for (size_t i = 0; i < wallet->account_count; i++) {
            wallet->accounts[i]->is_default = false;
        }
    }
    
    wallet->accounts[wallet->account_count++] = account;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep6_wallet_remove_account(neoc_nep6_wallet_t *wallet,
                                              const char *address) {
    if (!wallet || !address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    for (size_t i = 0; i < wallet->account_count; i++) {
        if (strcmp(wallet->accounts[i]->address, address) == 0) {
            // Free account
            neoc_nep6_account_free(wallet->accounts[i]);
            
            // Shift remaining accounts
            for (size_t j = i; j < wallet->account_count - 1; j++) {
                wallet->accounts[j] = wallet->accounts[j + 1];
            }
            
            wallet->account_count--;
            return NEOC_SUCCESS;
        }
    }
    
    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Account not found");
}

neoc_error_t neoc_nep6_wallet_get_account(const neoc_nep6_wallet_t *wallet,
                                           const char *address,
                                           neoc_nep6_account_t **account) {
    if (!wallet || !address || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    for (size_t i = 0; i < wallet->account_count; i++) {
        if (strcmp(wallet->accounts[i]->address, address) == 0) {
            *account = wallet->accounts[i];
            return NEOC_SUCCESS;
        }
    }
    
    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Account not found");
}

neoc_error_t neoc_nep6_wallet_get_default_account(const neoc_nep6_wallet_t *wallet,
                                                   neoc_nep6_account_t **account) {
    if (!wallet || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    for (size_t i = 0; i < wallet->account_count; i++) {
        if (wallet->accounts[i]->is_default) {
            *account = wallet->accounts[i];
            return NEOC_SUCCESS;
        }
    }
    
    // If no default, return first account
    if (wallet->account_count > 0) {
        *account = wallet->accounts[0];
        return NEOC_SUCCESS;
    }
    
    return neoc_error_set(NEOC_ERROR_NOT_FOUND, "No accounts in wallet");
}

size_t neoc_nep6_wallet_get_account_count(const neoc_nep6_wallet_t *wallet) {
    return wallet ? wallet->account_count : 0;
}

neoc_error_t neoc_nep6_wallet_get_account_by_index(const neoc_nep6_wallet_t *wallet,
                                                    size_t index,
                                                    neoc_nep6_account_t **account) {
    if (!wallet || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (index >= wallet->account_count) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_BOUNDS, "Index out of bounds");
    }
    
    *account = wallet->accounts[index];
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep6_account_decrypt_private_key(const neoc_nep6_account_t *account,
                                                    const char *password,
                                                    uint8_t *private_key,
                                                    size_t private_key_len) {
    if (!account || !password || !private_key) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (private_key_len < 32) {
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small");
    }
    
    if (!account->key) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "No encrypted key");
    }
    
    // Decrypt using NEP-2
    return neoc_nep2_decrypt(account->key, password, NULL, private_key, private_key_len);
}

const char* neoc_nep6_account_get_address(const neoc_nep6_account_t *account) {
    return account ? account->address : NULL;
}

const char* neoc_nep6_account_get_label(const neoc_nep6_account_t *account) {
    return account ? account->label : NULL;
}

bool neoc_nep6_account_is_default(const neoc_nep6_account_t *account) {
    return account ? account->is_default : false;
}

void neoc_nep6_account_free(neoc_nep6_account_t *account) {
    if (!account) return;
    
    neoc_free(account->address);
    neoc_free(account->label);
    neoc_free(account->key);
    
    if (account->contract) {
        neoc_free(account->contract->script);
        if (account->contract->parameters) {
            for (size_t i = 0; i < account->contract->parameter_count; i++) {
                neoc_free(account->contract->parameters[i].name);
            }
            neoc_free(account->contract->parameters);
        }
        neoc_free(account->contract);
    }
    
    neoc_free(account->extra);
    neoc_free(account);
}

void neoc_nep6_wallet_free(neoc_nep6_wallet_t *wallet) {
    if (!wallet) return;
    
    neoc_free(wallet->name);
    neoc_free(wallet->version);
    
    for (size_t i = 0; i < wallet->account_count; i++) {
        neoc_nep6_account_free(wallet->accounts[i]);
    }
    neoc_free(wallet->accounts);
    
    neoc_free(wallet->extra);
    neoc_free(wallet);
}

// Simplified JSON serialization (without external JSON library)
neoc_error_t neoc_nep6_wallet_to_json(const neoc_nep6_wallet_t *wallet,
                                       char **json,
                                       size_t *json_len) {
    if (!wallet || !json || !json_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Estimate size
    size_t estimated_size = 4096 + wallet->account_count * 512;
    char *buffer = neoc_malloc(estimated_size);
    if (!buffer) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    // Build JSON manually (simplified)
    int offset = snprintf(buffer, estimated_size,
        "{\n"
        "  \"name\": \"%s\",\n"
        "  \"version\": \"%s\",\n"
        "  \"scrypt\": {\n"
        "    \"n\": %u,\n"
        "    \"r\": %u,\n"
        "    \"p\": %u\n"
        "  },\n"
        "  \"accounts\": [\n",
        wallet->name ? wallet->name : "",
        wallet->version ? wallet->version : "",
        wallet->scrypt.n,
        wallet->scrypt.r,
        wallet->scrypt.p
    );
    
    for (size_t i = 0; i < wallet->account_count; i++) {
        neoc_nep6_account_t *acc = wallet->accounts[i];
        offset += snprintf(buffer + offset, estimated_size - offset,
            "    {\n"
            "      \"address\": \"%s\",\n"
            "      \"label\": %s,\n"
            "      \"isDefault\": %s,\n"
            "      \"lock\": %s,\n"
            "      \"key\": \"%s\",\n"
            "      \"contract\": {\n"
            "        \"script\": \"",
            acc->address ? acc->address : "",
            acc->label ? (char[256]){0} : "null",  // Simplified
            acc->is_default ? "true" : "false",
            acc->lock ? "true" : "false",
            acc->key ? acc->key : ""
        );
        
        if (acc->label) {
            snprintf(buffer + offset - strlen("null,\n"), 256, "\"%s\",\n", acc->label);
        }
        
        offset = strlen(buffer);
        
        // Add the script (empty for now) and parameters
        offset += snprintf(buffer + offset, estimated_size - offset, 
            "\",\n"
            "        \"parameters\": []");
        
        if (acc->contract && acc->contract->is_deployed) {
            offset += snprintf(buffer + offset, estimated_size - offset,
                ",\n"
                "        \"deployed\": true\n"
                "      }\n"
                "    }%s\n",
                i < wallet->account_count - 1 ? "," : ""
            );
        } else {
            offset += snprintf(buffer + offset, estimated_size - offset,
                "],\n"
                "        \"deployed\": false\n"
                "      }\n"
                "    }%s\n",
                i < wallet->account_count - 1 ? "," : ""
            );
        }
    }
    
    offset += snprintf(buffer + offset, estimated_size - offset,
        "  ]\n"
        "}\n"
    );
    
    *json = buffer;
    *json_len = offset;
    
    return NEOC_SUCCESS;
}

// Simplified file I/O
neoc_error_t neoc_nep6_wallet_to_file(const neoc_nep6_wallet_t *wallet,
                                       const char *filename) {
    if (!wallet || !filename) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    char *json = NULL;
    size_t json_len = 0;
    neoc_error_t err = neoc_nep6_wallet_to_json(wallet, &json, &json_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        neoc_free(json);
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Failed to open file");
    }
    
    size_t written = fwrite(json, 1, json_len, file);
    fclose(file);
    neoc_free(json);
    
    if (written != json_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Failed to write file");
    }
    
    return NEOC_SUCCESS;
}

// JSON loading functions (returns NOT_IMPLEMENTED if JSON parser not available)
neoc_error_t neoc_nep6_wallet_from_json(const char *json_str,
                                         neoc_nep6_wallet_t **wallet) {
    if (!json_str || !wallet) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // This would require a JSON parser - simplified stub
    #ifdef HAVE_CJSON
    cJSON *json_obj = cJSON_Parse(json_str);
    if (!json_obj) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON");
    }
    
    *wallet = neoc_calloc(1, sizeof(neoc_nep6_wallet_t));
    if (!*wallet) {
        cJSON_Delete(json_obj);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate wallet");
    }
    
    // Parse name
    cJSON *name = cJSON_GetObjectItem(json_obj, "name");
    if (name && cJSON_IsString(name)) {
        (*wallet)->name = strdup(name->valuestring);
    }
    
    // Parse version
    cJSON *version = cJSON_GetObjectItem(json_obj, "version");
    if (version && cJSON_IsString(version)) {
        (*wallet)->version = strdup(version->valuestring);
    }
    
    // Parse scrypt parameters
    cJSON *scrypt = cJSON_GetObjectItem(json_obj, "scrypt");
    if (scrypt) {
        cJSON *n = cJSON_GetObjectItem(scrypt, "n");
        cJSON *r = cJSON_GetObjectItem(scrypt, "r");
        cJSON *p = cJSON_GetObjectItem(scrypt, "p");
        
        if (n) (*wallet)->scrypt.n = n->valueint;
        if (r) (*wallet)->scrypt.r = r->valueint;
        if (p) (*wallet)->scrypt.p = p->valueint;
    }
    
    // Parse accounts
    cJSON *accounts = cJSON_GetObjectItem(json_obj, "accounts");
    if (accounts && cJSON_IsArray(accounts)) {
        (*wallet)->account_count = cJSON_GetArraySize(accounts);
        (*wallet)->accounts = neoc_calloc((*wallet)->account_count, sizeof(neoc_nep6_account_t*));
        
        for (size_t i = 0; i < (*wallet)->account_count; i++) {
            cJSON *account = cJSON_GetArrayItem(accounts, (int)i);
            // Parse each account...
            neoc_nep6_account_t *acc = neoc_calloc(1, sizeof(neoc_nep6_account_t));
            
            cJSON *address = cJSON_GetObjectItem(account, "address");
            if (address && cJSON_IsString(address)) {
                acc->address = strdup(address->valuestring);
            }
            
            cJSON *label = cJSON_GetObjectItem(account, "label");
            if (label && cJSON_IsString(label)) {
                acc->label = strdup(label->valuestring);
            }
            
            cJSON *isDefault = cJSON_GetObjectItem(account, "isDefault");
            if (isDefault && cJSON_IsBool(isDefault)) {
                acc->is_default = cJSON_IsTrue(isDefault);
            }
            
            cJSON *lock = cJSON_GetObjectItem(account, "lock");
            if (lock && cJSON_IsBool(lock)) {
                acc->lock = cJSON_IsTrue(lock);
            }
            
            cJSON *key = cJSON_GetObjectItem(account, "key");
            if (key && cJSON_IsString(key)) {
                acc->key = strdup(key->valuestring);
            }
            
            (*wallet)->accounts[i] = acc;
        }
    }
    
    cJSON_Delete(json_obj);
    return NEOC_SUCCESS;
#else
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support not compiled in");
#endif
}

neoc_error_t neoc_nep6_wallet_from_file(const char *filename,
                                         neoc_nep6_wallet_t **wallet) {
    if (!filename || !wallet) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Read file contents
    FILE *file = fopen(filename, "r");
    if (!file) {
        return neoc_error_set(NEOC_ERROR_FILE, "Failed to open wallet file");
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || file_size > 10485760) { // Max 10MB for wallet file
        fclose(file);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid wallet file size");
    }
    
    // Read file contents
    char *content = neoc_malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    fclose(file);
    
    if (read_size != (size_t)file_size) {
        neoc_free(content);
        return neoc_error_set(NEOC_ERROR_FILE, "Failed to read wallet file");
    }
    
    content[file_size] = '\0';
    
    // Parse JSON into wallet
    neoc_error_t err = neoc_nep6_wallet_from_json(content, wallet);
    neoc_free(content);
    
    return err;
}
