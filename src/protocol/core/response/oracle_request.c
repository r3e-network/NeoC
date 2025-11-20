/**
 * @file oracle_request.c
 * @brief OracleRequest RPC model implementation
 */

#include "neoc/protocol/core/response/oracle_request.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_oracle_request_create(int request_id,
                                        const neoc_hash256_t *original_tx_hash,
                                        int gas_for_response,
                                        const char *url,
                                        const char *filter,
                                        const neoc_hash160_t *callback_contract,
                                        const char *callback_method,
                                        const char *user_data,
                                        neoc_oracle_request_t **oracle_request) {
    if (!original_tx_hash || !url || !callback_contract || !callback_method || !oracle_request) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *oracle_request = neoc_calloc(1, sizeof(neoc_oracle_request_t));
    if (!*oracle_request) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*oracle_request)->request_id = request_id;
    (*oracle_request)->gas_for_response = gas_for_response;

    (*oracle_request)->original_tx_hash = neoc_malloc(sizeof(neoc_hash256_t));
    (*oracle_request)->callback_contract = neoc_malloc(sizeof(neoc_hash160_t));
    if (!(*oracle_request)->original_tx_hash || !(*oracle_request)->callback_contract) {
        neoc_oracle_request_free(*oracle_request);
        *oracle_request = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    *(*oracle_request)->original_tx_hash = *original_tx_hash;
    *(*oracle_request)->callback_contract = *callback_contract;

    (*oracle_request)->url = dup_string(url);
    (*oracle_request)->filter = dup_string(filter);
    (*oracle_request)->callback_method = dup_string(callback_method);
    (*oracle_request)->user_data = dup_string(user_data);

    if (!(*oracle_request)->url || !(*oracle_request)->callback_method) {
        neoc_oracle_request_free(*oracle_request);
        *oracle_request = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    return NEOC_SUCCESS;
}

void neoc_oracle_request_free(neoc_oracle_request_t *oracle_request) {
    if (!oracle_request) {
        return;
    }
    neoc_free(oracle_request->original_tx_hash);
    neoc_free(oracle_request->callback_contract);
    neoc_free(oracle_request->url);
    neoc_free(oracle_request->filter);
    neoc_free(oracle_request->callback_method);
    neoc_free(oracle_request->user_data);
    neoc_free(oracle_request);
}

neoc_error_t neoc_oracle_request_response_create(const char *jsonrpc,
                                                 int id,
                                                 neoc_oracle_request_t *result,
                                                 const char *error,
                                                 int error_code,
                                                 neoc_oracle_request_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = neoc_calloc(1, sizeof(neoc_oracle_request_response_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->jsonrpc = dup_string(jsonrpc ? jsonrpc : "2.0");
    if (!(*response)->jsonrpc) {
        neoc_oracle_request_response_free(*response);
        *response = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    (*response)->id = id;
    (*response)->result = result;
    (*response)->error_code = error_code;

    if (error) {
        (*response)->error = dup_string(error);
        if (!(*response)->error) {
            neoc_oracle_request_response_free(*response);
            *response = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    return NEOC_SUCCESS;
}

void neoc_oracle_request_response_free(neoc_oracle_request_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_oracle_request_free(response->result);
    }
    neoc_free(response);
}

neoc_error_t neoc_oracle_request_from_json(const char *json_str,
                                           neoc_oracle_request_t **oracle_request) {
    if (!json_str || !oracle_request) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *oracle_request = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    int64_t request_id = 0;
    neoc_json_get_int(json, "id", &request_id);
    const char *url = neoc_json_get_string(json, "url");
    const char *filter = neoc_json_get_string(json, "filter");
    const char *callback_method = neoc_json_get_string(json, "callback");
    const char *user_data = neoc_json_get_string(json, "userData");

    int64_t gas_for_resp = 0;
    neoc_json_get_int(json, "gasForResponse", &gas_for_resp);

    const char *tx_hash_hex = neoc_json_get_string(json, "txHash");
    const char *contract_hex = neoc_json_get_string(json, "callbackcontract");
    if (!contract_hex) {
        contract_hex = neoc_json_get_string(json, "callbackContract");
    }

    neoc_hash256_t tx_hash;
    neoc_hash160_t callback_contract;
    if (!tx_hash_hex ||
        neoc_hash256_from_hex(&tx_hash, tx_hash_hex) != NEOC_SUCCESS ||
        !contract_hex ||
        neoc_hash160_from_hex(&callback_contract, contract_hex) != NEOC_SUCCESS ||
        !url || !callback_method) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_error_t err = neoc_oracle_request_create((int)request_id,
                                                  &tx_hash,
                                                  (int)gas_for_resp,
                                                  url,
                                                  filter,
                                                  &callback_contract,
                                                  callback_method,
                                                  user_data,
                                                  oracle_request);
    neoc_json_free(json);
    return err;
}

#include <stdio.h> // for snprintf
neoc_error_t neoc_oracle_request_response_from_json(const char *json_str,
                                                    neoc_oracle_request_response_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_oracle_request_response_t *parsed = neoc_calloc(1, sizeof(neoc_oracle_request_response_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_string(neoc_json_get_string(json, "jsonrpc"));
    if (!parsed->jsonrpc) {
        parsed->jsonrpc = dup_string("2.0");
    }

    int64_t id_val = 0;
    if (neoc_json_get_int(json, "id", &id_val) == NEOC_SUCCESS) {
        parsed->id = (int)id_val;
    }

    neoc_json_t *error_obj = neoc_json_get_object(json, "error");
    if (error_obj) {
        int64_t code = 0;
        if (neoc_json_get_int(error_obj, "code", &code) == NEOC_SUCCESS) {
            parsed->error_code = (int)code;
        }
        const char *message = neoc_json_get_string(error_obj, "message");
        if (message) {
            parsed->error = dup_string(message);
        }
    }

    neoc_json_t *result = neoc_json_get_object(json, "result");
    if (!result) {
        result = json;
    }

    char *result_json = neoc_json_to_string(result);
    if (result_json) {
        neoc_oracle_request_t *req = NULL;
        if (neoc_oracle_request_from_json(result_json, &req) == NEOC_SUCCESS) {
            parsed->result = req;
            parsed->error_code = 0;
        }
        neoc_free(result_json);
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_oracle_request_to_json(const neoc_oracle_request_t *oracle_request,
                                         char **json_str) {
    if (!oracle_request || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_json_add_int(json, "id", oracle_request->request_id);
    neoc_json_add_int(json, "gasForResponse", oracle_request->gas_for_response);
    if (oracle_request->url) {
        neoc_json_add_string(json, "url", oracle_request->url);
    }
    if (oracle_request->filter) {
        neoc_json_add_string(json, "filter", oracle_request->filter);
    }
    if (oracle_request->callback_method) {
        neoc_json_add_string(json, "callback", oracle_request->callback_method);
    }
    if (oracle_request->user_data) {
        neoc_json_add_string(json, "userData", oracle_request->user_data);
    }
    if (oracle_request->original_tx_hash) {
        char tx_hex[NEOC_HASH256_STRING_LENGTH] = {0};
        if (neoc_hash256_to_hex(oracle_request->original_tx_hash, tx_hex, sizeof(tx_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(json, "txHash", tx_hex);
        }
    }
    if (oracle_request->callback_contract) {
        char contract_hex[NEOC_HASH160_STRING_LENGTH] = {0};
        if (neoc_hash160_to_hex(oracle_request->callback_contract, contract_hex, sizeof(contract_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(json, "callbackcontract", contract_hex);
        }
    }

    *json_str = neoc_json_to_string(json);
    neoc_json_free(json);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_oracle_request_copy(const neoc_oracle_request_t *src,
                                      neoc_oracle_request_t **dest) {
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    return neoc_oracle_request_create(src->request_id,
                                      src->original_tx_hash,
                                      src->gas_for_response,
                                      src->url,
                                      src->filter,
                                      src->callback_contract,
                                      src->callback_method,
                                      src->user_data,
                                      dest);
}

bool neoc_oracle_request_equals(const neoc_oracle_request_t *request1,
                                const neoc_oracle_request_t *request2) {
    if (request1 == request2) {
        return true;
    }
    if (!request1 || !request2 || !request1->original_tx_hash || !request2->original_tx_hash ||
        !request1->callback_contract || !request2->callback_contract) {
        return false;
    }
    if (request1->request_id != request2->request_id ||
        request1->gas_for_response != request2->gas_for_response) {
        return false;
    }

    if (memcmp(request1->original_tx_hash->data, request2->original_tx_hash->data, NEOC_HASH256_SIZE) != 0) {
        return false;
    }
    if (memcmp(request1->callback_contract->data, request2->callback_contract->data, NEOC_HASH160_SIZE) != 0) {
        return false;
    }
    const char *url1 = request1->url ? request1->url : "";
    const char *url2 = request2->url ? request2->url : "";
    const char *filter1 = request1->filter ? request1->filter : "";
    const char *filter2 = request2->filter ? request2->filter : "";
    const char *cb1 = request1->callback_method ? request1->callback_method : "";
    const char *cb2 = request2->callback_method ? request2->callback_method : "";
    const char *ud1 = request1->user_data ? request1->user_data : "";
    const char *ud2 = request2->user_data ? request2->user_data : "";

    return strcmp(url1, url2) == 0 &&
           strcmp(filter1, filter2) == 0 &&
           strcmp(cb1, cb2) == 0 &&
           strcmp(ud1, ud2) == 0;
}
