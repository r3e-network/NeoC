/**
 * @file node_plugin_type.c
 * @brief Neo node plugin type implementation
 */

#include "neoc/types/node_plugin_type.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>

const char* neoc_node_plugin_type_to_string(neoc_node_plugin_type_t type) {
    switch (type) {
        case NEOC_NODE_PLUGIN_APPLICATION_LOGS:         return "ApplicationLogs";
        case NEOC_NODE_PLUGIN_CORE_METRICS:             return "CoreMetrics";
        case NEOC_NODE_PLUGIN_IMPORT_BLOCKS:            return "ImportBlocks";
        case NEOC_NODE_PLUGIN_LEVEL_DB_STORE:           return "LevelDBStore";
        case NEOC_NODE_PLUGIN_ROCKS_DB_STORE:           return "RocksDBStore";
        case NEOC_NODE_PLUGIN_RPC_NEP17_TRACKER:        return "RpcNep17Tracker";
        case NEOC_NODE_PLUGIN_RPC_SECURITY:             return "RpcSecurity";
        case NEOC_NODE_PLUGIN_RPC_SERVER_PLUGIN:        return "RpcServerPlugin";
        case NEOC_NODE_PLUGIN_RPC_SYSTEM_ASSET_TRACKER: return "RpcSystemAssetTrackerPlugin";
        case NEOC_NODE_PLUGIN_SIMPLE_POLICY:            return "SimplePolicyPlugin";
        case NEOC_NODE_PLUGIN_STATES_DUMPER:            return "StatesDumper";
        case NEOC_NODE_PLUGIN_SYSTEM_LOG:               return "SystemLog";
        default:                                         return "Unknown";
    }
}

neoc_error_t neoc_node_plugin_type_from_string(const char *str, neoc_node_plugin_type_t *type) {
    if (!str || !type) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (strcmp(str, "ApplicationLogs") == 0) {
        *type = NEOC_NODE_PLUGIN_APPLICATION_LOGS;
    } else if (strcmp(str, "CoreMetrics") == 0) {
        *type = NEOC_NODE_PLUGIN_CORE_METRICS;
    } else if (strcmp(str, "ImportBlocks") == 0) {
        *type = NEOC_NODE_PLUGIN_IMPORT_BLOCKS;
    } else if (strcmp(str, "LevelDBStore") == 0) {
        *type = NEOC_NODE_PLUGIN_LEVEL_DB_STORE;
    } else if (strcmp(str, "RocksDBStore") == 0) {
        *type = NEOC_NODE_PLUGIN_ROCKS_DB_STORE;
    } else if (strcmp(str, "RpcNep17Tracker") == 0) {
        *type = NEOC_NODE_PLUGIN_RPC_NEP17_TRACKER;
    } else if (strcmp(str, "RpcSecurity") == 0) {
        *type = NEOC_NODE_PLUGIN_RPC_SECURITY;
    } else if (strcmp(str, "RpcServerPlugin") == 0) {
        *type = NEOC_NODE_PLUGIN_RPC_SERVER_PLUGIN;
    } else if (strcmp(str, "RpcSystemAssetTrackerPlugin") == 0) {
        *type = NEOC_NODE_PLUGIN_RPC_SYSTEM_ASSET_TRACKER;
    } else if (strcmp(str, "SimplePolicyPlugin") == 0) {
        *type = NEOC_NODE_PLUGIN_SIMPLE_POLICY;
    } else if (strcmp(str, "StatesDumper") == 0) {
        *type = NEOC_NODE_PLUGIN_STATES_DUMPER;
    } else if (strcmp(str, "SystemLog") == 0) {
        *type = NEOC_NODE_PLUGIN_SYSTEM_LOG;
    } else {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, 
                             "Unknown node plugin type string");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_node_plugin_type_get_all(neoc_node_plugin_type_t **types, size_t *count) {
    if (!types || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    static const neoc_node_plugin_type_t all_types[] = {
        NEOC_NODE_PLUGIN_APPLICATION_LOGS,
        NEOC_NODE_PLUGIN_CORE_METRICS,
        NEOC_NODE_PLUGIN_IMPORT_BLOCKS,
        NEOC_NODE_PLUGIN_LEVEL_DB_STORE,
        NEOC_NODE_PLUGIN_ROCKS_DB_STORE,
        NEOC_NODE_PLUGIN_RPC_NEP17_TRACKER,
        NEOC_NODE_PLUGIN_RPC_SECURITY,
        NEOC_NODE_PLUGIN_RPC_SERVER_PLUGIN,
        NEOC_NODE_PLUGIN_RPC_SYSTEM_ASSET_TRACKER,
        NEOC_NODE_PLUGIN_SIMPLE_POLICY,
        NEOC_NODE_PLUGIN_STATES_DUMPER,
        NEOC_NODE_PLUGIN_SYSTEM_LOG
    };
    
    *count = sizeof(all_types) / sizeof(all_types[0]);
    *types = neoc_malloc(*count * sizeof(neoc_node_plugin_type_t));
    if (!*types) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate types array");
    }
    
    memcpy(*types, all_types, *count * sizeof(neoc_node_plugin_type_t));
    return NEOC_SUCCESS;
}

bool neoc_node_plugin_type_is_storage(neoc_node_plugin_type_t type) {
    return type == NEOC_NODE_PLUGIN_LEVEL_DB_STORE ||
           type == NEOC_NODE_PLUGIN_ROCKS_DB_STORE;
}

bool neoc_node_plugin_type_is_rpc(neoc_node_plugin_type_t type) {
    return type == NEOC_NODE_PLUGIN_RPC_NEP17_TRACKER ||
           type == NEOC_NODE_PLUGIN_RPC_SECURITY ||
           type == NEOC_NODE_PLUGIN_RPC_SERVER_PLUGIN ||
           type == NEOC_NODE_PLUGIN_RPC_SYSTEM_ASSET_TRACKER;
}
