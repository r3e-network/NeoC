/**
 * @file neo_c.c
 * @brief Main NeoC client implementation
 * 
 * Based on Swift source: protocol/NeoSwift.swift
 * Provides the primary Neo blockchain client interface
 */

#include "neoc/neoc.h"
#include "neoc/protocol/neo_c.h"
#include "neoc/protocol/neo_c_config.h"
#include "neoc/protocol/service.h"
#include "neoc/protocol/rx/json_rpc2_0_rx.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * @brief Internal structure for reactive extensions
 */
typedef struct neoc_neo_c_rx_t {
    neoc_json_rpc2_0_rx_t *json_rpc_rx;  /**< JSON-RPC reactive extensions */
    neoc_neo_c_t *neo_c;                 /**< Parent NeoC instance */
    pthread_mutex_t mutex;                /**< Mutex for thread safety */
} neoc_neo_c_rx_t;

/**
 * @brief Create a new NeoC client
 */
neoc_neo_c_t *neoc_neo_c_create(neoc_neo_c_config_t *config, neoc_service_t *service) {
    if (!config || !service) {
        return NULL;
    }
    
    neoc_neo_c_t *neo_c = (neoc_neo_c_t *)neoc_malloc(sizeof(neoc_neo_c_t));
    if (!neo_c) {
        return NULL;
    }
    
    neo_c->config = config;
    neo_c->neo_c_service = service;
    neo_c->neo_c_rx = NULL;  // Lazy initialization
    
    return neo_c;
}

/**
 * @brief Build a NeoC client with default configuration
 */
neoc_neo_c_t *neoc_neo_c_build(neoc_service_t *service) {
    if (!service) {
        return NULL;
    }
    
    neoc_neo_c_config_t *config = neoc_neo_c_config_create();
    if (!config) {
        return NULL;
    }
    
    return neoc_neo_c_create(config, service);
}

/**
 * @brief Build a NeoC client with custom configuration
 */
neoc_neo_c_t *neoc_neo_c_build_with_config(neoc_service_t *service, neoc_neo_c_config_t *config) {
    if (!service || !config) {
        return NULL;
    }
    
    return neoc_neo_c_create(config, service);
}

/**
 * @brief Get the NeoNameService resolver script hash
 */
const neoc_hash160_t *neoc_neo_c_get_nns_resolver(const neoc_neo_c_t *neo_c) {
    if (!neo_c || !neo_c->config) {
        return NULL;
    }
    
    return neo_c->config->nns_resolver;
}

/**
 * @brief Get the block interval in milliseconds
 */
int neoc_neo_c_get_block_interval(const neoc_neo_c_t *neo_c) {
    if (!neo_c || !neo_c->config) {
        return NEOC_CONFIG_DEFAULT_BLOCK_TIME;
    }
    
    return neo_c->config->block_interval;
}

/**
 * @brief Get the polling interval in milliseconds
 */
int neoc_neo_c_get_polling_interval(const neoc_neo_c_t *neo_c) {
    if (!neo_c || !neo_c->config) {
        return NEOC_CONFIG_DEFAULT_BLOCK_TIME;
    }
    
    return neo_c->config->polling_interval;
}

/**
 * @brief Get the maximum valid until block increment
 */
int neoc_neo_c_get_max_valid_until_block_increment(const neoc_neo_c_t *neo_c) {
    if (!neo_c || !neo_c->config) {
        return NEOC_CONFIG_MAX_VALID_UNTIL_BLOCK_INCREMENT_BASE / NEOC_CONFIG_DEFAULT_BLOCK_TIME;
    }
    
    return neo_c->config->max_valid_until_block_increment;
}

/**
 * @brief Get the network magic number (fetches from node if not set)
 */
