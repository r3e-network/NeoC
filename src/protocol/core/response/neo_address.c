/**
 * @file neo_address.c
 * @brief Neo address helpers aligned with NeoSwift NeoAddress
 */

#include "neoc/protocol/core/response/neo_address.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static neoc_error_t neoc_neo_address_allocate(neoc_neo_address_t **out) {
    if (!out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: output pointer missing");
    }

    neoc_neo_address_t *address = neoc_calloc(1, sizeof(neoc_neo_address_t));
    if (!address) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: allocation failed");
    }

    *out = address;
    return NEOC_SUCCESS;
}

static void neoc_neo_address_reset(neoc_neo_address_t *addr) {
    if (!addr) {
        return;
    }
    if (addr->address) {
        neoc_free(addr->address);
        addr->address = NULL;
    }
    if (addr->label) {
        neoc_free(addr->label);
        addr->label = NULL;
    }
}

neoc_error_t neoc_neo_address_create(const char *address,
                                     bool has_key,
                                     const char *label,
                                     bool watch_only,
                                     neoc_neo_address_t **neo_address) {
    if (!address || !neo_address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: invalid create arguments");
    }

    neoc_neo_address_t *result = NULL;
    neoc_error_t err = neoc_neo_address_allocate(&result);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    result->address = neoc_strdup(address);
    if (!result->address) {
        neoc_neo_address_free(result);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: address copy failed");
    }

    result->has_key = has_key;
    result->watch_only = watch_only;

    if (label) {
        result->label = neoc_strdup(label);
        if (!result->label) {
            neoc_neo_address_free(result);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: label copy failed");
        }
    }

    *neo_address = result;
    return NEOC_SUCCESS;
}

void neoc_neo_address_free(neoc_neo_address_t *neo_address) {
    if (!neo_address) {
        return;
    }
    neoc_neo_address_reset(neo_address);
    neoc_free(neo_address);
}

neoc_error_t neoc_neo_address_copy(const neoc_neo_address_t *source,
                                   neoc_neo_address_t **copy) {
    if (!source || !copy) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: copy arguments invalid");
    }
    return neoc_neo_address_create(source->address,
                                   source->has_key,
                                   source->label,
                                   source->watch_only,
                                   copy);
}

const char *neoc_neo_address_get_address(const neoc_neo_address_t *neo_address) {
    return neo_address ? neo_address->address : NULL;
}

bool neoc_neo_address_has_key(const neoc_neo_address_t *neo_address) {
    return neo_address ? neo_address->has_key : false;
}

const char *neoc_neo_address_get_label(const neoc_neo_address_t *neo_address) {
    return neo_address ? neo_address->label : NULL;
}

bool neoc_neo_address_is_watch_only(const neoc_neo_address_t *neo_address) {
    return neo_address ? neo_address->watch_only : false;
}

neoc_error_t neoc_neo_address_set_label(neoc_neo_address_t *neo_address,
                                        const char *label) {
    if (!neo_address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: set_label state missing");
    }

    char *copy = NULL;
    if (label) {
        copy = neoc_strdup(label);
        if (!copy) {
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: label allocation failed");
        }
    }

    if (neo_address->label) {
        neoc_free(neo_address->label);
    }
    neo_address->label = copy;
    return NEOC_SUCCESS;
}

#ifdef HAVE_CJSON
static neoc_error_t neoc_neo_address_parse_bool(const cJSON *json,
                                                const char *key,
                                                bool *value) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (!item) {
        *value = false;
        return NEOC_SUCCESS;
    }

    if (cJSON_IsBool(item)) {
        *value = cJSON_IsTrue(item);
        return NEOC_SUCCESS;
    }

    if (cJSON_IsNumber(item)) {
        *value = item->valueint != 0;
        return NEOC_SUCCESS;
    }

    if (cJSON_IsString(item) && item->valuestring) {
        *value = strcmp(item->valuestring, "true") == 0 || strcmp(item->valuestring, "1") == 0;
        return NEOC_SUCCESS;
    }

    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "neo_address: invalid boolean token");
}
#endif

neoc_error_t neoc_neo_address_from_json(const char *json_str,
                                        neoc_neo_address_t **neo_address) {
    if (!json_str || !neo_address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: from_json arguments invalid");
    }

#ifndef HAVE_CJSON
    (void)json_str;
    (void)neo_address;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "neo_address: cJSON not available");
#else
    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "neo_address: JSON is not an object");
    }

    const cJSON *address_item = cJSON_GetObjectItemCaseSensitive(root, "address");
    if (!address_item || !cJSON_IsString(address_item) || !address_item->valuestring) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "neo_address: missing address field");
    }

    bool has_key = false;
    neoc_error_t err = neoc_neo_address_parse_bool(root, "haskey", &has_key);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return err;
    }

    bool watch_only = false;
    err = neoc_neo_address_parse_bool(root, "watchonly", &watch_only);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return err;
    }

    const cJSON *label_item = cJSON_GetObjectItemCaseSensitive(root, "label");
    const char *label = NULL;
    if (label_item && cJSON_IsString(label_item) && label_item->valuestring) {
        label = label_item->valuestring;
    }

    neoc_error_t create_err = neoc_neo_address_create(address_item->valuestring,
                                                      has_key,
                                                      label,
                                                      watch_only,
                                                      neo_address);
    cJSON_Delete(root);
    return create_err;
#endif
}

neoc_error_t neoc_neo_address_to_json(const neoc_neo_address_t *neo_address,
                                      char **json_str) {
    if (!neo_address || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: to_json arguments invalid");
    }

#ifndef HAVE_CJSON
    (void)neo_address;
    (void)json_str;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "neo_address: serialization requires cJSON");
#else
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: JSON object allocation failed");
    }

    cJSON_AddStringToObject(root, "address", neo_address->address ? neo_address->address : "");
    cJSON_AddBoolToObject(root, "haskey", neo_address->has_key);
    cJSON_AddBoolToObject(root, "watchonly", neo_address->watch_only);

    if (neo_address->label) {
        cJSON_AddStringToObject(root, "label", neo_address->label);
    }

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!*json_str) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: JSON print failed");
    }
    return NEOC_SUCCESS;
#endif
}

bool neoc_neo_address_equals(const neoc_neo_address_t *address1,
                             const neoc_neo_address_t *address2) {
    if (address1 == address2) {
        return true;
    }
    if (!address1 || !address2) {
        return false;
    }

    if (address1->has_key != address2->has_key || address1->watch_only != address2->watch_only) {
        return false;
    }

    if ((address1->address && !address2->address) || (!address1->address && address2->address)) {
        return false;
    }
    if (address1->address && strcmp(address1->address, address2->address) != 0) {
        return false;
    }

    if ((address1->label && !address2->label) || (!address1->label && address2->label)) {
        return false;
    }
    if (address1->label && strcmp(address1->label, address2->label) != 0) {
        return false;
    }

    return true;
}

neoc_error_t neoc_neo_address_copy_address_string(const neoc_neo_address_t *neo_address,
                                                  char **address_copy) {
    if (!neo_address || !address_copy || !neo_address->address) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_address: copy_address_string arguments invalid");
    }

    char *copy = neoc_strdup(neo_address->address);
    if (!copy) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_address: address duplication failed");
    }

    *address_copy = copy;
    return NEOC_SUCCESS;
}

