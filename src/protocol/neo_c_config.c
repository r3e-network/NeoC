/**
 * @file neo_c_config.c
 * @brief NeoC configuration management implementation
 * 
 * Based on Swift source: protocol/NeoSwiftConfig.swift
 * Provides configuration options for Neo blockchain connections
 */

#include "neoc/neoc.h"
#include "neoc/protocol/neo_c_config.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include "neoc/types/hash160.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * @brief Global configuration state
 */
static neoc_global_config_t g_global_config = {
    .address_version = NEOC_CONFIG_DEFAULT_ADDRESS_VERSION,
    .request_counter = NULL
};

static pthread_mutex_t g_config_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_config_initialized = false;

/**
 * @brief Counter implementation using pthread mutex
 */
struct neoc_counter_internal {
    int value;
    pthread_mutex_t mutex;
};

/**
 * @brief Create a new counter
 */
neoc_counter_t *neoc_counter_create(void) {
    struct neoc_counter_internal *counter = 
        (struct neoc_counter_internal *)neoc_malloc(sizeof(struct neoc_counter_internal));
    if (!counter) {
        return NULL;
    }
    
    counter->value = 1;  // Start at 1 like Swift implementation
    if (pthread_mutex_init(&counter->mutex, NULL) != 0) {
        neoc_free(counter);
        return NULL;
    }
    
    return (neoc_counter_t *)counter;
}

/**
 * @brief Get current counter value and increment
 */
int neoc_counter_get_and_increment(neoc_counter_t *counter) {
    if (!counter) {
        return 0;
    }
    
    struct neoc_counter_internal *internal = (struct neoc_counter_internal *)counter;
    
    pthread_mutex_lock(&internal->mutex);
    int value = internal->value;
    internal->value++;
    pthread_mutex_unlock(&internal->mutex);
    
    return value;
}

/**
 * @brief Set counter value
 */
void neoc_counter_set(neoc_counter_t *counter, int value) {
    if (!counter) {
        return;
    }
    
    struct neoc_counter_internal *internal = (struct neoc_counter_internal *)counter;
    
    pthread_mutex_lock(&internal->mutex);
    internal->value = value;
    pthread_mutex_unlock(&internal->mutex);
}

/**
 * @brief Reset counter to 1
 */
void neoc_counter_reset(neoc_counter_t *counter) {
    neoc_counter_set(counter, 1);
}

/**
 * @brief Free counter
 */
void neoc_counter_free(neoc_counter_t *counter) {
    if (!counter) {
        return;
    }
    
    struct neoc_counter_internal *internal = (struct neoc_counter_internal *)counter;
    pthread_mutex_destroy(&internal->mutex);
    neoc_free(counter);
}

/**
 * @brief Initialize global configuration state
 */
