/**
 * @file neo_list_plugins.c
 * @brief listplugins RPC response implementation
 */

#include "neoc/protocol/core/response/neo_list_plugins.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

static neoc_error_t copy_interfaces(const char **interfaces,
                                    size_t interfaces_count,
                                    char ***out_interfaces) {
    if (!out_interfaces) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out_interfaces = NULL;

    if (!interfaces || interfaces_count == 0) {
        return NEOC_SUCCESS;
    }

    char **copy = neoc_calloc(interfaces_count, sizeof(char *));
    if (!copy) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < interfaces_count; i++) {
        copy[i] = dup_string(interfaces[i]);
        if (!copy[i]) {
            for (size_t j = 0; j <= i; j++) {
                neoc_free(copy[j]);
            }
            neoc_free(copy);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *out_interfaces = copy;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_plugin_create(const char *name,
                                    const char *version,
                                    const char **interfaces,
                                    size_t interfaces_count,
                                    neoc_neo_plugin_t **plugin) {
    if (!plugin || !name || !version) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *plugin = neoc_calloc(1, sizeof(neoc_neo_plugin_t));
    if (!*plugin) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*plugin)->name = dup_string(name);
    (*plugin)->version = dup_string(version);
    if (!(*plugin)->name || !(*plugin)->version) {
        neoc_neo_plugin_free(*plugin);
        neoc_free(*plugin);
        *plugin = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_error_t err = copy_interfaces(interfaces, interfaces_count, &(*plugin)->interfaces);
    if (err != NEOC_SUCCESS) {
        neoc_neo_plugin_free(*plugin);
        neoc_free(*plugin);
        *plugin = NULL;
        return err;
    }
    (*plugin)->interfaces_count = interfaces_count;
    return NEOC_SUCCESS;
}

void neoc_neo_plugin_free(neoc_neo_plugin_t *plugin) {
    if (!plugin) {
        return;
    }
    neoc_free(plugin->name);
    neoc_free(plugin->version);
    if (plugin->interfaces) {
        for (size_t i = 0; i < plugin->interfaces_count; i++) {
            neoc_free(plugin->interfaces[i]);
        }
        neoc_free(plugin->interfaces);
    }
}

neoc_error_t neoc_neo_list_plugins_create(int id,
                                          neoc_neo_plugin_t *plugins,
                                          size_t plugins_count,
                                          const char *error,
                                          int error_code,
                                          neoc_neo_list_plugins_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = neoc_calloc(1, sizeof(neoc_neo_list_plugins_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->jsonrpc = dup_string("2.0");
    if (!(*response)->jsonrpc) {
        neoc_free(*response);
        *response = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->id = id;
    (*response)->result = plugins;
    (*response)->result_count = plugins_count;
    (*response)->error_code = error_code;

    if (error) {
        (*response)->error = dup_string(error);
        if (!(*response)->error) {
            neoc_neo_list_plugins_free(*response);
            *response = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    return NEOC_SUCCESS;
}

void neoc_neo_list_plugins_free(neoc_neo_list_plugins_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        for (size_t i = 0; i < response->result_count; i++) {
            neoc_neo_plugin_free(&response->result[i]);
        }
        neoc_free(response->result);
    }
    neoc_free(response);
}

static neoc_error_t parse_interfaces(neoc_json_t *array,
                                     char ***interfaces_out,
                                     size_t *count_out) {
    if (!interfaces_out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *interfaces_out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }

    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    char **interfaces = neoc_calloc(count, sizeof(char *));
    if (!interfaces) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        const char *iface = entry ? neoc_json_get_string(entry, "value") : NULL;
        if (!iface) {
            /* entry may simply be a string, not an object */
            iface = entry ? neoc_json_get_string(entry, "interface") : NULL;
        }
        if (!iface) {
            iface = entry ? neoc_json_get_string(entry, "name") : NULL;
        }
        if (!iface && entry) {
            iface = neoc_json_get_string(entry, "");
        }
        if (!iface) {
            iface = entry ? neoc_json_get_string(entry, NULL) : NULL;
        }
        if (!iface) {
            /* Fallback to raw array item as string */
            iface = neoc_json_get_string(array, "");
        }

        iface = entry && neoc_json_is_array(entry) ? NULL : iface;
        if (!iface && entry) {
            iface = neoc_json_get_string(entry, ""); /* attempt empty key */
        }

        if (!iface && entry) {
            /* Try treating element as string directly */
            iface = neoc_json_get_string(entry, NULL);
        }

        const char *chosen = iface;
        if (!chosen && entry) {
            /* As last resort, serialize the element */
            chosen = neoc_json_get_string(entry, "interface");
        }

        if (!chosen) {
            neoc_free(interfaces);
            return NEOC_ERROR_INVALID_FORMAT;
        }

        interfaces[i] = dup_string(chosen);
        if (!interfaces[i]) {
            for (size_t j = 0; j <= i; j++) {
                neoc_free(interfaces[j]);
            }
            neoc_free(interfaces);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *interfaces_out = interfaces;
    *count_out = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_list_plugins_from_json(const char *json_str,
                                             neoc_neo_list_plugins_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_list_plugins_t *parsed = neoc_calloc(1, sizeof(neoc_neo_list_plugins_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_string(neoc_json_get_string(json, "jsonrpc"));
    if (!parsed->jsonrpc) {
        parsed->jsonrpc = dup_string("2.0");
    }

    int64_t id_val = 0;
    if (neoc_json_get_int(json, "id", &id_val) == NEOC_SUCCESS) {
        parsed->id = (int)id_val;
    }

    neoc_json_t *error_obj = neoc_json_get_object(json, "error");
    if (error_obj) {
        int64_t code = 0;
        if (neoc_json_get_int(error_obj, "code", &code) == NEOC_SUCCESS) {
            parsed->error_code = (int)code;
        }
        const char *message = neoc_json_get_string(error_obj, "message");
        if (message) {
            parsed->error = dup_string(message);
        }
    }

    neoc_json_t *result_array = neoc_json_get_array(json, "result");
    if (!result_array) {
        result_array = neoc_json_get_array(json, "plugins");
    }

    if (result_array) {
        size_t count = neoc_json_array_size(result_array);
        if (count > 0) {
            neoc_neo_plugin_t *plugins = neoc_calloc(count, sizeof(neoc_neo_plugin_t));
            if (!plugins) {
                neoc_neo_list_plugins_free(parsed);
                neoc_json_free(json);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }

            for (size_t i = 0; i < count; i++) {
                neoc_json_t *entry = neoc_json_array_get(result_array, i);
                const char *name = entry ? neoc_json_get_string(entry, "name") : NULL;
                const char *version = entry ? neoc_json_get_string(entry, "version") : NULL;
                neoc_json_t *interfaces_arr = entry ? neoc_json_get_array(entry, "interfaces") : NULL;

                plugins[i].name = dup_string(name ? name : "");
                plugins[i].version = dup_string(version ? version : "");
                parse_interfaces(interfaces_arr, &plugins[i].interfaces, &plugins[i].interfaces_count);

                if (!plugins[i].name || !plugins[i].version) {
                    for (size_t j = 0; j <= i; j++) {
                        neoc_neo_plugin_free(&plugins[j]);
                    }
                    neoc_free(plugins);
                    neoc_neo_list_plugins_free(parsed);
                    neoc_json_free(json);
                    return NEOC_ERROR_OUT_OF_MEMORY;
                }
            }

            parsed->result = plugins;
            parsed->result_count = count;
            parsed->error_code = 0;
        }
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_neo_plugin_t *neoc_neo_list_plugins_get_plugins(const neoc_neo_list_plugins_t *response) {
    return response ? response->result : NULL;
}

bool neoc_neo_list_plugins_has_plugins(const neoc_neo_list_plugins_t *response) {
    return response && response->result && response->result_count > 0 && response->error == NULL;
}

size_t neoc_neo_list_plugins_get_count(const neoc_neo_list_plugins_t *response) {
    return response ? response->result_count : 0;
}

neoc_neo_plugin_t *neoc_neo_list_plugins_get_plugin(const neoc_neo_list_plugins_t *response,
                                                    size_t index) {
    if (!response || !response->result || index >= response->result_count) {
        return NULL;
    }
    return &response->result[index];
}

neoc_neo_plugin_t *neoc_neo_list_plugins_find_plugin(const neoc_neo_list_plugins_t *response,
                                                     const char *name) {
    if (!response || !response->result || !name) {
        return NULL;
    }
    for (size_t i = 0; i < response->result_count; i++) {
        const char *plugin_name = response->result[i].name ? response->result[i].name : "";
        if (strcmp(plugin_name, name) == 0) {
            return &response->result[i];
        }
    }
    return NULL;
}

const char *neoc_neo_plugin_get_name(const neoc_neo_plugin_t *plugin) {
    return plugin ? plugin->name : NULL;
}

const char *neoc_neo_plugin_get_version(const neoc_neo_plugin_t *plugin) {
    return plugin ? plugin->version : NULL;
}

size_t neoc_neo_plugin_get_interfaces_count(const neoc_neo_plugin_t *plugin) {
    return plugin ? plugin->interfaces_count : 0;
}

const char *neoc_neo_plugin_get_interface(const neoc_neo_plugin_t *plugin,
                                          size_t index) {
    if (!plugin || !plugin->interfaces || index >= plugin->interfaces_count) {
        return NULL;
    }
    return plugin->interfaces[index];
}

bool neoc_neo_plugin_supports_interface(const neoc_neo_plugin_t *plugin,
                                        const char *interface_name) {
    if (!plugin || !interface_name || !plugin->interfaces) {
        return false;
    }
    for (size_t i = 0; i < plugin->interfaces_count; i++) {
        const char *iface = plugin->interfaces[i] ? plugin->interfaces[i] : "";
        if (strcmp(iface, interface_name) == 0) {
            return true;
        }
    }
    return false;
}

static neoc_error_t interfaces_to_json(const neoc_neo_plugin_t *plugin, neoc_json_t *obj) {
    if (!plugin || !obj) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (plugin->interfaces && plugin->interfaces_count > 0) {
        neoc_json_t *arr = neoc_json_create_array();
        if (!arr) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        for (size_t i = 0; i < plugin->interfaces_count; i++) {
            if (plugin->interfaces[i]) {
                neoc_json_t *item = neoc_json_create_object();
                if (!item) {
                    neoc_json_free(arr);
                    return NEOC_ERROR_OUT_OF_MEMORY;
                }
                neoc_json_add_string(item, "name", plugin->interfaces[i]);
                neoc_json_array_add(arr, item);
            }
        }
        neoc_json_add_object(obj, "interfaces", arr);
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_list_plugins_to_json(const neoc_neo_list_plugins_t *response,
                                           char **json_str) {
    if (!response || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (response->jsonrpc) {
        neoc_json_add_string(root, "jsonrpc", response->jsonrpc);
    }
    neoc_json_add_int(root, "id", response->id);

    if (response->error) {
        neoc_json_t *error_obj = neoc_json_create_object();
        if (!error_obj) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        neoc_json_add_int(error_obj, "code", response->error_code);
        neoc_json_add_string(error_obj, "message", response->error);
        neoc_json_add_object(root, "error", error_obj);
    } else if (response->result && response->result_count > 0) {
        neoc_json_t *arr = neoc_json_create_array();
        if (!arr) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        for (size_t i = 0; i < response->result_count; i++) {
            const neoc_neo_plugin_t *plugin = &response->result[i];
            neoc_json_t *obj = neoc_json_create_object();
            if (!obj) {
                neoc_json_free(arr);
                neoc_json_free(root);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            if (plugin->name) {
                neoc_json_add_string(obj, "name", plugin->name);
            }
            if (plugin->version) {
                neoc_json_add_string(obj, "version", plugin->version);
            }
            interfaces_to_json(plugin, obj);
            neoc_json_array_add(arr, obj);
        }
        neoc_json_add_object(root, "result", arr);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}
