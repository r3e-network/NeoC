/**
 * @file neo_account_state.c
 * @brief Neo account state helpers mirroring NeoSwift behaviour
 */

#include "neoc/protocol/core/response/neo_account_state.h"

#include "neoc/crypto/ec_public_key.h"
#include "neoc/utils/decode.h"
#include "neoc/utils/neoc_hex.h"

#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static void neoc_neo_account_state_reset(neoc_neo_account_state_t *state) {
    if (!state) {
        return;
    }
    if (state->public_key) {
        neoc_ec_public_key_free(state->public_key);
        state->public_key = NULL;
    }
    if (state->balance_height) {
        neoc_free(state->balance_height);
        state->balance_height = NULL;
    }
}

void neoc_neo_account_state_free(neoc_neo_account_state_t *state) {
    if (!state) {
        return;
    }
    neoc_neo_account_state_reset(state);
    neoc_free(state);
}

static neoc_error_t neoc_neo_account_state_allocate(neoc_neo_account_state_t **state_out) {
    if (!state_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "neo_account_state: output pointer is NULL");
    }

    neoc_neo_account_state_t *state = neoc_calloc(1, sizeof(neoc_neo_account_state_t));
    if (!state) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_account_state: allocation failed");
    }

    *state_out = state;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_account_state_create(int64_t balance,
                                           const int64_t *balance_height,
                                           const neoc_ec_public_key_t *public_key,
                                           neoc_neo_account_state_t **state) {
    neoc_neo_account_state_t *new_state = NULL;
    neoc_error_t err = neoc_neo_account_state_allocate(&new_state);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    new_state->balance = balance;

    if (balance_height) {
        new_state->balance_height = neoc_malloc(sizeof(int64_t));
        if (!new_state->balance_height) {
            neoc_neo_account_state_free(new_state);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "neo_account_state: balance height allocation failed");
        }
        *new_state->balance_height = *balance_height;
    }

    if (public_key) {
        err = neoc_ec_public_key_clone(public_key, &new_state->public_key);
        if (err != NEOC_SUCCESS) {
            neoc_neo_account_state_free(new_state);
            return err;
        }
    }

    *state = new_state;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_account_state_create_no_vote(int64_t balance,
                                                   int64_t update_height,
                                                   neoc_neo_account_state_t **state) {
    return neoc_neo_account_state_create(balance, &update_height, NULL, state);
}

neoc_error_t neoc_neo_account_state_create_no_balance(neoc_neo_account_state_t **state) {
    return neoc_neo_account_state_create(0, NULL, NULL, state);
}

#ifdef HAVE_CJSON
static neoc_error_t neoc_neo_account_state_parse_vote(const cJSON *root,
                                                      neoc_ec_public_key_t **public_key) {
    const cJSON *vote = cJSON_GetObjectItemCaseSensitive(root, "voteTo");
    if (!vote) {
        return NEOC_SUCCESS;
    }

    if (cJSON_IsNull(vote)) {
        *public_key = NULL;
        return NEOC_SUCCESS;
    }

    if (!cJSON_IsString(vote) || !vote->valuestring) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NeoAccountState voteTo is not a string");
    }

    size_t key_len = 0;
    uint8_t *key_bytes = neoc_hex_decode_alloc(vote->valuestring, &key_len);
    if (!key_bytes) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NeoAccountState voteTo hex decode failed");
    }

    neoc_error_t err = neoc_ec_public_key_from_bytes(key_bytes, key_len, public_key);
    neoc_free(key_bytes);
    if (err != NEOC_SUCCESS) {
        return neoc_error_set(err, "NeoAccountState voteTo public key decode failed");
    }

    return NEOC_SUCCESS;
}
#endif

neoc_error_t neoc_neo_account_state_from_json(const char *json_str,
                                              neoc_neo_account_state_t **state) {
    if (!json_str || !state) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "NeoAccountState JSON input invalid");
    }

#ifndef HAVE_CJSON
    (void)json_str;
    (void)state;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "NeoAccountState parsing requires cJSON");
#else
    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NeoAccountState JSON is not an object");
    }

    const cJSON *balance_item = cJSON_GetObjectItemCaseSensitive(root, "balance");
    if (!balance_item || !cJSON_IsNumber(balance_item)) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NeoAccountState missing balance");
    }
    int64_t balance = (int64_t)balance_item->valuedouble;

    int64_t height_value = 0;
    const int64_t *height_ptr = NULL;
    const cJSON *height_item = cJSON_GetObjectItemCaseSensitive(root, "balanceHeight");
    if (height_item) {
        if (cJSON_IsNumber(height_item)) {
            height_value = (int64_t)height_item->valuedouble;
            height_ptr = &height_value;
        } else if (cJSON_IsString(height_item) && height_item->valuestring) {
            int temp_height = 0;
            if (neoc_decode_int_from_string(height_item->valuestring, &temp_height) == NEOC_SUCCESS) {
                height_value = temp_height;
                height_ptr = &height_value;
            }
        }
    }

    neoc_ec_public_key_t *public_key = NULL;
    neoc_error_t err = neoc_neo_account_state_parse_vote(root, &public_key);
    if (err != NEOC_SUCCESS) {
        if (public_key) {
            neoc_ec_public_key_free(public_key);
        }
        cJSON_Delete(root);
        return err;
    }

    neoc_neo_account_state_t *parsed = NULL;
    err = neoc_neo_account_state_create(balance, height_ptr, public_key, &parsed);
    if (public_key) {
        neoc_ec_public_key_free(public_key);
    }
    cJSON_Delete(root);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    *state = parsed;
    return NEOC_SUCCESS;
#endif
}

neoc_error_t neoc_neo_account_state_to_json(const neoc_neo_account_state_t *state,
                                            char **json_str) {
    if (!state || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "NeoAccountState serialization arguments invalid");
    }

#ifndef HAVE_CJSON
    (void)state;
    (void)json_str;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "NeoAccountState serialization requires cJSON");
#else
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "NeoAccountState JSON allocation failed");
    }

    cJSON_AddNumberToObject(root, "balance", (double)state->balance);

    if (state->balance_height) {
        cJSON_AddNumberToObject(root, "balanceHeight", (double)(*state->balance_height));
    }

    if (state->public_key) {
        uint8_t buffer[65];
        size_t len = sizeof(buffer);
        neoc_error_t err = neoc_ec_public_key_to_bytes(state->public_key, buffer, &len);
        if (err != NEOC_SUCCESS) {
            cJSON_Delete(root);
            return err;
        }

        char *encoded = neoc_hex_encode_alloc(buffer, len, false, false);
        if (!encoded) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "NeoAccountState voteTo encoding failed");
        }
        cJSON_AddStringToObject(root, "voteTo", encoded);
        neoc_free(encoded);
    }

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!*json_str) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "NeoAccountState JSON output allocation failed");
    }
    return NEOC_SUCCESS;
#endif
}

bool neoc_neo_account_state_has_vote(const neoc_neo_account_state_t *state) {
    return state && state->public_key;
}

bool neoc_neo_account_state_has_balance(const neoc_neo_account_state_t *state) {
    return state && state->balance > 0;
}

neoc_error_t neoc_neo_account_state_copy(const neoc_neo_account_state_t *src,
                                         neoc_neo_account_state_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "NeoAccountState copy arguments invalid");
    }

    return neoc_neo_account_state_create(src->balance,
                                         src->balance_height,
                                         src->public_key,
                                         dest);
}
