/**
 * @file non_fungible_token.h
 * @brief NEP-11 non-fungible token standard interface
 */

#ifndef NEOC_NON_FUNGIBLE_TOKEN_H
#define NEOC_NON_FUNGIBLE_TOKEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/contract/token.h"
#include "neoc/contract/iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NFT properties structure
 */
typedef struct neoc_nft_properties {
    char *name;                         // NFT name
    char *description;                  // Description
    char *image;                        // Image URL
    char *token_uri;                    // Token URI
    uint8_t *extra_data;                // Extra data
    size_t extra_data_len;              // Extra data length
} neoc_nft_properties_t;

/**
 * Non-fungible token structure (NEP-11)
 */
typedef struct neoc_nft {
    neoc_token_t base;                  // Base token
    bool divisible;                     // Whether NFTs are divisible
    void *rpc_client;                   // RPC client for blockchain interaction
} neoc_nft_t;

typedef neoc_nft_t neoc_non_fungible_token_t;

/**
 * Create non-fungible token instance
 * @param contract_hash Token contract hash
 * @param divisible Whether tokens are divisible
 * @param token Output token instance
 * @return Error code
 */
neoc_error_t neoc_nft_create(neoc_hash160_t *contract_hash,
                              bool divisible,
                              neoc_non_fungible_token_t **token);

/**
 * Get NFT symbol
 * @param token The token
 * @param symbol Output symbol string (caller must free)
 * @return Error code
 */
neoc_error_t neoc_nft_symbol(neoc_non_fungible_token_t *token, char **symbol);

/**
 * Get NFT decimals
 * @param token The token
 * @param decimals Output decimal count
 * @return Error code
 */
neoc_error_t neoc_nft_decimals(neoc_non_fungible_token_t *token, uint8_t *decimals);

/**
 * Get NFT total supply
 * @param token The token
 * @param supply Output total supply
 * @return Error code
 */
neoc_error_t neoc_nft_total_supply(neoc_non_fungible_token_t *token, uint64_t *supply);

/**
 * Get NFT balance for account
 * @param token The token
 * @param account Account hash
 * @param balance Output balance
 * @return Error code
 */
neoc_error_t neoc_nft_balance_of(neoc_non_fungible_token_t *token,
                                  neoc_hash160_t *account,
                                  int64_t *balance);

/**
 * Get tokens owned by account
 * @param token The token
 * @param account Account hash
 * @param iterator Output token ID iterator
 * @return Error code
 */
neoc_error_t neoc_nft_tokens_of(neoc_non_fungible_token_t *token,
                                 neoc_hash160_t *account,
                                 neoc_iterator_t **iterator);

/**
 * Transfer NFT
 * @param token The token
 * @param to Destination account
 * @param token_id Token ID to transfer
 * @param data Additional data
 * @param data_len Data length
 * @return Error code
 */
neoc_error_t neoc_nft_transfer(neoc_non_fungible_token_t *token,
                                neoc_hash160_t *to,
                                uint8_t *token_id,
                                size_t token_id_len,
                                uint8_t *data,
                                size_t data_len);

/**
 * Get NFT owner
 * @param token The token
 * @param token_id Token ID
 * @param token_id_len Token ID length
 * @param owners Output owners (may be multiple if divisible)
 * @param count Output owner count
 * @return Error code
 */
neoc_error_t neoc_nft_owner_of(neoc_non_fungible_token_t *token,
                                uint8_t *token_id,
                                size_t token_id_len,
                                neoc_hash160_t ***owners,
                                size_t *count);

/**
 * Get NFT properties
 * @param token The token
 * @param token_id Token ID
 * @param token_id_len Token ID length
 * @param properties Output properties
 * @return Error code
 */
neoc_error_t neoc_nft_properties(neoc_non_fungible_token_t *token,
                                  uint8_t *token_id,
                                  size_t token_id_len,
                                  neoc_nft_properties_t **properties);

/**
 * Get all NFT tokens
 * @param token The token
 * @param iterator Output token iterator
 * @return Error code
 */
neoc_error_t neoc_nft_tokens(neoc_non_fungible_token_t *token,
                              neoc_iterator_t **iterator);

/**
 * Set RPC client for blockchain interaction
 * @param token Token instance
 * @param rpc_client RPC client instance
 * @return Error code
 */
neoc_error_t neoc_nft_set_rpc_client(neoc_non_fungible_token_t *token, void *rpc_client);

/**
 * Free non-fungible token
 * @param token Token to free
 */
void neoc_nft_free(neoc_non_fungible_token_t *token);

/**
 * Free NFT properties
 * @param properties Properties to free
 */
void neoc_nft_properties_free(neoc_nft_properties_t *properties);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NON_FUNGIBLE_TOKEN_H
