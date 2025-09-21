/**
 * @file neo_c_express.h
 * @brief NeoC Express protocol implementation for C
 * 
 * Based on Swift source: protocol/NeoSwiftExpress.swift
 * Provides Neo-express specific functionality for development and testing
 */

#ifndef NEOC_PROTOCOL_NEO_C_EXPRESS_H
#define NEOC_PROTOCOL_NEO_C_EXPRESS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/neo_c.h"
#include "neoc/protocol/neo_c_service.h"
#include "neoc/protocol/contract_response_types.h"
#include "neoc/types/hash160.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations for express response types
 */
typedef struct neoc_populated_blocks_t neoc_populated_blocks_t;
typedef struct neoc_nep17_contract_t neoc_nep17_contract_t;
typedef struct neoc_express_contract_state_t neoc_express_contract_state_t;
typedef struct neoc_oracle_request_t neoc_oracle_request_t;
typedef struct neoc_transaction_attribute_t neoc_transaction_attribute_t;
typedef struct neoc_express_shutdown_t neoc_express_shutdown_t;

/**
 * @brief NeoSwiftExpress structure
 * 
 * Based on Swift NeoSwiftExpress class
 * Extends NeoSwift with Neo-express specific functionality
 */
typedef struct {
    neoc_neo_c_t *base;                 /**< Base NeoC client */
} neoc_neo_c_express_t;

/**
 * @brief Create a NeoSwiftExpress client
 * @param base_client Base NeoSwift client (takes ownership)
 * @return Newly allocated express client or NULL on failure
 */
neoc_neo_c_express_t *neoc_neo_c_express_create(neoc_neo_c_t *base_client);

/**
 * @brief Build a NeoSwiftExpress client with service
 * @param service Neo service implementation
 * @return Newly allocated express client or NULL on failure
 */
neoc_neo_c_express_t *neoc_neo_c_express_build(neoc_service_t *service);

/**
 * @brief Build a NeoSwiftExpress client with service and config
 * @param service Neo service implementation
 * @param config Custom configuration
 * @return Newly allocated express client or NULL on failure
 */
neoc_neo_c_express_t *neoc_neo_c_express_build_with_config(neoc_service_t *service, 
                                                                   neoc_neo_c_config_t *config);

/**
 * @brief Get the base NeoSwift client
 * @param express Express client
 * @return Base client (do not free)
 */
neoc_neo_c_t *neoc_neo_c_express_get_base(neoc_neo_c_express_t *express);

// Neo-express specific JSON-RPC methods

/**
 * @brief Get populated blocks from neo-express
 * @param express Express client
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_get_populated_blocks(neoc_neo_c_express_t *express,
                                                         neoc_request_t **request_out);

/**
 * @brief Get all deployed NEP-17 contracts
 * @param express Express client
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_get_nep17_contracts(neoc_neo_c_express_t *express,
                                                       neoc_request_t **request_out);

/**
 * @brief Get contract storage entries
 * @param express Express client
 * @param contract_hash Contract hash
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_get_contract_storage(neoc_neo_c_express_t *express,
                                                        const neoc_hash160_t *contract_hash,
                                                        neoc_request_t **request_out);

/**
 * @brief List all deployed contracts
 * @param express Express client
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_list_contracts(neoc_neo_c_express_t *express,
                                                   neoc_request_t **request_out);

/**
 * @brief Create a checkpoint file
 * @param express Express client
 * @param filename Checkpoint filename
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_create_checkpoint(neoc_neo_c_express_t *express,
                                                     const char *filename,
                                                     neoc_request_t **request_out);

/**
 * @brief List all current oracle requests
 * @param express Express client
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_list_oracle_requests(neoc_neo_c_express_t *express,
                                                         neoc_request_t **request_out);

/**
 * @brief Create an oracle response transaction
 * @param express Express client
 * @param oracle_response Oracle response attribute
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_create_oracle_response_tx(neoc_neo_c_express_t *express,
                                                             const neoc_transaction_attribute_t *oracle_response,
                                                             neoc_request_t **request_out);

/**
 * @brief Shutdown the neo-express instance
 * @param express Express client
 * @param request_out Output request (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_express_shutdown(neoc_neo_c_express_t *express,
                                             neoc_request_t **request_out);

// Async versions of express methods

/**
 * @brief Get populated blocks asynchronously
 * @param express Express client
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_express_get_populated_blocks_async(neoc_neo_c_express_t *express,
                                                              void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                              void *user_data);

/**
 * @brief Get NEP-17 contracts asynchronously
 * @param express Express client
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_express_get_nep17_contracts_async(neoc_neo_c_express_t *express,
                                                             void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                             void *user_data);

/**
 * @brief Get contract storage asynchronously
 * @param express Express client
 * @param contract_hash Contract hash
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_express_get_contract_storage_async(neoc_neo_c_express_t *express,
                                                              const neoc_hash160_t *contract_hash,
                                                              void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                              void *user_data);

/**
 * @brief List contracts asynchronously
 * @param express Express client
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_express_list_contracts_async(neoc_neo_c_express_t *express,
                                                        void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                        void *user_data);

/**
 * @brief Create checkpoint asynchronously
 * @param express Express client
 * @param filename Checkpoint filename
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_express_create_checkpoint_async(neoc_neo_c_express_t *express,
                                                           const char *filename,
                                                           void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                           void *user_data);

/**
 * @brief Shutdown neo-express asynchronously
 * @param express Express client
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_express_shutdown_async(neoc_neo_c_express_t *express,
                                                   void (*callback)(neoc_response_t *response, neoc_error_t error, void *user_data),
                                                   void *user_data);

/**
 * @brief Free NeoSwiftExpress client
 * @param express Express client to free
 */
void neoc_neo_c_express_free(neoc_neo_c_express_t *express);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_NEO_C_EXPRESS_H */
