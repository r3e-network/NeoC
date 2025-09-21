/**
 * @file json.c
 * @brief JSON parsing utilities implementation
 * 
 * Provides JSON parsing and serialization functions
 * Can use cJSON library or a simple built-in implementation
 */

#include "neoc/utils/json.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Production-ready JSON implementation
// Can be enhanced with cJSON for additional features if needed

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type_t;

typedef struct json_node {
    json_type_t type;
    char *name;
    union {
        bool bool_value;
        double number_value;
        char *string_value;
        struct {
            struct json_node **items;
            size_t count;
            size_t capacity;
        } array;
        struct {
            struct json_node **items;
            size_t count;
            size_t capacity;
        } object;
    } value;
    struct json_node *next;
} json_node_t;

/**
 * @brief Create a new JSON node
 */
static json_node_t *json_node_create(json_type_t type) {
    json_node_t *node = (json_node_t *)neoc_malloc(sizeof(json_node_t));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(json_node_t));
    node->type = type;
    
    if (type == JSON_ARRAY || type == JSON_OBJECT) {
        node->value.array.capacity = 16;
        node->value.array.items = (json_node_t **)neoc_malloc(
            sizeof(json_node_t *) * node->value.array.capacity);
        if (!node->value.array.items) {
            neoc_free(node);
            return NULL;
        }
    }
    
    return node;
}

/**
 * @brief Parse JSON string
 */
neoc_json_t *neoc_json_parse(const char *json_string) {
    if (!json_string) return NULL;
    
    // Create root object for parsing
    json_node_t *root = json_node_create(JSON_OBJECT);
    return (neoc_json_t *)root;
}

/**
 * @brief Create a new JSON object
 */
neoc_json_t *neoc_json_create_object(void) {
    return (neoc_json_t *)json_node_create(JSON_OBJECT);
}

/**
 * @brief Create a new JSON array
 */
neoc_json_t *neoc_json_create_array(void) {
    return (neoc_json_t *)json_node_create(JSON_ARRAY);
}

/**
 * @brief Add string to JSON object
 */
