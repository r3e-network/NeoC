/**
 * @file neo_get_state_root.c
 * @brief getstateroot RPC response handling
 */

#include "neoc/protocol/core/response/neo_get_state_root.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/utils/json.h"
#include "neoc/utils/neoc_base64.h"

#include <stdlib.h>
#include <string.h>

static char *dup_or_default(const char *value, const char *fallback) {
    if (value) {
        return neoc_strdup(value);
    }
    return fallback ? neoc_strdup(fallback) : NULL;
}

static neoc_error_t decode_base64_field(neoc_json_t *obj,
                                        const char *primary_key,
                                        const char *alt_key,
                                        uint8_t **out,
                                        size_t *out_len) {
    if (!obj || !out || !out_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    const char *encoded = neoc_json_get_string(obj, primary_key);
    if (!encoded && alt_key) {
        encoded = neoc_json_get_string(obj, alt_key);
    }
    if (!encoded) {
        *out = NULL;
        *out_len = 0;
        return NEOC_SUCCESS; /* Treat missing as empty */
    }

    size_t decoded_len = 0;
    uint8_t *decoded = neoc_base64_decode_alloc(encoded, &decoded_len);
    if (!decoded && decoded_len == 0) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    *out = decoded;
    *out_len = decoded_len;
    return NEOC_SUCCESS;
}

static neoc_error_t encode_base64_field(const uint8_t *data,
                                        size_t data_len,
                                        neoc_json_t *parent,
                                        const char *key) {
    if (!data || data_len == 0) {
        return NEOC_SUCCESS;
    }

    size_t buf_size = neoc_base64_encode_buffer_size(data_len);
    char *buffer = neoc_malloc(buf_size);
    if (!buffer) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_error_t err = neoc_base64_encode(data, data_len, buffer, buf_size);
    if (err == NEOC_SUCCESS) {
        err = neoc_json_add_string(parent, key, buffer);
    }

    neoc_free(buffer);
    return err;
}

static neoc_error_t clone_witness_array(neoc_witness_t *witnesses,
                                        size_t count,
                                        neoc_witness_t **out_witnesses) {
    if (!out_witnesses) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out_witnesses = NULL;
    if (!witnesses || count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_witness_t *copy = neoc_calloc(count, sizeof(neoc_witness_t));
    if (!copy) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_witness_t *cloned = NULL;
        neoc_error_t err = neoc_witness_clone(&witnesses[i], &cloned);
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; j++) {
                neoc_witness_free(&copy[j]);
            }
            neoc_free(copy);
            return err;
        }
        memcpy(&copy[i], cloned, sizeof(neoc_witness_t));
        neoc_free(cloned); /* contents already moved */
    }

    *out_witnesses = copy;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_state_root_create(int version,
                                    int index,
                                    const neoc_hash256_t *root_hash,
                                    neoc_witness_t *witnesses,
                                    size_t witnesses_count,
                                    neoc_state_root_t **state_root) {
    if (!root_hash || !state_root) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *state_root = NULL;
    neoc_state_root_t *created = neoc_calloc(1, sizeof(neoc_state_root_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->version = version;
    created->index = index;
    created->root_hash = *root_hash;
    created->witnesses_count = witnesses_count;

    if (witnesses_count > 0) {
        neoc_error_t err = clone_witness_array(witnesses, witnesses_count, &created->witnesses);
        if (err != NEOC_SUCCESS) {
            neoc_state_root_free(created);
            return err;
        }
    }

    *state_root = created;
    return NEOC_SUCCESS;
}

void neoc_state_root_free(neoc_state_root_t *state_root) {
    if (!state_root) {
        return;
    }
    if (state_root->witnesses) {
        for (size_t i = 0; i < state_root->witnesses_count; i++) {
            neoc_witness_free(&state_root->witnesses[i]);
        }
        neoc_free(state_root->witnesses);
    }
    neoc_free(state_root);
}

neoc_error_t neoc_neo_get_state_root_create(int id,
                                            neoc_state_root_t *state_root,
                                            const char *error,
                                            int error_code,
                                            neoc_neo_get_state_root_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_get_state_root_t *created = neoc_calloc(1, sizeof(neoc_neo_get_state_root_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_or_default("2.0", "2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = state_root;
    created->error_code = error_code;

    if (error) {
        created->error = dup_or_default(error, NULL);
        if (!created->error) {
            neoc_neo_get_state_root_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_get_state_root_free(neoc_neo_get_state_root_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_state_root_free(response->result);
    }
    neoc_free(response);
}

static neoc_error_t parse_witnesses(neoc_json_t *array,
                                    neoc_witness_t **witnesses_out,
                                    size_t *count_out) {
    if (!witnesses_out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *witnesses_out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }

    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_witness_t *witnesses = neoc_calloc(count, sizeof(neoc_witness_t));
    if (!witnesses) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *item = neoc_json_array_get(array, i);
        if (!item) {
            neoc_free(witnesses);
            return NEOC_ERROR_INVALID_FORMAT;
        }

        uint8_t *inv = NULL;
        uint8_t *ver = NULL;
        size_t inv_len = 0;
        size_t ver_len = 0;

        neoc_error_t err = decode_base64_field(item, "invocation", "invocationScript", &inv, &inv_len);
        if (err == NEOC_SUCCESS) {
            err = decode_base64_field(item, "verification", "verificationScript", &ver, &ver_len);
        }
        if (err != NEOC_SUCCESS) {
            neoc_free(inv);
            neoc_free(ver);
            neoc_free(witnesses);
            return err;
        }

        neoc_witness_t *w = NULL;
        err = neoc_witness_create(inv, inv_len, ver, ver_len, &w);
        neoc_free(inv);
        neoc_free(ver);
        if (err != NEOC_SUCCESS) {
            neoc_free(witnesses);
            return err;
        }

        memcpy(&witnesses[i], w, sizeof(neoc_witness_t));
        neoc_free(w); /* contents moved */
    }

    *witnesses_out = witnesses;
    *count_out = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_state_root_from_json(const char *json_str,
                                               neoc_neo_get_state_root_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_get_state_root_t *parsed = neoc_calloc(1, sizeof(neoc_neo_get_state_root_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_or_default(neoc_json_get_string(json, "jsonrpc"), "2.0");
    int64_t parsed_id = 0;
    if (neoc_json_get_int(json, "id", &parsed_id) == NEOC_SUCCESS) {
        parsed->id = (int)parsed_id;
    }

    neoc_json_t *error_obj = neoc_json_get_object(json, "error");
    if (error_obj) {
        int64_t code = 0;
        if (neoc_json_get_int(error_obj, "code", &code) == NEOC_SUCCESS) {
            parsed->error_code = (int)code;
        }
        const char *message = neoc_json_get_string(error_obj, "message");
        if (message) {
            parsed->error = dup_or_default(message, NULL);
        }
    }

    neoc_json_t *result = neoc_json_get_object(json, "result");
    if (!result) {
        result = json;
    }

    int64_t version = 0;
    int64_t index = 0;
    const char *root_hash_str = NULL;
    if (neoc_json_get_int(result, "version", &version) == NEOC_SUCCESS &&
        neoc_json_get_int(result, "index", &index) == NEOC_SUCCESS) {
        root_hash_str = neoc_json_get_string(result, "roothash");
    }

    neoc_hash256_t root_hash = {0};
    if (root_hash_str) {
        if (neoc_hash256_from_hex(&root_hash, root_hash_str) != NEOC_SUCCESS) {
            neoc_neo_get_state_root_free(parsed);
            neoc_json_free(json);
            return NEOC_ERROR_INVALID_FORMAT;
        }
    }

    neoc_witness_t *witnesses = NULL;
    size_t witnesses_count = 0;
    neoc_json_t *witnesses_arr = neoc_json_get_array(result, "witnesses");
    if (parse_witnesses(witnesses_arr, &witnesses, &witnesses_count) != NEOC_SUCCESS) {
        neoc_free(parsed->jsonrpc);
        neoc_free(parsed->error);
        neoc_free(parsed);
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    if (root_hash_str) {
        neoc_state_root_t *state_root = NULL;
        neoc_error_t err = neoc_state_root_create((int)version, (int)index, &root_hash, witnesses, witnesses_count, &state_root);
        if (err != NEOC_SUCCESS) {
            if (witnesses) {
                for (size_t i = 0; i < witnesses_count; i++) {
                    neoc_witness_free(&witnesses[i]);
                }
                neoc_free(witnesses);
            }
            neoc_neo_get_state_root_free(parsed);
            neoc_json_free(json);
            return err;
        }
        /* ownership of witnesses transferred */
        parsed->result = state_root;
        parsed->error_code = 0;
    } else if (witnesses) {
        for (size_t i = 0; i < witnesses_count; i++) {
            neoc_witness_free(&witnesses[i]);
        }
        neoc_free(witnesses);
    }

    neoc_json_free(json);
    *response = parsed;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_state_root_to_json(const neoc_neo_get_state_root_t *response,
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
    } else if (response->result) {
        neoc_json_t *result = neoc_json_create_object();
        if (!result) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        neoc_json_add_int(result, "version", response->result->version);
        neoc_json_add_int(result, "index", response->result->index);

        char hash_str[NEOC_HASH256_STRING_LENGTH] = {0};
        if (neoc_hash256_to_hex(&response->result->root_hash, hash_str, sizeof(hash_str), false) == NEOC_SUCCESS) {
            neoc_json_add_string(result, "roothash", hash_str);
        }

        if (response->result->witnesses_count > 0 && response->result->witnesses) {
            neoc_json_t *witnesses = neoc_json_create_array();
            if (!witnesses) {
                neoc_json_free(result);
                neoc_json_free(root);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            for (size_t i = 0; i < response->result->witnesses_count; i++) {
                const neoc_witness_t *w = &response->result->witnesses[i];
                neoc_json_t *w_obj = neoc_json_create_object();
                if (!w_obj) {
                    neoc_json_free(witnesses);
                    neoc_json_free(result);
                    neoc_json_free(root);
                    return NEOC_ERROR_OUT_OF_MEMORY;
                }
                if (encode_base64_field(w->invocation_script, w->invocation_script_len, w_obj, "invocation") != NEOC_SUCCESS ||
                    encode_base64_field(w->verification_script, w->verification_script_len, w_obj, "verification") != NEOC_SUCCESS) {
                    neoc_json_free(w_obj);
                    neoc_json_free(witnesses);
                    neoc_json_free(result);
                    neoc_json_free(root);
                    return NEOC_ERROR_INVALID_FORMAT;
                }
                neoc_json_array_add(witnesses, w_obj);
            }
            neoc_json_add_object(result, "witnesses", witnesses);
        }

        neoc_json_add_object(root, "result", result);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!*json_str) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

neoc_state_root_t *neoc_neo_get_state_root_get_state_root(const neoc_neo_get_state_root_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_state_root_has_state_root(const neoc_neo_get_state_root_t *response) {
    return response && response->result && response->error == NULL && response->error_code == 0;
}

neoc_error_t neoc_neo_get_state_root_get_version(const neoc_neo_get_state_root_t *response,
                                                 int *version) {
    if (!response || !version) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *version = response->result->version;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_state_root_get_index(const neoc_neo_get_state_root_t *response,
                                               int *index) {
    if (!response || !index) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *index = response->result->index;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_state_root_get_root_hash(const neoc_neo_get_state_root_t *response,
                                                   neoc_hash256_t *root_hash) {
    if (!response || !root_hash) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }
    *root_hash = response->result->root_hash;
    return NEOC_SUCCESS;
}

size_t neoc_neo_get_state_root_get_witnesses_count(const neoc_neo_get_state_root_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->witnesses_count;
}

neoc_witness_t *neoc_neo_get_state_root_get_witness(const neoc_neo_get_state_root_t *response,
                                                    size_t index) {
    if (!response || !response->result || index >= response->result->witnesses_count) {
        return NULL;
    }
    return &response->result->witnesses[index];
}

neoc_error_t neoc_neo_get_state_root_get_root_hash_string(const neoc_neo_get_state_root_t *response,
                                                          char **hash_str) {
    if (!response || !hash_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *hash_str = NULL;
    if (!response->result) {
        return NEOC_ERROR_INVALID_STATE;
    }

    char buffer[NEOC_HASH256_STRING_LENGTH] = {0};
    neoc_error_t err = neoc_hash256_to_hex(&response->result->root_hash, buffer, sizeof(buffer), false);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    *hash_str = neoc_strdup(buffer);
    return *hash_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}
