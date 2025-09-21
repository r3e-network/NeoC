/**
 * @file token.h
 * @brief Base token interface
 */

#ifndef NEOC_TOKEN_H
#define NEOC_TOKEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Token type enumeration
 */
typedef enum neoc_token_type {
    NEOC_TOKEN_TYPE_FUNGIBLE,            // NEP-17 fungible token
    NEOC_TOKEN_TYPE_NON_FUNGIBLE,        // NEP-11 non-fungible token
    NEOC_TOKEN_TYPE_GOVERNANCE           // Governance token (NEO/GAS)
} neoc_token_type_t;

/**
 * Base token structure
 */
typedef struct neoc_token {
    neoc_hash160_t *contract_hash;      // Token contract hash
    char *symbol;                       // Token symbol
    char *name;                         // Token name
    neoc_token_type_t type;             // Token type
} neoc_token_t;

/**
 * Create token instance
 * @param contract_hash Contract hash
 * @param type Token type
 * @param token Output token
 * @return Error code
 */
neoc_error_t neoc_token_create(neoc_hash160_t *contract_hash,
                                neoc_token_type_t type,
                                neoc_token_t **token);

/**
 * Get token symbol
 * @param token The token
 * @return Token symbol
 */
const char* neoc_token_get_symbol(neoc_token_t *token);

/**
 * Get token name
 * @param token The token
 * @return Token name
 */
const char* neoc_token_get_name(neoc_token_t *token);

/**
 * Get token contract hash
 * @param token The token
 * @return Contract hash
 */
neoc_hash160_t* neoc_token_get_contract_hash(neoc_token_t *token);

/**
 * Free token
 * @param token Token to free
 */
void neoc_token_free(neoc_token_t *token);

#ifdef __cplusplus
}
#endif

#endif // NEOC_TOKEN_H
