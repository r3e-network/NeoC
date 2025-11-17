#include "neoc/utils/json.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>

static char *copy_cjson_string(const char *input) {
    if (!input) {
        return NULL;
    }

    size_t len = strlen(input) + 1;
    char *buffer = neoc_malloc(len);
    if (!buffer) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate JSON string");
        return NULL;
    }
    memcpy(buffer, input, len);
    return buffer;
}

neoc_json_t *neoc_json_parse(const char *json_string) {
    if (!json_string) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON parse: NULL input");
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_string);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            neoc_error_set(NEOC_ERROR_INVALID_FORMAT, error_ptr);
        } else {
            neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse JSON string");
        }
    }
    return (neoc_json_t *)root;
}

neoc_json_t *neoc_json_create_object(void) {
    cJSON *object = cJSON_CreateObject();
    if (!object) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create JSON object");
    }
    return (neoc_json_t *)object;
}

neoc_json_t *neoc_json_create_array(void) {
    cJSON *array = cJSON_CreateArray();
    if (!array) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create JSON array");
    }
    return (neoc_json_t *)array;
}

neoc_error_t neoc_json_add_string(neoc_json_t *object, const char *name, const char *value) {
    if (!object || !name || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON add string: invalid arguments");
    }

    if (!cJSON_AddStringToObject((cJSON *)object, name, value)) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to add string to JSON object");
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_json_add_number(neoc_json_t *object, const char *name, double value) {
    if (!object || !name) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON add number: invalid arguments");
    }

    if (!cJSON_AddNumberToObject((cJSON *)object, name, value)) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to add number to JSON object");
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_json_add_int(neoc_json_t *object, const char *name, int64_t value) {
    return neoc_json_add_number(object, name, (double)value);
}

neoc_error_t neoc_json_add_bool(neoc_json_t *object, const char *name, bool value) {
    if (!object || !name) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON add bool: invalid arguments");
    }

    if (!cJSON_AddBoolToObject((cJSON *)object, name, value)) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to add bool to JSON object");
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_json_add_object(neoc_json_t *object, const char *name, neoc_json_t *child) {
    if (!object || !name || !child) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON add object: invalid arguments");
    }

    if (!cJSON_IsObject((cJSON *)object)) {
        return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "JSON add object: parent is not object");
    }

    cJSON_AddItemToObject((cJSON *)object, name, (cJSON *)child);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_json_array_add(neoc_json_t *array, neoc_json_t *item) {
    if (!array || !item) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON array add: invalid arguments");
    }

    if (!cJSON_IsArray((cJSON *)array)) {
        return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "JSON array add: target is not array");
    }

    cJSON_AddItemToArray((cJSON *)array, (cJSON *)item);
    return NEOC_SUCCESS;
}

const char *neoc_json_get_string(const neoc_json_t *object, const char *name) {
    if (!object || !name) {
        return NULL;
    }

    const cJSON *item = cJSON_GetObjectItemCaseSensitive((const cJSON *)object, name);
    if (!item || !cJSON_IsString(item) || item->valuestring == NULL) {
        return NULL;
    }

    return item->valuestring;
}

neoc_error_t neoc_json_get_number(const neoc_json_t *object, const char *name, double *value) {
    if (!object || !name || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON get number: invalid arguments");
    }

    const cJSON *item = cJSON_GetObjectItemCaseSensitive((const cJSON *)object, name);
    if (!item) {
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "JSON get number: item not found");
    }

    if (!cJSON_IsNumber(item)) {
        return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "JSON get number: item is not numeric");
    }

    *value = item->valuedouble;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_json_get_int(const neoc_json_t *object, const char *name, int64_t *value) {
    double temp = 0.0;
    neoc_error_t err = neoc_json_get_number(object, name, &temp);
    if (err == NEOC_SUCCESS) {
        *value = (int64_t)temp;
    }
    return err;
}

neoc_error_t neoc_json_get_bool(const neoc_json_t *object, const char *name, bool *value) {
    if (!object || !name || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "JSON get bool: invalid arguments");
    }

    const cJSON *item = cJSON_GetObjectItemCaseSensitive((const cJSON *)object, name);
    if (!item) {
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "JSON get bool: item not found");
    }

    if (!cJSON_IsBool(item)) {
        return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "JSON get bool: item is not boolean");
    }

    *value = cJSON_IsTrue(item);
    return NEOC_SUCCESS;
}

neoc_json_t *neoc_json_get_object(const neoc_json_t *object, const char *name) {
    if (!object || !name) {
        return NULL;
    }
    const cJSON *item = cJSON_GetObjectItemCaseSensitive((const cJSON *)object, name);
    if (!item || !cJSON_IsObject(item)) {
        return NULL;
    }
    return (neoc_json_t *)item;
}

