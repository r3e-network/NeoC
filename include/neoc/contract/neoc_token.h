/**
 * @file neo_token.h
 * @brief NEO governance token contract interface
 */

#ifndef NEOC_NEO_TOKEN_H
#define NEOC_NEO_TOKEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/contract/fungible_token.h"
#include "neoc/crypto/e_c_point.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_types_alt.h"


// NEO Token Constants
#define NEO_TOKEN_ID 0x01
#define NEO_TOKEN_SYMBOL "NEO"
#define NEO_TOKEN_DECIMALS 0
#define NEO_TOKEN_TOTAL_SUPPLY 100000000

#ifdef __cplusplus
extern "C" {
#endif

// NEO token contract hash
extern const neoc_hash160_t NEOC_NEO_TOKEN_HASH;

/**
 * NEO token structure
 */
typedef struct neoc_neo_token {
    neoc_fungible_token_t base;         // Base fungible token
    char symbol[10];                    // Token symbol
    uint8_t decimals;                   // Token decimals
    uint64_t total_supply;              // Total supply
} neoc_neo_token_t;

/**
 * Candidate info structure
 */
typedef struct neoc_candidate_info {
    neoc_ec_point_t *public_key;        // Candidate public key
    int64_t votes;                      // Vote count
    bool registered;                    // Registration status
} neoc_candidate_info_t;

/**
 * Create NEO token instance
 * @param token Output NEO token
 * @return Error code
 */
neoc_error_t neoc_neo_token_create(neoc_neo_token_t **token);

/**
 * Register as validator candidate
 * @param public_key Validator public key
 * @return Error code
 */
neoc_error_t neoc_neo_token_register_candidate(neoc_ec_point_t *public_key);

/**
 * Unregister as validator candidate
 * @param public_key Validator public key
 * @return Error code
 */
neoc_error_t neoc_neo_token_unregister_candidate(neoc_ec_point_t *public_key);

/**
 * Vote for validators
 * @param account Voting account
 * @param vote_to Validator public key to vote for
 * @return Error code
 */
neoc_error_t neoc_neo_token_vote(neoc_hash160_t *account,
                                  neoc_ec_point_t *vote_to);

/**
 * Get all candidates
 * @param candidates Output candidates array
 * @param count Output candidate count
 * @return Error code
 */
neoc_error_t neoc_neo_token_get_candidates(neoc_candidate_info_t ***candidates,
                                            size_t *count);

/**
 * Get committee members
 * @param members Output committee member public keys
 * @param count Output member count
 * @return Error code
 */
neoc_error_t neoc_neo_token_get_committee(neoc_ec_point_t ***members,
                                           size_t *count);

/**
 * Get next block validators
 * @param validators Output validator public keys
 * @param count Output validator count
 * @return Error code
 */
neoc_error_t neoc_neo_token_get_next_block_validators(neoc_ec_point_t ***validators,
                                                       size_t *count);

/**
 * Get GAS per block reward
 * @return GAS reward amount
 */
int64_t neoc_neo_token_get_gas_per_block(void);

/**
 * Calculate unclaimed GAS
 * @param account Account hash
 * @param end_height End block height
 * @param unclaimed Output unclaimed GAS
 * @return Error code
 */
neoc_error_t neoc_neo_token_unclaimed_gas(neoc_hash160_t *account,
                                           uint32_t end_height,
                                           int64_t *unclaimed);

/**
 * Free NEO token
 * @param token Token to free
 */
void neoc_neo_token_free(neoc_neo_token_t *token);

/**
 * Free candidate info
 * @param info Candidate info to free
 */
void neoc_candidate_info_free(neoc_candidate_info_t *info);


// NEO Token Constants
#define NEO_TOKEN_ID 0x01
#define NEO_TOKEN_SYMBOL "NEO"
#define NEO_TOKEN_DECIMALS 0
#define NEO_TOKEN_TOTAL_SUPPLY 100000000

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_TOKEN_H
