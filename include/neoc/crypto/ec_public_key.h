/**
 * @file ec_public_key.h
 * @brief Backwards compatible include for elliptic curve public key helpers.
 *
 * The consolidated API exposes public key structures through ec_key_pair.h.
 * This wrapper keeps legacy includes working without pulling in duplicate
 * type definitions.
 */

#ifndef NEOC_CRYPTO_EC_PUBLIC_KEY_COMPAT_H
#define NEOC_CRYPTO_EC_PUBLIC_KEY_COMPAT_H

#include "neoc/crypto/ec_key_pair.h"

#endif /* NEOC_CRYPTO_EC_PUBLIC_KEY_COMPAT_H */
