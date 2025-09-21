/**
 * @file neoc.h
 * @brief Main header file for NeoC SDK - C library for Neo blockchain development
 * 
 * This header provides comprehensive access to all NeoC SDK functionality including:
 * - Core types (Hash160, Hash256)
 * - Cryptographic operations (signing, verification, key management)
 * - Transaction building and serialization
 * - Smart contract interaction
 * - Wallet management
 * - Protocol communication
 * 
 * @version 1.0.0
 * @author NeoC SDK Team
 * @copyright 2024
 */

#ifndef NEOC_H
#define NEOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Core SDK Components */
#include "neoc/neoc_error.h"
#include "neoc/neo_constants.h"
#include "neoc/neoc_memory.h"

/* Core Types */
#include "neoc/types/neoc_types.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"

/* Utilities */
#include "neoc/utils/neoc_hex.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/utils/neoc_base64.h"
#include "neoc/utils/neoc_bytes_utils.h"

/* Cryptography */
#include "neoc/crypto/neoc_hash.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/ecdsa_signature.h"
#include "neoc/crypto/sign.h"
#include "neoc/crypto/wif.h"
#include "neoc/crypto/nep2.h"

/* Script */
#include "neoc/script/script_builder.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/script/script_reader.h"
#include "neoc/script/opcode.h"
#include "neoc/script/interop_service.h"
#include "neoc/script/script_helper.h"

/* Wallet */
#include "neoc/wallet/nep6.h"

/* Contract */
#include "neoc/contract/nef_file.h"
#include "neoc/contract/contract_manifest.h"
#include "neoc/contract/contract_error.h"
#include "neoc/contract/neoc_uri.h"
#include "neoc/contract/native_contracts.h"

/* Note: Additional headers will be added as more modules are implemented
 * - neoc/protocol/neoc_rpc_client.h (full implementation)
 */

/**
 * @brief Initialize the NeoC SDK
 * 
 * Must be called before using any NeoC functions.
 * Initializes OpenSSL and other dependencies.
 * 
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_init(void);

/**
 * @brief Cleanup the NeoC SDK
 * 
 * Should be called when done using NeoC functions.
 * Cleans up OpenSSL and other resources.
 */
void neoc_cleanup(void);

/**
 * @brief Get NeoC SDK version string
 * 
 * @return Version string (e.g., "1.0.0")
 */
const char* neoc_get_version(void);

/**
 * @brief Get NeoC SDK build information
 * 
 * @return Build information string
 */
const char* neoc_get_build_info(void);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_H */
