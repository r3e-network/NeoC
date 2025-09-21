/**
 * @file neo_transaction.c
 * @brief NEO transaction implementation
 */

#include "neoc/transaction/neo_transaction.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/serialization/binary_writer.h"
#include <string.h>
#include <stdlib.h>

// Structure defined in header file neoc/transaction/neo_transaction.h

neoc_error_t neoc_transaction_create(neoc_transaction_t **transaction) {
    if (!transaction) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transaction pointer");
    }
    
    *transaction = neoc_calloc(1, sizeof(neoc_transaction_t));
    if (!*transaction) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate transaction");
    }
    
    // Initialize with defaults
    (*transaction)->version = 0;
    (*transaction)->nonce = (uint32_t)rand();
    // Initialize signers array
    (*transaction)->signers = neoc_calloc(4, sizeof(neoc_signer_t*));
    if (!(*transaction)->signers) {
        neoc_free(*transaction);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate signers");
    }
    
    // Initialize attributes array
    (*transaction)->attributes = neoc_calloc(4, sizeof(neoc_transaction_attribute_t*));
    if (!(*transaction)->attributes) {
        neoc_free((*transaction)->signers);
        neoc_free(*transaction);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate attributes");
    }
    
    // Initialize witnesses array
    (*transaction)->witnesses = neoc_calloc(4, sizeof(neoc_witness_t*));
    if (!(*transaction)->witnesses) {
        neoc_free((*transaction)->attributes);
        neoc_free((*transaction)->signers);
        neoc_free(*transaction);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witnesses");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_set_version(neoc_transaction_t *tx, uint8_t version) {
    if (!tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transaction");
    }
    tx->version = version;
    // Hash invalidated on version change
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_set_nonce(neoc_transaction_t *tx, uint32_t nonce) {
    if (!tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transaction");
    }
    tx->nonce = nonce;
    // Hash invalidated on version change
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_set_system_fee(neoc_transaction_t *tx, uint64_t fee) {
    if (!tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transaction");
    }
    tx->system_fee = fee;
    // Hash invalidated on version change
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_set_network_fee(neoc_transaction_t *tx, uint64_t fee) {
    if (!tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transaction");
    }
    tx->network_fee = fee;
    // Hash invalidated on version change
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_set_valid_until_block(neoc_transaction_t *tx, uint32_t block) {
    if (!tx) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid transaction");
    }
    tx->valid_until_block = block;
    // Hash invalidated on version change
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_set_script(neoc_transaction_t *tx, 
                                         const uint8_t *script, 
                                         size_t script_size) {
    if (!tx || !script) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Free existing script
    if (tx->script) {
        neoc_free(tx->script);
    }
    
    tx->script = neoc_malloc(script_size);
    if (!tx->script) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate script");
    }
    
    memcpy(tx->script, script, script_size);
    tx->script_len = script_size;
    
    // Hash invalidated on version change
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_add_signer(neoc_transaction_t *tx, const neoc_signer_t *signer) {
    if (!tx || !signer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Resize if needed
    if (tx->signers_count >= 4) {
        size_t new_capacity = (tx->signers_count + 1) * 2;
        neoc_signer_t **new_signers = neoc_realloc(tx->signers, 
                                                    new_capacity * sizeof(neoc_signer_t*));
        if (!new_signers) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize signers");
        }
        tx->signers = new_signers;
    }
    
    // Copy signer
    neoc_signer_t *new_signer = NULL;
    neoc_error_t err = neoc_signer_copy(signer, &new_signer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    tx->signers[tx->signers_count++] = new_signer;
    
    // Hash invalidated on version change
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_add_witness(neoc_transaction_t *transaction, neoc_witness_t *witness) {
    if (!transaction || !witness) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Resize if needed
    if (transaction->witnesses_count >= 4) {
        size_t new_capacity = (transaction->witnesses_count + 1) * 2;
        neoc_witness_t **new_witnesses = neoc_realloc(transaction->witnesses,
                                                       new_capacity * sizeof(neoc_witness_t*));
        if (!new_witnesses) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize witnesses");
        }
        transaction->witnesses = new_witnesses;
    }
    
    // Copy witness
    neoc_witness_t *new_witness = NULL;
    neoc_error_t err = neoc_witness_clone(witness, &new_witness);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    transaction->witnesses[transaction->witnesses_count++] = new_witness;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_get_hash(neoc_transaction_t *transaction, neoc_hash256_t *hash) {
    if (!transaction || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Use cached hash if available
    // Hash would be calculated dynamically
    
    // Serialize transaction without witnesses
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(1024, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Write transaction data
    err = neoc_binary_writer_write_byte(writer, transaction->version);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint32(writer, transaction->nonce);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint64(writer, transaction->system_fee);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint64(writer, transaction->network_fee);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint32(writer, transaction->valid_until_block);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Write signers
    err = neoc_binary_writer_write_var_int(writer, transaction->signers_count);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    for (size_t i = 0; i < transaction->signers_count; i++) {
        err = neoc_signer_serialize(transaction->signers[i], writer);
        if (err != NEOC_SUCCESS) goto cleanup;
    }
    
    // Write attributes
    err = neoc_binary_writer_write_var_int(writer, transaction->attributes_count);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    for (size_t i = 0; i < transaction->attributes_count; i++) {
        err = neoc_binary_writer_write_byte(writer, transaction->attributes[i]->type);
        if (err != NEOC_SUCCESS) goto cleanup;
        
        // Serialize attribute data based on type
        switch (transaction->attributes[i]->type) {
            case NEOC_TRANSACTION_ATTRIBUTE_HIGH_PRIORITY:
                // High priority has no additional data
                break;
                
            case NEOC_TRANSACTION_ATTRIBUTE_ORACLE_RESPONSE:
                // Oracle response: id (8 bytes) + code (1 byte) + result (var bytes)
                if (transaction->attributes[i]->data.oracle_response.id) {
                    err = neoc_binary_writer_write_u64_le(writer, transaction->attributes[i]->data.oracle_response.id);
                    if (err != NEOC_SUCCESS) goto cleanup;
                }
                err = neoc_binary_writer_write_byte(writer, transaction->attributes[i]->data.oracle_response.code);
                if (err != NEOC_SUCCESS) goto cleanup;
                err = neoc_binary_writer_write_var_bytes(writer, 
                    transaction->attributes[i]->data.oracle_response.result,
                    transaction->attributes[i]->data.oracle_response.result_len);
                if (err != NEOC_SUCCESS) goto cleanup;
                break;
                
            case NEOC_TRANSACTION_ATTRIBUTE_NOT_VALID_BEFORE:
                // Height (4 bytes)
                err = neoc_binary_writer_write_u32_le(writer, transaction->attributes[i]->data.not_valid_before.height);
                if (err != NEOC_SUCCESS) goto cleanup;
                break;
                
            case NEOC_TRANSACTION_ATTRIBUTE_CONFLICTS:
                // Hash (32 bytes)
                err = neoc_binary_writer_write_bytes(writer, 
                    transaction->attributes[i]->data.conflicts.hash.data, 32);
                if (err != NEOC_SUCCESS) goto cleanup;
                break;
                
            default:
                // Unknown attribute type - write no data
                break;
        }
    }
    
    // Write script
    err = neoc_binary_writer_write_var_bytes(writer, transaction->script, transaction->script_len);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_len);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Calculate hash
    err = neoc_hash256_from_data(hash, data, data_len);
    neoc_free(data);
    
    // Hash calculated successfully
    
cleanup:
    neoc_binary_writer_free(writer);
    return err;
}

neoc_error_t neoc_transaction_serialize(neoc_transaction_t *transaction,
                                         uint8_t **data,
                                         size_t *data_len) {
    if (!transaction || !data || !data_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(1024, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Serialize full transaction including witnesses
    // [Same as get_hash but includes witnesses at the end]
    
    // Write transaction data (same as get_hash)
    err = neoc_binary_writer_write_byte(writer, transaction->version);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint32(writer, transaction->nonce);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint64(writer, transaction->system_fee);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint64(writer, transaction->network_fee);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    err = neoc_binary_writer_write_uint32(writer, transaction->valid_until_block);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Write signers
    err = neoc_binary_writer_write_var_int(writer, transaction->signers_count);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    for (size_t i = 0; i < transaction->signers_count; i++) {
        err = neoc_signer_serialize(transaction->signers[i], writer);
        if (err != NEOC_SUCCESS) goto cleanup;
    }
    
    // Write attributes
    err = neoc_binary_writer_write_var_int(writer, transaction->attributes_count);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    for (size_t i = 0; i < transaction->attributes_count; i++) {
        err = neoc_binary_writer_write_byte(writer, transaction->attributes[i]->type);
        if (err != NEOC_SUCCESS) goto cleanup;
        
        // Serialize attribute data based on type
        switch (transaction->attributes[i]->type) {
            case NEOC_TRANSACTION_ATTRIBUTE_HIGH_PRIORITY:
                // High priority has no additional data
                break;
                
            case NEOC_TRANSACTION_ATTRIBUTE_ORACLE_RESPONSE:
                // Oracle response: id (8 bytes) + code (1 byte) + result (var bytes)
                if (transaction->attributes[i]->data.oracle_response.id) {
                    err = neoc_binary_writer_write_u64_le(writer, transaction->attributes[i]->data.oracle_response.id);
                    if (err != NEOC_SUCCESS) goto cleanup;
                }
                err = neoc_binary_writer_write_byte(writer, transaction->attributes[i]->data.oracle_response.code);
                if (err != NEOC_SUCCESS) goto cleanup;
                err = neoc_binary_writer_write_var_bytes(writer, 
                    transaction->attributes[i]->data.oracle_response.result,
                    transaction->attributes[i]->data.oracle_response.result_len);
                if (err != NEOC_SUCCESS) goto cleanup;
                break;
                
            case NEOC_TRANSACTION_ATTRIBUTE_NOT_VALID_BEFORE:
                // Height (4 bytes)
                err = neoc_binary_writer_write_u32_le(writer, transaction->attributes[i]->data.not_valid_before.height);
                if (err != NEOC_SUCCESS) goto cleanup;
                break;
                
            case NEOC_TRANSACTION_ATTRIBUTE_CONFLICTS:
                // Hash (32 bytes)
                err = neoc_binary_writer_write_bytes(writer, 
                    transaction->attributes[i]->data.conflicts.hash.data, 32);
                if (err != NEOC_SUCCESS) goto cleanup;
                break;
                
            default:
                // Unknown attribute type - write no data
                break;
        }
    }
    
    // Write script
    err = neoc_binary_writer_write_var_bytes(writer, transaction->script, transaction->script_len);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Write witnesses
    err = neoc_binary_writer_write_var_int(writer, transaction->witnesses_count);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    for (size_t i = 0; i < transaction->witnesses_count; i++) {
        uint8_t *witness_bytes = NULL;
        size_t witness_len = 0;
        err = neoc_witness_serialize(transaction->witnesses[i], &witness_bytes, &witness_len);
        if (err != NEOC_SUCCESS) goto cleanup;
        
        err = neoc_binary_writer_write_bytes(writer, witness_bytes, witness_len);
        neoc_free(witness_bytes);
        if (err != NEOC_SUCCESS) goto cleanup;
    }
    
    // Get serialized data
    err = neoc_binary_writer_to_array(writer, data, data_len);
    
cleanup:
    neoc_binary_writer_free(writer);
    return err;
}

neoc_error_t neoc_transaction_get_signers(neoc_transaction_t *transaction,
                                           neoc_signer_t ***signers,
                                           size_t *count) {
    if (!transaction || !signers || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *signers = transaction->signers;
    *count = transaction->signers_count;
    
    return NEOC_SUCCESS;
}

void neoc_transaction_free(neoc_transaction_t *tx) {
    if (!tx) return;
    
    // Free signers
    for (size_t i = 0; i < tx->signers_count; i++) {
        neoc_signer_free(tx->signers[i]);
    }
    neoc_free(tx->signers);
    
    // Free attributes
    for (size_t i = 0; i < tx->attributes_count; i++) {
        neoc_transaction_attribute_free(tx->attributes[i]);
    }
    neoc_free(tx->attributes);
    
    // Free witnesses
    for (size_t i = 0; i < tx->witnesses_count; i++) {
        neoc_witness_free(tx->witnesses[i]);
    }
    neoc_free(tx->witnesses);
    
    // Free script
    if (tx->script) {
        neoc_free(tx->script);
    }
    
    neoc_free(tx);
}
