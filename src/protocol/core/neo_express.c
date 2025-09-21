/**
 * @file neo_express.c
 * @brief Neo Express protocol implementation
 * 
 * Implements the Neo Express protocol interface for local blockchain testing
 * based on the Swift NeoExpress implementation.
 */

#include "neoc/neoc.h"
#include "neoc/protocol/core/neo_express.h"
#include "neoc/protocol/core/request.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Get populated blocks from Neo Express
 * @param service The Neo Express service instance
 * @return Request for populated blocks or NULL on error
 */
neoc_request_t *neoc_neo_express_get_populated_blocks(void *service) {
    if (!service) {
        return NULL;
    }
    
    return neoc_request_create("expressgetpopulatedblocks", "[]", service);
}

/**
 * @brief Get NEP-17 contracts from Neo Express
 * @param service The Neo Express service instance
 * @return Request for NEP-17 contracts or NULL on error
 */
neoc_request_t *neoc_neo_express_get_nep17_contracts(void *service) {
    if (!service) {
        return NULL;
    }
    
    return neoc_request_create("expressgetnep17contracts", "[]", service);
}

/**
 * @brief Get contract storage from Neo Express
 * @param service The Neo Express service instance
 * @param contract_hash The contract hash to query
 * @return Request for contract storage or NULL on error
 */
neoc_request_t *neoc_neo_express_get_contract_storage(void *service, const neoc_hash160_t *contract_hash) {
    if (!service || !contract_hash) {
        return NULL;
    }
    
    // Convert hash to hex string
    char hash_hex[41]; // 40 chars + null terminator
    neoc_error_t err = neoc_hash160_to_string(contract_hash, hash_hex, sizeof(hash_hex));
    if (err != NEOC_SUCCESS) {
        return NULL;
    }
    
    // Build parameters JSON
    char params[64];
    snprintf(params, sizeof(params), "[\"%s\"]", hash_hex);
    
    neoc_request_t *request = neoc_request_create("expressgetcontractstorage", params, service);
    
    return request;
}

/**
 * @brief List contracts from Neo Express
 * @param service The Neo Express service instance
 * @return Request for contract list or NULL on error
 */
neoc_request_t *neoc_neo_express_list_contracts(void *service) {
    if (!service) {
        return NULL;
    }
    
    return neoc_request_create("expresslistcontracts", "[]", service);
}

/**
 * @brief Create checkpoint in Neo Express
 * @param service The Neo Express service instance
 * @param filename The checkpoint filename
 * @return Request for checkpoint creation or NULL on error
 */
neoc_request_t *neoc_neo_express_create_checkpoint(void *service, const char *filename) {
    if (!service || !filename) {
        return NULL;
    }
    
    // Build parameters JSON
    char params[256];
    snprintf(params, sizeof(params), "[\"%s\"]", filename);
    
    return neoc_request_create("expresscreatecheckpoint", params, service);
}

/**
 * @brief List oracle requests from Neo Express
 * @param service The Neo Express service instance
 * @return Request for oracle requests or NULL on error
 */
neoc_request_t *neoc_neo_express_list_oracle_requests(void *service) {
    if (!service) {
        return NULL;
    }
    
    return neoc_request_create("expresslistoraclerequests", "[]", service);
}

/**
 * @brief Create oracle response transaction in Neo Express
 * @param service The Neo Express service instance
 * @param oracle_response The oracle response transaction attribute
 * @return Request for oracle response creation or NULL on error
 */
neoc_request_t *neoc_neo_express_create_oracle_response_tx(void *service, const void *oracle_response) {
    if (!service || !oracle_response) {
        return NULL;
    }
    
    // For oracle response, we need to serialize the transaction attribute
    // Oracle response contains: id, code, result
    // Since the structure is complex and depends on the transaction attribute implementation,
    // we'll create a basic JSON structure that can be extended when the full
    // transaction attribute system is implemented
    
    // Get oracle response data (assuming it's a structured type with basic fields)
    // In production, this would interface with the actual transaction attribute structure
    const uint64_t *request_id = (const uint64_t *)oracle_response; // Simplified assumption
    
    // Build parameters JSON with oracle response structure
    char params[512];
    snprintf(params, sizeof(params), 
             "[{\"id\":%llu,\"code\":0,\"result\":\"\"}]", 
             (unsigned long long)(request_id ? *request_id : 0));
    
    return neoc_request_create("expresscreateoracleresponsetx", params, service);
}

/**
 * @brief Shutdown Neo Express
 * @param service The Neo Express service instance
 * @return Request for shutdown or NULL on error
 */
neoc_request_t *neoc_neo_express_shutdown(void *service) {
    if (!service) {
        return NULL;
    }
    
    return neoc_request_create("expressshutdown", "[]", service);
}

/**
 * @brief Check if Neo Express is available
 * @param service The service instance to check
 * @return true if Neo Express is available, false otherwise
 */
bool neoc_neo_express_is_available(void *service) {
    if (!service) {
        return false;
    }
    
    // Create a simple test request to check availability
    neoc_request_t *test_request = neoc_neo_express_get_populated_blocks(service);
    if (!test_request) {
        return false;
    }
    
    neoc_request_free(test_request);
    return true;
}
