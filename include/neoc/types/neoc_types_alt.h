/**
 * @file neo_types.h
 * @brief NEO blockchain specific types
 */

#ifndef NEOC_NEO_TYPES_H
#define NEOC_NEO_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/types/neoc_hash160.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/protocol/core/response/neo_account_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NEO candidate structure
 */
typedef struct neoc_neo_candidate {
    neoc_ec_public_key_t *public_key;
    int64_t votes;
} neoc_neo_candidate_t;

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEO_TYPES_H
