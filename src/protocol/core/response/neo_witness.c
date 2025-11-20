/**
 * @file neo_witness.c
 * @brief NeoWitness RPC model implementation
 */

#include "neoc/protocol/core/response/neo_witness.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/transaction/witness.h"
#include "neoc/utils/neoc_base64.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

static neoc_error_t base64_encode_bytes(const uint8_t *data, size_t len, char **out) {
    if (!out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out = NULL;
    size_t buf_size = neoc_base64_encode_buffer_size(len);
    char *encoded = neoc_malloc(buf_size);
    if (!encoded) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    neoc_error_t err = neoc_base64_encode(data, len, encoded, buf_size);
    if (err != NEOC_SUCCESS) {
        neoc_free(encoded);
        return err;
    }
    *out = encoded;
    return NEOC_SUCCESS;
}

static neoc_error_t base64_decode_string(const char *str, uint8_t **out, size_t *out_len) {
    if (!str || !out || !out_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    size_t decoded_len = 0;
    uint8_t *decoded = neoc_base64_decode_alloc(str, &decoded_len);
    if (!decoded && decoded_len == 0) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    *out = decoded;
    *out_len = decoded_len;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_witness_create(const char *invocation,
                                     const char *verification,
                                     neoc_neo_witness_t **witness) {
    if (!witness) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *witness = neoc_calloc(1, sizeof(neoc_neo_witness_t));
    if (!*witness) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    (*witness)->invocation = dup_string(invocation ? invocation : "");
    (*witness)->verification = dup_string(verification ? verification : "");
    if (!(*witness)->invocation || !(*witness)->verification) {
        neoc_neo_witness_free(*witness);
        *witness = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_witness_from_witness(const neoc_witness_t *witness,
                                           neoc_neo_witness_t **neo_witness) {
    if (!witness || !neo_witness) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    char *inv = NULL;
    char *ver = NULL;
    neoc_error_t err = base64_encode_bytes(witness->invocation_script,
                                           witness->invocation_script_len,
                                           &inv);
    if (err == NEOC_SUCCESS) {
        err = base64_encode_bytes(witness->verification_script,
                                  witness->verification_script_len,
                                  &ver);
    }
    if (err != NEOC_SUCCESS) {
        neoc_free(inv);
        return err;
    }
    err = neoc_neo_witness_create(inv, ver, neo_witness);
    neoc_free(inv);
    neoc_free(ver);
    return err;
}

void neoc_neo_witness_free(neoc_neo_witness_t *witness) {
    if (!witness) {
        return;
    }
    neoc_free(witness->invocation);
    neoc_free(witness->verification);
    neoc_free(witness);
}

neoc_error_t neoc_neo_witness_from_json(const char *json_str,
                                        neoc_neo_witness_t **witness) {
    if (!json_str || !witness) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *witness = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    const char *inv = neoc_json_get_string(json, "invocation");
    const char *ver = neoc_json_get_string(json, "verification");

    neoc_error_t err = neoc_neo_witness_create(inv ? inv : "", ver ? ver : "", witness);
    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_neo_witness_to_json(const neoc_neo_witness_t *witness,
                                      char **json_str) {
    if (!witness || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;
    neoc_json_t *obj = neoc_json_create_object();
    if (!obj) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    if (witness->invocation) {
        neoc_json_add_string(obj, "invocation", witness->invocation);
    }
    if (witness->verification) {
        neoc_json_add_string(obj, "verification", witness->verification);
    }
    *json_str = neoc_json_to_string(obj);
    neoc_json_free(obj);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_neo_witness_copy(const neoc_neo_witness_t *src,
                                   neoc_neo_witness_t **dest) {
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    return neoc_neo_witness_create(src->invocation, src->verification, dest);
}

bool neoc_neo_witness_equals(const neoc_neo_witness_t *witness1,
                             const neoc_neo_witness_t *witness2) {
    if (witness1 == witness2) {
        return true;
    }
    if (!witness1 || !witness2) {
        return false;
    }
    const char *inv1 = witness1->invocation ? witness1->invocation : "";
    const char *inv2 = witness2->invocation ? witness2->invocation : "";
    const char *ver1 = witness1->verification ? witness1->verification : "";
    const char *ver2 = witness2->verification ? witness2->verification : "";
    return strcmp(inv1, inv2) == 0 && strcmp(ver1, ver2) == 0;
}

neoc_error_t neoc_neo_witness_to_witness(const neoc_neo_witness_t *neo_witness,
                                         neoc_witness_t **witness) {
    if (!neo_witness || !witness) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    uint8_t *inv = NULL;
    uint8_t *ver = NULL;
    size_t inv_len = 0;
    size_t ver_len = 0;

    neoc_error_t err = base64_decode_string(neo_witness->invocation ? neo_witness->invocation : "",
                                            &inv,
                                            &inv_len);
    if (err == NEOC_SUCCESS) {
        err = base64_decode_string(neo_witness->verification ? neo_witness->verification : "",
                                   &ver,
                                   &ver_len);
    }
    if (err != NEOC_SUCCESS) {
        neoc_free(inv);
        neoc_free(ver);
        return err;
    }

    err = neoc_witness_create(inv, inv_len, ver, ver_len, witness);
    neoc_free(inv);
    neoc_free(ver);
    return err;
}

neoc_error_t neoc_neo_witness_get_invocation_size(const neoc_neo_witness_t *witness,
                                                  size_t *size) {
    if (!witness || !size) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *size = witness->invocation ? strlen(witness->invocation) : 0;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_witness_get_verification_size(const neoc_neo_witness_t *witness,
                                                    size_t *size) {
    if (!witness || !size) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *size = witness->verification ? strlen(witness->verification) : 0;
    return NEOC_SUCCESS;
}
