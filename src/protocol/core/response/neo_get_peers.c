/**
 * @file neo_get_peers.c
 * @brief Neo peers response implementation
 * 
 * Based on Swift source: protocol/core/response/NeoGetPeers.swift
 * Implements Neo network peer information handling and parsing
 */

#include "../../../../include/neoc/neoc_error.h"
#include "../../../../include/neoc/neoc_memory.h"
#include "../../../../include/neoc/protocol/core/response/neo_get_peers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Create a new address entry structure
 */
neoc_error_t neoc_address_entry_create(const char *address,
                                        int port,
                                        neoc_address_entry_t **entry) {
    if (!address || !entry) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *entry = NULL;
    
    neoc_address_entry_t *new_entry = neoc_malloc(sizeof(neoc_address_entry_t));
    if (!new_entry) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Copy address string
    new_entry->address = neoc_strdup(address);
    if (!new_entry->address) {
        neoc_free(new_entry);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_entry->port = port;
    
    *entry = new_entry;
    return NEOC_SUCCESS;
}

/**
 * @brief Free an address entry structure
 */
void neoc_address_entry_free(neoc_address_entry_t *entry) {
    if (!entry) {
        return;
    }
    
    if (entry->address) {
        neoc_free(entry->address);
    }
    
    neoc_free(entry);
}

/**
 * @brief Helper function to copy address entry array
 */
static neoc_error_t copy_address_entries(const neoc_address_entry_t *source,
                                          size_t count,
                                          neoc_address_entry_t **dest) {
    if (!source || !dest || count == 0) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *dest = neoc_malloc(sizeof(neoc_address_entry_t) * count);
    if (!*dest) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < count; i++) {
        neoc_address_entry_t *new_entry;
        neoc_error_t error = neoc_address_entry_create(source[i].address, source[i].port, &new_entry);
        if (error != NEOC_SUCCESS) {
            // Clean up previously allocated entries
            for (size_t j = 0; j < i; j++) {
                neoc_address_entry_free(&(*dest)[j]);
            }
            neoc_free(*dest);
            *dest = NULL;
            return error;
        }
        (*dest)[i] = *new_entry;
        neoc_free(new_entry);  // Free the wrapper, but keep the copied data
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Helper function to free address entry array
 */
static void free_address_entries(neoc_address_entry_t *entries, size_t count) {
    if (!entries) {
        return;
    }
    
    for (size_t i = 0; i < count; i++) {
        if (entries[i].address) {
            neoc_free(entries[i].address);
        }
    }
    
    neoc_free(entries);
}

/**
 * @brief Create a new peers structure
 */
neoc_error_t neoc_peers_create(neoc_address_entry_t *connected,
                                size_t connected_count,
                                neoc_address_entry_t *bad,
                                size_t bad_count,
                                neoc_address_entry_t *unconnected,
                                size_t unconnected_count,
                                neoc_peers_t **peers) {
    if (!peers) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *peers = NULL;
    
    neoc_peers_t *new_peers = neoc_malloc(sizeof(neoc_peers_t));
    if (!new_peers) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize all fields
    memset(new_peers, 0, sizeof(neoc_peers_t));
    
    // Copy connected peers
    if (connected && connected_count > 0) {
        neoc_error_t error = copy_address_entries(connected, connected_count, &new_peers->connected);
        if (error != NEOC_SUCCESS) {
            neoc_free(new_peers);
            return error;
        }
        new_peers->connected_count = connected_count;
    }
    
    // Copy bad peers
    if (bad && bad_count > 0) {
        neoc_error_t error = copy_address_entries(bad, bad_count, &new_peers->bad);
        if (error != NEOC_SUCCESS) {
            free_address_entries(new_peers->connected, new_peers->connected_count);
            neoc_free(new_peers);
            return error;
        }
        new_peers->bad_count = bad_count;
    }
    
    // Copy unconnected peers
    if (unconnected && unconnected_count > 0) {
        neoc_error_t error = copy_address_entries(unconnected, unconnected_count, &new_peers->unconnected);
        if (error != NEOC_SUCCESS) {
            free_address_entries(new_peers->connected, new_peers->connected_count);
            free_address_entries(new_peers->bad, new_peers->bad_count);
            neoc_free(new_peers);
            return error;
        }
        new_peers->unconnected_count = unconnected_count;
    }
    
    *peers = new_peers;
    return NEOC_SUCCESS;
}

/**
 * @brief Free a peers structure
 */
void neoc_peers_free(neoc_peers_t *peers) {
    if (!peers) {
        return;
    }
    
    free_address_entries(peers->connected, peers->connected_count);
    free_address_entries(peers->bad, peers->bad_count);
    free_address_entries(peers->unconnected, peers->unconnected_count);
    
    neoc_free(peers);
}

/**
 * @brief Create a new Neo get peers response
 */
neoc_error_t neoc_neo_get_peers_create(int id,
                                        neoc_peers_t *peers,
                                        const char *error,
                                        int error_code,
                                        neoc_neo_get_peers_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *response = NULL;
    
    neoc_neo_get_peers_t *new_response = neoc_malloc(sizeof(neoc_neo_get_peers_t));
    if (!new_response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize response
    new_response->jsonrpc = neoc_strdup("2.0");
    if (!new_response->jsonrpc) {
        neoc_free(new_response);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    new_response->id = id;
    new_response->result = peers;  // Take ownership
    new_response->error_code = error_code;
    
    if (error) {
        new_response->error = neoc_strdup(error);
        if (!new_response->error) {
            neoc_free(new_response->jsonrpc);
            neoc_free(new_response);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    } else {
        new_response->error = NULL;
    }
    
    *response = new_response;
    return NEOC_SUCCESS;
}

/**
 * @brief Free a Neo get peers response
 */
void neoc_neo_get_peers_free(neoc_neo_get_peers_t *response) {
    if (!response) {
        return;
    }
    
    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
    }
    
    if (response->error) {
        neoc_free(response->error);
    }
    
    if (response->result) {
        neoc_peers_free(response->result);
    }
    
    neoc_free(response);
}

/**
 * @brief Parse JSON response into Neo get peers structure
 */
neoc_error_t neoc_neo_get_peers_from_json(const char *json_str,
                                           neoc_neo_get_peers_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *response = NULL;
    
    // Parse JSON using the neoc_json library
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Create response structure
    *response = neoc_calloc(1, sizeof(neoc_neo_get_peers_t));
    if (!*response) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Parse connected peers array
    neoc_json_t *connected = neoc_json_get_array(json, "connected");
    if (connected) {
        (*response)->connected_count = neoc_json_array_size(connected);
        if ((*response)->connected_count > 0) {
            (*response)->connected_peers = neoc_calloc((*response)->connected_count, sizeof(neoc_peer_t));
            if (!(*response)->connected_peers) {
                neoc_neo_get_peers_free(*response);
                *response = NULL;
                neoc_json_free(json);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            
            for (size_t i = 0; i < (*response)->connected_count; i++) {
                neoc_json_t *peer_json = neoc_json_array_get(connected, i);
                if (peer_json) {
                    const char *addr = neoc_json_get_string(peer_json, "address");
                    if (addr) {
                        (*response)->connected_peers[i].address = neoc_strdup(addr);
                    }
                    (*response)->connected_peers[i].port = (uint16_t)neoc_json_get_int(peer_json, "port");
                }
            }
        }
    }
    
    // Parse bad peers array similarly
    neoc_json_t *bad = neoc_json_get_array(json, "bad");
    if (bad) {
        (*response)->bad_count = neoc_json_array_size(bad);
        if ((*response)->bad_count > 0) {
            (*response)->bad_peers = neoc_calloc((*response)->bad_count, sizeof(neoc_peer_t));
            if (!(*response)->bad_peers) {
                neoc_neo_get_peers_free(*response);
                *response = NULL;
                neoc_json_free(json);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            
            for (size_t i = 0; i < (*response)->bad_count; i++) {
                neoc_json_t *peer_json = neoc_json_array_get(bad, i);
                if (peer_json) {
                    const char *addr = neoc_json_get_string(peer_json, "address");
                    if (addr) {
                        (*response)->bad_peers[i].address = neoc_strdup(addr);
                    }
                    (*response)->bad_peers[i].port = (uint16_t)neoc_json_get_int(peer_json, "port");
                }
            }
        }
    }
    
    // Parse unconnected peers array similarly
    neoc_json_t *unconnected = neoc_json_get_array(json, "unconnected");
    if (unconnected) {
        (*response)->unconnected_count = neoc_json_array_size(unconnected);
        if ((*response)->unconnected_count > 0) {
            (*response)->unconnected_peers = neoc_calloc((*response)->unconnected_count, sizeof(neoc_peer_t));
            if (!(*response)->unconnected_peers) {
                neoc_neo_get_peers_free(*response);
                *response = NULL;
                neoc_json_free(json);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            
            for (size_t i = 0; i < (*response)->unconnected_count; i++) {
                neoc_json_t *peer_json = neoc_json_array_get(unconnected, i);
                if (peer_json) {
                    const char *addr = neoc_json_get_string(peer_json, "address");
                    if (addr) {
                        (*response)->unconnected_peers[i].address = neoc_strdup(addr);
                    }
                    (*response)->unconnected_peers[i].port = (uint16_t)neoc_json_get_int(peer_json, "port");
                }
            }
        }
    }
    
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

/**
 * @brief Convert Neo get peers response to JSON string
 */
neoc_error_t neoc_neo_get_peers_to_json(const neoc_neo_get_peers_t *response,
                                         char **json_str) {
    if (!response || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *json_str = NULL;
    
    // Create JSON object
    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add connected peers array
    neoc_json_t *connected = neoc_json_create_array();
    if (response->connected_peers && response->connected_count > 0) {
        for (size_t i = 0; i < response->connected_count; i++) {
            neoc_json_t *peer = neoc_json_create_object();
            neoc_json_add_string(peer, "address", response->connected_peers[i].address);
            neoc_json_add_int(peer, "port", response->connected_peers[i].port);
            neoc_json_array_add(connected, peer);
        }
    }
    neoc_json_add_array(json, "connected", connected);
    
    // Add bad peers array
    neoc_json_t *bad = neoc_json_create_array();
    if (response->bad_peers && response->bad_count > 0) {
        for (size_t i = 0; i < response->bad_count; i++) {
            neoc_json_t *peer = neoc_json_create_object();
            neoc_json_add_string(peer, "address", response->bad_peers[i].address);
            neoc_json_add_int(peer, "port", response->bad_peers[i].port);
            neoc_json_array_add(bad, peer);
        }
    }
    neoc_json_add_array(json, "bad", bad);
    
    // Add unconnected peers array
    neoc_json_t *unconnected = neoc_json_create_array();
    if (response->unconnected_peers && response->unconnected_count > 0) {
        for (size_t i = 0; i < response->unconnected_count; i++) {
            neoc_json_t *peer = neoc_json_create_object();
            neoc_json_add_string(peer, "address", response->unconnected_peers[i].address);
            neoc_json_add_int(peer, "port", response->unconnected_peers[i].port);
            neoc_json_array_add(unconnected, peer);
        }
    }
    neoc_json_add_array(json, "unconnected", unconnected);
    
    // Convert to string
    *json_str = neoc_json_to_string(json);
    neoc_json_free(json);
    
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get peers data from response (convenience function)
 */
neoc_peers_t *neoc_neo_get_peers_get_peers(const neoc_neo_get_peers_t *response) {
    if (!response) {
        return NULL;
    }
    
    return response->result;
}

/**
 * @brief Check if the response contains valid peers data
 */
bool neoc_neo_get_peers_has_peers(const neoc_neo_get_peers_t *response) {
    if (!response) {
        return false;
    }
    
    return (response->result != NULL && response->error_code == 0);
}

/**
 * @brief Get number of connected peers
 */
size_t neoc_neo_get_peers_get_connected_count(const neoc_neo_get_peers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    
    return response->result->connected_count;
}

/**
 * @brief Get number of bad peers
 */
size_t neoc_neo_get_peers_get_bad_count(const neoc_neo_get_peers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    
    return response->result->bad_count;
}

/**
 * @brief Get number of unconnected peers
 */
size_t neoc_neo_get_peers_get_unconnected_count(const neoc_neo_get_peers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    
    return response->result->unconnected_count;
}

/**
 * @brief Get total number of known peers
 */
size_t neoc_neo_get_peers_get_total_count(const neoc_neo_get_peers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    
    return response->result->connected_count + 
           response->result->bad_count + 
           response->result->unconnected_count;
}

/**
 * @brief Get connected peer by index
 */
neoc_address_entry_t *neoc_neo_get_peers_get_connected_peer(const neoc_neo_get_peers_t *response,
                                                             size_t index) {
    if (!response || !response->result || index >= response->result->connected_count) {
        return NULL;
    }
    
    return &response->result->connected[index];
}

/**
 * @brief Get bad peer by index
 */
neoc_address_entry_t *neoc_neo_get_peers_get_bad_peer(const neoc_neo_get_peers_t *response,
                                                       size_t index) {
    if (!response || !response->result || index >= response->result->bad_count) {
        return NULL;
    }
    
    return &response->result->bad[index];
}

/**
 * @brief Get unconnected peer by index
 */
neoc_address_entry_t *neoc_neo_get_peers_get_unconnected_peer(const neoc_neo_get_peers_t *response,
                                                               size_t index) {
    if (!response || !response->result || index >= response->result->unconnected_count) {
        return NULL;
    }
    
    return &response->result->unconnected[index];
}

