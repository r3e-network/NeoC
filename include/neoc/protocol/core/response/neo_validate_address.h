/**
 * @file neo_validate_address.h
 * @brief Address validation response
 * 
 * Based on Swift source: protocol/core/response/NeoValidateAddress.swift
 * Response structure for validateaddress RPC call
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_VALIDATE_ADDRESS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_VALIDATE_ADDRESS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Address validation result
 * 
 * Contains information about whether an address is valid and its properties
 */
typedef struct {
    char *address;                       /**< The address that was validated */
    bool valid;                          /**< Whether the address is valid */
    bool is_script_hash;                 /**< Whether address represents a script hash */
    bool is_multisig;                    /**< Whether address is multi-signature */
    bool is_standard;                    /**< Whether address is standard format */
    char *script_hash;                   /**< Script hash (if valid) */
} neoc_neo_validate_address_result_t;

/**
 * @brief Complete response for validateaddress RPC call
 * 
 * Standard JSON-RPC response structure containing validation result
 */
typedef struct {
    char *jsonrpc;                       /**< JSON-RPC version ("2.0") */
    int id;                              /**< Request ID */
    neoc_neo_validate_address_result_t *result; /**< Validation result (NULL if error) */
    char *error;                         /**< Error message (NULL if success) */
    int error_code;                      /**< Error code (0 if success) */
} neoc_neo_validate_address_response_t;

/**
 * @brief Create address validation result
 * 
 * @param address The address to validate
 * @param valid Whether the address is valid
 * @param result Output validation result (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_result_create(const char *address,
                                                      bool valid,
                                                      neoc_neo_validate_address_result_t **result);

/**
 * @brief Set extended validation properties
 * 
 * @param result The validation result
 * @param is_script_hash Whether address represents script hash
 * @param is_multisig Whether address is multi-signature
 * @param is_standard Whether address is standard format
 * @param script_hash Script hash string (can be NULL)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_result_set_properties(neoc_neo_validate_address_result_t *result,
                                                              bool is_script_hash,
                                                              bool is_multisig,
                                                              bool is_standard,
                                                              const char *script_hash);

/**
 * @brief Create validate address response
 * 
 * @param id Request ID
 * @param result Validation result (can be NULL for error response)
 * @param error Error message (can be NULL for success response)
 * @param error_code Error code (0 for success)
 * @param response Output response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_response_create(int id,
                                                        neoc_neo_validate_address_result_t *result,
                                                        const char *error,
                                                        int error_code,
                                                        neoc_neo_validate_address_response_t **response);

/**
 * @brief Parse validate address response from JSON
 * 
 * @param json_string JSON response string
 * @param response Output parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_response_from_json(const char *json_string,
                                                           neoc_neo_validate_address_response_t **response);

/**
 * @brief Convert validate address response to JSON
 * 
 * @param response The response to convert
 * @param json_string Output JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_response_to_json(const neoc_neo_validate_address_response_t *response,
                                                         char **json_string);

/**
 * @brief Check if response indicates success
 * 
 * @param response The response to check
 * @return True if response is successful
 */
bool neoc_neo_validate_address_response_is_success(const neoc_neo_validate_address_response_t *response);

/**
 * @brief Check if address is valid from response
 * 
 * @param response The response to check
 * @return True if address is valid, false if invalid or error
 */
bool neoc_neo_validate_address_response_is_valid_address(const neoc_neo_validate_address_response_t *response);

/**
 * @brief Get script hash from validation result
 * 
 * @param response The response
 * @param script_hash Output script hash string (do not free, NULL if not available)
 * @return NEOC_SUCCESS if available, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_response_get_script_hash(const neoc_neo_validate_address_response_t *response,
                                                                 const char **script_hash);

/**
 * @brief Check if address is multi-signature from response
 * 
 * @param response The response to check
 * @return True if address is multi-signature, false otherwise
 */
bool neoc_neo_validate_address_response_is_multisig(const neoc_neo_validate_address_response_t *response);

/**
 * @brief Check if address is standard format from response
 * 
 * @param response The response to check
 * @return True if address is standard format, false otherwise
 */
bool neoc_neo_validate_address_response_is_standard(const neoc_neo_validate_address_response_t *response);

/**
 * @brief Copy validation result
 * 
 * @param source Source validation result
 * @param copy Output copied result (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_neo_validate_address_result_copy(const neoc_neo_validate_address_result_t *source,
                                                    neoc_neo_validate_address_result_t **copy);

/**
 * @brief Free validation result
 * 
 * @param result The validation result to free
 */
void neoc_neo_validate_address_result_free(neoc_neo_validate_address_result_t *result);

/**
 * @brief Free validate address response
 * 
 * @param response The response to free
 */
void neoc_neo_validate_address_response_free(neoc_neo_validate_address_response_t *response);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_VALIDATE_ADDRESS_H */
