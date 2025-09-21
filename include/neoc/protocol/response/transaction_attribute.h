#ifndef NEOC_RESPONSE_TRANSACTION_ATTRIBUTE_H
#define NEOC_RESPONSE_TRANSACTION_ATTRIBUTE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Transaction attribute types
typedef enum {
    NEOC_TX_ATTR_HIGH_PRIORITY = 0x01,
    NEOC_TX_ATTR_ORACLE_RESPONSE = 0x11,
    NEOC_TX_ATTR_NOT_VALID_BEFORE = 0x20,
    NEOC_TX_ATTR_CONFLICTS = 0x21
} neoc_transaction_attribute_type_t;

// Oracle response code
typedef enum {
    NEOC_ORACLE_SUCCESS = 0x00,
    NEOC_ORACLE_PROTOCOL_NOT_SUPPORTED = 0x10,
    NEOC_ORACLE_CONSENSUS_UNREACHABLE = 0x12,
    NEOC_ORACLE_NOT_FOUND = 0x14,
    NEOC_ORACLE_TIMEOUT = 0x16,
    NEOC_ORACLE_FORBIDDEN = 0x18,
    NEOC_ORACLE_RESPONSE_TOO_LARGE = 0x1a,
    NEOC_ORACLE_INSUFFICIENT_FUNDS = 0x1c,
    NEOC_ORACLE_CONTENT_TYPE_NOT_SUPPORTED = 0x1f,
    NEOC_ORACLE_ERROR = 0xff
} neoc_oracle_response_code_t;

// Transaction attribute structure
typedef struct neoc_transaction_attribute {
    neoc_transaction_attribute_type_t type;
    union {
        struct {
            uint64_t id;
            neoc_oracle_response_code_t code;
            uint8_t* result;
            size_t result_size;
        } oracle_response;
        struct {
            uint32_t height;
        } not_valid_before;
        struct {
            uint8_t hash[32];
        } conflicts;
    } data;
} neoc_transaction_attribute_t;

// Create high priority attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_high_priority(void);

// Create oracle response attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_oracle_response(
    uint64_t id,
    neoc_oracle_response_code_t code,
    const uint8_t* result,
    size_t result_size
);

// Create not valid before attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_not_valid_before(uint32_t height);

// Create conflicts attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_create_conflicts(const uint8_t* hash);

// Free transaction attribute
void neoc_transaction_attribute_free(neoc_transaction_attribute_t* attr);

// Clone transaction attribute
neoc_transaction_attribute_t* neoc_transaction_attribute_clone(const neoc_transaction_attribute_t* attr);

// Compare transaction attributes
bool neoc_transaction_attribute_equals(
    const neoc_transaction_attribute_t* a,
    const neoc_transaction_attribute_t* b
);

// Get attribute type name
const char* neoc_transaction_attribute_type_name(neoc_transaction_attribute_type_t type);

// Parse from JSON
neoc_transaction_attribute_t* neoc_transaction_attribute_from_json(const char* json_str);

// Convert to JSON
char* neoc_transaction_attribute_to_json(const neoc_transaction_attribute_t* attr);

// Serialize to bytes
uint8_t* neoc_transaction_attribute_serialize(const neoc_transaction_attribute_t* attr, size_t* out_length);

// Deserialize from bytes
neoc_transaction_attribute_t* neoc_transaction_attribute_deserialize(const uint8_t* data, size_t length);

// Get serialized size
size_t neoc_transaction_attribute_get_size(const neoc_transaction_attribute_t* attr);

#ifdef __cplusplus
}
#endif

#endif // NEOC_RESPONSE_TRANSACTION_ATTRIBUTE_H
