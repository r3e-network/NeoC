#ifndef NEOC_WITNESS_H
#define NEOC_WITNESS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transaction witness structure
 */
typedef struct {
    uint8_t *invocation_script;    // Invocation script (signatures)
    size_t invocation_script_len;  // Invocation script length
    uint8_t *verification_script;  // Verification script (public keys)
    size_t verification_script_len;// Verification script length
} neoc_witness_t;

/**
 * @brief Create a new witness
 * 
 * @param invocation_script Invocation script bytes
 * @param invocation_len Length of invocation script
 * @param verification_script Verification script bytes
 * @param verification_len Length of verification script
 * @param witness Output witness (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_witness_create(const uint8_t *invocation_script,
                                  size_t invocation_len,
                                  const uint8_t *verification_script,
                                  size_t verification_len,
                                  neoc_witness_t **witness);

/**
 * @brief Create witness from account signature
 * 
 * @param signature Signature bytes
 * @param signature_len Signature length
 * @param public_key Public key bytes
 * @param public_key_len Public key length
 * @param witness Output witness (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_witness_create_from_signature(const uint8_t *signature,
                                                 size_t signature_len,
                                                 const uint8_t *public_key,
                                                 size_t public_key_len,
                                                 neoc_witness_t **witness);

/**
 * @brief Get witness size for serialization
 * 
 * @param witness The witness
 * @return Size in bytes
 */
size_t neoc_witness_get_size(const neoc_witness_t *witness);

/**
 * @brief Serialize witness to bytes
 * 
 * @param witness The witness
 * @param bytes Output bytes (caller must free)
 * @param bytes_len Output length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_witness_serialize(const neoc_witness_t *witness,
                                     uint8_t **bytes,
                                     size_t *bytes_len);

/**
 * @brief Deserialize witness from bytes
 * 
 * @param bytes Witness bytes
 * @param bytes_len Length of bytes
 * @param witness Output witness (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_witness_deserialize(const uint8_t *bytes,
                                       size_t bytes_len,
                                       neoc_witness_t **witness);

/**
 * @brief Free a witness
 * 
 * @param witness The witness to free
 */
void neoc_witness_free(neoc_witness_t *witness);

/**
 * @brief Convert witness to JSON string
 * 
 * @param witness The witness
 * @return JSON string (caller must free) or NULL on error
 */
char* neoc_witness_to_json(const neoc_witness_t *witness);

/**
 * @brief Clone a witness structure
 * 
 * @param source Source witness
 * @param dest Output destination witness (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_witness_clone(const neoc_witness_t *source, neoc_witness_t **dest);

#ifdef __cplusplus
}
#endif

#endif // NEOC_WITNESS_H
