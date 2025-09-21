#include "neoc/protocol/core/response/express_shutdown.h"
#include "neoc/utils/decode.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#include <cjson/cJSON.h>
#include <string.h>

neoc_error_t neoc_express_shutdown_create(int process_id,
                                          neoc_express_shutdown_t **shutdown) {
    if (!shutdown) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid shutdown pointer");
    }
    *shutdown = neoc_calloc(1, sizeof(neoc_express_shutdown_t));
    if (!*shutdown) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate ExpressShutdown");
    }
    (*shutdown)->process_id = process_id;
    return NEOC_SUCCESS;
}

void neoc_express_shutdown_free(neoc_express_shutdown_t *shutdown) {
    if (!shutdown) return;
    neoc_free(shutdown);
}

static neoc_error_t parse_process_id(const cJSON *node, int *out_id) {
    if (!node || !out_id) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    if (cJSON_IsNumber(node)) {
        *out_id = (int)node->valuedouble;
        return NEOC_SUCCESS;
    }
    if (cJSON_IsString(node) && node->valuestring) {
        return neoc_decode_int_from_string(node->valuestring, out_id);
    }
    return NEOC_ERROR_INVALID_FORMAT;
}

neoc_error_t neoc_express_shutdown_from_json(const char *json_str,
                                             neoc_express_shutdown_t **shutdown) {
    if (!json_str || !shutdown) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid input");
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON");
    }

    const cJSON *process = cJSON_GetObjectItemCaseSensitive(root, "process-id");
    int process_id = 0;
    neoc_error_t err = parse_process_id(process, &process_id);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return neoc_error_set(err, "Missing or invalid process-id");
    }

    err = neoc_express_shutdown_create(process_id, shutdown);
    cJSON_Delete(root);
    return err;
}

neoc_error_t neoc_express_shutdown_response_create(const char *jsonrpc,
                                                   int id,
                                                   neoc_express_shutdown_t *result,
                                                   const char *error,
                                                   int error_code,
                                                   neoc_express_shutdown_response_t **response) {
    if (!response) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid response pointer");
    }
    neoc_express_shutdown_response_t *resp = neoc_calloc(1, sizeof(*resp));
    if (!resp) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate response");
    }
    if (jsonrpc) {
        resp->jsonrpc = neoc_strdup(jsonrpc);
        if (!resp->jsonrpc) {
            neoc_express_shutdown_response_free(resp);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate jsonrpc");
        }
    }
    resp->id = id;
    resp->result = result;
    if (error) {
        resp->error = neoc_strdup(error);
        if (!resp->error) {
            neoc_express_shutdown_response_free(resp);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to duplicate error message");
        }
    }
    resp->error_code = error_code;
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_express_shutdown_response_free(neoc_express_shutdown_response_t *response) {
    if (!response) return;
    if (response->jsonrpc) neoc_free(response->jsonrpc);
    if (response->error) neoc_free(response->error);
    if (response->result) neoc_express_shutdown_free(response->result);
    neoc_free(response);
}

neoc_error_t neoc_express_shutdown_response_from_json(const char *json_str,
                                                      neoc_express_shutdown_response_t **response) {
    if (!json_str || !response) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid input");
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid JSON");
    }

    const cJSON *jsonrpc = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    const cJSON *id = cJSON_GetObjectItemCaseSensitive(root, "id");
    const cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");

    int request_id = 0;
    if (id) {
        if (cJSON_IsNumber(id)) {
            request_id = (int)id->valuedouble;
        } else if (cJSON_IsString(id) && id->valuestring) {
            neoc_decode_int_from_string(id->valuestring, &request_id);
        }
    }

    neoc_express_shutdown_t *result_struct = NULL;
    const char *error_message = NULL;
    int error_code = 0;

    if (error_obj && cJSON_IsObject(error_obj)) {
        const cJSON *code = cJSON_GetObjectItemCaseSensitive(error_obj, "code");
        const cJSON *message = cJSON_GetObjectItemCaseSensitive(error_obj, "message");
        if (code && cJSON_IsNumber(code)) {
            error_code = (int)code->valuedouble;
        }
        if (message && cJSON_IsString(message)) {
            error_message = message->valuestring;
        }
    } else if (result_obj) {
        char *result_str = cJSON_PrintUnformatted(result_obj);
        if (!result_str) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to print result JSON");
        }
        neoc_error_t err = neoc_express_shutdown_from_json(result_str, &result_struct);
        cJSON_free(result_str);
        if (err != NEOC_SUCCESS) {
            cJSON_Delete(root);
            return err;
        }
    }

    neoc_error_t create_err = neoc_express_shutdown_response_create(jsonrpc && cJSON_IsString(jsonrpc) ? jsonrpc->valuestring : "2.0",
                                                                    request_id,
                                                                    result_struct,
                                                                    error_message,
                                                                    error_code,
                                                                    response);
    cJSON_Delete(root);
    if (create_err != NEOC_SUCCESS && result_struct) {
        neoc_express_shutdown_free(result_struct);
    }
    return create_err;
}
