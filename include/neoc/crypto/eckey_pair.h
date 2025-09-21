/**
 * @file eckey_pair.h
 * @brief EC Key Pair for NeoC SDK
 * 
 * This header redirects to the main EC Key Pair implementation.
 * Use this for compatibility with Swift naming conventions.
 */

#ifndef NEOC_CRYPTO_ECKEY_PAIR_H
#define NEOC_CRYPTO_ECKEY_PAIR_H

// Redirect to the main implementation
#include "neoc/crypto/ec_key_pair.h"

#ifdef __cplusplus
extern "C" {
#endif

// Type aliases for Swift compatibility
typedef neoc_ec_key_pair_t eckey_pair_t;
typedef neoc_ec_public_key_t ecpublic_key_t;
typedef neoc_ec_private_key_t ecprivate_key_t;

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_ECKEY_PAIR_H */
