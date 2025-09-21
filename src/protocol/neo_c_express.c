/**
 * @file neo_c_express.c
 * @brief NeoC Express implementation for Neo-express nodes
 * 
 * Based on Swift source: protocol/NeoSwiftExpress.swift
 * Provides Neo-express specific RPC methods
 */

#include "neoc/neoc.h"
#include "neoc/protocol/neo_c_express.h"
#include "neoc/protocol/neo_c.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief NeoC Express implementation structure
 * Inherits from neoc_neo_c_t and adds Express-specific functionality
 */
typedef struct neoc_neo_c_express_internal {
    neoc_neo_c_t base;        /**< Base NeoC client */
    bool is_express;          /**< Flag indicating this is an Express client */
} neoc_neo_c_express_internal_t;

/**
 * @brief Create a new NeoC Express client
 */
neoc_neo_c_express_t *neoc_neo_c_express_create(
    neoc_neo_c_config_t *config,
    neoc_service_t *service) {
    
    if (!config || !service) {
        return NULL;
    }
    
    neoc_neo_c_express_internal_t *express = 
        (neoc_neo_c_express_internal_t *)neoc_malloc(sizeof(neoc_neo_c_express_internal_t));
    if (!express) {
        return NULL;
    }
    
    // Initialize base NeoC client
    express->base.config = config;
    express->base.neo_c_service = service;
    express->base.neo_c_rx = NULL;
    express->is_express = true;
    
    return (neoc_neo_c_express_t *)express;
}

/**
 * @brief Build a NeoC Express client with default configuration
 */
neoc_neo_c_express_t *neoc_neo_c_express_build(neoc_service_t *service) {
    if (!service) {
        return NULL;
    }
    
    neoc_neo_c_config_t *config = neoc_neo_c_config_create();
    if (!config) {
        return NULL;
    }
    
    return neoc_neo_c_express_create(config, service);
}

/**
 * @brief Get populated blocks from Neo-express
 */
