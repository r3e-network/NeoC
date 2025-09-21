/**
 * @file neo_name_service.h
 * @brief NEO Name Service interface
 */

#ifndef NEOC_NEO_NAME_SERVICE_H
#define NEOC_NEO_NAME_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/types/neoc_hash160.h"
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NNS record types
 */
typedef enum {
    NEOC_NNS_RECORD_TYPE_A = 1,
    NEOC_NNS_RECORD_TYPE_CNAME = 5,
    NEOC_NNS_RECORD_TYPE_TXT = 16,
    NEOC_NNS_RECORD_TYPE_AAAA = 28
} neoc_nns_record_type_t;

/**
 * Opaque types
 */
typedef struct neoc_nns neoc_nns_t;
typedef struct neoc_nns neoc_neo_name_service_t;

/**
 * Create a new NNS instance
 */
neoc_error_t neoc_nns_create(neoc_neo_name_service_t **nns);

/**
 * Resolve a domain name to its record
 */
neoc_error_t neoc_nns_resolve(neoc_neo_name_service_t *nns,
                               const char *name,
                               neoc_nns_record_type_t type,
                               char **result);

/**
 * Check if a domain name is available
 */
neoc_error_t neoc_nns_is_available(neoc_nns_t *nns,
                                    const char *name,
                                    bool *available);

/**
 * Register a new domain name
 */
neoc_error_t neoc_nns_register(neoc_nns_t *nns,
                                const char *name,
                                const neoc_hash160_t *owner);

/**
 * Set a record for a domain
 */
neoc_error_t neoc_nns_set_record(neoc_nns_t *nns,
                                  const char *name,
                                  neoc_nns_record_type_t type,
                                  const char *data);

/**
 * Get the registration price for a domain name
 */
neoc_error_t neoc_nns_get_price(neoc_nns_t *nns,
                                 uint32_t length,
                                 uint64_t *price);

/**
 * Set RPC client for blockchain interaction
 */
neoc_error_t neoc_nns_set_rpc_client(neoc_nns_t *nns, void *rpc_client);

/**
 * Free NNS instance
 */
void neoc_nns_free(neoc_nns_t *nns);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_NAME_SERVICE_H
