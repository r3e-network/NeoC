#include "neoc/protocol/core/response/contract_storage_entry.h"

#include "neoc/utils/json.h"
#include "neoc/utils/neoc_hex.h"

#include <string.h>

static neoc_contract_storage_entry_t *neoc_contract_storage_entry_alloc(void) {
    return (neoc_contract_storage_entry_t *)neoc_calloc(
        1, sizeof(neoc_contract_storage_entry_t));
}

static void neoc_contract_storage_entry_free_internal(
    neoc_contract_storage_entry_t *entry) {
    if (!entry) {
        return;
    }
    if (entry->key) {
        neoc_free(entry->key);
    }
    if (entry->value) {
        neoc_free(entry->value);
    }
    neoc_free(entry);
}

static neoc_error_t neoc_contract_storage_entry_set_bytes(
    const uint8_t *key_bytes,
    size_t key_len,
    const uint8_t *value_bytes,
    size_t value_len,
    neoc_contract_storage_entry_t **entry) {
    if (!entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Entry output pointer is NULL");
    }

    *entry = neoc_contract_storage_entry_alloc();
    if (!*entry) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to allocate storage entry");
    }

    if (key_len > 0) {
        (*entry)->key = (uint8_t *)neoc_malloc(key_len);
        if (!(*entry)->key) {
            neoc_contract_storage_entry_free_internal(*entry);
            *entry = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY,
                                  "Failed to allocate storage key");
        }
        memcpy((*entry)->key, key_bytes, key_len);
        (*entry)->key_length = key_len;
    }

    if (value_len > 0) {
        (*entry)->value = (uint8_t *)neoc_malloc(value_len);
        if (!(*entry)->value) {
            neoc_contract_storage_entry_free_internal(*entry);
            *entry = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY,
                                  "Failed to allocate storage value");
        }
        memcpy((*entry)->value, value_bytes, value_len);
        (*entry)->value_length = value_len;
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_storage_entry_create_swift(
    const char *key,
    const char *value,
    neoc_contract_storage_entry_t **entry) {
    if (!key || !value || !entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid contract storage arguments");
    }

    size_t key_len = 0;
    size_t value_len = 0;
    uint8_t *key_bytes = neoc_hex_decode_alloc(key, &key_len);
    if (!key_bytes && key_len > 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_HEX,
                              "Failed to decode storage key");
    }

    uint8_t *value_bytes = neoc_hex_decode_alloc(value, &value_len);
    if (!value_bytes && value_len > 0) {
        if (key_bytes) {
            neoc_free(key_bytes);
        }
        return neoc_error_set(NEOC_ERROR_INVALID_HEX,
                              "Failed to decode storage value");
    }

    neoc_error_t err =
        neoc_contract_storage_entry_set_bytes(key_bytes, key_len,
                                              value_bytes, value_len, entry);

    if (key_bytes) {
        neoc_free(key_bytes);
    }
    if (value_bytes) {
        neoc_free(value_bytes);
    }

    return err;
}

neoc_error_t neoc_contract_storage_entry_from_json_swift(
    const char *json_str,
    neoc_contract_storage_entry_t **entry) {
    if (!json_str || !entry) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid JSON storage entry arguments");
    }

    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT,
                              "Failed to parse storage entry JSON");
    }

    const char *key = neoc_json_get_string(json, "key");
    const char *value = neoc_json_get_string(json, "value");

    if (!key || !value) {
        neoc_json_free(json);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT,
                              "Storage entry JSON missing key or value");
    }

    neoc_error_t err =
        neoc_contract_storage_entry_create_swift(key, value, entry);
    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_contract_storage_entry_to_json_swift(
    const neoc_contract_storage_entry_t *entry,
    char **json_str) {
    if (!entry || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid entry for JSON serialization");
    }

    *json_str = NULL;
    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                              "JSON support not available");
    }

    char *key_hex =
        neoc_hex_encode_alloc(entry->key, entry->key_length, false, false);
    char *value_hex =
        neoc_hex_encode_alloc(entry->value, entry->value_length, false, false);

    if ((!key_hex && entry->key_length > 0) ||
        (!value_hex && entry->value_length > 0)) {
        if (key_hex) neoc_free(key_hex);
        if (value_hex) neoc_free(value_hex);
        neoc_json_free(root);
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to encode storage hex values");
    }

    neoc_error_t err = NEOC_SUCCESS;
    err = neoc_json_add_string(root, "key", key_hex ? key_hex : "");
    if (err == NEOC_SUCCESS) {
        err = neoc_json_add_string(root, "value", value_hex ? value_hex : "");
    }

    if (key_hex) {
        neoc_free(key_hex);
    }
    if (value_hex) {
        neoc_free(value_hex);
    }

    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    char *serialized = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!serialized) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to serialize storage entry JSON");
    }

    *json_str = serialized;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_storage_entry_copy_swift(
    const neoc_contract_storage_entry_t *src,
    neoc_contract_storage_entry_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid storage copy arguments");
    }

    return neoc_contract_storage_entry_set_bytes(src->key, src->key_length,
                                                 src->value,
                                                 src->value_length, dest);
}

bool neoc_contract_storage_entry_equals_swift(
    const neoc_contract_storage_entry_t *entry1,
    const neoc_contract_storage_entry_t *entry2) {
    if (entry1 == entry2) {
        return true;
    }
    if (!entry1 || !entry2) {
        return false;
    }

    if (entry1->key_length != entry2->key_length ||
        entry1->value_length != entry2->value_length) {
        return false;
    }

    if ((entry1->key_length > 0 &&
         memcmp(entry1->key, entry2->key, entry1->key_length) != 0) ||
        (entry1->value_length > 0 &&
         memcmp(entry1->value, entry2->value, entry1->value_length) != 0)) {
        return false;
    }

    return true;
}

neoc_error_t neoc_contract_storage_entry_get_key_string(
    const neoc_contract_storage_entry_t *entry,
    char **key_str) {
    if (!entry || !key_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid arguments for key string");
    }

    *key_str = neoc_hex_encode_alloc(entry->key, entry->key_length, false,
                                     false);
    if (!*key_str && entry->key_length > 0) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to encode storage key");
    }

    if (!*key_str) {
        *key_str = neoc_strdup("");
        if (!*key_str) {
            return neoc_error_set(NEOC_ERROR_MEMORY,
                                  "Failed to allocate empty key string");
        }
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_contract_storage_entry_get_value_string(
    const neoc_contract_storage_entry_t *entry,
    char **value_str) {
    if (!entry || !value_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "Invalid arguments for value string");
    }

    *value_str = neoc_hex_encode_alloc(entry->value, entry->value_length,
                                       false, false);
    if (!*value_str && entry->value_length > 0) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to encode storage value");
    }

    if (!*value_str) {
        *value_str = neoc_strdup("");
        if (!*value_str) {
            return neoc_error_set(NEOC_ERROR_MEMORY,
                                  "Failed to allocate empty value string");
        }
    }

    return NEOC_SUCCESS;
}