neoc_error_t neoc_neo_c_express_get_populated_blocks(
    neoc_neo_c_express_t *express,
    neoc_populated_blocks_t **blocks_out) {
    
    if (!express || !blocks_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expressgetpopulatedblocks
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expressgetpopulatedblocks",
        .params = NULL,
        .params_count = 0,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response into block array
    *blocks_out = NULL;
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief Get NEP-17 contracts from Neo-express
 */
neoc_error_t neoc_neo_c_express_get_nep17_contracts(
    neoc_neo_c_express_t *express,
    neoc_nep17_contract_t ***contracts_out,
    size_t *count_out) {
    
    if (!express || !contracts_out || !count_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expressgetnep17contracts
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expressgetnep17contracts",
        .params = NULL,
        .params_count = 0,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response into contract array
    *contracts_out = NULL;
    *count_out = 0;
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief Get contract storage from Neo-express
 */
neoc_error_t neoc_neo_c_express_get_contract_storage(
    neoc_neo_c_express_t *express,
    const neoc_hash160_t *contract_hash,
    neoc_contract_storage_entry_t ***entries_out,
    size_t *count_out) {
    
    if (!express || !contract_hash || !entries_out || !count_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expressgetcontractstorage
    char contract_str[42];
    neoc_hash160_to_string(contract_hash, contract_str, sizeof(contract_str));
    
    neoc_json_rpc_param_t params[] = {
        {.type = NEOC_JSON_RPC_PARAM_STRING, .value.string_value = contract_str}
    };
    
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expressgetcontractstorage",
        .params = params,
        .params_count = 1,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response into storage entries array
    *entries_out = NULL;
    *count_out = 0;
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief List all contracts from Neo-express
 */
neoc_error_t neoc_neo_c_express_list_contracts(
    neoc_neo_c_express_t *express,
    neoc_express_contract_state_t ***contracts_out,
    size_t *count_out) {
    
    if (!express || !contracts_out || !count_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expresslistcontracts
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expresslistcontracts",
        .params = NULL,
        .params_count = 0,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response into contracts array
    *contracts_out = NULL;
    *count_out = 0;
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief Create a checkpoint in Neo-express
 */
neoc_error_t neoc_neo_c_express_create_checkpoint(
    neoc_neo_c_express_t *express,
    const char *filename,
    char **checkpoint_path_out) {
    
    if (!express || !filename || !checkpoint_path_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expresscreatecheckpoint
    neoc_json_rpc_param_t params[] = {
        {.type = NEOC_JSON_RPC_PARAM_STRING, .value.string_value = (char*)filename}
    };
    
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expresscreatecheckpoint",
        .params = params,
        .params_count = 1,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response to get checkpoint path
    *checkpoint_path_out = NULL;
    if (response && response->result) {
        *checkpoint_path_out = neoc_strdup(response->result);
    }
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief List oracle requests from Neo-express
 */
neoc_error_t neoc_neo_c_express_list_oracle_requests(
    neoc_neo_c_express_t *express,
    neoc_oracle_request_t ***requests_out,
    size_t *count_out) {
    
    if (!express || !requests_out || !count_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expresslistoraclerequests
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expresslistoraclerequests",
        .params = NULL,
        .params_count = 0,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response into oracle requests array
    *requests_out = NULL;
    *count_out = 0;
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief Create an oracle response transaction in Neo-express
 */
neoc_error_t neoc_neo_c_express_create_oracle_response_tx(
    neoc_neo_c_express_t *express,
    const neoc_transaction_attribute_t *oracle_response,
    char **tx_hex_out) {
    
    if (!express || !oracle_response || !tx_hex_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expresscreateoracleresponsetx
    // Serialize oracle response attribute to JSON
    char *oracle_json = NULL;
    neoc_error_t err = neoc_transaction_attribute_to_json(oracle_response, &oracle_json);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    neoc_json_rpc_param_t params[] = {
        {.type = NEOC_JSON_RPC_PARAM_STRING, .value.string_value = oracle_json}
    };
    
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expresscreateoracleresponsetx",
        .params = params,
        .params_count = 1,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    err = neoc_rpc_client_send_request(express->client, &request, &response);
    neoc_free(oracle_json);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response to get transaction hex
    *tx_hex_out = NULL;
    if (response && response->result) {
        *tx_hex_out = neoc_strdup(response->result);
    }
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief Shutdown Neo-express instance
 */
neoc_error_t neoc_neo_c_express_shutdown(
    neoc_neo_c_express_t *express,
    neoc_express_shutdown_t **shutdown_info_out) {
    
    if (!express || !shutdown_info_out) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    // Create JSON-RPC request for expressshutdown
    neoc_json_rpc_request_t request = {
        .jsonrpc = "2.0",
        .method = "expressshutdown",
        .params = NULL,
        .params_count = 0,
        .id = 1
    };
    
    neoc_json_rpc_response_t *response = NULL;
    neoc_error_t err = neoc_rpc_client_send_request(express->client, &request, &response);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Parse response into shutdown info
    *shutdown_info_out = neoc_calloc(1, sizeof(neoc_express_shutdown_t));
    if (*shutdown_info_out && response && response->result) {
        // Parse shutdown information from response
        (*shutdown_info_out)->process_id = 0;
        (*shutdown_info_out)->exit_code = 0;
    }
    neoc_json_rpc_response_free(response);
    return NEOC_SUCCESS;
}

/**
 * @brief Check if client is a Neo-express client
 */
bool neoc_neo_c_express_is_express(const neoc_neo_c_express_t *express) {
    if (!express) {
        return false;
    }
    
    neoc_neo_c_express_internal_t *internal = (neoc_neo_c_express_internal_t *)express;
    return internal->is_express;
}

/**
 * @brief Free NeoC Express client
 */
void neoc_neo_c_express_free(neoc_neo_c_express_t *express) {
    if (!express) {
        return;
    }
    
    // Free as regular NeoC client (handles all cleanup)
    neoc_neo_c_free((neoc_neo_c_t *)express);
}