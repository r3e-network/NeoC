/**
 * @file call_flags.h
 * @brief Neo contract call flags enumeration
 */

#ifndef NEOC_CALL_FLAGS_H_GUARD
#define NEOC_CALL_FLAGS_H_GUARD

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Neo contract call flags
 * These flags control what operations a contract is allowed to perform
 */
typedef enum {
    NEOC_CALL_FLAG_NONE           = 0x00,   ///< No permissions
    NEOC_CALL_FLAG_READ_STATES    = 0x01,   ///< Can read blockchain state
    NEOC_CALL_FLAG_WRITE_STATES   = 0x02,   ///< Can write blockchain state  
    NEOC_CALL_FLAG_ALLOW_CALL     = 0x04,   ///< Can call other contracts
    NEOC_CALL_FLAG_ALLOW_NOTIFY   = 0x08,   ///< Can emit notifications
    NEOC_CALL_FLAG_STATES         = 0x03,   ///< READ_STATES | WRITE_STATES
    NEOC_CALL_FLAG_READ_ONLY      = 0x05,   ///< READ_STATES | ALLOW_CALL
    NEOC_CALL_FLAG_ALL            = 0x0F    ///< All permissions
} neoc_call_flags_t;

/**
 * Convert call flags to byte value
 * @param flags The call flags
 * @return Byte value
 */
uint8_t neoc_call_flags_to_byte(neoc_call_flags_t flags);

/**
 * Convert byte value to call flags
 * @param value The byte value
 * @param flags Output call flags
 * @return Error code
 */
neoc_error_t neoc_call_flags_from_byte(uint8_t value, neoc_call_flags_t *flags);

/**
 * Convert call flags to string
 * @param flags The call flags
 * @return String representation (caller must free)
 */
char* neoc_call_flags_to_string(neoc_call_flags_t flags);

/**
 * Convert string to call flags
 * @param str The string representation
 * @param flags Output call flags
 * @return Error code
 */
neoc_error_t neoc_call_flags_from_string(const char *str, neoc_call_flags_t *flags);

/**
 * Check if flags contain specific flag
 * @param flags The call flags to check
 * @param flag The flag to check for
 * @return True if flag is present
 */
bool neoc_call_flags_has_flag(neoc_call_flags_t flags, neoc_call_flags_t flag);

/**
 * Combine two call flags
 * @param a First flags
 * @param b Second flags
 * @return Combined flags
 */
neoc_call_flags_t neoc_call_flags_combine(neoc_call_flags_t a, neoc_call_flags_t b);

#ifdef __cplusplus
}
#endif

#endif // NEOC_CALL_FLAGS_H_GUARD
