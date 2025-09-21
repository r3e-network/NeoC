/**
 * @file hash256.h
 * @brief Hash256 type for NeoC SDK
 * 
 * This header redirects to the main Hash256 implementation.
 * Use this for compatibility with Swift naming conventions.
 */

#ifndef NEOC_TYPES_HASH256_H
#define NEOC_TYPES_HASH256_H

// Redirect to the main implementation
#include "neoc/types/neoc_hash256.h"

#ifdef __cplusplus
extern "C" {
#endif

// Type alias for Swift compatibility
typedef neoc_hash256_t hash256_t;

#ifdef __cplusplus
}
#endif

#endif /* NEOC_TYPES_HASH256_H */
