/**
 * @file native_contract_state.c
 * @brief Native contract state helpers mirroring NeoSwift behaviour
 */

#include "neoc/protocol/core/response/native_contract_state.h"

#include "neoc/contract/contract_manifest.h"
#include "neoc/protocol/response/contract_nef.h"
#include "neoc/utils/decode.h"
#include "neoc/types/neoc_hash160.h"

#include <limits.h>
#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static void neoc_native_contract_state_free_active_in(char **values, size_t count) {
    if (!values) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_free(values[i]);
    }
    neoc_free(values);
}

void neoc_native_contract_state_free(neoc_native_contract_state_t *state) {
    if (!state) {
        return;
    }

    if (state->manifest) {
        neoc_contract_manifest_free(state->manifest);
    }

    if (state->nef) {
        neoc_contract_nef_free(state->nef);
    }

    neoc_free(state->update_history);
    neoc_native_contract_state_free_active_in(state->active_in, state->active_in_count);
    neoc_free(state);
}

static neoc_error_t neoc_native_contract_state_alloc(int id,
                                                     const neoc_hash160_t *hash,
                                                     const neoc_contract_manifest_t *manifest,
                                                     const neoc_contract_nef_t *nef,
                                                     const int *update_history,
                                                     size_t update_history_count,
                                                     neoc_native_contract_state_t **out_state) {
    if (!hash || !manifest || !nef || !out_state) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid native contract arguments");
    }

    neoc_native_contract_state_t *state = neoc_calloc(1, sizeof(neoc_native_contract_state_t));
    if (!state) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Unable to allocate native contract state");
    }

    state->id = id;
    memcpy(&state->hash, hash, sizeof(neoc_hash160_t));

    state->manifest = neoc_contract_manifest_clone(manifest);
    if (!state->manifest) {
        neoc_native_contract_state_free(state);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to clone manifest");
    }

    state->nef = neoc_contract_nef_clone(nef);
    if (!state->nef) {
        neoc_native_contract_state_free(state);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to clone NEF");
    }

    if (update_history && update_history_count > 0) {
        state->update_history = neoc_malloc(sizeof(int) * update_history_count);
        if (!state->update_history) {
            neoc_native_contract_state_free(state);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to clone update history");
        }
        memcpy(state->update_history, update_history, sizeof(int) * update_history_count);
        state->update_history_count = update_history_count;
    }

    *out_state = state;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_native_contract_state_create_swift(int id,
                                                     const neoc_hash160_t *hash,
                                                     const neoc_contract_manifest_t *manifest,
                                                     const neoc_contract_nef_t *nef,
                                                     const int *update_history,
                                                     size_t update_history_count,
                                                     neoc_native_contract_state_t **state) {
    return neoc_native_contract_state_alloc(id,
                                            hash,
                                            manifest,
                                            nef,
                                            update_history,
                                            update_history_count,
                                            state);
}

static neoc_error_t neoc_native_contract_state_parse_id(const cJSON *item, int *out_id) {
    if (!item || !out_id) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Missing contract identifier");
    }

    if (cJSON_IsNumber(item)) {
        *out_id = item->valueint;
        return NEOC_SUCCESS;
    }

    if (cJSON_IsString(item) && item->valuestring) {
        int value = 0;
        if (neoc_decode_int_from_string(item->valuestring, &value) == NEOC_SUCCESS) {
            *out_id = value;
            return NEOC_SUCCESS;
        }
    }

    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid native contract id");
}

