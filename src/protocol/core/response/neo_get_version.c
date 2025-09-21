/**
 * @file neo_get_version.c
 * @brief Neo getversion RPC response implementation
 * 
 * Based on Swift source: protocol/core/response/NeoGetVersion.swift
 * Implements Neo node version information handling and parsing
 */

#include "../../../../include/neoc/neoc_error.h"
#include "../../../../include/neoc/neoc_memory.h"
#include "../../../../include/neoc/protocol/core/response/neo_get_version.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Create Neo version structure
 */
neoc_error_t neoc_neo_version_create(neoc_neo_version_t **version) {
    if (!version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *version = NULL;
    
    neoc_neo_version_t *new_version = neoc_malloc(sizeof(neoc_neo_version_t));
    if (!new_version) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize all fields to zero/NULL
    memset(new_version, 0, sizeof(neoc_neo_version_t));
    
    *version = new_version;
    return NEOC_SUCCESS;
}

/**
 * @brief Set basic version information
 */
neoc_error_t neoc_neo_version_set_basic_info(neoc_neo_version_t *version,
                                              uint32_t tcp_port,
                                              uint32_t ws_port,
                                              uint32_t nonce,
                                              const char *user_agent) {
    if (!version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    version->tcp_port = tcp_port;
    version->ws_port = ws_port;
    version->nonce = nonce;
    
    // Free existing user agent and set new one
    if (version->user_agent) {
        neoc_free(version->user_agent);
    }
    
    if (user_agent) {
        version->user_agent = neoc_strdup(user_agent);
        if (!version->user_agent) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    } else {
        version->user_agent = NULL;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Set protocol configuration
 */
neoc_error_t neoc_neo_version_set_protocol_config(neoc_neo_version_t *version,
                                                   uint32_t network,
                                                   uint32_t address_version,
                                                   uint32_t ms_per_block,
                                                   uint32_t max_transactions_per_block,
                                                   uint32_t memory_pool_max_transactions,
                                                   uint32_t max_trace_results,
                                                   uint64_t initial_gas_distribution) {
    if (!version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    version->protocol.network = network;
    version->protocol.address_version = address_version;
    version->protocol.ms_per_block = ms_per_block;
    version->protocol.max_transactions_per_block = max_transactions_per_block;
    version->protocol.memory_pool_max_transactions = memory_pool_max_transactions;
    version->protocol.max_trace_results = max_trace_results;
    version->protocol.initial_gas_distribution = initial_gas_distribution;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Set valid signers array
 */
neoc_error_t neoc_neo_version_set_valid_signers(neoc_neo_version_t *version,
                                                 const char **signers,
                                                 size_t count) {
    if (!version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free existing signers
    if (version->protocol.valid_signers) {
        for (size_t i = 0; i < version->protocol.valid_signers_count; i++) {
            neoc_free(version->protocol.valid_signers[i]);
        }
        neoc_free(version->protocol.valid_signers);
        version->protocol.valid_signers = NULL;
        version->protocol.valid_signers_count = 0;
    }
    
    if (signers && count > 0) {
        version->protocol.valid_signers = neoc_malloc(sizeof(char*) * count);
        if (!version->protocol.valid_signers) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        
        // Copy each signer string
        for (size_t i = 0; i < count; i++) {
            if (signers[i]) {
                version->protocol.valid_signers[i] = neoc_strdup(signers[i]);
                if (!version->protocol.valid_signers[i]) {
                    // Clean up on failure
                    for (size_t j = 0; j < i; j++) {
                        neoc_free(version->protocol.valid_signers[j]);
                    }
                    neoc_free(version->protocol.valid_signers);
                    version->protocol.valid_signers = NULL;
                    return NEOC_ERROR_OUT_OF_MEMORY;
                }
            } else {
                version->protocol.valid_signers[i] = NULL;
            }
        }
        
        version->protocol.valid_signers_count = count;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Set committee members array
 */
neoc_error_t neoc_neo_version_set_committee_members(neoc_neo_version_t *version,
                                                     const char **members,
                                                     size_t count) {
    if (!version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free existing committee members
    if (version->protocol.committee_members) {
        for (size_t i = 0; i < version->protocol.committee_members_count; i++) {
            neoc_free(version->protocol.committee_members[i]);
        }
        neoc_free(version->protocol.committee_members);
        version->protocol.committee_members = NULL;
        version->protocol.committee_members_count = 0;
    }
    
    if (members && count > 0) {
        version->protocol.committee_members = neoc_malloc(sizeof(char*) * count);
        if (!version->protocol.committee_members) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        
        // Copy each member string
        for (size_t i = 0; i < count; i++) {
            if (members[i]) {
                version->protocol.committee_members[i] = neoc_strdup(members[i]);
                if (!version->protocol.committee_members[i]) {
                    // Clean up on failure
                    for (size_t j = 0; j < i; j++) {
                        neoc_free(version->protocol.committee_members[j]);
                    }
                    neoc_free(version->protocol.committee_members);
                    version->protocol.committee_members = NULL;
                    return NEOC_ERROR_OUT_OF_MEMORY;
                }
            } else {
                version->protocol.committee_members[i] = NULL;
            }
        }
        
        version->protocol.committee_members_count = count;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Set seed list array
 */
neoc_error_t neoc_neo_version_set_seed_list(neoc_neo_version_t *version,
                                             const char **seeds,
                                             size_t count) {
    if (!version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free existing seed list
    if (version->protocol.seed_list) {
        for (size_t i = 0; i < version->protocol.seed_list_count; i++) {
            neoc_free(version->protocol.seed_list[i]);
        }
        neoc_free(version->protocol.seed_list);
        version->protocol.seed_list = NULL;
        version->protocol.seed_list_count = 0;
    }
    
    if (seeds && count > 0) {
        version->protocol.seed_list = neoc_malloc(sizeof(char*) * count);
        if (!version->protocol.seed_list) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        
        // Copy each seed string
        for (size_t i = 0; i < count; i++) {
            if (seeds[i]) {
                version->protocol.seed_list[i] = neoc_strdup(seeds[i]);
                if (!version->protocol.seed_list[i]) {
                    // Clean up on failure
                    for (size_t j = 0; j < i; j++) {
                        neoc_free(version->protocol.seed_list[j]);
                    }
                    neoc_free(version->protocol.seed_list);
                    version->protocol.seed_list = NULL;
                    return NEOC_ERROR_OUT_OF_MEMORY;
                }
            } else {
                version->protocol.seed_list[i] = NULL;
            }
        }
        
        version->protocol.seed_list_count = count;
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get user agent string
 */
const char* neoc_neo_version_get_user_agent(const neoc_neo_version_t *version) {
    if (!version) {
        return NULL;
    }
    
    return version->user_agent;
}

/**
 * @brief Get network magic number
 */
uint32_t neoc_neo_version_get_network(const neoc_neo_version_t *version) {
    if (!version) {
        return 0;
    }
    
    return version->protocol.network;
}

/**
 * @brief Get milliseconds per block
 */
uint32_t neoc_neo_version_get_ms_per_block(const neoc_neo_version_t *version) {
    if (!version) {
        return 0;
    }
    
    return version->protocol.ms_per_block;
}

/**
 * @brief Get max transactions per block
 */
uint32_t neoc_neo_version_get_max_transactions_per_block(const neoc_neo_version_t *version) {
    if (!version) {
        return 0;
    }
    
    return version->protocol.max_transactions_per_block;
}

/**
 * @brief Get valid signers
 */
neoc_error_t neoc_neo_version_get_valid_signers(const neoc_neo_version_t *version,
                                                 const char ***signers,
                                                 size_t *count) {
    if (!version || !signers || !count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *signers = (const char**)version->protocol.valid_signers;
    *count = version->protocol.valid_signers_count;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get committee members
 */
neoc_error_t neoc_neo_version_get_committee_members(const neoc_neo_version_t *version,
                                                     const char ***members,
                                                     size_t *count) {
    if (!version || !members || !count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *members = (const char**)version->protocol.committee_members;
    *count = version->protocol.committee_members_count;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Check if node supports specific protocol version
 */
bool neoc_neo_version_supports_protocol(const neoc_neo_version_t *version,
                                         const char *required_version) {
    if (!version || !required_version) {
        return false;
    }
    
    // Parse version numbers from the protocol structure
    // Compare protocol version with required version
    if (version->protocol) {
        // Parse required version (format: "major.minor.patch")
        int req_major = 0, req_minor = 0, req_patch = 0;
        if (sscanf(required_version, "%d.%d.%d", &req_major, &req_minor, &req_patch) >= 2) {
            // Compare with protocol version
            if (version->protocol->major > req_major) {
                return true;
            }
            if (version->protocol->major == req_major) {
                if (version->protocol->minor > req_minor) {
                    return true;
                }
                if (version->protocol->minor == req_minor && version->protocol->patch >= req_patch) {
                    return true;
                }
            }
        }
    }
    
    // Fallback: check user agent string for version information
    if (version->user_agent && strstr(version->user_agent, required_version)) {
        return true;
    }
    
    return false;
}

/**
 * @brief Copy Neo version structure
 */
neoc_error_t neoc_neo_version_copy(const neoc_neo_version_t *source,
                                    neoc_neo_version_t **copy) {
    if (!source || !copy) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *copy = NULL;
    
    // Create new version
    neoc_error_t error = neoc_neo_version_create(copy);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    neoc_neo_version_t *dest = *copy;
    
    // Copy basic info
    error = neoc_neo_version_set_basic_info(dest, source->tcp_port, source->ws_port,
                                             source->nonce, source->user_agent);
    if (error != NEOC_SUCCESS) {
        neoc_neo_version_free(dest);
        *copy = NULL;
        return error;
    }
    
    // Copy protocol config
    error = neoc_neo_version_set_protocol_config(dest,
                                                  source->protocol.network,
                                                  source->protocol.address_version,
                                                  source->protocol.ms_per_block,
                                                  source->protocol.max_transactions_per_block,
                                                  source->protocol.memory_pool_max_transactions,
                                                  source->protocol.max_trace_results,
                                                  source->protocol.initial_gas_distribution);
    if (error != NEOC_SUCCESS) {
        neoc_neo_version_free(dest);
        *copy = NULL;
        return error;
    }
    
    // Copy arrays
    if (source->protocol.valid_signers_count > 0) {
        error = neoc_neo_version_set_valid_signers(dest,
                                                    (const char**)source->protocol.valid_signers,
                                                    source->protocol.valid_signers_count);
        if (error != NEOC_SUCCESS) {
            neoc_neo_version_free(dest);
            *copy = NULL;
            return error;
        }
    }
    
    if (source->protocol.committee_members_count > 0) {
        error = neoc_neo_version_set_committee_members(dest,
                                                        (const char**)source->protocol.committee_members,
                                                        source->protocol.committee_members_count);
        if (error != NEOC_SUCCESS) {
            neoc_neo_version_free(dest);
            *copy = NULL;
            return error;
        }
    }
    
    if (source->protocol.seed_list_count > 0) {
        error = neoc_neo_version_set_seed_list(dest,
                                                (const char**)source->protocol.seed_list,
                                                source->protocol.seed_list_count);
        if (error != NEOC_SUCCESS) {
            neoc_neo_version_free(dest);
            *copy = NULL;
            return error;
        }
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Free Neo version structure
 */
void neoc_neo_version_free(neoc_neo_version_t *version) {
    if (!version) {
        return;
    }
    
    // Free user agent
    if (version->user_agent) {
        neoc_free(version->user_agent);
    }
    
    // Free protocol arrays
    if (version->protocol.valid_signers) {
        for (size_t i = 0; i < version->protocol.valid_signers_count; i++) {
            if (version->protocol.valid_signers[i]) {
                neoc_free(version->protocol.valid_signers[i]);
            }
        }
        neoc_free(version->protocol.valid_signers);
    }
    
    if (version->protocol.committee_members) {
        for (size_t i = 0; i < version->protocol.committee_members_count; i++) {
            if (version->protocol.committee_members[i]) {
                neoc_free(version->protocol.committee_members[i]);
            }
        }
        neoc_free(version->protocol.committee_members);
    }
    
    if (version->protocol.seed_list) {
        for (size_t i = 0; i < version->protocol.seed_list_count; i++) {
            if (version->protocol.seed_list[i]) {
                neoc_free(version->protocol.seed_list[i]);
            }
        }
        neoc_free(version->protocol.seed_list);
    }
    
    neoc_free(version);
}

/**
 * @brief Create version response
 */
neoc_error_t neoc_neo_get_version_response_create(int id,
                                                   neoc_neo_version_t *result,
                                                   const char *error,
                                                   int error_code,
                                                   neoc_neo_get_version_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *response = NULL;
    
    neoc_neo_get_version_response_t *new_response = neoc_malloc(sizeof(neoc_neo_get_version_response_t));
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
    new_response->result = result;
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
 * @brief Free version response
 */
void neoc_neo_get_version_response_free(neoc_neo_get_version_response_t *response) {
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
        neoc_neo_version_free(response->result);
    }
    
    neoc_free(response);
}
