/**
 * @file invocation_script.h
 * @brief Neo invocation script implementation
 * 
 * Based on Swift source: script/InvocationScript.swift
 * An invocation script is part of a witness and contains the input data
 * for the verification script (usually signatures).
 */

#ifndef NEOC_SCRIPT_INVOCATION_SCRIPT_H
#define NEOC_SCRIPT_INVOCATION_SCRIPT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Invocation script structure
 * 
 * Contains the Neo VM instructions that provide input to verification script
 */
typedef struct {
    uint8_t *script;         /**< The invocation script as byte array */
    size_t script_length;    /**< Length of the script in bytes */
} neoc_invocation_script_t;

/**
 * @brief Create invocation script from byte array
 * 
 * Equivalent to Swift InvocationScript.init(_ script: Bytes)
 * 
 * @param script Script bytes
 * @param script_length Length of script
 * @param invocation_script Output invocation script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_create(const uint8_t *script,
                                            size_t script_length,
                                            neoc_invocation_script_t **invocation_script);

/**
 * @brief Create empty invocation script
 * 
 * @param invocation_script Output invocation script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_create_empty(neoc_invocation_script_t **invocation_script);

/**
 * @brief Create invocation script with single signature
 * 
 * @param signature Signature bytes
 * @param signature_length Length of signature
 * @param invocation_script Output invocation script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_create_single_sig(const uint8_t *signature,
                                                       size_t signature_length,
                                                       neoc_invocation_script_t **invocation_script);

/**
 * @brief Create invocation script with multiple signatures
 * 
 * @param signatures Array of signature byte arrays
 * @param signature_lengths Array of signature lengths
 * @param signature_count Number of signatures
 * @param invocation_script Output invocation script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_create_multi_sig(const uint8_t **signatures,
                                                      const size_t *signature_lengths,
                                                      size_t signature_count,
                                                      neoc_invocation_script_t **invocation_script);

/**
 * @brief Add signature to invocation script
 * 
 * @param invocation_script The invocation script to modify
 * @param signature Signature bytes to add
 * @param signature_length Length of signature
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_add_signature(neoc_invocation_script_t *invocation_script,
                                                   const uint8_t *signature,
                                                   size_t signature_length);

/**
 * @brief Get script bytes
 * 
 * @param invocation_script The invocation script
 * @param script Output pointer to script bytes (do not free)
 * @param script_length Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_get_script(const neoc_invocation_script_t *invocation_script,
                                                const uint8_t **script,
                                                size_t *script_length);

/**
 * @brief Get script size in bytes
 * 
 * Equivalent to Swift InvocationScript.size
 * 
 * @param invocation_script The invocation script
 * @return Size in bytes
 */
size_t neoc_invocation_script_get_size(const neoc_invocation_script_t *invocation_script);

/**
 * @brief Check if invocation script is empty
 * 
 * @param invocation_script The invocation script
 * @return True if empty
 */
bool neoc_invocation_script_is_empty(const neoc_invocation_script_t *invocation_script);

/**
 * @brief Extract signatures from invocation script
 * 
 * @param invocation_script The invocation script
 * @param signatures Output array of signature byte arrays (caller must free)
 * @param signature_lengths Output array of signature lengths (caller must free)
 * @param signature_count Output number of signatures
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_extract_signatures(const neoc_invocation_script_t *invocation_script,
                                                        uint8_t ***signatures,
                                                        size_t **signature_lengths,
                                                        size_t *signature_count);

/**
 * @brief Free signatures array returned by extract_signatures
 * 
 * @param signatures Array of signature byte arrays
 * @param signature_lengths Array of signature lengths
 * @param signature_count Number of signatures
 */
void neoc_invocation_script_free_signatures(uint8_t **signatures,
                                             size_t *signature_lengths,
                                             size_t signature_count);

/**
 * @brief Copy invocation script
 * 
 * @param source Source invocation script
 * @param copy Output copied script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_copy(const neoc_invocation_script_t *source,
                                          neoc_invocation_script_t **copy);

/**
 * @brief Compare two invocation scripts for equality
 * 
 * Equivalent to Swift InvocationScript.== operator
 * 
 * @param script1 First invocation script
 * @param script2 Second invocation script
 * @return True if equal
 */
bool neoc_invocation_script_equals(const neoc_invocation_script_t *script1,
                                    const neoc_invocation_script_t *script2);

/**
 * @brief Calculate hash of invocation script
 * 
 * Equivalent to Swift InvocationScript.hash(into:)
 * 
 * @param invocation_script The invocation script
 * @return Hash value
 */
uint32_t neoc_invocation_script_hash(const neoc_invocation_script_t *invocation_script);

/**
 * @brief Serialize invocation script
 * 
 * Equivalent to Swift InvocationScript.serialize(_:)
 * 
 * @param invocation_script The invocation script
 * @param serialized Output serialized data (caller must free)
 * @param serialized_length Output length of serialized data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_serialize(const neoc_invocation_script_t *invocation_script,
                                               uint8_t **serialized,
                                               size_t *serialized_length);

/**
 * @brief Deserialize invocation script
 * 
 * Equivalent to Swift InvocationScript.deserialize(_:)
 * 
 * @param serialized Serialized data
 * @param serialized_length Length of serialized data
 * @param invocation_script Output invocation script (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_deserialize(const uint8_t *serialized,
                                                 size_t serialized_length,
                                                 neoc_invocation_script_t **invocation_script);

/**
 * @brief Clear all data from invocation script
 * 
 * @param invocation_script The invocation script to clear
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_invocation_script_clear(neoc_invocation_script_t *invocation_script);

/**
 * @brief Free invocation script
 * 
 * @param invocation_script The invocation script to free
 */
void neoc_invocation_script_free(neoc_invocation_script_t *invocation_script);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_SCRIPT_INVOCATION_SCRIPT_H */
