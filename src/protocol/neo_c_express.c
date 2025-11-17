/**
 * @file neo_c_express.c
 * @brief NeoC Express request helpers (synchronous and callback-based).
 */

#include "neoc/protocol/neo_c_express.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/core/neo_express.h"
#include "neoc/protocol/core/request.h"
#include "neoc/protocol/service.h"

static neoc_error_t express_invalid_argument(const char *message) {
    return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, message);
}

static neoc_service_t *express_get_service(neoc_neo_c_express_t *express) {
    if (!express || !express->base) {
        return NULL;
    }
    return neoc_neo_c_get_service(express->base);
}

static neoc_error_t express_create_simple_request(
    neoc_neo_c_express_t *express,
    neoc_request_t **request_out,
    neoc_request_t *(*builder)(void *service)) {
    if (!request_out) {
        return express_invalid_argument("request_out is NULL");
    }
    *request_out = NULL;

    if (!express || !express->base || !builder) {
        return express_invalid_argument("Express client is not configured");
    }

    neoc_service_t *service = express_get_service(express);
    if (!service) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE,
                              "Express client missing base service");
    }

    neoc_request_t *request = builder(service);
    if (!request) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to allocate Neo Express request");
    }

    *request_out = request;
    return NEOC_SUCCESS;
}

static neoc_error_t express_send_request(
    neoc_neo_c_express_t *express,
    neoc_request_t *request,
    void (*callback)(neoc_response_t *, neoc_error_t, void *),
    void *user_data) {
    if (!callback || !request) {
        if (request) {
            neoc_request_free(request);
        }
        return express_invalid_argument("Callback or request is NULL");
    }

    neoc_service_t *service = express_get_service(express);
    if (!service) {
        neoc_request_free(request);
        return neoc_error_set(NEOC_ERROR_INVALID_STATE,
                              "Express client missing base service");
    }

    neoc_response_t *response = NULL;
    neoc_error_t err = neoc_service_send_request(service, request, &response);
    callback(response, err, user_data);
    return err;
}

neoc_neo_c_express_t *neoc_neo_c_express_create(neoc_neo_c_t *base_client) {
    if (!base_client) {
        express_invalid_argument("Base client is NULL");
        return NULL;
    }

    neoc_neo_c_express_t *express = neoc_calloc(1, sizeof(neoc_neo_c_express_t));
    if (!express) {
        neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY,
                       "Failed to allocate express client");
        return NULL;
    }
    express->base = base_client;
    return express;
}

neoc_neo_c_express_t *neoc_neo_c_express_build(neoc_service_t *service) {
    if (!service) {
        express_invalid_argument("Service is NULL");
        return NULL;
    }

    neoc_neo_c_t *base = neoc_neo_c_build(service);
    if (!base) {
        neoc_service_free(service);
        return NULL;
    }

    neoc_neo_c_express_t *express = neoc_neo_c_express_create(base);
    if (!express) {
        neoc_neo_c_free(base);
    }
    return express;
}

neoc_neo_c_express_t *neoc_neo_c_express_build_with_config(neoc_service_t *service,
                                                           neoc_neo_c_config_t *config) {
    if (!service || !config) {
        express_invalid_argument("Service or config is NULL");
        return NULL;
    }

    neoc_neo_c_t *base = neoc_neo_c_build_with_config(service, config);
    if (!base) {
        neoc_service_free(service);
        neoc_neo_c_config_free(config);
        return NULL;
    }

    neoc_neo_c_express_t *express = neoc_neo_c_express_create(base);
    if (!express) {
        neoc_neo_c_free(base);
    }
    return express;
}

neoc_neo_c_t *neoc_neo_c_express_get_base(neoc_neo_c_express_t *express) {
    return express ? express->base : NULL;
}

neoc_error_t neoc_neo_c_express_get_populated_blocks(neoc_neo_c_express_t *express,
                                                     neoc_request_t **request_out) {
    return express_create_simple_request(
        express, request_out, neoc_neo_express_get_populated_blocks);
}

neoc_error_t neoc_neo_c_express_get_nep17_contracts(neoc_neo_c_express_t *express,
                                                    neoc_request_t **request_out) {
    return express_create_simple_request(
        express, request_out, neoc_neo_express_get_nep17_contracts);
}

