#ifndef NEOC_SCRIPT_BUILDER_H
#define NEOC_SCRIPT_BUILDER_H

#include <stdint.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Script builder type (forward declaration)
 */
typedef struct neoc_script_builder_t neoc_script_builder_t;

/**
 * @brief Build verification script from public key
 * 
 * Creates a script that verifies signatures for the given public key.
 * The script format is: PUSH<pubkey> SYSCALL<CheckSig>
 * 
 * @param public_key The public key bytes (33 or 65 bytes)
 * @param public_key_len Length of public key (33 or 65)
 * @param script Output script bytes (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_build_verification_script(const uint8_t *public_key,
                                                            size_t public_key_len,
                                                            uint8_t **script,
                                                            size_t *script_len);

#ifdef __cplusplus
}
#endif

#endif // NEOC_SCRIPT_BUILDER_H
