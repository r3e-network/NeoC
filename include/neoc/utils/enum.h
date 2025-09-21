/**
 * @file enum.h
 * @brief Enum utility functions and ByteEnum protocol implementation
 * 
 * Based on Swift source: utils/Enum.swift
 * Provides enum validation, conversion, and serialization utilities
 */

#ifndef NEOC_UTILS_ENUM_H
#define NEOC_UTILS_ENUM_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic byte-based enum entry structure
 * 
 * Equivalent to Swift ByteEnum protocol members
 */
typedef struct {
    uint8_t byte_value;      /**< Byte representation */
    const char *json_value;  /**< JSON string representation */
    const char *name;        /**< Enum case name */
} neoc_byte_enum_entry_t;

/**
 * @brief Byte enum definition structure
 */
typedef struct {
    const neoc_byte_enum_entry_t *entries;  /**< Array of enum entries */
    size_t count;                           /**< Number of entries */
    const char *type_name;                  /**< Name of the enum type */
} neoc_byte_enum_def_t;

/**
 * @brief Find enum entry by byte value
 * 
 * Equivalent to Swift ByteEnum.valueOf(_:)
 * 
 * @param enum_def Enum definition
 * @param byte_value Byte value to find
 * @param entry Output: pointer to found entry (do not free)
 * @return NEOC_SUCCESS if found, NEOC_ERROR_NOT_FOUND otherwise
 */
neoc_error_t neoc_enum_find_by_byte(const neoc_byte_enum_def_t *enum_def,
                                     uint8_t byte_value,
                                     const neoc_byte_enum_entry_t **entry);

/**
 * @brief Find enum entry by byte value (throwing version)
 * 
 * Equivalent to Swift ByteEnum.throwingValueOf(_:)
 * 
 * @param enum_def Enum definition
 * @param byte_value Byte value to find
 * @param entry Output: pointer to found entry (do not free)
 * @return NEOC_SUCCESS if found, error code otherwise
 */
neoc_error_t neoc_enum_throwing_value_of(const neoc_byte_enum_def_t *enum_def,
                                          uint8_t byte_value,
                                          const neoc_byte_enum_entry_t **entry);

/**
 * @brief Find enum entry by JSON value string
 * 
 * Equivalent to Swift ByteEnum.fromJsonValue(_:)
 * 
 * @param enum_def Enum definition
 * @param json_value JSON string value to find
 * @param entry Output: pointer to found entry (do not free)
 * @return NEOC_SUCCESS if found, NEOC_ERROR_NOT_FOUND otherwise
 */
neoc_error_t neoc_enum_find_by_json_value(const neoc_byte_enum_def_t *enum_def,
                                           const char *json_value,
                                           const neoc_byte_enum_entry_t **entry);

/**
 * @brief Find enum entry by name
 * 
 * @param enum_def Enum definition
 * @param name Name to find
 * @param entry Output: pointer to found entry (do not free)
 * @return NEOC_SUCCESS if found, NEOC_ERROR_NOT_FOUND otherwise
 */
neoc_error_t neoc_enum_find_by_name(const neoc_byte_enum_def_t *enum_def,
                                     const char *name,
                                     const neoc_byte_enum_entry_t **entry);

/**
 * @brief Parse enum from JSON (supports string, byte, or int values)
 * 
 * Equivalent to Swift ByteEnum.init(from decoder:)
 * 
 * @param enum_def Enum definition
 * @param json_str JSON string containing enum value
 * @param entry Output: pointer to parsed entry (do not free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_enum_parse_json(const neoc_byte_enum_def_t *enum_def,
                                   const char *json_str,
                                   const neoc_byte_enum_entry_t **entry);

/**
 * @brief Serialize enum entry to JSON string
 * 
 * Equivalent to Swift ByteEnum.encode(to encoder:)
 * 
 * @param entry Enum entry to serialize
 * @param json_str Output: JSON string representation (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_enum_to_json(const neoc_byte_enum_entry_t *entry,
                                char **json_str);

/**
 * @brief Calculate hash for enum entry
 * 
 * Equivalent to Swift ByteEnum.hash(into:)
 * 
 * @param entry Enum entry
 * @return Hash value
 */
uint32_t neoc_enum_hash(const neoc_byte_enum_entry_t *entry);

/**
 * @brief Compare two enum entries for equality
 * 
 * @param entry1 First entry
 * @param entry2 Second entry
 * @return True if equal
 */
bool neoc_enum_equals(const neoc_byte_enum_entry_t *entry1,
                       const neoc_byte_enum_entry_t *entry2);

/**
 * @brief Validate enum definition
 * 
 * Checks for duplicate byte values, JSON values, and names
 * 
 * @param enum_def Enum definition to validate
 * @return NEOC_SUCCESS if valid, error code otherwise
 */
neoc_error_t neoc_enum_validate_definition(const neoc_byte_enum_def_t *enum_def);

/**
 * @brief Get all enum entries
 * 
 * Equivalent to Swift CaseIterable.allCases
 * 
 * @param enum_def Enum definition
 * @param entries Output: array of all entries (do not free)
 * @param count Output: number of entries
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_enum_get_all_cases(const neoc_byte_enum_def_t *enum_def,
                                      const neoc_byte_enum_entry_t **entries,
                                      size_t *count);

/**
 * @brief Check if byte value exists in enum
 * 
 * @param enum_def Enum definition
 * @param byte_value Byte value to check
 * @return True if value exists
 */
bool neoc_enum_contains_byte(const neoc_byte_enum_def_t *enum_def, uint8_t byte_value);

/**
 * @brief Check if JSON value exists in enum
 * 
 * @param enum_def Enum definition
 * @param json_value JSON value to check
 * @return True if value exists
 */
bool neoc_enum_contains_json_value(const neoc_byte_enum_def_t *enum_def, const char *json_value);

/**
 * @brief Get enum type name
 * 
 * @param enum_def Enum definition
 * @return Type name string (do not free)
 */
const char *neoc_enum_get_type_name(const neoc_byte_enum_def_t *enum_def);

/**
 * @brief Get number of enum cases
 * 
 * @param enum_def Enum definition
 * @return Number of cases
 */
size_t neoc_enum_get_case_count(const neoc_byte_enum_def_t *enum_def);

// Macro helpers for defining byte enums

/**
 * @brief Macro to define a byte enum entry
 */
#define NEOC_BYTE_ENUM_ENTRY(byte_val, json_val, name_val) \
    { .byte_value = (byte_val), .json_value = (json_val), .name = (name_val) }

/**
 * @brief Macro to define a byte enum definition
 */
#define NEOC_BYTE_ENUM_DEF(type_name_val, entries_array) \
    { .entries = (entries_array), .count = sizeof(entries_array)/sizeof(entries_array[0]), .type_name = (type_name_val) }

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_ENUM_H */