#ifdef HAVE_CJSON
static neoc_error_t neoc_native_contract_state_parse_update_history(const cJSON *root,
                                                                    int **history,
                                                                    size_t *count) {
    const cJSON *history_array = cJSON_GetObjectItemCaseSensitive(root, "updatehistory");
    if (!history_array || !cJSON_IsArray(history_array)) {
        *history = NULL;
        *count = 0;
        return NEOC_SUCCESS;
    }

    size_t array_size = (size_t)cJSON_GetArraySize(history_array);
    if (array_size == 0) {
        *history = NULL;
        *count = 0;
        return NEOC_SUCCESS;
    }

    int *values = neoc_malloc(sizeof(int) * array_size);
    if (!values) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate update history");
    }

    for (size_t i = 0; i < array_size; i++) {
        const cJSON *entry = cJSON_GetArrayItem(history_array, (int)i);
        if (cJSON_IsNumber(entry)) {
            values[i] = entry->valueint;
        } else if (cJSON_IsString(entry) && entry->valuestring) {
            int decoded = 0;
            if (neoc_decode_int_from_string(entry->valuestring, &decoded) == NEOC_SUCCESS) {
                values[i] = decoded;
            } else {
                neoc_free(values);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid update history entry");
            }
        } else {
            neoc_free(values);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unexpected update history entry");
        }
    }

    *history = values;
    *count = array_size;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_native_contract_state_parse_active_in(const cJSON *root,
                                                               char ***active_in,
                                                               size_t *active_count) {
    const cJSON *active = cJSON_GetObjectItemCaseSensitive(root, "activeIn");
    if (!active || !cJSON_IsArray(active)) {
        *active_in = NULL;
        *active_count = 0;
        return NEOC_SUCCESS;
    }

    size_t array_size = (size_t)cJSON_GetArraySize(active);
    if (array_size == 0) {
        *active_in = NULL;
        *active_count = 0;
        return NEOC_SUCCESS;
    }

    char **values = neoc_calloc(array_size, sizeof(char *));
    if (!values) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate activeIn array");
    }

    for (size_t i = 0; i < array_size; i++) {
        const cJSON *entry = cJSON_GetArrayItem(active, (int)i);
        if (!cJSON_IsString(entry) || !entry->valuestring) {
            neoc_native_contract_state_free_active_in(values, array_size);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid activeIn entry");
        }
        values[i] = neoc_strdup(entry->valuestring);
        if (!values[i]) {
            neoc_native_contract_state_free_active_in(values, array_size);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to copy activeIn entry");
        }
    }

    *active_in = values;
    *active_count = array_size;
    return NEOC_SUCCESS;
}
#endif

neoc_error_t neoc_native_contract_state_from_json_swift(const char *json_str,
                                                        neoc_native_contract_state_t **state) {
    if (!json_str || !state) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid JSON input");
    }

#ifndef HAVE_CJSON
    (void)json_str;
    (void)state;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "cJSON support not enabled");
#else
    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid native contract JSON");
    }

    int id = 0;
    neoc_error_t err = neoc_native_contract_state_parse_id(cJSON_GetObjectItem(root, "id"), &id);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return err;
    }

    const cJSON *hash_item = cJSON_GetObjectItemCaseSensitive(root, "hash");
    if (!hash_item || !cJSON_IsString(hash_item) || !hash_item->valuestring) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Missing native contract hash");
    }

    neoc_hash160_t hash;
    if (neoc_hash160_from_string(hash_item->valuestring, &hash) != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid native contract hash");
    }

    const cJSON *manifest_obj = cJSON_GetObjectItemCaseSensitive(root, "manifest");
    if (!manifest_obj) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Missing manifest section");
    }

    char *manifest_json = cJSON_PrintUnformatted(manifest_obj);
    if (!manifest_json) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode manifest JSON");
    }

    neoc_contract_manifest_t *manifest = NULL;
    err = neoc_contract_manifest_from_json(manifest_json, &manifest);
    neoc_free(manifest_json);
    if (err != NEOC_SUCCESS || !manifest) {
        cJSON_Delete(root);
        if (manifest) neoc_contract_manifest_free(manifest);
        return err != NEOC_SUCCESS ? err : neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Malformed manifest JSON");
    }

    const cJSON *nef_obj = cJSON_GetObjectItemCaseSensitive(root, "nef");
    if (!nef_obj) {
        cJSON_Delete(root);
        neoc_contract_manifest_free(manifest);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Missing NEF section");
    }

    char *nef_json = cJSON_PrintUnformatted(nef_obj);
    if (!nef_json) {
        cJSON_Delete(root);
        neoc_contract_manifest_free(manifest);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode NEF JSON");
    }

    neoc_contract_nef_t *nef = neoc_contract_nef_from_json(nef_json);
    neoc_free(nef_json);
    if (!nef) {
        cJSON_Delete(root);
        neoc_contract_manifest_free(manifest);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Malformed NEF JSON");
    }

    int *history = NULL;
    size_t history_count = 0;
    err = neoc_native_contract_state_parse_update_history(root, &history, &history_count);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        neoc_contract_manifest_free(manifest);
        neoc_contract_nef_free(nef);
        return err;
    }

    char **active_in = NULL;
    size_t active_count = 0;
    err = neoc_native_contract_state_parse_active_in(root, &active_in, &active_count);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        neoc_contract_manifest_free(manifest);
        neoc_contract_nef_free(nef);
        neoc_free(history);
        return err;
    }

    neoc_native_contract_state_t *parsed_state = NULL;
    err = neoc_native_contract_state_alloc(id, &hash, manifest, nef, history, history_count, &parsed_state);
    if (err == NEOC_SUCCESS) {
        parsed_state->active_in = active_in;
        parsed_state->active_in_count = active_count;
    } else {
        neoc_native_contract_state_free_active_in(active_in, active_count);
    }

    neoc_free(history);
    neoc_contract_manifest_free(manifest);
    neoc_contract_nef_free(nef);
    cJSON_Delete(root);

    if (err != NEOC_SUCCESS) {
        return err;
    }

    *state = parsed_state;
    return NEOC_SUCCESS;
