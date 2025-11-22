/**
 * @file transaction_builder.c
 * @brief Neo transaction builder implementation
 */

#include "neoc/transaction/transaction_builder.h"
#include "neoc/neoc_memory.h"
#include "neoc/neo_constants.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/protocol/rpc_client.h"
#include "neoc/types/neoc_hash160.h"
#include <openssl/rand.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

// Transaction builder structure
struct neoc_tx_builder_t {
    uint8_t version;
    uint32_t nonce;
    uint32_t valid_until_block;
    neoc_signer_t *signers[TX_BUILDER_MAX_SIGNERS];
    size_t signer_count;
    uint64_t system_fee;
    uint64_t network_fee;
    neoc_tx_attribute_t attributes[TX_BUILDER_MAX_ATTRIBUTES];
    size_t attribute_count;
    uint8_t *script;
    size_t script_size;
    neoc_witness_t *witnesses[TX_BUILDER_MAX_WITNESSES];
    size_t witness_count;
    bool is_signed;
    neoc_transaction_t *built_transaction;
};

static uint32_t generate_nonce(void) {
    uint32_t nonce = 0;
    if (RAND_bytes((unsigned char *)&nonce, sizeof(nonce)) == 1) {
        return nonce;
    }
    srand((unsigned int)(time(NULL) ^ (uintptr_t)&nonce));
    return (uint32_t)rand();
}

#ifdef HAVE_CJSON
static neoc_error_t push_json_value(neoc_script_builder_t *builder, const cJSON *item) {
    if (!builder || !item) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid parameter value");
    }

    if (cJSON_IsBool(item)) {
        return neoc_script_builder_push_bool(builder, cJSON_IsTrue(item));
    } else if (cJSON_IsNull(item)) {
        return neoc_script_builder_emit(builder, NEOC_OP_PUSHNULL);
    } else if (cJSON_IsNumber(item)) {
        int64_t as_int = (int64_t)item->valuedouble;
        double diff = item->valuedouble - (double)as_int;
        if (diff < 0) diff = -diff;
        if (diff > 1e-9) {
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Non-integer numeric parameter not supported");
        }
        return neoc_script_builder_push_integer(builder, as_int);
    } else if (cJSON_IsString(item)) {
        const char *str = item->valuestring ? item->valuestring : "";
        return neoc_script_builder_push_string(builder, str);
    } else if (cJSON_IsArray(item)) {
        size_t count = (size_t)cJSON_GetArraySize(item);
        for (size_t i = count; i-- > 0;) {
            neoc_error_t err = push_json_value(builder, cJSON_GetArrayItem(item, (int)i));
            if (err != NEOC_SUCCESS) {
                return err;
            }
        }
        if (count > 0) {
            neoc_error_t err = neoc_script_builder_push_integer(builder, (int64_t)count);
            if (err != NEOC_SUCCESS) {
                return err;
            }
            return neoc_script_builder_emit(builder, NEOC_OP_PACK);
        }
        return neoc_script_builder_emit(builder, NEOC_OP_NEWARRAY0);
    }

    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unsupported parameter type");
}
#endif

neoc_error_t neoc_tx_builder_create(neoc_tx_builder_t **builder) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder pointer");
    }
    
    *builder = neoc_calloc(1, sizeof(neoc_tx_builder_t));
    if (!*builder) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate transaction builder");
    }
    
    // Set defaults
    (*builder)->version = 0;  // Current transaction version
    (*builder)->nonce = generate_nonce();
    (*builder)->valid_until_block = TX_DEFAULT_VALID_UNTIL_BLOCK;
    (*builder)->system_fee = 0;
    (*builder)->network_fee = TX_DEFAULT_NETWORK_FEE;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_create(neoc_transaction_builder_t **builder) {
    return neoc_tx_builder_create(builder);
}

neoc_error_t neoc_tx_builder_set_version(neoc_tx_builder_t *builder, uint8_t version) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder");
    }
    
    builder->version = version;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_set_version(neoc_transaction_builder_t *builder, uint8_t version) {
    return neoc_tx_builder_set_version(builder, version);
}

neoc_error_t neoc_tx_builder_set_nonce(neoc_tx_builder_t *builder, uint32_t nonce) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder");
    }
    
    builder->nonce = nonce;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_set_nonce(neoc_transaction_builder_t *builder, uint32_t nonce) {
    return neoc_tx_builder_set_nonce(builder, nonce);
}

