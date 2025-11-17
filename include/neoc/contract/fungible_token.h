/**
 * @file fungible_token.h
 * @brief NEP-17 fungible token standard interface
 */

#ifndef NEOC_FUNGIBLE_TOKEN_H
#define NEOC_FUNGIBLE_TOKEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/contract/token.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fungible token structure (NEP-17)
 */
typedef struct neoc_fungible_token {
    neoc_token_t base;                  // Base token
    uint8_t decimals;                   // Token decimals
    uint64_t total_supply;              // Total supply (fractions)
} neoc_fungible_token_t;

/**
 * Create fungible token instance
 * @param contract_hash Token contract hash
 * @param token Output token instance
 * @return Error code
 */
neoc_error_t neoc_fungible_token_create(neoc_hash160_t *contract_hash,
                                         neoc_fungible_token_t **token);

/**
 * Get token balance for account
 * @param token The token
 * @param account Account hash
 * @param balance Output balance
 * @return Error code
 */
neoc_error_t neoc_fungible_token_balance_of(neoc_fungible_token_t *token,
                                             neoc_hash160_t *account,
                                             int64_t *balance);

/**
 * Transfer tokens
 * @param token The token
 * @param from Source account
 * @param to Destination account
 * @param amount Amount to transfer
 * @param data Additional data
 * @param data_len Data length
 * @return Error code
 */
neoc_error_t neoc_fungible_token_transfer(neoc_fungible_token_t *token,
                                           neoc_hash160_t *from,
                                           neoc_hash160_t *to,
                                           int64_t amount,
                                           uint8_t *data,
                                           size_t data_len);

/**
 * Get token decimals
 * @param token The token
 * @return Number of decimals
 */
uint8_t neoc_fungible_token_get_decimals(neoc_fungible_token_t *token);

/**
 * Get total supply
 * @param token The token
 * @return Total supply
 */
uint64_t neoc_fungible_token_get_total_supply(neoc_fungible_token_t *token);

/**
 * Free fungible token
 * @param token Token to free
 */
void neoc_fungible_token_free(neoc_fungible_token_t *token);

#ifdef __cplusplus
}
#endif

#endif // NEOC_FUNGIBLE_TOKEN_H
