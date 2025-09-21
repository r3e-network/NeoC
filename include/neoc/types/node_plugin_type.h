/**
 * @file node_plugin_type.h
 * @brief Neo node plugin type enumeration
 */

#ifndef NEOC_NODE_PLUGIN_TYPE_H_GUARD
#define NEOC_NODE_PLUGIN_TYPE_H_GUARD

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Neo node plugin types
 * These represent different types of plugins that can be loaded into a Neo node
 */
typedef enum {
    NEOC_NODE_PLUGIN_APPLICATION_LOGS,         ///< Application logs plugin
    NEOC_NODE_PLUGIN_CORE_METRICS,             ///< Core metrics plugin
    NEOC_NODE_PLUGIN_IMPORT_BLOCKS,            ///< Import blocks plugin
    NEOC_NODE_PLUGIN_LEVEL_DB_STORE,           ///< LevelDB storage plugin
    NEOC_NODE_PLUGIN_ROCKS_DB_STORE,           ///< RocksDB storage plugin
    NEOC_NODE_PLUGIN_RPC_NEP17_TRACKER,        ///< RPC NEP-17 tracker plugin
    NEOC_NODE_PLUGIN_RPC_SECURITY,             ///< RPC security plugin
    NEOC_NODE_PLUGIN_RPC_SERVER_PLUGIN,        ///< RPC server plugin
    NEOC_NODE_PLUGIN_RPC_SYSTEM_ASSET_TRACKER, ///< RPC system asset tracker plugin
    NEOC_NODE_PLUGIN_SIMPLE_POLICY,            ///< Simple policy plugin
    NEOC_NODE_PLUGIN_STATES_DUMPER,            ///< States dumper plugin
    NEOC_NODE_PLUGIN_SYSTEM_LOG                ///< System log plugin
} neoc_node_plugin_type_t;

/**
 * Convert node plugin type to string representation
 * @param type The plugin type
 * @return String representation (statically allocated)
 */
const char* neoc_node_plugin_type_to_string(neoc_node_plugin_type_t type);

/**
 * Convert string to node plugin type
 * @param str The string representation
 * @param type Output plugin type
 * @return Error code
 */
neoc_error_t neoc_node_plugin_type_from_string(const char *str, neoc_node_plugin_type_t *type);

/**
 * Get all available plugin types
 * @param types Output array of plugin types (caller must free)
 * @param count Output count of plugin types
 * @return Error code
 */
neoc_error_t neoc_node_plugin_type_get_all(neoc_node_plugin_type_t **types, size_t *count);

/**
 * Check if plugin type is storage-related
 * @param type The plugin type
 * @return True if storage-related
 */
bool neoc_node_plugin_type_is_storage(neoc_node_plugin_type_t type);

/**
 * Check if plugin type is RPC-related
 * @param type The plugin type
 * @return True if RPC-related
 */
bool neoc_node_plugin_type_is_rpc(neoc_node_plugin_type_t type);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NODE_PLUGIN_TYPE_H_GUARD
