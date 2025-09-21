/**
 * @file neo_list_plugins.h
 * @brief Neo plugins list response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoListPlugins.swift
 * Provides functionality for retrieving loaded plugin information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_LIST_PLUGINS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_LIST_PLUGINS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a Neo plugin
 * 
 * This structure contains information about a plugin loaded by the Neo node,
 * including its name, version, and supported interfaces.
 */
typedef struct {
    char *name;                         /**< Plugin name */
    char *version;                      /**< Plugin version */
    char **interfaces;                  /**< Array of interface names */
    size_t interfaces_count;            /**< Number of interfaces */
} neoc_neo_plugin_t;

/**
 * @brief Response structure for listplugins RPC call
 * 
 * Contains the list of plugins returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_neo_plugin_t *result;          /**< Array of plugins (NULL if error) */
    size_t result_count;                /**< Number of plugins */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_list_plugins_t;

/**
 * @brief Create a new plugin structure
 * 
 * @param name Plugin name
 * @param version Plugin version
 * @param interfaces Array of interface names
 * @param interfaces_count Number of interfaces
 * @param plugin Pointer to store the created plugin (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_plugin_create(
    const char *name,
    const char *version,
    const char **interfaces,
    size_t interfaces_count,
    neoc_neo_plugin_t **plugin
);

/**
 * @brief Free a plugin structure
 * 
 * @param plugin Plugin to free
 */
void neoc_neo_plugin_free(neoc_neo_plugin_t *plugin);

/**
 * @brief Create a new Neo list plugins response
 * 
 * @param id Request ID
 * @param plugins Array of plugins (can be NULL for error responses)
 * @param plugins_count Number of plugins
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_list_plugins_create(
    int id,
    neoc_neo_plugin_t *plugins,
    size_t plugins_count,
    const char *error,
    int error_code,
    neoc_neo_list_plugins_t **response
);

/**
 * @brief Free a Neo list plugins response
 * 
 * @param response Response to free
 */
void neoc_neo_list_plugins_free(neoc_neo_list_plugins_t *response);

/**
 * @brief Parse JSON response into Neo list plugins structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_list_plugins_from_json(
    const char *json_str,
    neoc_neo_list_plugins_t **response
);

/**
 * @brief Convert Neo list plugins response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_list_plugins_to_json(
    const neoc_neo_list_plugins_t *response,
    char **json_str
);

/**
 * @brief Get plugins array from response (convenience function)
 * 
 * @param response Response to get plugins from
 * @return Plugins array pointer, or NULL if error or no result
 */
neoc_neo_plugin_t *neoc_neo_list_plugins_get_plugins(
    const neoc_neo_list_plugins_t *response
);

/**
 * @brief Check if the response contains valid plugins data
 * 
 * @param response Response to check
 * @return true if response has valid plugins, false otherwise
 */
bool neoc_neo_list_plugins_has_plugins(
    const neoc_neo_list_plugins_t *response
);

/**
 * @brief Get number of plugins in the response
 * 
 * @param response Response to count plugins from
 * @return Number of plugins, or 0 on error
 */
size_t neoc_neo_list_plugins_get_count(
    const neoc_neo_list_plugins_t *response
);

/**
 * @brief Get plugin by index
 * 
 * @param response Response to get plugin from
 * @param index Index of the plugin to retrieve
 * @return Plugin pointer, or NULL if index out of bounds or error
 */
neoc_neo_plugin_t *neoc_neo_list_plugins_get_plugin(
    const neoc_neo_list_plugins_t *response,
    size_t index
);

/**
 * @brief Find plugin by name
 * 
 * @param response Response to search in
 * @param name Name of the plugin to find
 * @return Plugin pointer, or NULL if not found or error
 */
neoc_neo_plugin_t *neoc_neo_list_plugins_find_plugin(
    const neoc_neo_list_plugins_t *response,
    const char *name
);

/**
 * @brief Get plugin name
 * 
 * @param plugin Plugin to get name from
 * @return Plugin name, or NULL if error
 */
const char *neoc_neo_plugin_get_name(
    const neoc_neo_plugin_t *plugin
);

/**
 * @brief Get plugin version
 * 
 * @param plugin Plugin to get version from
 * @return Plugin version, or NULL if error
 */
const char *neoc_neo_plugin_get_version(
    const neoc_neo_plugin_t *plugin
);

/**
 * @brief Get plugin interfaces count
 * 
 * @param plugin Plugin to get interfaces count from
 * @return Number of interfaces, or 0 on error
 */
size_t neoc_neo_plugin_get_interfaces_count(
    const neoc_neo_plugin_t *plugin
);

/**
 * @brief Get plugin interface by index
 * 
 * @param plugin Plugin to get interface from
 * @param index Index of the interface
 * @return Interface name, or NULL if index out of bounds or error
 */
const char *neoc_neo_plugin_get_interface(
    const neoc_neo_plugin_t *plugin,
    size_t index
);

/**
 * @brief Check if plugin supports specific interface
 * 
 * @param plugin Plugin to check
 * @param interface_name Interface name to check for
 * @return true if plugin supports the interface, false otherwise
 */
bool neoc_neo_plugin_supports_interface(
    const neoc_neo_plugin_t *plugin,
    const char *interface_name
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_LIST_PLUGINS_H */
