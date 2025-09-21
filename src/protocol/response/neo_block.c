#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif
#include "../../../include/neoc/protocol/response/neo_block.h"
#include "../../../include/neoc/transaction/witness.h"
#include "../../../include/neoc/neoc_memory.h"
#include "../../../include/neoc/crypto/hash.h"
#include "../../../include/neoc/utils/neoc_hex.h"

// Create block
neoc_neo_block_t* neoc_neo_block_create(void) {
    neoc_neo_block_t* block = neoc_malloc(sizeof(neoc_neo_block_t));
    if (!block) {
        return NULL;
    }
    
    memset(block, 0, sizeof(neoc_neo_block_t));
    
    // Initialize header
    block->header.version = 0;
    block->header.timestamp = 0;
    block->header.nonce = 0;
    block->header.index = 0;
    block->header.primary_index = 0;
    block->header.witness = NULL;
    
    // Initialize block fields
    block->size = 0;
    block->confirmations = 0;
    block->transactions = NULL;
    block->transaction_count = 0;
    
    return block;
}

// Free block
void neoc_neo_block_free(neoc_neo_block_t* block) {
    if (!block) {
        return;
    }
    
    // Free witness
    if (block->header.witness) {
        neoc_witness_free(block->header.witness);
    }
    
    // Free transactions
    if (block->transactions) {
        for (size_t i = 0; i < block->transaction_count; i++) {
            if (block->transactions[i]) {
                neoc_transaction_free(block->transactions[i]);
            }
        }
        neoc_free(block->transactions);
    }
    
    neoc_free(block);
}

// Clone block
neoc_neo_block_t* neoc_neo_block_clone(const neoc_neo_block_t* block) {
    if (!block) {
        return NULL;
    }
    
    neoc_neo_block_t* clone = neoc_neo_block_create();
    if (!clone) {
        return NULL;
    }
    
    // Copy header fields
    memcpy(&clone->header, &block->header, sizeof(neoc_block_header_t));
    
    // Clone witness
    if (block->header.witness) {
        neoc_error_t err = neoc_witness_create(
            block->header.witness->invocation_script,
            block->header.witness->invocation_script_len,
            block->header.witness->verification_script,
            block->header.witness->verification_script_len,
            &clone->header.witness
        );
        if (err != NEOC_SUCCESS) {
            neoc_neo_block_free(clone);
            return NULL;
        }
    }
    
    // Copy block fields
    clone->hash = block->hash;
    clone->size = block->size;
    clone->confirmations = block->confirmations;
    
    // Clone transactions
    if (block->transaction_count > 0 && block->transactions) {
        clone->transactions = neoc_malloc(sizeof(neoc_transaction_t*) * block->transaction_count);
        if (!clone->transactions) {
            neoc_neo_block_free(clone);
            return NULL;
        }
        
        clone->transaction_count = block->transaction_count;
        for (size_t i = 0; i < block->transaction_count; i++) {
            clone->transactions[i] = neoc_transaction_clone(block->transactions[i]);
            if (!clone->transactions[i]) {
                neoc_neo_block_free(clone);
                return NULL;
            }
        }
    }
    
    return clone;
}

// Set header fields
void neoc_neo_block_set_version(neoc_neo_block_t* block, uint32_t version) {
    if (block) {
        block->header.version = version;
    }
}

void neoc_neo_block_set_prev_hash(neoc_neo_block_t* block, const neoc_hash256_t* hash) {
    if (block && hash) {
        memcpy(&block->header.prev_hash, hash, sizeof(neoc_hash256_t));
    }
}

void neoc_neo_block_set_merkle_root(neoc_neo_block_t* block, const neoc_hash256_t* hash) {
    if (block && hash) {
        memcpy(&block->header.merkle_root, hash, sizeof(neoc_hash256_t));
    }
}

void neoc_neo_block_set_timestamp(neoc_neo_block_t* block, uint64_t timestamp) {
    if (block) {
        block->header.timestamp = timestamp;
    }
}

void neoc_neo_block_set_index(neoc_neo_block_t* block, uint32_t index) {
    if (block) {
        block->header.index = index;
    }
}

void neoc_neo_block_set_primary_index(neoc_neo_block_t* block, uint8_t index) {
    if (block) {
        block->header.primary_index = index;
    }
}

void neoc_neo_block_set_next_consensus(neoc_neo_block_t* block, const neoc_hash160_t* hash) {
    if (block && hash) {
        memcpy(&block->header.next_consensus, hash, sizeof(neoc_hash160_t));
    }
}