neoc_error_t neoc_neo_c_get_network_magic_number(neoc_neo_c_t *neo_c, int *magic_out) {
    if (!neo_c || !magic_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Check if already cached
    if (neo_c->config->network_magic) {
        *magic_out = *neo_c->config->network_magic;
        return NEOC_SUCCESS;
    }
    
    // Fetch network magic from node via RPC
    neoc_request_t request = {
        .id = 1,
        .jsonrpc = "2.0",
        .method = "getversion",
        .params = "[]"
    };
    
    neoc_response_t response = {0};
    neoc_error_t err = neoc_rpc_client_send_request(neo_c->http_service, &request, &response);
    
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse the network magic from response
    if (response.result && response.result->type == NEOC_STACK_ITEM_MAP) {
        // Extract network field from version response
        neoc_stack_item_t *network = neoc_stack_map_get(response.result, "network");
        if (network && network->type == NEOC_STACK_ITEM_INTEGER) {
            int magic = (int)network->value.integer;
            
            // Cache the value
            if (!neo_c->config->network_magic) {
                neo_c->config->network_magic = neoc_malloc(sizeof(int));
            }
            if (neo_c->config->network_magic) {
                *neo_c->config->network_magic = magic;
            }
            
            *magic_out = magic;
            neoc_response_free(&response);
            return NEOC_SUCCESS;
        }
    }
    
    neoc_response_free(&response);
    return NEOC_ERROR_INVALID_RESPONSE;
}

/**
 * @brief Get the network magic number as bytes
 */
neoc_error_t neoc_neo_c_get_network_magic_number_bytes(neoc_neo_c_t *neo_c, uint8_t bytes_out[4]) {
    if (!neo_c || !bytes_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    int magic;
    neoc_error_t error = neoc_neo_c_get_network_magic_number(neo_c, &magic);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    // Convert to big-endian bytes
    uint32_t magic_uint = (uint32_t)(magic & 0xFFFFFFFF);
    bytes_out[0] = (uint8_t)((magic_uint >> 24) & 0xFF);
    bytes_out[1] = (uint8_t)((magic_uint >> 16) & 0xFF);
    bytes_out[2] = (uint8_t)((magic_uint >> 8) & 0xFF);
    bytes_out[3] = (uint8_t)(magic_uint & 0xFF);
    
    return NEOC_SUCCESS;
}

/**
 * @brief Send a generic request (internal implementation)
 */
neoc_error_t neoc_neo_c_send_request(neoc_neo_c_t *neo_c,
                                    const neoc_byte_array_t *request_data,
                                    neoc_response_t **response_out) {
    if (!neo_c || !request_data || !response_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Use the underlying service to send the request
    return neoc_service_send(neo_c->neo_c_service, request_data, response_out);
}

/**
 * @brief Send a generic request asynchronously
 */
neoc_error_t neoc_neo_c_send_request_async(neoc_neo_c_t *neo_c,
                                          const neoc_byte_array_t *request_data,
                                          neoc_neo_c_callback_t callback,
                                          void *user_data) {
    if (!neo_c || !request_data || !callback) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Use the underlying service to send the request asynchronously
    return neoc_service_send_async(neo_c->neo_c_service, request_data,
                                  (neoc_service_callback_t)callback, user_data);
}

/**
 * @brief Get reactive extensions interface (lazy initialization)
 */
neoc_neo_c_rx_t *neoc_neo_c_get_rx(neoc_neo_c_t *neo_c) {
    if (!neo_c) {
        return NULL;
    }
    
    // Lazy initialization of reactive extensions
    if (!neo_c->neo_c_rx) {
        neoc_neo_c_rx_t *rx = (neoc_neo_c_rx_t *)neoc_malloc(sizeof(neoc_neo_c_rx_t));
        if (rx) {
            rx->neo_c = neo_c;
            rx->json_rpc_rx = neoc_json_rpc2_0_rx_create(neo_c, neo_c->config->scheduled_executor);
            pthread_mutex_init(&rx->mutex, NULL);
            neo_c->neo_c_rx = rx;
        }
    }
    
    return neo_c->neo_c_rx;
}

/**
 * @brief Get client configuration
 */
neoc_neo_c_config_t *neoc_neo_c_get_config(neoc_neo_c_t *neo_c) {
    return neo_c ? neo_c->config : NULL;
}

/**
 * @brief Get underlying service
 */
neoc_service_t *neoc_neo_c_get_service(neoc_neo_c_t *neo_c) {
    return neo_c ? neo_c->neo_c_service : NULL;
}

/**
 * @brief Check if client allows transmission on fault
 */
bool neoc_neo_c_allows_transmission_on_fault(const neoc_neo_c_t *neo_c) {
    if (!neo_c || !neo_c->config) {
        return false;
    }
    
    return neo_c->config->allows_transmission_on_fault;
}

/**
 * @brief Free reactive extensions
 */
static void neoc_neo_c_rx_free(neoc_neo_c_rx_t *rx) {
    if (!rx) {
        return;
    }
    
    if (rx->json_rpc_rx) {
        neoc_json_rpc2_0_rx_free(rx->json_rpc_rx);
    }
    
    pthread_mutex_destroy(&rx->mutex);
    neoc_free(rx);
}

/**
 * @brief Free NeoC client
 */
void neoc_neo_c_free(neoc_neo_c_t *neo_c) {
    if (!neo_c) {
        return;
    }
    
    // Free reactive extensions if initialized
    if (neo_c->neo_c_rx) {
        neoc_neo_c_rx_free(neo_c->neo_c_rx);
    }
    
    // Free config and service (we own them)
    if (neo_c->config) {
        neoc_neo_c_config_free(neo_c->config);
    }
    
    if (neo_c->neo_c_service) {
        neoc_service_free(neo_c->neo_c_service);
    }
    
    neoc_free(neo_c);
}