#endif
}

neoc_error_t neoc_native_contract_state_to_json_swift(const neoc_native_contract_state_t *state,
                                                      char **json_str) {
    if (!state || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid native contract state");
    }

#ifndef HAVE_CJSON
    (void)state;
    (void)json_str;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "cJSON support not enabled");
#else
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to create JSON object");
    }

    cJSON_AddNumberToObject(root, "id", state->id);

    char hash_str[41];
    if (neoc_hash160_to_string(&state->hash, hash_str, sizeof(hash_str)) != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid hash data");
    }
    cJSON_AddStringToObject(root, "hash", hash_str);

    char *manifest_json = NULL;
    size_t manifest_len = 0;
    neoc_error_t err = neoc_contract_manifest_to_json(state->manifest, &manifest_json, &manifest_len);
    if (err != NEOC_SUCCESS || !manifest_json) {
        cJSON_Delete(root);
        return err != NEOC_SUCCESS ? err : neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode manifest");
    }

    cJSON *manifest_obj = cJSON_Parse(manifest_json);
    neoc_free(manifest_json);
    if (!manifest_obj) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid manifest JSON");
    }
    cJSON_AddItemToObject(root, "manifest", manifest_obj);

    char *nef_json = neoc_contract_nef_to_json(state->nef);
    if (!nef_json) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode NEF");
    }

    cJSON *nef_obj = cJSON_Parse(nef_json);
    neoc_free(nef_json);
    if (!nef_obj) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid NEF JSON");
    }
    cJSON_AddItemToObject(root, "nef", nef_obj);

    if (state->update_history_count > 0 && state->update_history) {
        cJSON *history_array = cJSON_CreateArray();
        if (!history_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode update history");
        }
        for (size_t i = 0; i < state->update_history_count; i++) {
            cJSON_AddItemToArray(history_array, cJSON_CreateNumber(state->update_history[i]));
        }
        cJSON_AddItemToObject(root, "updatehistory", history_array);
    }

    if (state->active_in_count > 0 && state->active_in) {
        cJSON *active_array = cJSON_CreateArray();
        if (!active_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode activeIn");
        }
        for (size_t i = 0; i < state->active_in_count; i++) {
            cJSON_AddItemToArray(active_array, cJSON_CreateString(state->active_in[i] ? state->active_in[i] : ""));
        }
        cJSON_AddItemToObject(root, "activeIn", active_array);
    }

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!*json_str) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to serialize native contract state");
    }

    return NEOC_SUCCESS;
#endif
}