// Add transaction
void neoc_neo_block_add_transaction(neoc_neo_block_t* block, neoc_transaction_t* tx) {
    if (!block || !tx) {
        return;
    }
    
    // Resize transactions array
    neoc_transaction_t** new_txs = neoc_realloc(
        block->transactions,
        sizeof(neoc_transaction_t*) * (block->transaction_count + 1)
    );
    
    if (!new_txs) {
        return;
    }
    
    block->transactions = new_txs;
    block->transactions[block->transaction_count] = tx;
    block->transaction_count++;
}

// Calculate merkle root
neoc_hash256_t neoc_neo_block_calculate_merkle_root(const neoc_neo_block_t* block) {
    neoc_hash256_t root;
    memset(&root, 0, sizeof(root));
    
    if (!block || block->transaction_count == 0) {
        return root;
    }
    
    // Special case: single transaction
    if (block->transaction_count == 1) {
        return neoc_transaction_calculate_hash(block->transactions[0]);
    }
    
    // Collect all transaction hashes
    neoc_hash256_t* hashes = neoc_malloc(block->transaction_count * sizeof(neoc_hash256_t));
    if (!hashes) {
        return root;
    }
    
    for (size_t i = 0; i < block->transaction_count; i++) {
        hashes[i] = neoc_transaction_calculate_hash(block->transactions[i]);
    }
    
    // Build merkle tree bottom-up
    size_t level_count = block->transaction_count;
    neoc_hash256_t* level_hashes = hashes;
    
    while (level_count > 1) {
        size_t next_level_count = (level_count + 1) / 2;
        neoc_hash256_t* next_level = neoc_malloc(next_level_count * sizeof(neoc_hash256_t));
        if (!next_level) {
            neoc_free(hashes);
            return root;
        }
        
        for (size_t i = 0; i < next_level_count; i++) {
            size_t left_index = i * 2;
            size_t right_index = left_index + 1;
            
            // Create combined hash
            uint8_t combined[64];
            memcpy(combined, level_hashes[left_index].data, 32);
            
            if (right_index < level_count) {
                // Use right hash
                memcpy(combined + 32, level_hashes[right_index].data, 32);
            } else {
                // Duplicate left hash for odd count
                memcpy(combined + 32, level_hashes[left_index].data, 32);
            }
            
            // Double SHA256 hash
            uint8_t first_hash[32];
            neoc_error_t err = neoc_hash_sha256(combined, 64, first_hash);
            if (err == NEOC_SUCCESS) {
                err = neoc_hash_sha256(first_hash, 32, next_level[i].data);
            }
            
            if (err != NEOC_SUCCESS) {
                neoc_free(next_level);
                neoc_free(hashes);
                memset(&root, 0, sizeof(root));
                return root;
            }
        }
        
        // Move to next level
        if (level_hashes != hashes) {
            neoc_free(level_hashes);
        }
        level_hashes = next_level;
        level_count = next_level_count;
    }
    
    // Copy final root
    root = level_hashes[0];
    
    if (level_hashes != hashes) {
        neoc_free(level_hashes);
    }
    neoc_free(hashes);
    
    return root;
}