neoc_error_t neoc_global_config_init(void) {
    pthread_mutex_lock(&g_config_mutex);
    
    if (g_config_initialized) {
        pthread_mutex_unlock(&g_config_mutex);
        return NEOC_SUCCESS;
    }
    
    // Create global request counter
    g_global_config.request_counter = neoc_counter_create();
    if (!g_global_config.request_counter) {
        pthread_mutex_unlock(&g_config_mutex);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    g_global_config.address_version = NEOC_CONFIG_DEFAULT_ADDRESS_VERSION;
    g_config_initialized = true;
    
    pthread_mutex_unlock(&g_config_mutex);
    return NEOC_SUCCESS;
}

/**
 * @brief Cleanup global configuration state
 */
void neoc_global_config_cleanup(void) {
    pthread_mutex_lock(&g_config_mutex);
    
    if (g_global_config.request_counter) {
        neoc_counter_free(g_global_config.request_counter);
        g_global_config.request_counter = NULL;
    }
    
    g_config_initialized = false;
    pthread_mutex_unlock(&g_config_mutex);
}

/**
 * @brief Get global request counter
 */
neoc_counter_t *neoc_global_config_get_request_counter(void) {
    pthread_mutex_lock(&g_config_mutex);
    neoc_counter_t *counter = g_global_config.request_counter;
    pthread_mutex_unlock(&g_config_mutex);
    return counter;
}

/**
 * @brief Get the global address version
 */
uint8_t neoc_neo_c_config_get_address_version(void) {
    pthread_mutex_lock(&g_config_mutex);
    uint8_t version = g_global_config.address_version;
    pthread_mutex_unlock(&g_config_mutex);
    return version;
}

/**
 * @brief Set the global address version
 */
void neoc_neo_c_config_set_address_version(uint8_t address_version) {
    pthread_mutex_lock(&g_config_mutex);
    g_global_config.address_version = address_version;
    pthread_mutex_unlock(&g_config_mutex);
}

/**
 * @brief Create a new NeoC configuration with default values
 */
neoc_neo_c_config_t *neoc_neo_c_config_create(void) {
    neoc_neo_c_config_t *config = (neoc_neo_c_config_t *)neoc_malloc(sizeof(neoc_neo_c_config_t));
    if (!config) {
        return NULL;
    }
    
    // Initialize with default values
    config->network_magic = NULL;
    config->block_interval = NEOC_CONFIG_DEFAULT_BLOCK_TIME;
    config->max_valid_until_block_increment = 
        NEOC_CONFIG_MAX_VALID_UNTIL_BLOCK_INCREMENT_BASE / NEOC_CONFIG_DEFAULT_BLOCK_TIME;
    config->polling_interval = NEOC_CONFIG_DEFAULT_BLOCK_TIME;
    config->allows_transmission_on_fault = false;
    config->scheduled_executor = NULL;
    
    // Create default NNS resolver hash from hex string
    config->nns_resolver = neoc_hash160_from_string(NEOC_CONFIG_MAINNET_NNS_CONTRACT_HASH);
    if (!config->nns_resolver) {
        neoc_free(config);
        return NULL;
    }
    
    // Ensure global config is initialized
    neoc_global_config_init();
    
    return config;
}

/**
 * @brief Create a new NeoC configuration with custom parameters
 */
neoc_neo_c_config_t *neoc_neo_c_config_create_custom(
    const int *network_magic,
    int block_interval,
    int max_valid_until_block_increment,
    int polling_interval,
    bool allows_transmission_on_fault,
    const neoc_hash160_t *nns_resolver) {
    
    neoc_neo_c_config_t *config = (neoc_neo_c_config_t *)neoc_malloc(sizeof(neoc_neo_c_config_t));
    if (!config) {
        return NULL;
    }
    
    // Copy network magic if provided
    if (network_magic) {
        config->network_magic = (int *)neoc_malloc(sizeof(int));
        if (!config->network_magic) {
            neoc_free(config);
            return NULL;
        }
        *config->network_magic = *network_magic;
    } else {
        config->network_magic = NULL;
    }
    
    config->block_interval = block_interval;
    config->max_valid_until_block_increment = max_valid_until_block_increment;
    config->polling_interval = polling_interval;
    config->allows_transmission_on_fault = allows_transmission_on_fault;
    config->scheduled_executor = NULL;
    
    // Copy NNS resolver
    if (nns_resolver) {
        config->nns_resolver = neoc_hash160_copy(nns_resolver);
    } else {
        config->nns_resolver = neoc_hash160_from_string(NEOC_CONFIG_MAINNET_NNS_CONTRACT_HASH);
    }
    
    if (!config->nns_resolver) {
        if (config->network_magic) {
            neoc_free(config->network_magic);
        }
        neoc_free(config);
        return NULL;
    }
    
    // Ensure global config is initialized
    neoc_global_config_init();
    
    return config;
}

/**
 * @brief Set the polling interval
 */
neoc_error_t neoc_neo_c_config_set_polling_interval(neoc_neo_c_config_t *config, int polling_interval) {
    if (!config) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (polling_interval <= 0) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    config->polling_interval = polling_interval;
    return NEOC_SUCCESS;
}

/**
 * @brief Set the network magic number
 */
neoc_error_t neoc_neo_c_config_set_network_magic(neoc_neo_c_config_t *config, int magic) {
    if (!config) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Check that magic fits in 32-bit unsigned integer
    if (magic < 0 || magic > 0xFFFFFFFF) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (!config->network_magic) {
        config->network_magic = (int *)neoc_malloc(sizeof(int));
        if (!config->network_magic) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }
    
    *config->network_magic = magic;
    return NEOC_SUCCESS;
}

/**
 * @brief Set the block interval
 */
neoc_error_t neoc_neo_c_config_set_block_interval(neoc_neo_c_config_t *config, int block_interval) {
    if (!config) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (block_interval <= 0) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    config->block_interval = block_interval;
    return NEOC_SUCCESS;
}

/**
 * @brief Set the maximum valid until block increment
 */
neoc_error_t neoc_neo_c_config_set_max_valid_until_block_increment(neoc_neo_c_config_t *config, int max_increment) {
    if (!config) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (max_increment <= 0) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    config->max_valid_until_block_increment = max_increment;
    return NEOC_SUCCESS;
}

/**
 * @brief Set the NeoNameService resolver
 */
neoc_error_t neoc_neo_c_config_set_nns_resolver(neoc_neo_c_config_t *config, const neoc_hash160_t *nns_resolver) {
    if (!config || !nns_resolver) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Free old resolver if exists
    if (config->nns_resolver) {
        neoc_hash160_free(config->nns_resolver);
    }
    
    // Copy new resolver
    config->nns_resolver = neoc_hash160_copy(nns_resolver);
    if (!config->nns_resolver) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Allow transmission on fault
 */
neoc_error_t neoc_neo_c_config_allow_transmission_on_fault(neoc_neo_c_config_t *config) {
    if (!config) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    config->allows_transmission_on_fault = true;
    return NEOC_SUCCESS;
}

/**
 * @brief Prevent transmission on fault
 */
neoc_error_t neoc_neo_c_config_prevent_transmission_on_fault(neoc_neo_c_config_t *config) {
    if (!config) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    config->allows_transmission_on_fault = false;
    return NEOC_SUCCESS;
}

/**
 * @brief Free NeoC configuration
 */
void neoc_neo_c_config_free(neoc_neo_c_config_t *config) {
    if (!config) {
        return;
    }
    
    if (config->network_magic) {
        neoc_free(config->network_magic);
    }
    
    if (config->nns_resolver) {
        neoc_hash160_free(config->nns_resolver);
    }
    
    neoc_free(config);
}

