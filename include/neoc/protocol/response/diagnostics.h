#ifndef NEOC_DIAGNOSTICS_H
#define NEOC_DIAGNOSTICS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Storage change structure
typedef struct neoc_storage_change {
    char* contract;         // Contract hash
    char* key;              // Storage key
    char* value;            // Storage value
} neoc_storage_change_t;

// Diagnostics invocation structure
typedef struct neoc_diagnostics_invocation {
    char* contract;                     // Contract hash
    char* method;                       // Method name
    char** arguments;                   // Arguments
    size_t arguments_count;             // Arguments count
    uint64_t gas_consumed;              // GAS consumed
    struct neoc_diagnostics_invocation** invocations;  // Nested invocations
    size_t invocations_count;          // Nested invocations count
} neoc_diagnostics_invocation_t;

// Diagnostics structure
typedef struct neoc_diagnostics {
    neoc_diagnostics_invocation_t** invocations;   // Invocation tree
    size_t invocations_count;                      // Invocations count
    neoc_storage_change_t** storage_changes;       // Storage changes
    size_t storage_changes_count;                  // Storage changes count
} neoc_diagnostics_t;

// Create diagnostics
neoc_diagnostics_t* neoc_diagnostics_create(void);

// Free diagnostics
void neoc_diagnostics_free(neoc_diagnostics_t* diagnostics);

// Clone diagnostics
neoc_diagnostics_t* neoc_diagnostics_clone(const neoc_diagnostics_t* diagnostics);

// Create diagnostics invocation
neoc_diagnostics_invocation_t* neoc_diagnostics_invocation_create(
    const char* contract,
    const char* method
);

// Free diagnostics invocation
void neoc_diagnostics_invocation_free(neoc_diagnostics_invocation_t* invocation);

// Add argument to invocation
void neoc_diagnostics_invocation_add_argument(
    neoc_diagnostics_invocation_t* invocation,
    const char* argument
);

// Add nested invocation
void neoc_diagnostics_invocation_add_nested(
    neoc_diagnostics_invocation_t* parent,
    neoc_diagnostics_invocation_t* nested
);

// Create storage change
neoc_storage_change_t* neoc_storage_change_create(
    const char* contract,
    const char* key,
    const char* value
);

// Free storage change
void neoc_storage_change_free(neoc_storage_change_t* change);

// Add invocation to diagnostics
void neoc_diagnostics_add_invocation(
    neoc_diagnostics_t* diagnostics,
    neoc_diagnostics_invocation_t* invocation
);

// Add storage change to diagnostics
void neoc_diagnostics_add_storage_change(
    neoc_diagnostics_t* diagnostics,
    neoc_storage_change_t* change
);

// Parse from JSON
neoc_diagnostics_t* neoc_diagnostics_from_json(const char* json_str);

// Convert to JSON
char* neoc_diagnostics_to_json(const neoc_diagnostics_t* diagnostics);

#ifdef __cplusplus
}
#endif

#endif // NEOC_DIAGNOSTICS_H