neoc_error_t neoc_native_contract_state_copy_swift(const neoc_native_contract_state_t *src,
                                                   neoc_native_contract_state_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid copy arguments");
    }

    neoc_error_t err = neoc_native_contract_state_alloc(src->id,
                                                        &src->hash,
                                                        src->manifest,
                                                        src->nef,
                                                        src->update_history,
                                                        src->update_history_count,
                                                        dest);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (src->active_in_count > 0 && src->active_in) {
        (*dest)->active_in = neoc_calloc(src->active_in_count, sizeof(char *));
        if (!(*dest)->active_in) {
            neoc_native_contract_state_free(*dest);
            *dest = NULL;
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to clone activeIn array");
        }

        for (size_t i = 0; i < src->active_in_count; i++) {
            if (src->active_in[i]) {
                (*dest)->active_in[i] = neoc_strdup(src->active_in[i]);
                if (!(*dest)->active_in[i]) {
                    (*dest)->active_in_count = src->active_in_count;
                    neoc_native_contract_state_free(*dest);
                    *dest = NULL;
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to clone activeIn entry");
                }
            }
        }
        (*dest)->active_in_count = src->active_in_count;
    }

    return NEOC_SUCCESS;
}

static bool neoc_native_contract_state_compare_history(const neoc_native_contract_state_t *a,
                                                       const neoc_native_contract_state_t *b) {
    if (a->update_history_count != b->update_history_count) {
        return false;
    }
    if (!a->update_history_count) {
        return true;
    }
    return memcmp(a->update_history, b->update_history, sizeof(int) * a->update_history_count) == 0;
}

static bool neoc_native_contract_state_compare_active(const neoc_native_contract_state_t *a,
                                                      const neoc_native_contract_state_t *b) {
    if (a->active_in_count != b->active_in_count) {
        return false;
    }
    for (size_t i = 0; i < a->active_in_count; i++) {
        const char *left = a->active_in ? a->active_in[i] : NULL;
        const char *right = b->active_in ? b->active_in[i] : NULL;
        if ((left && !right) || (!left && right)) {
            return false;
        }
        if (left && right && strcmp(left, right) != 0) {
            return false;
        }
    }
    return true;
}

bool neoc_native_contract_state_equals_swift(const neoc_native_contract_state_t *state1,
                                             const neoc_native_contract_state_t *state2) {
    if (state1 == state2) {
        return true;
    }
    if (!state1 || !state2) {
        return false;
    }

    if (state1->id != state2->id) {
        return false;
    }

    if (memcmp(&state1->hash, &state2->hash, sizeof(neoc_hash160_t)) != 0) {
        return false;
    }

    if (!neoc_contract_nef_equals(state1->nef, state2->nef)) {
        return false;
    }

    char *manifest_json_1 = NULL;
    size_t manifest_len_1 = 0;
    if (neoc_contract_manifest_to_json(state1->manifest, &manifest_json_1, &manifest_len_1) != NEOC_SUCCESS) {
        neoc_free(manifest_json_1);
        return false;
    }

    char *manifest_json_2 = NULL;
    size_t manifest_len_2 = 0;
    if (neoc_contract_manifest_to_json(state2->manifest, &manifest_json_2, &manifest_len_2) != NEOC_SUCCESS) {
        neoc_free(manifest_json_1);
        neoc_free(manifest_json_2);
        return false;
    }

    bool manifest_equal = (manifest_len_1 == manifest_len_2) &&
                          memcmp(manifest_json_1, manifest_json_2, manifest_len_1) == 0;
    neoc_free(manifest_json_1);
    neoc_free(manifest_json_2);

    if (!manifest_equal) {
        return false;
    }

    if (!neoc_native_contract_state_compare_history(state1, state2)) {
        return false;
    }

    return neoc_native_contract_state_compare_active(state1, state2);
}

neoc_error_t neoc_native_contract_state_get_latest_update(const neoc_native_contract_state_t *state,
                                                          int *latest_update) {
    if (!state || !latest_update) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    if (!state->update_history || state->update_history_count == 0) {
        return neoc_error_set(NEOC_ERROR_NOT_FOUND, "No update history available");
    }

    int max_value = INT_MIN;
    for (size_t i = 0; i < state->update_history_count; i++) {
        if (state->update_history[i] > max_value) {
            max_value = state->update_history[i];
        }
    }

    *latest_update = max_value;
    return NEOC_SUCCESS;
}

bool neoc_native_contract_state_is_active_in_version(const neoc_native_contract_state_t *state,
                                                     const char *version) {
    if (!state || !version || !state->active_in || state->active_in_count == 0) {
        return false;
    }

    for (size_t i = 0; i < state->active_in_count; i++) {
        if (state->active_in[i] && strcmp(state->active_in[i], version) == 0) {
            return true;
        }
    }

    return false;
}

