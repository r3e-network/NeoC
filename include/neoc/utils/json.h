/**
 * @file json.h
 * @brief JSON parsing utilities for NeoC
 * 
 * Provides JSON parsing and serialization functions using cJSON library
 */

#ifndef NEOC_UTILS_JSON_H
#define NEOC_UTILS_JSON_H

#include "neoc/neoc_error.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque JSON object type
 */
typedef struct cJSON neoc_json_t;

/**
 * @brief Parse JSON string
 * @param json_string JSON string to parse
 * @return Parsed JSON object or NULL on error
 */
neoc_json_t *neoc_json_parse(const char *json_string);

/**
 * @brief Create a new JSON object
 * @return New JSON object or NULL on error
 */
neoc_json_t *neoc_json_create_object(void);

/**
 * @brief Create a new JSON array
 * @return New JSON array or NULL on error
 */
neoc_json_t *neoc_json_create_array(void);

/**
 * @brief Add string to JSON object
 * @param object JSON object
 * @param name Field name
 * @param value String value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_add_string(neoc_json_t *object, const char *name, const char *value);

/**
 * @brief Add number to JSON object
 * @param object JSON object
 * @param name Field name
 * @param value Numeric value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_add_number(neoc_json_t *object, const char *name, double value);

/**
 * @brief Add integer to JSON object
 * @param object JSON object
 * @param name Field name
 * @param value Integer value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_add_int(neoc_json_t *object, const char *name, int64_t value);

/**
 * @brief Add boolean to JSON object
 * @param object JSON object
 * @param name Field name
 * @param value Boolean value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_add_bool(neoc_json_t *object, const char *name, bool value);

/**
 * @brief Add object to JSON object
 * @param object Parent JSON object
 * @param name Field name
 * @param child Child object to add
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_add_object(neoc_json_t *object, const char *name, neoc_json_t *child);

/**
 * @brief Add item to JSON array
 * @param array JSON array
 * @param item Item to add
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_array_add(neoc_json_t *array, neoc_json_t *item);

/**
 * @brief Get string from JSON object
 * @param object JSON object
 * @param name Field name
 * @return String value or NULL if not found
 */
const char *neoc_json_get_string(const neoc_json_t *object, const char *name);

/**
 * @brief Get number from JSON object
 * @param object JSON object
 * @param name Field name
 * @param value Output for numeric value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_get_number(const neoc_json_t *object, const char *name, double *value);

/**
 * @brief Get integer from JSON object
 * @param object JSON object
 * @param name Field name
 * @param value Output for integer value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_get_int(const neoc_json_t *object, const char *name, int64_t *value);

/**
 * @brief Get boolean from JSON object
 * @param object JSON object
 * @param name Field name
 * @param value Output for boolean value
 * @return NEOC_SUCCESS on success
 */
neoc_error_t neoc_json_get_bool(const neoc_json_t *object, const char *name, bool *value);

/**
 * @brief Get object from JSON object
 * @param object JSON object
 * @param name Field name
 * @return Child object or NULL if not found
 */
neoc_json_t *neoc_json_get_object(const neoc_json_t *object, const char *name);

/**
 * @brief Get array from JSON object
 * @param object JSON object
 * @param name Field name
 * @return Array or NULL if not found
 */
neoc_json_t *neoc_json_get_array(const neoc_json_t *object, const char *name);

/**
 * @brief Get array size
 * @param array JSON array
 * @return Array size or 0 if not an array
 */
size_t neoc_json_array_size(const neoc_json_t *array);

/**
 * @brief Get array item by index
 * @param array JSON array
 * @param index Item index
 * @return Array item or NULL if out of bounds
 */
neoc_json_t *neoc_json_array_get(const neoc_json_t *array, size_t index);

/**
 * @brief Convert JSON to string
 * @param json JSON object
 * @return JSON string (must be freed) or NULL on error
 */
char *neoc_json_to_string(const neoc_json_t *json);

/**
 * @brief Convert JSON to formatted string
 * @param json JSON object
 * @return Formatted JSON string (must be freed) or NULL on error
 */
char *neoc_json_to_string_formatted(const neoc_json_t *json);

/**
 * @brief Free JSON object
 * @param json JSON object to free
 */
void neoc_json_free(neoc_json_t *json);

/**
 * @brief Check if JSON value is null
 * @param json JSON object
 * @return true if null or NULL pointer
 */
bool neoc_json_is_null(const neoc_json_t *json);

/**
 * @brief Check if JSON value is array
 * @param json JSON object
 * @return true if array
 */
bool neoc_json_is_array(const neoc_json_t *json);

/**
 * @brief Check if JSON value is object
 * @param json JSON object
 * @return true if object
 */
bool neoc_json_is_object(const neoc_json_t *json);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_JSON_H */
