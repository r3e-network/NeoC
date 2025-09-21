#ifndef NEOC_PROTOCOL_SERVICE_NEO_C_SERVICE_H
#define NEOC_PROTOCOL_SERVICE_NEO_C_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file neo_c_service.h
 * @brief NeoC service interface definitions
 */

#include "neoc/neoc_error.h"

// Forward declarations
typedef struct neoc_service neoc_service_t;
typedef struct neoc_http_service neoc_http_service_t;

/**
 * @brief Service type enumeration
 */
typedef enum {
    NEOC_SERVICE_HTTP = 1,
    NEOC_SERVICE_WEBSOCKET = 2
} neoc_service_type_t;

/**
 * @brief Base service structure
 */
typedef struct neoc_service {
    neoc_service_type_t type;
    char *endpoint_url;
    void *impl_data;
} neoc_service_t;

/**
 * @brief Service interface functions
 */
neoc_error_t neoc_service_create(const char *url, neoc_service_type_t type, neoc_service_t **service);
void neoc_service_free(neoc_service_t *service);
neoc_error_t neoc_service_send_request(neoc_service_t *service, const char *request_json, char **response_json);

#ifdef __cplusplus
}
#endif

#endif // NEOC_PROTOCOL_SERVICE_NEO_C_SERVICE_H
