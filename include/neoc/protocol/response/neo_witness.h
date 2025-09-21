#ifndef NEOC_NEO_WITNESS_H
#define NEOC_NEO_WITNESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Neo witness structure
typedef struct neoc_neo_witness {
    uint8_t* invocation_script;
    size_t invocation_script_size;
    uint8_t* verification_script;
    size_t verification_script_size;
} neoc_neo_witness_t;

// Create witness
neoc_neo_witness_t* neoc_neo_witness_create(
    const uint8_t* invocation_script,
    size_t invocation_size,
    const uint8_t* verification_script,
    size_t verification_size
);

// Create empty witness
neoc_neo_witness_t* neoc_neo_witness_create_empty(void);

// Free witness
void neoc_neo_witness_free(neoc_neo_witness_t* witness);

// Clone witness
neoc_neo_witness_t* neoc_neo_witness_clone(const neoc_neo_witness_t* witness);

// Set invocation script
void neoc_neo_witness_set_invocation_script(
    neoc_neo_witness_t* witness,
    const uint8_t* script,
    size_t size
);

// Set verification script
void neoc_neo_witness_set_verification_script(
    neoc_neo_witness_t* witness,
    const uint8_t* script,
    size_t size
);

// Compare witnesses
bool neoc_neo_witness_equals(
    const neoc_neo_witness_t* a,
    const neoc_neo_witness_t* b
);

// Get invocation script as hex string
char* neoc_neo_witness_get_invocation_hex(const neoc_neo_witness_t* witness);

// Get verification script as hex string
char* neoc_neo_witness_get_verification_hex(const neoc_neo_witness_t* witness);

// Parse from JSON
neoc_neo_witness_t* neoc_neo_witness_from_json(const char* json_str);

// Convert to JSON
char* neoc_neo_witness_to_json(const neoc_neo_witness_t* witness);

// Serialize to bytes
uint8_t* neoc_neo_witness_serialize(const neoc_neo_witness_t* witness, size_t* out_length);

// Deserialize from bytes
neoc_neo_witness_t* neoc_neo_witness_deserialize(const uint8_t* data, size_t length);

// Get serialized size
size_t neoc_neo_witness_get_size(const neoc_neo_witness_t* witness);

// Verify witness (check script execution)
bool neoc_neo_witness_verify(
    const neoc_neo_witness_t* witness,
    const uint8_t* message,
    size_t message_size
);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_WITNESS_H