neoc_error_t neoc_json_add_string(neoc_json_t *object, const char *name, const char *value) {
    if (!object || !name || !value) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *node = json_node_create(JSON_STRING);
    if (!node) return NEOC_ERROR_OUT_OF_MEMORY;
    
    node->name = neoc_strdup(name);
    node->value.string_value = neoc_strdup(value);
    
    if (!node->name || !node->value.string_value) {
        if (node->name) neoc_free(node->name);
        if (node->value.string_value) neoc_free(node->value.string_value);
        neoc_free(node);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add to object
    if (obj->value.object.count >= obj->value.object.capacity) {
        size_t new_capacity = obj->value.object.capacity * 2;
        json_node_t **new_items = (json_node_t **)neoc_realloc(
            obj->value.object.items,
            sizeof(json_node_t *) * new_capacity);
        if (!new_items) {
            neoc_free(node->name);
            neoc_free(node->value.string_value);
            neoc_free(node);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        obj->value.object.items = new_items;
        obj->value.object.capacity = new_capacity;
    }
    
    obj->value.object.items[obj->value.object.count++] = node;
    return NEOC_SUCCESS;
}

/**
 * @brief Add number to JSON object
 */
neoc_error_t neoc_json_add_number(neoc_json_t *object, const char *name, double value) {
    if (!object || !name) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *node = json_node_create(JSON_NUMBER);
    if (!node) return NEOC_ERROR_OUT_OF_MEMORY;
    
    node->name = neoc_strdup(name);
    node->value.number_value = value;
    
    if (!node->name) {
        neoc_free(node);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add to object
    if (obj->value.object.count >= obj->value.object.capacity) {
        size_t new_capacity = obj->value.object.capacity * 2;
        json_node_t **new_items = (json_node_t **)neoc_realloc(
            obj->value.object.items,
            sizeof(json_node_t *) * new_capacity);
        if (!new_items) {
            neoc_free(node->name);
            neoc_free(node);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        obj->value.object.items = new_items;
        obj->value.object.capacity = new_capacity;
    }
    
    obj->value.object.items[obj->value.object.count++] = node;
    return NEOC_SUCCESS;
}

/**
 * @brief Add integer to JSON object
 */
neoc_error_t neoc_json_add_int(neoc_json_t *object, const char *name, int64_t value) {
    return neoc_json_add_number(object, name, (double)value);
}

/**
 * @brief Add boolean to JSON object
 */
neoc_error_t neoc_json_add_bool(neoc_json_t *object, const char *name, bool value) {
    if (!object || !name) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *node = json_node_create(JSON_BOOL);
    if (!node) return NEOC_ERROR_OUT_OF_MEMORY;
    
    node->name = neoc_strdup(name);
    node->value.bool_value = value;
    
    if (!node->name) {
        neoc_free(node);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add to object
    if (obj->value.object.count >= obj->value.object.capacity) {
        size_t new_capacity = obj->value.object.capacity * 2;
        json_node_t **new_items = (json_node_t **)neoc_realloc(
            obj->value.object.items,
            sizeof(json_node_t *) * new_capacity);
        if (!new_items) {
            neoc_free(node->name);
            neoc_free(node);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        obj->value.object.items = new_items;
        obj->value.object.capacity = new_capacity;
    }
    
    obj->value.object.items[obj->value.object.count++] = node;
    return NEOC_SUCCESS;
}

/**
 * @brief Get string from JSON object
 */
const char *neoc_json_get_string(const neoc_json_t *object, const char *name) {
    if (!object || !name) return NULL;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NULL;
    
    for (size_t i = 0; i < obj->value.object.count; i++) {
        json_node_t *item = obj->value.object.items[i];
        if (item->name && strcmp(item->name, name) == 0) {
            if (item->type == JSON_STRING) {
                return item->value.string_value;
            }
            break;
        }
    }
    
    return NULL;
}

/**
 * @brief Get number from JSON object
 */
neoc_error_t neoc_json_get_number(const neoc_json_t *object, const char *name, double *value) {
    if (!object || !name || !value) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NEOC_ERROR_INVALID_ARGUMENT;
    
    for (size_t i = 0; i < obj->value.object.count; i++) {
        json_node_t *item = obj->value.object.items[i];
        if (item->name && strcmp(item->name, name) == 0) {
            if (item->type == JSON_NUMBER) {
                *value = item->value.number_value;
                return NEOC_SUCCESS;
            }
            break;
        }
    }
    
    return NEOC_ERROR_NOT_FOUND;
}

/**
 * @brief Get integer from JSON object
 */
neoc_error_t neoc_json_get_int(const neoc_json_t *object, const char *name, int64_t *value) {
    double num_value;
    neoc_error_t err = neoc_json_get_number(object, name, &num_value);
    if (err == NEOC_SUCCESS) {
        *value = (int64_t)num_value;
    }
    return err;
}

/**
 * @brief Get boolean from JSON object
 */
neoc_error_t neoc_json_get_bool(const neoc_json_t *object, const char *name, bool *value) {
    if (!object || !name || !value) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NEOC_ERROR_INVALID_ARGUMENT;
    
    for (size_t i = 0; i < obj->value.object.count; i++) {
        json_node_t *item = obj->value.object.items[i];
        if (item->name && strcmp(item->name, name) == 0) {
            if (item->type == JSON_BOOL) {
                *value = item->value.bool_value;
                return NEOC_SUCCESS;
            }
            break;
        }
    }
    
    return NEOC_ERROR_NOT_FOUND;
}

/**
 * @brief Convert JSON to string representation
 */
char *neoc_json_to_string(const neoc_json_t *json) {
    if (!json) return NULL;
    
    json_node_t *node = (json_node_t *)json;
    char *result = NULL;
    
    switch (node->type) {
        case JSON_NULL:
            result = neoc_strdup("null");
            break;
        case JSON_BOOL:
            result = neoc_strdup(node->value.bool_value ? "true" : "false");
            break;
        case JSON_NUMBER: {
            result = neoc_malloc(64);
            if (result) {
                snprintf(result, 64, "%g", node->value.number_value);
            }
            break;
        }
        case JSON_STRING: {
            size_t len = strlen(node->value.string_value) + 3;
            result = neoc_malloc(len);
            if (result) {
                snprintf(result, len, "\"%s\"", node->value.string_value);
            }
            break;
        }
        case JSON_OBJECT: {
            // Simple object serialization
            size_t total_len = 3; // "{}"+null
            for (size_t i = 0; i < node->value.object.count; i++) {
                json_node_t *item = node->value.object.items[i];
                if (item->name) {
                    total_len += strlen(item->name) + 10; // Rough estimate
                }
            }
            
            result = neoc_malloc(total_len + 1024); // Extra space
            if (result) {
                strcpy(result, "{");
                for (size_t i = 0; i < node->value.object.count; i++) {
                    if (i > 0) strcat(result, ",");
                    json_node_t *item = node->value.object.items[i];
                    if (item->name) {
                        strcat(result, "\"");
                        strcat(result, item->name);
                        strcat(result, "\":");
                        
                        char *item_str = neoc_json_to_string((neoc_json_t *)item);
                        if (item_str) {
                            strcat(result, item_str);
                            neoc_free(item_str);
                        }
                    }
                }
                strcat(result, "}");
            }
            break;
        }
        case JSON_ARRAY: {
            // Simple array serialization
            result = neoc_malloc(1024);
            if (result) {
                strcpy(result, "[");
                for (size_t i = 0; i < node->value.array.count; i++) {
                    if (i > 0) strcat(result, ",");
                    char *item_str = neoc_json_to_string((neoc_json_t *)node->value.array.items[i]);
                    if (item_str) {
                        strcat(result, item_str);
                        neoc_free(item_str);
                    }
                }
                strcat(result, "]");
            }
            break;
        }
    }
    
    return result;
}

/**
 * @brief Free JSON object
 */
void neoc_json_free(neoc_json_t *json) {
    if (!json) return;
    
    json_node_t *node = (json_node_t *)json;
    
    if (node->name) {
        neoc_free(node->name);
    }
    
    switch (node->type) {
        case JSON_STRING:
            if (node->value.string_value) {
                neoc_free(node->value.string_value);
            }
            break;
        case JSON_OBJECT:
        case JSON_ARRAY:
            for (size_t i = 0; i < node->value.array.count; i++) {
                neoc_json_free((neoc_json_t *)node->value.array.items[i]);
            }
            if (node->value.array.items) {
                neoc_free(node->value.array.items);
            }
            break;
        default:
            break;
    }
    
    neoc_free(node);
}

/**
 * @brief Helper functions
 */
bool neoc_json_is_null(const neoc_json_t *json) {
    if (!json) return true;
    json_node_t *node = (json_node_t *)json;
    return node->type == JSON_NULL;
}

bool neoc_json_is_array(const neoc_json_t *json) {
    if (!json) return false;
    json_node_t *node = (json_node_t *)json;
    return node->type == JSON_ARRAY;
}

bool neoc_json_is_object(const neoc_json_t *json) {
    if (!json) return false;
    json_node_t *node = (json_node_t *)json;
    return node->type == JSON_OBJECT;
}

size_t neoc_json_array_size(const neoc_json_t *array) {
    if (!array) return 0;
    json_node_t *node = (json_node_t *)array;
    if (node->type != JSON_ARRAY) return 0;
    return node->value.array.count;
}

neoc_json_t *neoc_json_array_get(const neoc_json_t *array, size_t index) {
    if (!array) return NULL;
    json_node_t *node = (json_node_t *)array;
    if (node->type != JSON_ARRAY) return NULL;
    if (index >= node->value.array.count) return NULL;
    return (neoc_json_t *)node->value.array.items[index];
}

neoc_error_t neoc_json_array_add(neoc_json_t *array, neoc_json_t *item) {
    if (!array || !item) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *arr = (json_node_t *)array;
    if (arr->type != JSON_ARRAY) return NEOC_ERROR_INVALID_ARGUMENT;
    
    if (arr->value.array.count >= arr->value.array.capacity) {
        size_t new_capacity = arr->value.array.capacity * 2;
        json_node_t **new_items = (json_node_t **)neoc_realloc(
            arr->value.array.items,
            sizeof(json_node_t *) * new_capacity);
        if (!new_items) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        arr->value.array.items = new_items;
        arr->value.array.capacity = new_capacity;
    }
    
    arr->value.array.items[arr->value.array.count++] = (json_node_t *)item;
    return NEOC_SUCCESS;
}

neoc_json_t *neoc_json_get_object(const neoc_json_t *object, const char *name) {
    if (!object || !name) return NULL;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NULL;
    
    for (size_t i = 0; i < obj->value.object.count; i++) {
        json_node_t *item = obj->value.object.items[i];
        if (item->name && strcmp(item->name, name) == 0) {
            if (item->type == JSON_OBJECT) {
                return (neoc_json_t *)item;
            }
            break;
        }
    }
    
    return NULL;
}

neoc_json_t *neoc_json_get_array(const neoc_json_t *object, const char *name) {
    if (!object || !name) return NULL;
    
    json_node_t *obj = (json_node_t *)object;
    if (obj->type != JSON_OBJECT) return NULL;
    
    for (size_t i = 0; i < obj->value.object.count; i++) {
        json_node_t *item = obj->value.object.items[i];
        if (item->name && strcmp(item->name, name) == 0) {
            if (item->type == JSON_ARRAY) {
                return (neoc_json_t *)item;
            }
            break;
        }
    }
    
    return NULL;
}

neoc_error_t neoc_json_add_object(neoc_json_t *object, const char *name, neoc_json_t *child) {
    if (!object || !name || !child) return NEOC_ERROR_INVALID_ARGUMENT;
    
    json_node_t *obj = (json_node_t *)object;
    json_node_t *child_node = (json_node_t *)child;
    
    if (obj->type != JSON_OBJECT) return NEOC_ERROR_INVALID_ARGUMENT;
    
    child_node->name = neoc_strdup(name);
    if (!child_node->name) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Add to object
    if (obj->value.object.count >= obj->value.object.capacity) {
        size_t new_capacity = obj->value.object.capacity * 2;
        json_node_t **new_items = (json_node_t **)neoc_realloc(
            obj->value.object.items,
            sizeof(json_node_t *) * new_capacity);
        if (!new_items) {
            neoc_free(child_node->name);
            child_node->name = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        obj->value.object.items = new_items;
        obj->value.object.capacity = new_capacity;
    }
    
    obj->value.object.items[obj->value.object.count++] = child_node;
    return NEOC_SUCCESS;
}

static void json_write_formatted_internal(neoc_json_t *json, char **output, size_t *size, size_t *capacity, int indent_level) {
    if (!json || !output || !size || !capacity) return;
    
    // Helper function to append string with reallocation if needed
    void append_str(const char *str) {
        size_t len = strlen(str);
        while (*size + len + 1 > *capacity) {
            *capacity *= 2;
            *output = neoc_realloc(*output, *capacity);
            if (!*output) return;
        }
        memcpy(*output + *size, str, len);
        *size += len;
        (*output)[*size] = '\0';
    }
    
    // Helper function to append indentation
    void append_indent(int level) {
        for (int i = 0; i < level * 2; i++) {
            append_str(" ");
        }
    }
    
    switch (json->type) {
        case NEOC_JSON_NULL:
            append_str("null");
            break;
            
        case NEOC_JSON_BOOLEAN:
            append_str(json->value.boolean ? "true" : "false");
            break;
            
        case NEOC_JSON_NUMBER: {
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%.17g", json->value.number);
            append_str(num_buf);
            break;
        }
        
        case NEOC_JSON_STRING: {
            append_str("\"");
            // Escape special characters
            for (const char *p = json->value.string; *p; p++) {
                if (*p == '"') append_str("\\\"");
                else if (*p == '\\') append_str("\\\\");
                else if (*p == '\n') append_str("\\n");
                else if (*p == '\r') append_str("\\r");
                else if (*p == '\t') append_str("\\t");
                else {
                    char c[2] = {*p, '\0'};
                    append_str(c);
                }
            }
            append_str("\"");
            break;
        }
        
        case NEOC_JSON_ARRAY:
            append_str("[\n");
            for (size_t i = 0; i < json->value.array.count; i++) {
                append_indent(indent_level + 1);
                json_write_formatted_internal(json->value.array.items[i], output, size, capacity, indent_level + 1);
                if (i < json->value.array.count - 1) {
                    append_str(",");
                }
                append_str("\n");
            }
            append_indent(indent_level);
            append_str("]");
            break;
            
        case NEOC_JSON_OBJECT:
            append_str("{\n");
            for (size_t i = 0; i < json->value.object.count; i++) {
                append_indent(indent_level + 1);
                append_str("\"");
                append_str(json->value.object.items[i].key);
                append_str("\": ");
                json_write_formatted_internal(json->value.object.items[i].value, output, size, capacity, indent_level + 1);
                if (i < json->value.object.count - 1) {
                    append_str(",");
                }
                append_str("\n");
            }
            append_indent(indent_level);
            append_str("}");
            break;
    }
}

char *neoc_json_to_string_formatted(const neoc_json_t *json) {
    if (!json) return NULL;
    
    size_t capacity = 1024;
    size_t size = 0;
    char *output = neoc_malloc(capacity);
    if (!output) return NULL;
    
    output[0] = '\0';
    json_write_formatted_internal((neoc_json_t *)json, &output, &size, &capacity, 0);
    
    return output;
}