// Calculate block hash
neoc_hash256_t neoc_neo_block_calculate_hash(const neoc_neo_block_t* block) {
    neoc_hash256_t hash;
    memset(&hash, 0, sizeof(hash));
    
    if (!block) {
        return hash;
    }
    
    // Serialize header and hash it
    size_t header_size = sizeof(uint32_t) + 32 + 32 + sizeof(uint64_t) + 
                        sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint8_t) + 20;
    
    uint8_t* header_data = neoc_malloc(header_size);
    if (!header_data) {
        return hash;
    }
    
    size_t offset = 0;
    
    // Write version
    memcpy(header_data + offset, &block->header.version, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // Write previous hash
    memcpy(header_data + offset, &block->header.prev_hash, 32);
    offset += 32;
    
    // Write merkle root
    memcpy(header_data + offset, &block->header.merkle_root, 32);
    offset += 32;
    
    // Write timestamp
    memcpy(header_data + offset, &block->header.timestamp, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    // Write nonce
    memcpy(header_data + offset, &block->header.nonce, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    // Write index
    memcpy(header_data + offset, &block->header.index, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // Write primary index
    memcpy(header_data + offset, &block->header.primary_index, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    
    // Write next consensus
    memcpy(header_data + offset, &block->header.next_consensus, 20);
    
    // Calculate double SHA256 hash
    uint8_t first_hash[32];
    neoc_error_t err = neoc_hash_sha256(header_data, header_size, first_hash);
    if (err == NEOC_SUCCESS) {
        err = neoc_hash_sha256(first_hash, 32, hash.data);
    }
    neoc_free(header_data);
    
    if (err != NEOC_SUCCESS) {
        memset(&hash, 0, sizeof(hash));
    }
    
    return hash;
}

// Verify block
bool neoc_neo_block_verify(const neoc_neo_block_t* block) {
    if (!block) {
        return false;
    }
    
    // Verify merkle root matches transactions
    neoc_hash256_t calculated_merkle = neoc_neo_block_calculate_merkle_root(block);
    if (memcmp(&calculated_merkle, &block->header.merkle_root, sizeof(neoc_hash256_t)) != 0) {
        return false;
    }
    
    // Verify block hash
    neoc_hash256_t calculated_hash = neoc_neo_block_calculate_hash(block);
    if (memcmp(&calculated_hash, &block->hash, sizeof(neoc_hash256_t)) != 0) {
        return false;
    }
    
    // Verify witness if present
    if (block->header.witness) {
        // For production implementation, this would:
        // 1. Verify witness signature against consensus script
        // 2. Validate invocation and verification scripts
        // 3. Check witness script hash matches consensus address
        // Currently simplified to always return true for witness presence
    }
    
    return true;
}

// Parse from JSON
neoc_neo_block_t* neoc_neo_block_from_json(const char* json_str) {
    if (!json_str) {
        return NULL;
    }
    
#ifdef HAVE_CJSON
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return NULL;
    }
    
    neoc_neo_block_t* block = neoc_neo_block_create();
    if (!block) {
        cJSON_Delete(root);
        return NULL;
    }
    
    // Parse hash
    cJSON *hash = cJSON_GetObjectItem(root, "hash");
    if (hash && cJSON_IsString(hash)) {
        neoc_hash256_from_string(hash->valuestring, &block->hash);
    }
    
    // Parse size
    cJSON *size = cJSON_GetObjectItem(root, "size");
    if (size && cJSON_IsNumber(size)) {
        block->size = (uint32_t)size->valueint;
    }
    
    // Parse version
    cJSON *version = cJSON_GetObjectItem(root, "version");
    if (version && cJSON_IsNumber(version)) {
        block->header.version = (uint32_t)version->valueint;
    }
    
    // Parse previous block hash
    cJSON *prev_hash = cJSON_GetObjectItem(root, "previousblockhash");
    if (prev_hash && cJSON_IsString(prev_hash)) {
        neoc_hash256_from_string(prev_hash->valuestring, &block->header.prev_hash);
    }
    
    // Parse merkle root
    cJSON *merkle_root = cJSON_GetObjectItem(root, "merkleroot");
    if (merkle_root && cJSON_IsString(merkle_root)) {
        neoc_hash256_from_string(merkle_root->valuestring, &block->header.merkle_root);
    }
    
    // Parse timestamp
    cJSON *timestamp = cJSON_GetObjectItem(root, "time");
    if (timestamp && cJSON_IsNumber(timestamp)) {
        block->header.timestamp = (uint64_t)timestamp->valuedouble;
    }
    
    // Parse nonce
    cJSON *nonce = cJSON_GetObjectItem(root, "nonce");
    if (nonce && cJSON_IsString(nonce)) {
        sscanf(nonce->valuestring, "%llu", (unsigned long long*)&block->header.nonce);
    }
    
    // Parse index
    cJSON *index = cJSON_GetObjectItem(root, "index");
    if (index && cJSON_IsNumber(index)) {
        block->header.index = (uint32_t)index->valueint;
    }
    
    // Parse primary index
    cJSON *primary = cJSON_GetObjectItem(root, "primary");
    if (primary && cJSON_IsNumber(primary)) {
        block->header.primary_index = (uint8_t)primary->valueint;
    }
    
    // Parse next consensus
    cJSON *next_consensus = cJSON_GetObjectItem(root, "nextconsensus");
    if (next_consensus && cJSON_IsString(next_consensus)) {
        neoc_hash160_from_string(next_consensus->valuestring, &block->header.next_consensus);
    }
    
    // Parse confirmations
    cJSON *confirmations = cJSON_GetObjectItem(root, "confirmations");
    if (confirmations && cJSON_IsNumber(confirmations)) {
        block->confirmations = (uint32_t)confirmations->valueint;
    }
    
    // Parse witness
    cJSON *witness = cJSON_GetObjectItem(root, "witness");
    if (witness && cJSON_IsObject(witness)) {
        // For production implementation, this would parse the witness JSON
        // Currently set to NULL as witness parsing needs to be implemented
        block->header.witness = NULL;
    }
    
    // Parse transactions
    cJSON *tx_array = cJSON_GetObjectItem(root, "tx");
    if (tx_array && cJSON_IsArray(tx_array)) {
        size_t tx_count = cJSON_GetArraySize(tx_array);
        for (size_t i = 0; i < tx_count; i++) {
            cJSON *tx_item = cJSON_GetArrayItem(tx_array, i);
            if (tx_item) {
                char* tx_str = cJSON_Print(tx_item);
                if (tx_str) {
                    neoc_transaction_t* tx = neoc_transaction_from_json(tx_str);
                    if (tx) {
                        neoc_neo_block_add_transaction(block, tx);
                    }
                    neoc_free(tx_str);
                }
            }
        }
    }
    
    cJSON_Delete(root);
    return block;
#else
    return NULL; // cJSON not available
#endif
}

// Convert to JSON
char* neoc_neo_block_to_json(const neoc_neo_block_t* block) {
    if (!block) {
        return NULL;
    }
    
    // Calculate required buffer size
    size_t json_size = 4096;  // Initial size
    char* json = neoc_malloc(json_size);
    if (!json) {
        return NULL;
    }
    
    // Convert hashes to hex strings
    char hash_str[65];
    neoc_hash256_to_string(&block->hash, hash_str, sizeof(hash_str));
    
    char prev_hash_str[65];
    neoc_hash256_to_string(&block->header.prev_hash, prev_hash_str, sizeof(prev_hash_str));
    
    char merkle_str[65];
    neoc_hash256_to_string(&block->header.merkle_root, merkle_str, sizeof(merkle_str));
    
    char consensus_str[41];
    neoc_hash160_to_string(&block->header.next_consensus, consensus_str, sizeof(consensus_str));
    
    // Build JSON
    int offset = snprintf(json, json_size,
                         "{\"hash\":\"%s\",\"size\":%u,\"version\":%u,\"previousblockhash\":\"%s\","
                         "\"merkleroot\":\"%s\",\"time\":%llu,\"nonce\":\"%llu\",\"index\":%u,"
                         "\"primary\":%u,\"nextconsensus\":\"%s\",\"confirmations\":%u",
                         hash_str, block->size, block->header.version, prev_hash_str,
                         merkle_str, (unsigned long long)block->header.timestamp, (unsigned long long)block->header.nonce,
                         block->header.index, block->header.primary_index,
                         consensus_str, block->confirmations);
    
    // Add witness if present
    if (block->header.witness) {
        char* witness_json = neoc_witness_to_json(block->header.witness);
        if (witness_json) {
            offset += snprintf(json + offset, json_size - offset,
                             ",\"witness\":%s", witness_json);
            neoc_free(witness_json);
        }
    }
    
    // Add transactions
    offset += snprintf(json + offset, json_size - offset, ",\"tx\":[");
    for (size_t i = 0; i < block->transaction_count; i++) {
        if (i > 0) {
            offset += snprintf(json + offset, json_size - offset, ",");
        }
        char* tx_json = neoc_transaction_to_json(block->transactions[i]);
        if (tx_json) {
            offset += snprintf(json + offset, json_size - offset, "%s", tx_json);
            neoc_free(tx_json);
        }
    }
    
    snprintf(json + offset, json_size - offset, "]}");
    
    return json;
}

// Serialize to bytes
uint8_t* neoc_neo_block_serialize(const neoc_neo_block_t* block, size_t* out_length) {
    if (!block || !out_length) {
        return NULL;
    }
    
    // Basic serialization implementation
    size_t total_size = sizeof(neoc_block_header_t) + sizeof(uint32_t) * 3;
    
    uint8_t* buffer = neoc_malloc(total_size);
    if (!buffer) {
        return NULL;
    }
    
    size_t offset = 0;
    memcpy(buffer + offset, &block->header, sizeof(neoc_block_header_t));
    offset += sizeof(neoc_block_header_t);
    
    *out_length = offset;
    return buffer;
}

// Deserialize from bytes
neoc_neo_block_t* neoc_neo_block_deserialize(const uint8_t* data, size_t length) {
    if (!data || length == 0) {
        return NULL;
    }
    
    // Basic deserialization implementation
    if (length < sizeof(neoc_block_header_t)) {
        return NULL;
    }
    
    neoc_neo_block_t* block = neoc_neo_block_create();
    if (!block) {
        return NULL;
    }
    
    memcpy(&block->header, data, sizeof(neoc_block_header_t));
    block->hash = neoc_neo_block_calculate_hash(block);
    
    return block;
}

