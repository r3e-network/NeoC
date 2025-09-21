/**
 * @file gas_token.h
 * @brief GAS token contract interface
 */

#ifndef NEOC_GAS_TOKEN_H
#define NEOC_GAS_TOKEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/contract/fungible_token.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

// GAS token contract hash
extern const neoc_hash160_t NEOC_GAS_TOKEN_HASH;

/**
 * GAS token structure
 */
typedef struct neoc_gas_token {
    neoc_fungible_token_t base;         // Base fungible token
} neoc_gas_token_t;

/**
 * Create GAS token instance
 * @param token Output GAS token
 * @return Error code
 */
neoc_error_t neoc_gas_token_create(neoc_gas_token_t **token);

/**
 * Get GAS balance for account
 * @param account Account hash
 * @param balance Output balance
 * @return Error code
 */
neoc_error_t neoc_gas_token_balance_of(neoc_hash160_t *account,
                                        int64_t *balance);

/**
 * Transfer GAS
 * @param from Source account
 * @param to Destination account
 * @param amount Amount to transfer (in fractions)
 * @param data Additional data
 * @param data_len Data length
 * @return Error code
 */
neoc_error_t neoc_gas_token_transfer(neoc_hash160_t *from,
                                      neoc_hash160_t *to,
                                      int64_t amount,
                                      uint8_t *data,
                                      size_t data_len);

/**
 * Refuel GAS for transaction
 * @param account Account to refuel
 * @param amount Amount of GAS
 * @return Error code
 */
neoc_error_t neoc_gas_token_refuel(neoc_hash160_t *account,
                                    int64_t amount);

/**
 * Free GAS token
 * @param token Token to free
 */
void neoc_gas_token_free(neoc_gas_token_t *token);

#ifdef __cplusplus
}
#endif

#endif // NEOC_GAS_TOKEN_H
