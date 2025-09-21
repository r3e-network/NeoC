/**
 * @file neo_express.h
 * @brief Neo Express protocol interface
 * 
 * Defines the Neo Express protocol interface for local blockchain testing
 * based on the Swift NeoExpress implementation.
 */

#ifndef NEOC_PROTOCOL_CORE_NEO_EXPRESS_H
#define NEOC_PROTOCOL_CORE_NEO_EXPRESS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct neoc_request neoc_request_t;

/**
 * @brief Get populated blocks from Neo Express
 * @param service The Neo Express service instance
 * @return Request for populated blocks or NULL on error
 */
neoc_request_t *neoc_neo_express_get_populated_blocks(void *service);

/**
 * @brief Get NEP-17 contracts from Neo Express
 * @param service The Neo Express service instance
 * @return Request for NEP-17 contracts or NULL on error
 */
neoc_request_t *neoc_neo_express_get_nep17_contracts(void *service);

/**
 * @brief Get contract storage from Neo Express
 * @param service The Neo Express service instance
 * @param contract_hash The contract hash to query
 * @return Request for contract storage or NULL on error
 */
neoc_request_t *neoc_neo_express_get_contract_storage(void *service, const neoc_hash160_t *contract_hash);

/**
 * @brief List contracts from Neo Express
 * @param service The Neo Express service instance
 * @return Request for contract list or NULL on error
 */
neoc_request_t *neoc_neo_express_list_contracts(void *service);

/**
 * @brief Create checkpoint in Neo Express
 * @param service The Neo Express service instance
 * @param filename The checkpoint filename
 * @return Request for checkpoint creation or NULL on error
 */
neoc_request_t *neoc_neo_express_create_checkpoint(void *service, const char *filename);

/**
 * @brief List oracle requests from Neo Express
 * @param service The Neo Express service instance
 * @return Request for oracle requests or NULL on error
 */
neoc_request_t *neoc_neo_express_list_oracle_requests(void *service);

/**
 * @brief Create oracle response transaction in Neo Express
 * @param service The Neo Express service instance
 * @param oracle_response The oracle response transaction attribute
 * @return Request for oracle response creation or NULL on error
 */
neoc_request_t *neoc_neo_express_create_oracle_response_tx(void *service, const void *oracle_response);

/**
 * @brief Shutdown Neo Express
 * @param service The Neo Express service instance
 * @return Request for shutdown or NULL on error
 */
neoc_request_t *neoc_neo_express_shutdown(void *service);

/**
 * @brief Check if Neo Express is available
 * @param service The service instance to check
 * @return true if Neo Express is available, false otherwise
 */
bool neoc_neo_express_is_available(void *service);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_NEO_EXPRESS_H */
