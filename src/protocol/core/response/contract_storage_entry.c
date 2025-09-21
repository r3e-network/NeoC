#include "neoc/protocol/core/response/contract_storage_entry.h"

neoc_error_t neoc_contract_storage_entry_create_swift(
    const char *key,
    const char *value,
    neoc_contract_storage_entry_t **entry) {
    (void)key;
    (void)value;
    if (entry) {
        *entry = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_storage_entry_create_swift not implemented");
}

neoc_error_t neoc_contract_storage_entry_from_json_swift(
    const char *json_str,
    neoc_contract_storage_entry_t **entry) {
    (void)json_str;
    if (entry) {
        *entry = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_storage_entry_from_json_swift not implemented");
}

neoc_error_t neoc_contract_storage_entry_to_json_swift(
    const neoc_contract_storage_entry_t *entry,
    char **json_str) {
    (void)entry;
    if (json_str) {
        *json_str = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_storage_entry_to_json_swift not implemented");
}

neoc_error_t neoc_contract_storage_entry_copy_swift(
    const neoc_contract_storage_entry_t *src,
    neoc_contract_storage_entry_t **dest) {
    (void)src;
    if (dest) {
        *dest = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_storage_entry_copy_swift not implemented");
}

bool neoc_contract_storage_entry_equals_swift(
    const neoc_contract_storage_entry_t *entry1,
    const neoc_contract_storage_entry_t *entry2) {
    (void)entry1;
    (void)entry2;
    return false;
}

neoc_error_t neoc_contract_storage_entry_get_key_string(
    const neoc_contract_storage_entry_t *entry,
    char **key_str) {
    (void)entry;
    if (key_str) {
        *key_str = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_storage_entry_get_key_string not implemented");
}

neoc_error_t neoc_contract_storage_entry_get_value_string(
    const neoc_contract_storage_entry_t *entry,
    char **value_str) {
    (void)entry;
    if (value_str) {
        *value_str = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "neoc_contract_storage_entry_get_value_string not implemented");
}
