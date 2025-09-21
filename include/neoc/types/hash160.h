/**
 * @file hash160.h
 * @brief Hash160 type for NeoC SDK
 * 
 * This header redirects to the main Hash160 implementation.
 * Use this for compatibility with Swift naming conventions.
 */

#ifndef NEOC_TYPES_HASH160_H
#define NEOC_TYPES_HASH160_H

// Redirect to the main implementation
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

// Type alias for Swift compatibility
typedef neoc_hash160_t hash160_t;

#ifdef __cplusplus
}
#endif

#endif /* NEOC_TYPES_HASH160_H */