neoc_json_t *neoc_json_get_array(const neoc_json_t *object, const char *name) {
    if (!object || !name) {
        return NULL;
    }
    const cJSON *item = cJSON_GetObjectItemCaseSensitive((const cJSON *)object, name);
    if (!item || !cJSON_IsArray(item)) {
        return NULL;
    }
    return (neoc_json_t *)item;
}

size_t neoc_json_array_size(const neoc_json_t *array) {
    if (!array || !cJSON_IsArray((const cJSON *)array)) {
        return 0;
    }
    return (size_t)cJSON_GetArraySize((const cJSON *)array);
}

neoc_json_t *neoc_json_array_get(const neoc_json_t *array, size_t index) {
    if (!array || !cJSON_IsArray((const cJSON *)array)) {
        return NULL;
    }
    cJSON *item = cJSON_GetArrayItem((cJSON *)array, (int)index);
    return (neoc_json_t *)item;
}

char *neoc_json_to_string(const neoc_json_t *json) {
    if (!json) {
        return NULL;
    }

    char *printed = cJSON_PrintUnformatted((const cJSON *)json);
    if (!printed) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to serialize JSON");
        return NULL;
    }

    char *copy = copy_cjson_string(printed);
    cJSON_free(printed);
    return copy;
}

char *neoc_json_to_string_formatted(const neoc_json_t *json) {
    if (!json) {
        return NULL;
    }

    char *printed = cJSON_Print((const cJSON *)json);
    if (!printed) {
        neoc_error_set(NEOC_ERROR_MEMORY, "Failed to serialize JSON (formatted)");
        return NULL;
    }

    char *copy = copy_cjson_string(printed);
    cJSON_free(printed);
    return copy;
}

void neoc_json_free(neoc_json_t *json) {
    if (!json) {
        return;
    }
    cJSON_Delete((cJSON *)json);
}

bool neoc_json_is_null(const neoc_json_t *json) {
    if (!json) {
        return true;
    }
    return cJSON_IsNull((const cJSON *)json);
}

bool neoc_json_is_array(const neoc_json_t *json) {
    if (!json) {
        return false;
    }
    return cJSON_IsArray((const cJSON *)json);
}

bool neoc_json_is_object(const neoc_json_t *json) {
    if (!json) {
        return false;
    }
    return cJSON_IsObject((const cJSON *)json);
}

#else /* HAVE_CJSON */

neoc_json_t *neoc_json_parse(const char *json_string) {
    (void)json_string;
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
    return NULL;
}

neoc_json_t *neoc_json_create_object(void) {
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
    return NULL;
}

neoc_json_t *neoc_json_create_array(void) {
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
    return NULL;
}

neoc_error_t neoc_json_add_string(neoc_json_t *object, const char *name, const char *value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_add_number(neoc_json_t *object, const char *name, double value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_add_int(neoc_json_t *object, const char *name, int64_t value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_add_bool(neoc_json_t *object, const char *name, bool value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_add_object(neoc_json_t *object, const char *name, neoc_json_t *child) {
    (void)object; (void)name; (void)child;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_array_add(neoc_json_t *array, neoc_json_t *item) {
    (void)array; (void)item;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

const char *neoc_json_get_string(const neoc_json_t *object, const char *name) {
    (void)object; (void)name;
    return NULL;
}

neoc_error_t neoc_json_get_number(const neoc_json_t *object, const char *name, double *value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_get_int(const neoc_json_t *object, const char *name, int64_t *value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_error_t neoc_json_get_bool(const neoc_json_t *object, const char *name, bool *value) {
    (void)object; (void)name; (void)value;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
}

neoc_json_t *neoc_json_get_object(const neoc_json_t *object, const char *name) {
    (void)object; (void)name;
    return NULL;
}

neoc_json_t *neoc_json_get_array(const neoc_json_t *object, const char *name) {
    (void)object; (void)name;
    return NULL;
}

size_t neoc_json_array_size(const neoc_json_t *array) {
    (void)array;
    return 0;
}

neoc_json_t *neoc_json_array_get(const neoc_json_t *array, size_t index) {
    (void)array; (void)index;
    return NULL;
}

char *neoc_json_to_string(const neoc_json_t *json) {
    (void)json;
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
    return NULL;
}

char *neoc_json_to_string_formatted(const neoc_json_t *json) {
    (void)json;
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON support not compiled");
    return NULL;
}

void neoc_json_free(neoc_json_t *json) {
    (void)json;
}

bool neoc_json_is_null(const neoc_json_t *json) {
    return json == NULL;
}

bool neoc_json_is_array(const neoc_json_t *json) {
    (void)json;
    return false;
}

bool neoc_json_is_object(const neoc_json_t *json) {
    (void)json;
    return false;
}

#endif /* HAVE_CJSON */
