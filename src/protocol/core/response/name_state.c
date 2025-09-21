#include "neoc/protocol/core/response/name_state.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/decode.h"

#include <cjson/cJSON.h>
#include <string.h>
#include <limits.h>

neoc_error_t neoc_name_state_create(const char *name,
                                    const int *expiration,
                                    const neoc_hash160_t *admin,
                                    neoc_name_state_t **state) {
    if (!state || !name) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_name_state_t *ns = neoc_calloc(1, sizeof(neoc_name_state_t));
    if (!ns) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate name state");
    }

    ns->name = neoc_strdup(name);
    if (!ns->name) {
        neoc_name_state_free(ns);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate name");
    }

    if (expiration) {
        ns->expiration = neoc_malloc(sizeof(int));
        if (!ns->expiration) {
            neoc_name_state_free(ns);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate expiration");
        }
        *ns->expiration = *expiration;
    }

    if (admin) {
        ns->admin = neoc_malloc(sizeof(neoc_hash160_t));
        if (!ns->admin) {
            neoc_name_state_free(ns);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate admin hash");
        }
        memcpy(ns->admin, admin, sizeof(neoc_hash160_t));
    }

    *state = ns;
    return NEOC_SUCCESS;
}

void neoc_name_state_free(neoc_name_state_t *state) {
    if (!state) return;
    if (state->name) neoc_free(state->name);
    if (state->expiration) neoc_free(state->expiration);
    if (state->admin) neoc_free(state->admin);
    neoc_free(state);
}

neoc_error_t neoc_name_state_from_json(const char *json_str,
                                       neoc_name_state_t **state) {
    if (!json_str || !state) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON");
    }

    const cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    if (!name || !cJSON_IsString(name) || !name->valuestring) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Missing name");
    }

    int expiration_value;
    const int *expiration_ptr = NULL;
    const cJSON *expiration = cJSON_GetObjectItemCaseSensitive(root, "expiration");
    if (expiration) {
        if (cJSON_IsNumber(expiration)) {
            expiration_value = (int)expiration->valuedouble;
            expiration_ptr = &expiration_value;
        } else if (cJSON_IsString(expiration) && expiration->valuestring) {
            if (neoc_decode_int_from_string(expiration->valuestring, &expiration_value) == NEOC_SUCCESS) {
                expiration_ptr = &expiration_value;
            } else {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid expiration value");
            }
        }
    }

    neoc_hash160_t admin_hash;
    const neoc_hash160_t *admin_ptr = NULL;
    const cJSON *admin = cJSON_GetObjectItemCaseSensitive(root, "admin");
    if (admin && cJSON_IsString(admin) && admin->valuestring) {
        if (neoc_hash160_from_string(admin->valuestring, &admin_hash) == NEOC_SUCCESS) {
            admin_ptr = &admin_hash;
        } else {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid admin hash");
        }
    }

    neoc_error_t err = neoc_name_state_create(name->valuestring,
                                              expiration_ptr,
                                              admin_ptr,
                                              state);
    cJSON_Delete(root);
    return err;
}

neoc_error_t neoc_name_state_to_json(const neoc_name_state_t *state,
                                     char **json_str) {
    if (!state || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to create JSON object");
    }

    cJSON_AddStringToObject(root, "name", state->name ? state->name : "");
    if (state->expiration) {
        cJSON_AddNumberToObject(root, "expiration", *state->expiration);
    }
    if (state->admin) {
        char hash_str[41];
        if (neoc_hash160_to_string(state->admin, hash_str, sizeof(hash_str)) == NEOC_SUCCESS) {
            cJSON_AddStringToObject(root, "admin", hash_str);
        }
    }

    char *output = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!output) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to print JSON");
    }
    *json_str = output;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_name_state_copy(const neoc_name_state_t *src,
                                  neoc_name_state_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    return neoc_name_state_create(src->name,
                                   src->expiration,
                                   src->admin,
                                   dest);
}

bool neoc_name_state_equals(const neoc_name_state_t *state1,
                            const neoc_name_state_t *state2) {
    if (state1 == state2) return true;
    if (!state1 || !state2) return false;

    if ((state1->name && !state2->name) || (!state1->name && state2->name)) return false;
    if (state1->name && state2->name && strcmp(state1->name, state2->name) != 0) return false;

    int exp1 = state1->expiration ? *state1->expiration : INT_MIN;
    int exp2 = state2->expiration ? *state2->expiration : INT_MIN;
    if ((state1->expiration && !state2->expiration) || (!state1->expiration && state2->expiration) || (state1->expiration && state2->expiration && exp1 != exp2)) {
        return false;
    }

    if ((state1->admin && !state2->admin) || (!state1->admin && state2->admin)) return false;
    if (state1->admin && state2->admin && memcmp(state1->admin, state2->admin, sizeof(neoc_hash160_t)) != 0) {
        return false;
    }

    return true;
}

bool neoc_name_state_is_expired(const neoc_name_state_t *state,
                                int current_time) {
    if (!state || !state->expiration) {
        return false;
    }
    return current_time >= *state->expiration;
}
