/**
 * @file interop_service.h
 * @brief Neo VM interop service definitions
 */

#ifndef NEOC_INTEROP_SERVICE_H
#define NEOC_INTEROP_SERVICE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interop service identifiers
 */
typedef enum {
    // System namespace
    NEOC_INTEROP_SYSTEM_BINARY_SERIALIZE,
    NEOC_INTEROP_SYSTEM_BINARY_DESERIALIZE,
    NEOC_INTEROP_SYSTEM_BINARY_BASE64ENCODE,
    NEOC_INTEROP_SYSTEM_BINARY_BASE64DECODE,
    NEOC_INTEROP_SYSTEM_BINARY_BASE58ENCODE,
    NEOC_INTEROP_SYSTEM_BINARY_BASE58DECODE,
    NEOC_INTEROP_SYSTEM_BINARY_ITOA,
    NEOC_INTEROP_SYSTEM_BINARY_ATOI,
    
    NEOC_INTEROP_SYSTEM_CONTRACT_CALL,
    NEOC_INTEROP_SYSTEM_CONTRACT_CALLNATIVE,
    NEOC_INTEROP_SYSTEM_CONTRACT_GETCALLFLAGS,
    NEOC_INTEROP_SYSTEM_CONTRACT_CREATESTANDARDACCOUNT,
    NEOC_INTEROP_SYSTEM_CONTRACT_CREATEMULTISIGACCOUNT,
    NEOC_INTEROP_SYSTEM_CONTRACT_GETHASH,
    
    NEOC_INTEROP_SYSTEM_CRYPTO_CHECKSIG,
    NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG,
    
    NEOC_INTEROP_SYSTEM_ITERATOR_CREATE,
    NEOC_INTEROP_SYSTEM_ITERATOR_NEXT,
    NEOC_INTEROP_SYSTEM_ITERATOR_VALUE,
    
    NEOC_INTEROP_SYSTEM_JSON_SERIALIZE,
    NEOC_INTEROP_SYSTEM_JSON_DESERIALIZE,
    
    NEOC_INTEROP_SYSTEM_RUNTIME_PLATFORM,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETTRIGGER,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETTIME,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETSCRIPTCONTAINER,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETEXECUTINGSCRIPTHASH,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETCALLINGSCRIPTHASH,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETENTRYSCRIPTHASH,
    NEOC_INTEROP_SYSTEM_RUNTIME_CHECKWITNESS,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETINVOCATIONCOUNTER,
    NEOC_INTEROP_SYSTEM_RUNTIME_LOG,
    NEOC_INTEROP_SYSTEM_RUNTIME_NOTIFY,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETNOTIFICATIONS,
    NEOC_INTEROP_SYSTEM_RUNTIME_GASREFUND,
    NEOC_INTEROP_SYSTEM_RUNTIME_BURNGAS,
    NEOC_INTEROP_SYSTEM_RUNTIME_CURRENTINDEX,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETNEXTVALIDATORS,
    NEOC_INTEROP_SYSTEM_RUNTIME_GETNETWORK,
    NEOC_INTEROP_SYSTEM_RUNTIME_LOADCONTRACT,
    
    NEOC_INTEROP_SYSTEM_STORAGE_GETCONTEXT,
    NEOC_INTEROP_SYSTEM_STORAGE_GETREADONLYCONTEXT,
    NEOC_INTEROP_SYSTEM_STORAGE_ASREADONLY,
    NEOC_INTEROP_SYSTEM_STORAGE_GET,
    NEOC_INTEROP_SYSTEM_STORAGE_FIND,
    NEOC_INTEROP_SYSTEM_STORAGE_PUT,
    NEOC_INTEROP_SYSTEM_STORAGE_DELETE,
    
    // Neo namespace - Native contracts
    NEOC_INTEROP_NEO_NATIVE_TOKENS_NEO,
    NEOC_INTEROP_NEO_NATIVE_TOKENS_GAS,
    NEOC_INTEROP_NEO_NATIVE_POLICY,
    NEOC_INTEROP_NEO_NATIVE_ROLEMANAGEMENT,
    NEOC_INTEROP_NEO_NATIVE_ORACLE,
    NEOC_INTEROP_NEO_NATIVE_LEDGER,
    NEOC_INTEROP_NEO_NATIVE_MANAGEMENT,
    NEOC_INTEROP_NEO_NATIVE_CRYPTO,
    NEOC_INTEROP_NEO_NATIVE_STD,
    
    NEOC_INTEROP_COUNT
} neoc_interop_service_t;

/**
 * @brief Interop service descriptor
 */
typedef struct {
    neoc_interop_service_t id;
    const char* name;
    uint32_t hash;
} neoc_interop_descriptor_t;

/**
 * @brief Get the interop service descriptor
 * 
 * @param service The interop service
 * @return The descriptor (do not free)
 */
const neoc_interop_descriptor_t* neoc_interop_get_descriptor(neoc_interop_service_t service);

/**
 * @brief Get the interop service hash
 * 
 * @param service The interop service
 * @return The 4-byte hash
 */
uint32_t neoc_interop_get_hash(neoc_interop_service_t service);

/**
 * @brief Get the interop service name
 * 
 * @param service The interop service
 * @return The name (do not free)
 */
const char* neoc_interop_get_name(neoc_interop_service_t service);

/**
 * @brief Find interop service by name
 * 
 * @param name The service name
 * @return The service ID or NEOC_INTEROP_COUNT if not found
 */
neoc_interop_service_t neoc_interop_find_by_name(const char* name);

/**
 * @brief Find interop service by hash
 * 
 * @param hash The service hash
 * @return The service ID or NEOC_INTEROP_COUNT if not found
 */
neoc_interop_service_t neoc_interop_find_by_hash(uint32_t hash);

#ifdef __cplusplus
}
#endif

#endif // NEOC_INTEROP_SERVICE_H
