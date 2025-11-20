/**
 * @file neo_response_aliases.c
 * @brief Implementation for common Neo RPC response aliases
 */

#include "neoc/protocol/core/response/neo_response_aliases.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *jsonrpc;
    int id;
    void *result;
    char *error;
    int error_code;
} neoc_generic_response_t;

static void free_base_fields(neoc_generic_response_t *response) {
    if (!response) {
        return;
    }
    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
    }
    if (response->error) {
        neoc_free(response->error);
    }
    if (response->result) {
        neoc_free(response->result);
    }
}

static neoc_error_t init_base_response(neoc_generic_response_t *response,
                                       int id,
                                       const char *error,
                                       int error_code) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    response->id = id;
    response->error_code = error_code;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_free(response->jsonrpc);
            response->jsonrpc = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_create_int_response(int id,
                                          const int *result,
                                          const char *error,
                                          int error_code,
                                          neoc_neo_block_count_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_block_count_t *created = neoc_calloc(1, sizeof(neoc_neo_block_count_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_error_t err =
        init_base_response((neoc_generic_response_t *)created, id, error, error_code);
    if (err != NEOC_SUCCESS) {
        neoc_free(created);
        return err;
    }

    if (result) {
        created->result = neoc_malloc(sizeof(int));
        if (!created->result) {
            neoc_neo_response_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        *created->result = *result;
    }

    *response = created;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_create_bool_response(int id,
                                           const bool *result,
                                           const char *error,
                                           int error_code,
                                           neoc_neo_boolean_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_boolean_response_t *created =
        neoc_calloc(1, sizeof(neoc_neo_boolean_response_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_error_t err =
        init_base_response((neoc_generic_response_t *)created, id, error, error_code);
    if (err != NEOC_SUCCESS) {
        neoc_free(created);
        return err;
    }

    if (result) {
        created->result = neoc_malloc(sizeof(bool));
        if (!created->result) {
            neoc_neo_response_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        *created->result = *result;
    }

    *response = created;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_create_string_response(int id,
                                             const char *result,
                                             const char *error,
                                             int error_code,
                                             neoc_neo_string_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_string_response_t *created =
        neoc_calloc(1, sizeof(neoc_neo_string_response_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_error_t err =
        init_base_response((neoc_generic_response_t *)created, id, error, error_code);
    if (err != NEOC_SUCCESS) {
        neoc_free(created);
        return err;
    }

    if (result) {
        created->result = neoc_strdup(result);
        if (!created->result) {
            neoc_neo_response_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_response_free(void *response) {
    neoc_generic_response_t *generic = (neoc_generic_response_t *)response;
    if (!generic) {
        return;
    }

    free_base_fields(generic);
    neoc_free(generic);
}
