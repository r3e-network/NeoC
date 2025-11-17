/**
 * @file neo_c_service.c
 * @brief Service bridge for NeoC requests/responses.
 *
 * Provides a thin layer that forwards requests to the configured vtable. This
 * matches Swift's NeoSwiftService behaviour where the concrete implementation
 * is responsible for networking while this base handles lifecycle and error
 * checking.
 */

#include "neoc/protocol/neo_c_service.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

static neoc_error_t validate_inputs(neoc_neo_c_service_t *service,
                                    const neoc_request_t *request) {
    if (!service || !service->vtable) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "service is NULL or unconfigured");
    }
    if (!request) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "request is NULL");
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_c_service_init(neoc_neo_c_service_t *service,
                                     const neoc_neo_c_service_vtable_t *vtable,
                                     void *impl_data) {
    if (!service || !vtable) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT,
                              "service or vtable is NULL");
    }

    service->vtable = vtable;
    service->impl_data = impl_data;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_c_service_send(neoc_neo_c_service_t *service,
                                     const neoc_request_t *request,
                                     neoc_response_t **response_out) {
    if (response_out) {
        *response_out = NULL;
    }

    neoc_error_t err = validate_inputs(service, request);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (!service->vtable->send) {
        return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                              "Service send not implemented");
    }

    return service->vtable->send(service, request, response_out);
}

neoc_error_t neoc_neo_c_service_send_async(neoc_neo_c_service_t *service,
                                           const neoc_request_t *request,
                                           void (*callback)(neoc_response_t *, neoc_error_t, void *),
                                           void *user_data) {
    neoc_error_t err = validate_inputs(service, request);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (!service->vtable->send_async) {
        return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                              "Service async send not implemented");
    }

    return service->vtable->send_async(service, request, callback, user_data);
}

void neoc_neo_c_service_free(neoc_neo_c_service_t *service) {
    if (!service) {
        return;
    }

    if (service->vtable && service->vtable->free_impl) {
        service->vtable->free_impl(service);
    }

    service->vtable = NULL;
    service->impl_data = NULL;
}
