/**
 * @file script_builder_full.h
 * @brief Complete script builder for Neo VM scripts
 */

#ifndef NEOC_SCRIPT_BUILDER_FULL_H
#define NEOC_SCRIPT_BUILDER_FULL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/script/opcode.h"
#include "neoc/script/interop_service.h"
#include "neoc/script/script_builder.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/contract/contract_parameter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Call flags for contract calls
 */
typedef enum {
    NEOC_CALL_FLAGS_NONE = 0x00,
    NEOC_CALL_FLAGS_READ_STATES = 0x01,
    NEOC_CALL_FLAGS_WRITE_STATES = 0x02,
    NEOC_CALL_FLAGS_ALLOW_CALL = 0x04,
    NEOC_CALL_FLAGS_ALLOW_NOTIFY = 0x08,
    NEOC_CALL_FLAGS_STATES = 0x03,      // READ_STATES | WRITE_STATES
    NEOC_CALL_FLAGS_READ_ONLY = 0x09,   // READ_STATES | ALLOW_NOTIFY
    NEOC_CALL_FLAGS_ALL = 0x0F          // All flags
} neoc_call_flags_t;

/**
 * @brief Create a new script builder
 * 
 * @param builder Output script builder (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_create(neoc_script_builder_t **builder);

/**
 * @brief Emit an opcode
 * 
 * @param builder The script builder
 * @param opcode The opcode to emit
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_emit(neoc_script_builder_t *builder, neoc_opcode_t opcode);

/**
 * @brief Emit an opcode with data
 * 
 * @param builder The script builder
 * @param opcode The opcode to emit
 * @param data The data to emit
 * @param data_len Length of data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_emit_with_data(neoc_script_builder_t *builder,
                                                 neoc_opcode_t opcode,
                                                 const uint8_t *data,
                                                 size_t data_len);

/**
 * @brief Emit a syscall
 * 
 * @param builder The script builder
 * @param service The interop service to call
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_emit_syscall(neoc_script_builder_t *builder,
                                               neoc_interop_service_t service);

/**
 * @brief Push integer onto the stack
 * 
 * @param builder The script builder
 * @param value The integer value
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_push_integer(neoc_script_builder_t *builder, int64_t value);

/**
 * @brief Push boolean onto the stack
 * 
 * @param builder The script builder
 * @param value The boolean value
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_push_bool(neoc_script_builder_t *builder, bool value);

/**
 * @brief Push data onto the stack
 * 
 * @param builder The script builder
 * @param data The data to push
 * @param data_len Length of data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_push_data(neoc_script_builder_t *builder,
                                            const uint8_t *data,
                                            size_t data_len);

/**
 * @brief Push string onto the stack
 * 
 * @param builder The script builder
 * @param str The string to push (UTF-8)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_push_string(neoc_script_builder_t *builder, const char *str);

/**
 * @brief Push contract parameter onto the stack
 * 
 * @param builder The script builder
 * @param param The contract parameter
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_push_param(neoc_script_builder_t *builder,
                                            const neoc_contract_parameter_t *param);

/**
 * @brief Push array of contract parameters onto the stack
 * 
 * @param builder The script builder
 * @param params Array of parameters
 * @param count Number of parameters
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_push_params(neoc_script_builder_t *builder,
                                             const neoc_contract_parameter_t **params,
                                             size_t count);

/**
 * @brief Emit a contract call
 * 
 * @param builder The script builder
 * @param script_hash The contract script hash
 * @param method The method name
 * @param params Array of parameters (can be NULL)
 * @param param_count Number of parameters
 * @param call_flags The call flags
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_contract_call(neoc_script_builder_t *builder,
                                               const neoc_hash160_t *script_hash,
                                               const char *method,
                                               const neoc_contract_parameter_t **params,
                                               size_t param_count,
                                               neoc_call_flags_t call_flags);

/**
 * @brief Build multi-signature verification script
 * 
 * @param minimum_signatures Minimum required signatures
 * @param public_keys Array of public keys
 * @param public_key_count Number of public keys
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_build_multisig_script(int minimum_signatures,
                                                       const uint8_t **public_keys,
                                                       const size_t *public_key_lens,
                                                       size_t public_key_count,
                                                       uint8_t **script,
                                                       size_t *script_len);

/**
 * @brief Get the built script
 * 
 * @param builder The script builder
 * @param script Output script (caller must free)
 * @param script_len Output script length
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_script_builder_to_array(const neoc_script_builder_t *builder,
                                          uint8_t **script,
                                          size_t *script_len);

/**
 * @brief Get current script size
 * 
 * @param builder The script builder
 * @return Current script size in bytes
 */
size_t neoc_script_builder_get_size(const neoc_script_builder_t *builder);

/**
 * @brief Reset the script builder
 * 
 * @param builder The script builder
 */
void neoc_script_builder_reset(neoc_script_builder_t *builder);

/**
 * @brief Free a script builder
 * 
 * @param builder The script builder to free
 */
void neoc_script_builder_free(neoc_script_builder_t *builder);

#ifdef __cplusplus
}
#endif

/**
 * Push null onto the stack
 */
neoc_error_t neoc_script_builder_push_null(neoc_script_builder_t *builder);

/**
 * Push a Hash160 onto the stack
 */
neoc_error_t neoc_script_builder_push_hash160(neoc_script_builder_t *builder,
                                               const neoc_hash160_t *hash);

/**
 * Emit an application call with params
 */
neoc_error_t neoc_script_builder_emit_app_call(neoc_script_builder_t *builder,
                                                const neoc_hash160_t *script_hash,
                                                const char *method,
                                                uint8_t param_count);

/**
 * Push integer with alias
 */
neoc_error_t neoc_script_builder_emit_push_int(neoc_script_builder_t *builder, int64_t value);

#endif // NEOC_SCRIPT_BUILDER_FULL_H