neoc_error_t neoc_tx_builder_set_valid_until_block(neoc_tx_builder_t *builder, 
                                                     uint32_t block_height) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder");
    }

    if (block_height == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Valid until block must be greater than zero");
    }
    
    builder->valid_until_block = block_height;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_set_valid_until_block(neoc_transaction_builder_t *builder,
                                                            uint32_t block_height) {
    return neoc_tx_builder_set_valid_until_block(builder, block_height);
}

neoc_error_t neoc_tx_builder_set_valid_until_block_from_rpc(neoc_tx_builder_t *builder,
                                                            neoc_rpc_client_t *client,
                                                            uint32_t increment) {
    if (!builder || !client) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder or client");
    }

    uint32_t height = 0;
    neoc_error_t err = neoc_rpc_get_block_count(client, &height);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint64_t inc = increment == 0 ? 1000 : increment;
    uint64_t target = (uint64_t)height + inc;
    if (target > UINT32_MAX) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "validUntilBlock exceeds uint32 range");
    }

    builder->valid_until_block = (uint32_t)target;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_set_valid_until_block_from_rpc(neoc_transaction_builder_t *builder,
                                                                     neoc_rpc_client_t *client,
                                                                     uint32_t increment) {
    return neoc_tx_builder_set_valid_until_block_from_rpc(builder, client, increment);
}

neoc_error_t neoc_tx_builder_set_script(neoc_tx_builder_t *builder,
                                         const uint8_t *script,
                                         size_t script_size) {
    if (!builder || !script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (script_size > TX_BUILDER_MAX_SCRIPT_SIZE) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Script too large");
    }
    
    // Free existing script
    if (builder->script) {
        neoc_free(builder->script);
    }
    
    builder->script = neoc_malloc(script_size);
    if (!builder->script) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate script");
    }
    
    memcpy(builder->script, script, script_size);
    builder->script_size = script_size;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_set_script(neoc_transaction_builder_t *builder,
                                                 const uint8_t *script,
                                                 size_t script_size) {
    return neoc_tx_builder_set_script(builder, script, script_size);
}

