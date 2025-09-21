#ifndef NEOC_RESPONSE_TRANSACTION_H
#define NEOC_RESPONSE_TRANSACTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../types/neoc_hash256.h"
#include "../../transaction/signer.h"
#include "../../transaction/witness.h"
#include "transaction_attribute.h"

#ifdef __cplusplus
extern "C" {
#endif

// Transaction structure for RPC responses
typedef struct neoc_transaction {
    neoc_hash256_t hash;
    uint32_t size;
    uint32_t version;
    uint64_t nonce;
    neoc_hash160_t sender;
    uint64_t system_fee;
    uint64_t network_fee;
    uint32_t valid_until_block;
    neoc_signer_t** signers;
    size_t signer_count;
    neoc_transaction_attribute_t** attributes;
    size_t attribute_count;
    uint8_t* script;
    size_t script_size;
    neoc_witness_t** witnesses;
    size_t witness_count;
    uint32_t block_index;
    uint64_t block_time;
    uint32_t confirmations;
    neoc_hash256_t block_hash;
    uint32_t vm_state;
} neoc_transaction_t;

// Create transaction
neoc_transaction_t* neoc_transaction_create(void);

// Free transaction
void neoc_transaction_free(neoc_transaction_t* tx);

// Clone transaction
neoc_transaction_t* neoc_transaction_clone(const neoc_transaction_t* tx);

// Set basic fields
void neoc_transaction_set_version(neoc_transaction_t* tx, uint32_t version);
void neoc_transaction_set_nonce(neoc_transaction_t* tx, uint64_t nonce);
void neoc_transaction_set_sender(neoc_transaction_t* tx, const neoc_hash160_t* sender);
void neoc_transaction_set_system_fee(neoc_transaction_t* tx, uint64_t fee);
void neoc_transaction_set_network_fee(neoc_transaction_t* tx, uint64_t fee);
void neoc_transaction_set_valid_until(neoc_transaction_t* tx, uint32_t block);

// Set script
void neoc_transaction_set_script(neoc_transaction_t* tx, const uint8_t* script, size_t size);

// Add signer
void neoc_transaction_add_signer(neoc_transaction_t* tx, neoc_signer_t* signer);

// Add attribute
void neoc_transaction_add_attribute(neoc_transaction_t* tx, neoc_transaction_attribute_t* attr);

// Add witness
void neoc_transaction_add_witness(neoc_transaction_t* tx, neoc_witness_t* witness);

// Calculate hash
neoc_hash256_t neoc_transaction_calculate_hash(const neoc_transaction_t* tx);

// Calculate fees
uint64_t neoc_transaction_calculate_network_fee(const neoc_transaction_t* tx);
uint64_t neoc_transaction_calculate_system_fee(const neoc_transaction_t* tx);

// Verify transaction
bool neoc_transaction_verify(const neoc_transaction_t* tx);

// Parse from JSON
neoc_transaction_t* neoc_transaction_from_json(const char* json_str);

// Convert to JSON
char* neoc_transaction_to_json(const neoc_transaction_t* tx);

// Serialize to bytes
uint8_t* neoc_transaction_serialize(const neoc_transaction_t* tx, size_t* out_length);

// Deserialize from bytes
neoc_transaction_t* neoc_transaction_deserialize(const uint8_t* data, size_t length);

// Get transaction ID (hash as string)
char* neoc_transaction_get_id(const neoc_transaction_t* tx);

#ifdef __cplusplus
}
#endif

#endif // NEOC_RESPONSE_TRANSACTION_H
