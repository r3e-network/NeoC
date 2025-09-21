/**
 * @file neo_v_m_state_type.h
 * @brief Neo VM execution state types
 */

#ifndef NEOC_NEO_VM_STATE_TYPE_H_GUARD
#define NEOC_NEO_VM_STATE_TYPE_H_GUARD

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Neo VM execution state types
 * These indicate the final state of VM execution
 */
typedef enum {
    NEOC_VM_STATE_NONE  = 0,        ///< No state (not executed)
    NEOC_VM_STATE_HALT  = 1,        ///< Execution completed successfully
    NEOC_VM_STATE_FAULT = 1 << 1,   ///< Execution failed with fault
    NEOC_VM_STATE_BREAK = 1 << 2    ///< Execution paused at breakpoint
} neoc_vm_state_t;

/**
 * Convert VM state to integer value
 * @param state The VM state
 * @return Integer value
 */
int neoc_vm_state_to_int(neoc_vm_state_t state);

/**
 * Convert integer value to VM state
 * @param value The integer value
 * @param state Output VM state
 * @return Error code
 */
neoc_error_t neoc_vm_state_from_int(int value, neoc_vm_state_t *state);

/**
 * Convert VM state to string representation
 * @param state The VM state
 * @return String representation (statically allocated)
 */
const char* neoc_vm_state_to_string(neoc_vm_state_t state);

/**
 * Convert string to VM state
 * @param str The string representation
 * @param state Output VM state
 * @return Error code
 */
neoc_error_t neoc_vm_state_from_string(const char *str, neoc_vm_state_t *state);

/**
 * Convert VM state to JSON string value
 * @param state The VM state
 * @return JSON string representation (statically allocated)
 */
const char* neoc_vm_state_to_json_value(neoc_vm_state_t state);

/**
 * Convert JSON string value to VM state
 * @param json_value The JSON string value
 * @param state Output VM state
 * @return Error code
 */
neoc_error_t neoc_vm_state_from_json_value(const char *json_value, neoc_vm_state_t *state);

/**
 * Check if VM state indicates successful execution
 * @param state The VM state
 * @return True if execution was successful
 */
bool neoc_vm_state_is_successful(neoc_vm_state_t state);

/**
 * Check if VM state indicates execution error
 * @param state The VM state
 * @return True if execution failed
 */
bool neoc_vm_state_is_error(neoc_vm_state_t state);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_VM_STATE_TYPE_H_GUARD
