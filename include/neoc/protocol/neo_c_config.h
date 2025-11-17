/**
 * @file neo_c_config.h
 * @brief NeoC configuration management for C
 * 
 * Based on Swift source: protocol/NeoSwiftConfig.swift
 * Provides configuration options for Neo blockchain connections
 */

#ifndef NEOC_PROTOCOL_NEO_C_CONFIG_H
#define NEOC_PROTOCOL_NEO_C_CONFIG_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash160.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NeoSwift configuration constants
 */
#define NEOC_CONFIG_DEFAULT_BLOCK_TIME 15000
#define NEOC_CONFIG_DEFAULT_ADDRESS_VERSION 0x35
#define NEOC_CONFIG_MAX_VALID_UNTIL_BLOCK_INCREMENT_BASE 86400000
#define NEOC_CONFIG_MAINNET_NNS_CONTRACT_HASH "0x50ac1c37690cc2cfc594472833cf57505d5f46de"
#define NEOC_CONFIG_DEFAULT_NETWORK_MAGIC 0x334F454E

/**
 * @brief Thread-safe counter for request IDs
 * 
 * Based on Swift Counter class
 */
typedef struct {
    int value;                          /**< Current counter value */
    void *mutex;                        /**< Thread synchronization mutex */
} neoc_counter_t;

/**
 * @brief NeoC configuration structure
 * 
 * Based on Swift NeoSwiftConfig class
 * Contains all configuration options for Neo blockchain interaction
 */
typedef struct {
    int *network_magic;                 /**< Network magic number (nullable) */
    int block_interval;                 /**< Block interval in milliseconds */
    int max_valid_until_block_increment; /**< Max valid until block increment */
    int polling_interval;               /**< Polling interval in milliseconds */
    bool allows_transmission_on_fault;  /**< Allow fault transmission */
    neoc_hash160_t *nns_resolver;       /**< NeoNameService resolver hash */
    void *scheduled_executor;           /**< Executor for scheduled tasks */
} neoc_neo_c_config_t;

/**
 * @brief Global configuration state
 */
typedef struct {
    uint8_t address_version;            /**< Global address version */
    neoc_counter_t *request_counter;    /**< Global request counter */
} neoc_global_config_t;

/**
 * @brief Create a new NeoSwift configuration with default values
 * @return Newly allocated configuration or NULL on failure
 */
neoc_neo_c_config_t *neoc_neo_c_config_create(void);

/**
 * @brief Create a new NeoSwift configuration with custom parameters
 * @param network_magic Network magic number (nullable)
 * @param block_interval Block interval in milliseconds
 * @param max_valid_until_block_increment Max valid until block increment
 * @param polling_interval Polling interval in milliseconds
 * @param allows_transmission_on_fault Allow fault transmission
 * @param nns_resolver NeoNameService resolver hash
 * @return Newly allocated configuration or NULL on failure
 */
neoc_neo_c_config_t *neoc_neo_c_config_create_custom(
    const int *network_magic,
    int block_interval,
    int max_valid_until_block_increment,
    int polling_interval,
    bool allows_transmission_on_fault,
    const neoc_hash160_t *nns_resolver
);

/**
 * @brief Set the polling interval
 * @param config Configuration to modify
 * @param polling_interval Polling interval in milliseconds
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_set_polling_interval(neoc_neo_c_config_t *config, int polling_interval);

/**
 * @brief Set the network magic number
 * @param config Configuration to modify
 * @param magic Network magic number
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_set_network_magic(neoc_neo_c_config_t *config, int magic);

/**
 * @brief Set the block interval
 * @param config Configuration to modify
 * @param block_interval Block interval in milliseconds
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_set_block_interval(neoc_neo_c_config_t *config, int block_interval);

/**
 * @brief Set the maximum valid until block increment
 * @param config Configuration to modify
 * @param max_increment Maximum valid until block increment
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_set_max_valid_until_block_increment(neoc_neo_c_config_t *config, int max_increment);

/**
 * @brief Set the NeoNameService resolver
 * @param config Configuration to modify
 * @param nns_resolver NeoNameService resolver hash
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_set_nns_resolver(neoc_neo_c_config_t *config, const neoc_hash160_t *nns_resolver);

/**
 * @brief Allow transmission on fault
 * @param config Configuration to modify
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_allow_transmission_on_fault(neoc_neo_c_config_t *config);

/**
 * @brief Prevent transmission on fault
 * @param config Configuration to modify
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_c_config_prevent_transmission_on_fault(neoc_neo_c_config_t *config);

/**
 * @brief Get the global address version
 * @return Current address version
 */
uint8_t neoc_neo_c_config_get_address_version(void);

/**
 * @brief Set the global address version
 * @param address_version Address version to set
 */
void neoc_neo_c_config_set_address_version(uint8_t address_version);

/**
 * @brief Create a new counter
 * @return Newly allocated counter or NULL on failure
 */
neoc_counter_t *neoc_counter_create(void);

/**
 * @brief Get current counter value and increment
 * @param counter Counter instance
 * @return Current value before increment
 */
int neoc_counter_get_and_increment(neoc_counter_t *counter);

/**
 * @brief Set counter value
 * @param counter Counter instance
 * @param value Value to set
 */
void neoc_counter_set(neoc_counter_t *counter, int value);

/**
 * @brief Reset counter to 1
 * @param counter Counter instance
 */
void neoc_counter_reset(neoc_counter_t *counter);

/**
 * @brief Free counter
 * @param counter Counter to free
 */
void neoc_counter_free(neoc_counter_t *counter);

/**
 * @brief Free NeoSwift configuration
 * @param config Configuration to free
 */
void neoc_neo_c_config_free(neoc_neo_c_config_t *config);

/**
 * @brief Initialize global configuration state
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_global_config_init(void);

/**
 * @brief Cleanup global configuration state
 */
void neoc_global_config_cleanup(void);

/**
 * @brief Get global request counter
 * @return Global request counter instance
 */
neoc_counter_t *neoc_global_config_get_request_counter(void);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_NEO_C_CONFIG_H */
