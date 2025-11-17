/**
 * @file witness_condition.h
 * @brief Witness condition types and structures
 * 
 * Based on Swift source: protocol/core/witnessrule/WitnessCondition.swift
 * Defines the complex conditional logic for witness validation
 */

#ifndef NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_CONDITION_H
#define NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_CONDITION_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/types/hash160.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef NEOC_FORWARD_DECLARATIONS
#define NEOC_FORWARD_DECLARATIONS
typedef struct neoc_binary_reader neoc_binary_reader_t;
typedef struct neoc_binary_writer neoc_binary_writer_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Witness condition types
 * 
 * These correspond to the different types of conditions that can be evaluated
 */
typedef enum {
    NEOC_WITNESS_CONDITION_BOOLEAN = 0x00,
    NEOC_WITNESS_CONDITION_NOT = 0x01,
    NEOC_WITNESS_CONDITION_AND = 0x02,
    NEOC_WITNESS_CONDITION_OR = 0x03,
    NEOC_WITNESS_CONDITION_SCRIPT_HASH = 0x18,
    NEOC_WITNESS_CONDITION_GROUP = 0x19,
    NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY = 0x20,
    NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT = 0x28,
    NEOC_WITNESS_CONDITION_CALLED_BY_GROUP = 0x29
} neoc_witness_condition_type_t;

#define NEOC_WITNESS_CONDITION_MAX_SUBITEMS 16
#define NEOC_WITNESS_CONDITION_MAX_NESTING_DEPTH 2

/**
 * @brief Forward declaration of witness condition
 */
typedef struct neoc_witness_condition neoc_witness_condition_t;

/**
 * @brief Witness condition structure
 * 
 * Represents a conditional expression for witness validation
 */
struct neoc_witness_condition {
    neoc_witness_condition_type_t type; /**< Condition type */
    
    union {
        struct {
            bool value;                 /**< Boolean value */
        } boolean;
        
        struct {
            neoc_witness_condition_t *expression; /**< Negated expression */
        } not_expr;
        
        struct {
            neoc_witness_condition_t **expressions; /**< Array of expressions */
            size_t count;               /**< Number of expressions */
        } logical;                      /**< AND/OR expressions */
        
        struct {
            neoc_hash160_t hash;        /**< Script or contract hash */
        } hash_condition;
        
        struct {
            neoc_ec_public_key_t *public_key; /**< Group key (owned) */
        } group_condition;
    } data;
};

/* ========== Creation and Destruction Functions ========== */

/**
 * @brief Create a boolean witness condition
 * 
 * @param value Boolean value
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_boolean(
    bool value,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create a NOT witness condition
 * 
 * @param expression Expression to negate (takes ownership)
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_not(
    neoc_witness_condition_t *expression,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create an AND witness condition
 * 
 * @param expressions Array of expressions (takes ownership)
 * @param count Number of expressions
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_and(
    neoc_witness_condition_t **expressions,
    size_t count,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create an OR witness condition
 * 
 * @param expressions Array of expressions (takes ownership)
 * @param count Number of expressions
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_or(
    neoc_witness_condition_t **expressions,
    size_t count,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create a script hash witness condition
 * 
 * @param hash Script hash
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_script_hash(
    const neoc_hash160_t *hash,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create a group witness condition
 * 
 * @param public_key Public key bytes
 * @param key_length Public key length
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_group(
    const neoc_ec_public_key_t *public_key,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create a "called by entry" witness condition
 * 
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_called_by_entry(
    neoc_witness_condition_t **condition
);

/**
 * @brief Create a "called by contract" witness condition
 * 
 * @param contract_hash Contract hash
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_called_by_contract(
    const neoc_hash160_t *contract_hash,
    neoc_witness_condition_t **condition
);

/**
 * @brief Create a "called by group" witness condition
 * 
 * @param group_key Group public key
 * @param key_length Key length
 * @param condition Pointer to store the created condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_create_called_by_group(
    const neoc_ec_public_key_t *group_key,
    neoc_witness_condition_t **condition
);

/**
 * @brief Free a witness condition and its resources
 * 
 * @param condition Condition to free
 */
void neoc_witness_condition_free(
    neoc_witness_condition_t *condition
);

/* ========== Utility Functions ========== */

/**
 * @brief Get the type of a witness condition
 * 
 * @param condition Witness condition
 * @return Condition type
 */
neoc_witness_condition_type_t neoc_witness_condition_get_type(
    const neoc_witness_condition_t *condition
);

/**
 * @brief Get boolean value from condition
 *
 * @param condition Witness condition (must be BOOLEAN)
 * @param value Output boolean value
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_witness_condition_get_boolean(
    const neoc_witness_condition_t *condition,
    bool *value
);

/**
 * @brief Get the byte value of a condition type
 * 
 * @param type Condition type
 * @return Byte value
 */
uint8_t neoc_witness_condition_type_get_byte(
    neoc_witness_condition_type_t type
);

/**
 * @brief Get JSON string representation of condition type
 * 
 * @param type Condition type
 * @return JSON string representation
 */
const char* neoc_witness_condition_type_to_json_string(
    neoc_witness_condition_type_t type
);

/**
 * @brief Parse JSON into witness condition
 * 
 * @param json_str JSON string to parse
 * @param condition Pointer to store the parsed condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_from_json(
    const char *json_str,
    neoc_witness_condition_t **condition
);

/**
 * @brief Convert witness condition to JSON string
 * 
 * @param condition Condition to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_to_json(
    const neoc_witness_condition_t *condition,
    char **json_str
);

/**
 * @brief Calculate the serialized size of a witness condition
 * 
 * @param condition Witness condition
 * @return Size in bytes
 */
size_t neoc_witness_condition_get_size(
    const neoc_witness_condition_t *condition
);

/**
 * @brief Serialize a witness condition using the Neo binary format.
 *
 * @param condition Condition to serialize
 * @param writer Binary writer to receive bytes
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_serialize(
    const neoc_witness_condition_t *condition,
    neoc_binary_writer_t *writer
);

/**
 * @brief Deserialize a witness condition from a binary reader.
 *
 * @param reader Binary reader positioned at the condition
 * @param condition Output condition (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_deserialize(
    neoc_binary_reader_t *reader,
    neoc_witness_condition_t **condition
);

/**
 * @brief Clone a witness condition
 * 
 * @param src Source condition
 * @param dest Pointer to store the cloned condition
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_witness_condition_clone(
    const neoc_witness_condition_t *src,
    neoc_witness_condition_t **dest
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_WITNESSRULE_WITNESS_CONDITION_H */
