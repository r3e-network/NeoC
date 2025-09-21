#ifndef NEOC_NEO_BLOCK_H
#define NEOC_NEO_BLOCK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../../types/neoc_hash256.h"
#include "../../../types/neoc_hash160.h"
#include "neo_witness.h"
#include "../../../transaction/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

// Neo block header structure
typedef struct neoc_block_header {
    uint32_t version;
    neoc_hash256_t prev_hash;
    neoc_hash256_t merkle_root;
    uint64_t timestamp;
    uint64_t nonce;
    uint32_t index;
    uint8_t primary_index;
    neoc_hash160_t next_consensus;
    neoc_witness_t* witness;
} neoc_block_header_t;

// Neo block structure
typedef struct neoc_neo_block {
    neoc_block_header_t header;
    neoc_hash256_t hash;
    uint32_t size;
    uint32_t confirmations;
    neoc_transaction_t** transactions;
    size_t transaction_count;
} neoc_neo_block_t;

// Create block
neoc_neo_block_t* neoc_neo_block_create(void);

// Free block
void neoc_neo_block_free(neoc_neo_block_t* block);

// Clone block
neoc_neo_block_t* neoc_neo_block_clone(const neoc_neo_block_t* block);

// Set header fields
void neoc_neo_block_set_version(neoc_neo_block_t* block, uint32_t version);
void neoc_neo_block_set_prev_hash(neoc_neo_block_t* block, const neoc_hash256_t* hash);
void neoc_neo_block_set_merkle_root(neoc_neo_block_t* block, const neoc_hash256_t* hash);
void neoc_neo_block_set_timestamp(neoc_neo_block_t* block, uint64_t timestamp);
void neoc_neo_block_set_index(neoc_neo_block_t* block, uint32_t index);
void neoc_neo_block_set_primary_index(neoc_neo_block_t* block, uint8_t index);
void neoc_neo_block_set_next_consensus(neoc_neo_block_t* block, const neoc_hash160_t* hash);

// Add transaction
void neoc_neo_block_add_transaction(neoc_neo_block_t* block, neoc_transaction_t* tx);

// Calculate merkle root
neoc_hash256_t neoc_neo_block_calculate_merkle_root(const neoc_neo_block_t* block);

// Calculate block hash
neoc_hash256_t neoc_neo_block_calculate_hash(const neoc_neo_block_t* block);

// Verify block
bool neoc_neo_block_verify(const neoc_neo_block_t* block);

// Parse from JSON
neoc_neo_block_t* neoc_neo_block_from_json(const char* json_str);

// Convert to JSON
char* neoc_neo_block_to_json(const neoc_neo_block_t* block);

// Serialize to bytes
uint8_t* neoc_neo_block_serialize(const neoc_neo_block_t* block, size_t* out_length);

// Deserialize from bytes
neoc_neo_block_t* neoc_neo_block_deserialize(const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_BLOCK_H
