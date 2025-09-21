/**
 * @file neo_c.h
 * @brief Main NeoC client interface for C
 * 
 * Based on Swift source: protocol/NeoSwift.swift
 * Provides the primary Neo blockchain client interface
 */

#ifndef NEOC_PROTOCOL_NEO_C_H
#define NEOC_PROTOCOL_NEO_C_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/neo_c_config.h"
#include "neoc/protocol/service.h"
#include "neoc/protocol/core/response.h"
#include "neoc/types/hash160.h"
#include "neoc/utils/array.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations
 */
typedef struct neoc_neo_c_t neoc_neo_c_t;
typedef struct neoc_neo_c_rx_t neoc_neo_c_rx_t;

/**
 * @brief Main NeoC client structure
 * 
 * Based on Swift NeoSwift class
 * Combines Neo protocol functionality with reactive extensions
 */
typedef struct neoc_neo_c_t {
    neoc_neo_c_config_t *config;    /**< Client configuration */
    neoc_service_t *neo_c_service;  /**< Underlying service implementation */
    neoc_neo_c_rx_t *neo_c_rx;  /**< Reactive extensions (lazy initialized) */
} neoc_neo_c_t;

/**
 * @brief Callback function for asynchronous operations
 * @param result Operation result (NULL on error)
 * @param error Error information (NULL on success)
 * @param user_data User-provided data
 */
typedef void (*neoc_neo_c_callback_t)(void *result, neoc_error_t error, void *user_data);

/**
 * @brief Create a new NeoSwift client
 * @param config Client configuration (takes ownership)
 * @param service Neo service implementation (takes ownership)
 * @return Newly allocated client or NULL on failure
 */
neoc_neo_c_t *neoc_neo_c_create(neoc_neo_c_config_t *config, neoc_service_t *service);

/**
 * @brief Build a NeoSwift client with default configuration
 * @param service Neo service implementation
 * @return Newly allocated client or NULL on failure
 */
neoc_neo_c_t *neoc_neo_c_build(neoc_service_t *service);

/**
 * @brief Build a NeoC client with custom configuration
 * @param service Neo service implementation
 * @param config Custom configuration
 * @return Newly allocated client or NULL on failure
 */
neoc_neo_c_t *neoc_neo_c_build_with_config(neoc_service_t *service, neoc_neo_c_config_t *config);

/**
 * @brief Get the NeoNameService resolver script hash
 * @param neo_c NeoC client
 * @return NNS resolver hash (do not free)
 */
const neoc_hash160_t *neoc_neo_c_get_nns_resolver(const neoc_neo_c_t *neo_c);

/**
 * @brief Get the block interval in milliseconds
 * @param neo_c NeoC client
 * @return Block interval
 */
int neoc_neo_c_get_block_interval(const neoc_neo_c_t *neo_c);

/**
 * @brief Get the polling interval in milliseconds
 * @param neo_c NeoC client
 * @return Polling interval
 */
int neoc_neo_c_get_polling_interval(const neoc_neo_c_t *neo_c);

/**
 * @brief Get the maximum valid until block increment
 * @param neo_c NeoC client
 * @return Maximum valid until block increment
 */
int neoc_neo_c_get_max_valid_until_block_increment(const neoc_neo_c_t *neo_c);

/**
 * @brief Get the network magic number (fetches from node if not set)
 * @param neo_c NeoC client
 * @param magic_out Output for magic number
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_get_network_magic_number(neoc_neo_c_t *neo_c, int *magic_out);

/**
 * @brief Get the network magic number as bytes
 * @param neo_c NeoC client
 * @param bytes_out Output for magic bytes (4 bytes)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_get_network_magic_number_bytes(neoc_neo_c_t *neo_c, uint8_t bytes_out[4]);

/**
 * @brief Send a generic request (internal implementation)
 * @param neo_c NeoC client
 * @param request_data Serialized request data
 * @param response_out Output for response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_send_request(neoc_neo_c_t *neo_c,
                                    const neoc_byte_array_t *request_data,
                                    neoc_response_t **response_out);

/**
 * @brief Send a generic request asynchronously
 * @param neo_c NeoC client
 * @param request_data Serialized request data
 * @param callback Completion callback
 * @param user_data User data for callback
 * @return NEOC_SUCCESS if request started, error code on immediate failure
 */
neoc_error_t neoc_neo_c_send_request_async(neoc_neo_c_t *neo_c,
                                          const neoc_byte_array_t *request_data,
                                          neoc_neo_c_callback_t callback,
                                          void *user_data);

/**
 * @brief Get reactive extensions interface
 * @param neo_c NeoC client
 * @return Reactive extensions interface (do not free)
 */
neoc_neo_c_rx_t *neoc_neo_c_get_rx(neoc_neo_c_t *neo_c);

/**
 * @brief Get client configuration
 * @param neo_c NeoC client
 * @return Configuration (do not free)
 */
neoc_neo_c_config_t *neoc_neo_c_get_config(neoc_neo_c_t *neo_c);

/**
 * @brief Get underlying service
 * @param neo_c NeoC client
 * @return Service implementation (do not free)
 */
neoc_service_t *neoc_neo_c_get_service(neoc_neo_c_t *neo_c);

/**
 * @brief Check if client allows transmission on fault
 * @param neo_c NeoC client
 * @return true if allows transmission on fault
 */
bool neoc_neo_c_allows_transmission_on_fault(const neoc_neo_c_t *neo_c);

/**
 * @brief Free NeoC client
 * @param neo_c Client to free
 */
void neoc_neo_c_free(neoc_neo_c_t *neo_c);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_NEO_C_H */