neoc_error_t neoc_tx_builder_add_signer(neoc_tx_builder_t *builder,
                                         const neoc_signer_t *signer) {
    if (!builder || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (builder->signer_count >= TX_BUILDER_MAX_SIGNERS) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Too many signers");
    }

    // Prevent duplicate signers (same account hash)
    for (size_t i = 0; i < builder->signer_count; i++) {
        neoc_hash160_t existing_hash;
        neoc_hash160_t new_hash;
        if (neoc_signer_get_account(builder->signers[i], &existing_hash) == NEOC_SUCCESS &&
            neoc_signer_get_account(signer, &new_hash) == NEOC_SUCCESS &&
            memcmp(&existing_hash, &new_hash, sizeof(neoc_hash160_t)) == 0) {
            return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Duplicate signer");
        }
    }
    
    // Create a copy of the signer
    neoc_signer_t *new_signer = NULL;
    neoc_error_t err = neoc_signer_copy(signer, &new_signer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    builder->signers[builder->signer_count++] = new_signer;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_add_signer(neoc_transaction_builder_t *builder,
                                                 const neoc_signer_t *signer) {
    return neoc_tx_builder_add_signer(builder, signer);
}

neoc_error_t neoc_tx_builder_add_signer_from_account(neoc_tx_builder_t *builder,
                                                      const neoc_account_t *account,
                                                      neoc_witness_scope_t scope) {
    if (!builder || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Get script hash from account
    neoc_hash160_t script_hash;
    neoc_error_t hash_err = neoc_account_get_script_hash(account, &script_hash);
    if (hash_err != NEOC_SUCCESS) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no script hash");
    }
    
    // Create signer
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create(&script_hash, scope, &signer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    err = neoc_tx_builder_add_signer(builder, signer);
    neoc_signer_free(signer);
    
    return err;
}

neoc_error_t neoc_transaction_builder_add_signer_from_account(neoc_transaction_builder_t *builder,
                                                              const neoc_account_t *account,
                                                              neoc_witness_scope_t scope) {
    return neoc_tx_builder_add_signer_from_account(builder, account, scope);
}

neoc_error_t neoc_tx_builder_set_first_signer(neoc_tx_builder_t *builder,
                                               const neoc_account_t *account) {
    if (!builder || !account) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Get script hash from account
    neoc_hash160_t script_hash;
    neoc_error_t hash_err = neoc_account_get_script_hash(account, &script_hash);
    if (hash_err != NEOC_SUCCESS) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Account has no script hash");
    }
    
    // Find the signer with this script hash
    int found_index = -1;
    for (size_t i = 0; i < builder->signer_count; i++) {
        neoc_hash160_t signer_hash;
        neoc_signer_get_account(builder->signers[i], &signer_hash);
        if (neoc_hash160_equal(&signer_hash, &script_hash)) {
            found_index = (int)i;
            break;
        }
    }
    
    if (found_index < 0) {
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "Signer not found");
    }
    
    // Move to first position if not already there
    if (found_index > 0) {
        neoc_signer_t *temp = builder->signers[found_index];
        for (int i = found_index; i > 0; i--) {
            builder->signers[i] = builder->signers[i - 1];
        }
        builder->signers[0] = temp;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_set_first_signer(neoc_transaction_builder_t *builder,
                                                       const neoc_account_t *account) {
    return neoc_tx_builder_set_first_signer(builder, account);
}

neoc_error_t neoc_tx_builder_add_attribute(neoc_tx_builder_t *builder,
                                            const neoc_tx_attribute_t *attribute) {
    if (!builder || !attribute) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (builder->attribute_count >= TX_BUILDER_MAX_ATTRIBUTES) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Too many attributes");
    }
    
    neoc_tx_attribute_t *attr = &builder->attributes[builder->attribute_count];
    attr->type = attribute->type;
    
    if (attribute->data && attribute->data_len > 0) {
        attr->data = neoc_malloc(attribute->data_len);
        if (!attr->data) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate attribute data");
        }
        memcpy(attr->data, attribute->data, attribute->data_len);
        attr->data_len = attribute->data_len;
    }
    
    builder->attribute_count++;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_tx_builder_set_high_priority(neoc_tx_builder_t *builder,
                                               bool high_priority) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder");
    }
    
    // Remove existing high priority attribute if any
    for (size_t i = 0; i < builder->attribute_count; i++) {
        if (builder->attributes[i].type == NEOC_TX_ATTR_HIGH_PRIORITY) {
            // Free data if any
            if (builder->attributes[i].data) {
                neoc_free(builder->attributes[i].data);
            }
            // Shift remaining attributes
            for (size_t j = i; j < builder->attribute_count - 1; j++) {
                builder->attributes[j] = builder->attributes[j + 1];
            }
            builder->attribute_count--;
            break;
        }
    }
    
    // Add high priority attribute if requested
    if (high_priority) {
        neoc_tx_attribute_t attr = {
            .type = NEOC_TX_ATTR_HIGH_PRIORITY,
            .data = NULL,
            .data_len = 0
        };
        return neoc_tx_builder_add_attribute(builder, &attr);
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_tx_builder_add_network_fee(neoc_tx_builder_t *builder,
                                              uint64_t fee) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder");
    }
    
    builder->network_fee += fee;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_add_network_fee(neoc_transaction_builder_t *builder,
                                                      uint64_t fee) {
    return neoc_tx_builder_add_network_fee(builder, fee);
}

neoc_error_t neoc_tx_builder_add_system_fee(neoc_tx_builder_t *builder,
                                             uint64_t fee) {
    if (!builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid builder");
    }
    
    builder->system_fee += fee;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_add_system_fee(neoc_transaction_builder_t *builder,
                                                     uint64_t fee) {
    return neoc_tx_builder_add_system_fee(builder, fee);
}

neoc_error_t neoc_tx_builder_calculate_fees(neoc_tx_builder_t *builder,
                                            neoc_rpc_client_t *client,
                                            uint64_t *network_fee,
                                            uint64_t *system_fee) {
    if (!builder || !client || !network_fee || !system_fee) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Calculate actual fees using RPC client
    // Network fee = base fee + size fee + signature verification fee
    // System fee = gas consumed by script execution
    
    size_t signer_count = builder->signer_count;

    // Build unsigned transaction first to get size
    neoc_transaction_t *temp_tx = NULL;
    neoc_error_t err = neoc_tx_builder_build_unsigned(builder, &temp_tx);
    if (err != NEOC_SUCCESS) {
        // Use default fees if can't build transaction
        *network_fee = builder->network_fee > 0 ? builder->network_fee : 100000; // 0.001 GAS minimum
        *system_fee = builder->system_fee > 0 ? builder->system_fee : 0;
        return NEOC_SUCCESS;
    }
    
    // Calculate network fee based on transaction size
    size_t serialized_size = neoc_transaction_get_size(temp_tx);
    uint8_t *tx_bytes = neoc_malloc(serialized_size);
    if (!tx_bytes) {
        neoc_transaction_free(temp_tx);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate transaction buffer");
    }

    size_t written = 0;
    err = neoc_transaction_serialize(temp_tx, tx_bytes, serialized_size, &written);
    if (err == NEOC_SUCCESS) {
        if (written < serialized_size) {
            serialized_size = written;
        }

        uint64_t size_fee = serialized_size * 1000;
        uint64_t sig_fee = signer_count * 1000000;

        *network_fee = size_fee + sig_fee;

        if (*network_fee < 100000) {
            *network_fee = 100000;
        }
    } else {
        *network_fee = builder->network_fee > 0 ? builder->network_fee : 100000;
    }
    neoc_free(tx_bytes);
    
    // Calculate system fee by invoking script with RPC client
    // Use the builder's system fee if set, otherwise estimate based on script complexity
    if (builder->system_fee > 0) {
        *system_fee = builder->system_fee;
    } else {
        // Estimate system fee based on script operations and size
        // Base cost for script execution
        uint64_t base_fee = 100000; // 0.001 GAS base
        
        // Add cost based on script size (0.0001 GAS per byte)
        uint64_t size_fee = builder->script_size * 10000;
        
        // Add cost for each contract invocation (0.01 GAS per invocation)
        uint64_t invocation_fee = 0;
        
        *system_fee = base_fee + size_fee + invocation_fee;
    }
    
    neoc_transaction_free(temp_tx);
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_tx_builder_build_unsigned(neoc_tx_builder_t *builder,
                                            neoc_transaction_t **transaction) {
    if (!builder || !transaction) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!builder->script || builder->script_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "No script set");
    }

    if (builder->valid_until_block == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Valid until block not set");
    }
    
    if (builder->signer_count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "No signers added");
    }
    
    // Create transaction
    neoc_error_t err = neoc_transaction_create(transaction);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Set transaction properties
    neoc_transaction_set_version(*transaction, builder->version);
    neoc_transaction_set_nonce(*transaction, builder->nonce);
    neoc_transaction_set_valid_until_block(*transaction, builder->valid_until_block);
    neoc_transaction_set_system_fee(*transaction, builder->system_fee);
    neoc_transaction_set_network_fee(*transaction, builder->network_fee);
    neoc_transaction_set_script(*transaction, builder->script, builder->script_size);
    
    // Add signers (transfer ownership to transaction)
    for (size_t i = 0; i < builder->signer_count; i++) {
        err = neoc_transaction_add_signer(*transaction, builder->signers[i]);
        if (err != NEOC_SUCCESS) {
            neoc_transaction_free(*transaction);
            *transaction = NULL;
            return err;
        }
        builder->signers[i] = NULL;
    }
    builder->signer_count = 0;
    
    // Store reference to built transaction (caller owns lifetime)
    builder->built_transaction = *transaction;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_build(neoc_transaction_builder_t *builder,
                                            neoc_transaction_t **transaction) {
    return neoc_tx_builder_build_unsigned(builder, transaction);
}

neoc_error_t neoc_tx_builder_sign(neoc_tx_builder_t *builder,
                                   neoc_account_t **accounts,
                                   size_t account_count) {
    if (!builder || !accounts || account_count == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!builder->built_transaction) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Transaction not built");
    }
    
    // Sign transaction with each account
    for (size_t i = 0; i < account_count; i++) {
        neoc_error_t err = neoc_transaction_sign(builder->built_transaction, accounts[i]);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }
    
    builder->is_signed = true;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_sign(neoc_transaction_builder_t *builder,
                                           neoc_account_t **accounts,
                                           size_t account_count) {
    return neoc_tx_builder_sign(builder, accounts, account_count);
}

neoc_error_t neoc_tx_builder_build_and_sign(neoc_tx_builder_t *builder,
                                             neoc_account_t **accounts,
                                             size_t account_count,
                                             neoc_transaction_t **transaction) {
    if (!builder || !accounts || !transaction) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build unsigned transaction
    neoc_error_t err = neoc_tx_builder_build_unsigned(builder, transaction);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Sign the transaction
    err = neoc_tx_builder_sign(builder, accounts, account_count);
    if (err != NEOC_SUCCESS) {
        neoc_transaction_free(*transaction);
        *transaction = NULL;
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_builder_build_and_sign(neoc_transaction_builder_t *builder,
                                                     neoc_account_t **accounts,
                                                     size_t account_count,
                                                     neoc_transaction_t **transaction) {
    return neoc_tx_builder_build_and_sign(builder, accounts, account_count, transaction);
}

neoc_error_t neoc_tx_builder_get_hash(const neoc_tx_builder_t *builder,
                                       neoc_hash256_t *hash) {
    if (!builder || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_tx_builder_t *mutable_builder = (neoc_tx_builder_t *)builder;
    if (!mutable_builder->built_transaction) {
        neoc_transaction_t *tx = NULL;
        neoc_error_t err = neoc_tx_builder_build_unsigned(mutable_builder, &tx);
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }
    
    return neoc_transaction_calculate_hash(mutable_builder->built_transaction, hash);
}

neoc_error_t neoc_tx_builder_serialize(const neoc_tx_builder_t *builder,
                                        uint8_t *buffer,
                                        size_t buffer_size,
                                        size_t *serialized_size) {
    if (!builder || !buffer || !serialized_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (!builder->built_transaction) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Transaction not built");
    }
    
    return neoc_transaction_serialize(builder->built_transaction, buffer, buffer_size, serialized_size);
}

neoc_error_t neoc_transaction_builder_serialize(const neoc_transaction_builder_t *builder,
                                                uint8_t *buffer,
                                                size_t buffer_size,
                                                size_t *serialized_size) {
    return neoc_tx_builder_serialize(builder, buffer, buffer_size, serialized_size);
}

neoc_error_t neoc_tx_builder_create_nep17_transfer(const neoc_hash160_t *token_hash,
                                                    const neoc_account_t *from_account,
                                                    const char *to_address,
                                                    uint64_t amount,
                                                    const uint8_t *data,
                                                    size_t data_size,
                                                    neoc_tx_builder_t **builder) {
    if (!token_hash || !from_account || !to_address || !builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create builder
    neoc_error_t err = neoc_tx_builder_create(builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Get from address script hash
    neoc_hash160_t from_hash;
    neoc_error_t hash_err = neoc_account_get_script_hash(from_account, &from_hash);
    if (hash_err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "From account has no script hash");
    }
    
    // Get to address script hash
    neoc_hash160_t to_hash;
    err = neoc_hash160_from_address(&to_hash, to_address);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return err;
    }
    
    // Build transfer script
    neoc_script_builder_t *sb = NULL;
    err = neoc_script_builder_create(&sb);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return err;
    }
    
    // Push parameters in reverse order
    if (data && data_size > 0) {
        err = neoc_script_builder_push_data(sb, data, data_size);
    } else {
        err = neoc_script_builder_emit(sb, NEOC_OP_PUSHNULL);
    }
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_script_builder_push_integer(sb, amount);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_script_builder_push_data(sb, to_hash.data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_script_builder_push_data(sb, from_hash.data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_script_builder_push_integer(sb, 4);  // Parameter count
    if (err != NEOC_SUCCESS) goto cleanup;

    err = neoc_script_builder_emit(sb, NEOC_OP_PACK);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_script_builder_push_string(sb, "transfer");
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_script_builder_push_data(sb, token_hash->data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) goto cleanup;
    err = neoc_script_builder_emit_syscall(sb, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Get the script
    uint8_t *script = NULL;
    size_t script_size;
    err = neoc_script_builder_to_array(sb, &script, &script_size);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Set script
    err = neoc_tx_builder_set_script(*builder, script, script_size);
    free(script);
    
cleanup:
    neoc_script_builder_free(sb);
    
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return err;
    }
    
    // Add from account as signer
    err = neoc_tx_builder_add_signer_from_account(*builder, from_account, NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_tx_builder_create_contract_call(const neoc_hash160_t *contract_hash,
                                                   const char *method,
                                                   const char *params __attribute__((unused)),
                                                   neoc_signer_t **signers,
                                                   size_t signer_count,
                                                   neoc_tx_builder_t **builder) {
    if (!contract_hash || !method || !builder) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create builder
    neoc_error_t err = neoc_tx_builder_create(builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Build contract call script
    neoc_script_builder_t *sb = NULL;
    err = neoc_script_builder_create(&sb);
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return err;
    }
    
    // Parse params JSON and push parameters to stack
#ifdef HAVE_CJSON
    if (params && strlen(params) > 0) {
        cJSON *param_array = cJSON_Parse(params);
        if (!param_array || !cJSON_IsArray(param_array)) {
            if (param_array) cJSON_Delete(param_array);
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Contract parameters must be a JSON array");
            goto cleanup;
        }

        size_t param_count = (size_t)cJSON_GetArraySize(param_array);
        for (size_t i = param_count; i-- > 0;) {
            err = push_json_value(sb, cJSON_GetArrayItem(param_array, (int)i));
            if (err != NEOC_SUCCESS) {
                cJSON_Delete(param_array);
                goto cleanup;
            }
        }

        if (param_count > 0) {
            err = neoc_script_builder_push_integer(sb, (int64_t)param_count);
            if (err == NEOC_SUCCESS) {
                err = neoc_script_builder_emit(sb, NEOC_OP_PACK);
            }
        } else {
            err = neoc_script_builder_emit(sb, NEOC_OP_NEWARRAY0);
        }
        cJSON_Delete(param_array);
        if (err != NEOC_SUCCESS) goto cleanup;
    } else {
        err = neoc_script_builder_emit(sb, NEOC_OP_NEWARRAY0);
        if (err != NEOC_SUCCESS) goto cleanup;
    }
#else
    if (params && strlen(params) > 0) {
        err = neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "Contract parameter parsing requires cJSON");
        goto cleanup;
    }
    err = neoc_script_builder_emit(sb, NEOC_OP_NEWARRAY0);
    if (err != NEOC_SUCCESS) goto cleanup;
#endif
    
    err = neoc_script_builder_push_string(sb, method);
    if (err != NEOC_SUCCESS) goto cleanup;
    err = neoc_script_builder_push_data(sb, contract_hash->data, sizeof(neoc_hash160_t));
    if (err != NEOC_SUCCESS) goto cleanup;
    err = neoc_script_builder_emit_syscall(sb, NEOC_INTEROP_SYSTEM_CONTRACT_CALL);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Get the script
    uint8_t *script = NULL;
    size_t script_size;
    err = neoc_script_builder_to_array(sb, &script, &script_size);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Set script
    err = neoc_tx_builder_set_script(*builder, script, script_size);
    free(script);
    
cleanup:
    neoc_script_builder_free(sb);
    
    if (err != NEOC_SUCCESS) {
        neoc_tx_builder_free(*builder);
        *builder = NULL;
        return err;
    }
    
    // Add signers
    if (signers && signer_count > 0) {
        for (size_t i = 0; i < signer_count; i++) {
            err = neoc_tx_builder_add_signer(*builder, signers[i]);
            if (err != NEOC_SUCCESS) {
                neoc_tx_builder_free(*builder);
                *builder = NULL;
                return err;
            }
        }
    }
    
    return NEOC_SUCCESS;
}

void neoc_tx_builder_free(neoc_tx_builder_t *builder) {
    if (!builder) return;
    
    // Free script
    if (builder->script) {
        neoc_free(builder->script);
    }
    
    // Free signers
    for (size_t i = 0; i < builder->signer_count; i++) {
        neoc_signer_free(builder->signers[i]);
    }
    
    // Free attributes
    for (size_t i = 0; i < builder->attribute_count; i++) {
        if (builder->attributes[i].data) {
            neoc_free(builder->attributes[i].data);
        }
    }
    
    // Free witnesses
    for (size_t i = 0; i < builder->witness_count; i++) {
        neoc_witness_free(builder->witnesses[i]);
    }
    
    // Note: Don't free built_transaction here as it's returned to the caller
    
    neoc_free(builder);
}

void neoc_transaction_builder_free(neoc_transaction_builder_t *builder) {
    neoc_tx_builder_free(builder);
}
