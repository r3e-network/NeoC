/**
 * @file neo_c_express.c
 * @brief Minimal NeoC Express stubs – functionality not yet implemented.
 */

#include "neoc/protocol/neo_c_express.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

static neoc_error_t express_not_implemented(void) {
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          "Neo Express features not implemented");
}

neoc_neo_c_express_t *neoc_neo_c_express_create(neoc_neo_c_t *base_client) {
    if (!base_client) {
        neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                       "Base client is NULL");
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
    (void)service;
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                   "Neo Express build not implemented");
    return NULL;
}

neoc_neo_c_express_t *neoc_neo_c_express_build_with_config(neoc_service_t *service,
                                                           neoc_neo_c_config_t *config) {
    (void)service;
    (void)config;
    neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                   "Neo Express build with config not implemented");
    return NULL;
}

neoc_neo_c_t *neoc_neo_c_express_get_base(neoc_neo_c_express_t *express) {
    return express ? express->base : NULL;
}

#define RETURN_NOT_IMPLEMENTED(ptr) \
    do {                             \
        if (ptr) { *(ptr) = NULL; }  \
        return express_not_implemented(); \
    } while (0)

neoc_error_t neoc_neo_c_express_get_populated_blocks(neoc_neo_c_express_t *express,
                                                     neoc_request_t **request_out) {
    (void)express;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_get_nep17_contracts(neoc_neo_c_express_t *express,
                                                    neoc_request_t **request_out) {
    (void)express;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_get_contract_storage(neoc_neo_c_express_t *express,
                                                     const neoc_hash160_t *contract_hash,
                                                     neoc_request_t **request_out) {
    (void)express;
    (void)contract_hash;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_list_contracts(neoc_neo_c_express_t *express,
                                               neoc_request_t **request_out) {
    (void)express;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_create_checkpoint(neoc_neo_c_express_t *express,
                                                  const char *filename,
                                                  neoc_request_t **request_out) {
    (void)express;
    (void)filename;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_list_oracle_requests(neoc_neo_c_express_t *express,
                                                     neoc_request_t **request_out) {
    (void)express;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_create_oracle_response_tx(neoc_neo_c_express_t *express,
                                                          const neoc_transaction_attribute_t *oracle_response,
                                                          neoc_request_t **request_out) {
    (void)express;
    (void)oracle_response;
    RETURN_NOT_IMPLEMENTED(request_out);
}

neoc_error_t neoc_neo_c_express_shutdown(neoc_neo_c_express_t *express,
                                         neoc_request_t **request_out) {
    (void)express;
    RETURN_NOT_IMPLEMENTED(request_out);
}

#define RETURN_ASYNC_NOT_IMPLEMENTED() \
    do { return express_not_implemented(); } while (0)

neoc_error_t neoc_neo_c_express_get_populated_blocks_async(neoc_neo_c_express_t *express,
                                                           void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                           void *user_data) {
    (void)express;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_get_nep17_contracts_async(neoc_neo_c_express_t *express,
                                                          void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                          void *user_data) {
    (void)express;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_get_contract_storage_async(neoc_neo_c_express_t *express,
                                                           const neoc_hash160_t *contract_hash,
                                                           void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                           void *user_data) {
    (void)express;
    (void)contract_hash;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_list_contracts_async(neoc_neo_c_express_t *express,
                                                     void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                     void *user_data) {
    (void)express;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_create_checkpoint_async(neoc_neo_c_express_t *express,
                                                        const char *filename,
                                                        void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                        void *user_data) {
    (void)express;
    (void)filename;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_list_oracle_requests_async(neoc_neo_c_express_t *express,
                                                           void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                           void *user_data) {
    (void)express;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_create_oracle_response_tx_async(neoc_neo_c_express_t *express,
                                                                const neoc_transaction_attribute_t *oracle_response,
                                                                void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                                                void *user_data) {
    (void)express;
    (void)oracle_response;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}

neoc_error_t neoc_neo_c_express_shutdown_async(neoc_neo_c_express_t *express,
                                               void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                               void *user_data) {
    (void)express;
    (void)callback;
    (void)user_data;
    RETURN_ASYNC_NOT_IMPLEMENTED();
}