neoc_error_t neoc_neo_c_express_get_contract_storage(neoc_neo_c_express_t *express,
                                                     const neoc_hash160_t *contract_hash,
                                                     neoc_request_t **request_out) {
    if (!contract_hash) {
        return express_invalid_argument("Contract hash is NULL");
    }
    if (!request_out) {
        return express_invalid_argument("request_out is NULL");
    }

    neoc_service_t *service = express_get_service(express);
    if (!service) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE,
                              "Express client missing base service");
    }

    neoc_request_t *request = neoc_neo_express_get_contract_storage(service, contract_hash);
    if (!request) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to create contract storage request");
    }

    *request_out = request;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_c_express_list_contracts(neoc_neo_c_express_t *express,
                                               neoc_request_t **request_out) {
    return express_create_simple_request(
        express, request_out, neoc_neo_express_list_contracts);
}

neoc_error_t neoc_neo_c_express_create_checkpoint(neoc_neo_c_express_t *express,
                                                  const char *filename,
                                                  neoc_request_t **request_out) {
    if (!filename || !request_out) {
        return express_invalid_argument("Filename or request_out is NULL");
    }

    neoc_service_t *service = express_get_service(express);
    if (!service) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE,
                              "Express client missing base service");
    }

    neoc_request_t *request = neoc_neo_express_create_checkpoint(service, filename);
    if (!request) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to create checkpoint request");
    }

    *request_out = request;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_c_express_list_oracle_requests(neoc_neo_c_express_t *express,
                                                     neoc_request_t **request_out) {
    return express_create_simple_request(
        express, request_out, neoc_neo_express_list_oracle_requests);
}

neoc_error_t neoc_neo_c_express_create_oracle_response_tx(neoc_neo_c_express_t *express,
                                                          const neoc_transaction_attribute_t *oracle_response,
                                                          neoc_request_t **request_out) {
    if (!oracle_response || !request_out) {
        return express_invalid_argument("Oracle response or request_out is NULL");
    }

    neoc_service_t *service = express_get_service(express);
    if (!service) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE,
                              "Express client missing base service");
    }

    neoc_request_t *request = neoc_neo_express_create_oracle_response_tx(
        service, oracle_response);
    if (!request) {
        return neoc_error_set(NEOC_ERROR_MEMORY,
                              "Failed to create oracle response request");
    }

    *request_out = request;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_c_express_shutdown(neoc_neo_c_express_t *express,
                                         neoc_request_t **request_out) {
    return express_create_simple_request(
        express, request_out, neoc_neo_express_shutdown);
}

#define RETURN_ASYNC_NOT_IMPLEMENTED() \
    do { return express_not_implemented(); } while (0)

neoc_error_t neoc_neo_c_express_get_populated_blocks_async(neoc_neo_c_express_t *express,
                                                    void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                    void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_get_populated_blocks(express, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_get_nep17_contracts_async(neoc_neo_c_express_t *express,
                                                          void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                          void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_get_nep17_contracts(express, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_get_contract_storage_async(neoc_neo_c_express_t *express,
                                                           const neoc_hash160_t *contract_hash,
                                                           void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                           void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_get_contract_storage(express, contract_hash, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_list_contracts_async(neoc_neo_c_express_t *express,
                                                     void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                     void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_list_contracts(express, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_create_checkpoint_async(neoc_neo_c_express_t *express,
                                                        const char *filename,
                                                        void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                        void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_create_checkpoint(express, filename, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_list_oracle_requests_async(neoc_neo_c_express_t *express,
                                                           void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                           void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_list_oracle_requests(express, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_create_oracle_response_tx_async(neoc_neo_c_express_t *express,
                                                                const neoc_transaction_attribute_t *oracle_response,
                                                                void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                                void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_create_oracle_response_tx(express,
                                                                    oracle_response,
                                                                    &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

neoc_error_t neoc_neo_c_express_shutdown_async(neoc_neo_c_express_t *express,
                                               void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                               void *user_data) {
    if (!callback) {
        return express_invalid_argument("Callback is NULL");
    }
    neoc_request_t *request = NULL;
    neoc_error_t err = neoc_neo_c_express_shutdown(express, &request);
    if (err != NEOC_SUCCESS) {
        callback(NULL, err, user_data);
        return err;
    }
    return express_send_request(express, request, callback, user_data);
}

void neoc_neo_c_express_free(neoc_neo_c_express_t *express) {
    if (!express) {
        return;
    }

    if (express->base) {
        neoc_neo_c_free(express->base);
    }

    neoc_free(express);
}
