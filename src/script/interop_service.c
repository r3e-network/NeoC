/**
 * @file interop_service.c
 * @brief Implementation of Neo VM interop services
 */

#include "neoc/script/interop_service.h"
#include "neoc/crypto/neoc_hash.h"
#include <string.h>

static neoc_interop_descriptor_t interop_table[] = {
    // System namespace
    {NEOC_INTEROP_SYSTEM_BINARY_SERIALIZE, "System.Binary.Serialize", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_DESERIALIZE, "System.Binary.Deserialize", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_BASE64ENCODE, "System.Binary.Base64Encode", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_BASE64DECODE, "System.Binary.Base64Decode", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_BASE58ENCODE, "System.Binary.Base58Encode", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_BASE58DECODE, "System.Binary.Base58Decode", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_ITOA, "System.Binary.Itoa", 0},
    {NEOC_INTEROP_SYSTEM_BINARY_ATOI, "System.Binary.Atoi", 0},
    
    {NEOC_INTEROP_SYSTEM_CONTRACT_CALL, "System.Contract.Call", 0},
    {NEOC_INTEROP_SYSTEM_CONTRACT_CALLNATIVE, "System.Contract.CallNative", 0},
    {NEOC_INTEROP_SYSTEM_CONTRACT_GETCALLFLAGS, "System.Contract.GetCallFlags", 0},
    {NEOC_INTEROP_SYSTEM_CONTRACT_CREATESTANDARDACCOUNT, "System.Contract.CreateStandardAccount", 0},
    {NEOC_INTEROP_SYSTEM_CONTRACT_CREATEMULTISIGACCOUNT, "System.Contract.CreateMultiSigAccount", 0},
    {NEOC_INTEROP_SYSTEM_CONTRACT_GETHASH, "System.Contract.GetHash", 0},
    
    {NEOC_INTEROP_SYSTEM_CRYPTO_CHECKSIG, "System.Crypto.CheckSig", 0},
    {NEOC_INTEROP_SYSTEM_CRYPTO_CHECKMULTISIG, "System.Crypto.CheckMultisig", 0},
    
    {NEOC_INTEROP_SYSTEM_ITERATOR_CREATE, "System.Iterator.Create", 0},
    {NEOC_INTEROP_SYSTEM_ITERATOR_NEXT, "System.Iterator.Next", 0},
    {NEOC_INTEROP_SYSTEM_ITERATOR_VALUE, "System.Iterator.Value", 0},
    
    {NEOC_INTEROP_SYSTEM_JSON_SERIALIZE, "System.Json.Serialize", 0},
    {NEOC_INTEROP_SYSTEM_JSON_DESERIALIZE, "System.Json.Deserialize", 0},
    
    {NEOC_INTEROP_SYSTEM_RUNTIME_PLATFORM, "System.Runtime.Platform", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETTRIGGER, "System.Runtime.GetTrigger", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETTIME, "System.Runtime.GetTime", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETSCRIPTCONTAINER, "System.Runtime.GetScriptContainer", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETEXECUTINGSCRIPTHASH, "System.Runtime.GetExecutingScriptHash", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETCALLINGSCRIPTHASH, "System.Runtime.GetCallingScriptHash", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETENTRYSCRIPTHASH, "System.Runtime.GetEntryScriptHash", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_CHECKWITNESS, "System.Runtime.CheckWitness", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETINVOCATIONCOUNTER, "System.Runtime.GetInvocationCounter", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_LOG, "System.Runtime.Log", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_NOTIFY, "System.Runtime.Notify", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETNOTIFICATIONS, "System.Runtime.GetNotifications", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GASREFUND, "System.Runtime.GasRefund", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_BURNGAS, "System.Runtime.BurnGas", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_CURRENTINDEX, "System.Runtime.CurrentIndex", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETNEXTVALIDATORS, "System.Runtime.GetNextValidators", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_GETNETWORK, "System.Runtime.GetNetwork", 0},
    {NEOC_INTEROP_SYSTEM_RUNTIME_LOADCONTRACT, "System.Runtime.LoadContract", 0},
    
    {NEOC_INTEROP_SYSTEM_STORAGE_GETCONTEXT, "System.Storage.GetContext", 0},
    {NEOC_INTEROP_SYSTEM_STORAGE_GETREADONLYCONTEXT, "System.Storage.GetReadOnlyContext", 0},
    {NEOC_INTEROP_SYSTEM_STORAGE_ASREADONLY, "System.Storage.AsReadOnly", 0},
    {NEOC_INTEROP_SYSTEM_STORAGE_GET, "System.Storage.Get", 0},
    {NEOC_INTEROP_SYSTEM_STORAGE_FIND, "System.Storage.Find", 0},
    {NEOC_INTEROP_SYSTEM_STORAGE_PUT, "System.Storage.Put", 0},
    {NEOC_INTEROP_SYSTEM_STORAGE_DELETE, "System.Storage.Delete", 0},
    
    // Neo namespace - Native contracts
    {NEOC_INTEROP_NEO_NATIVE_TOKENS_NEO, "Neo.Native.Tokens.NEO", 0},
    {NEOC_INTEROP_NEO_NATIVE_TOKENS_GAS, "Neo.Native.Tokens.GAS", 0},
    {NEOC_INTEROP_NEO_NATIVE_POLICY, "Neo.Native.Policy", 0},
    {NEOC_INTEROP_NEO_NATIVE_ROLEMANAGEMENT, "Neo.Native.RoleManagement", 0},
    {NEOC_INTEROP_NEO_NATIVE_ORACLE, "Neo.Native.Oracle", 0},
    {NEOC_INTEROP_NEO_NATIVE_LEDGER, "Neo.Native.Ledger", 0},
    {NEOC_INTEROP_NEO_NATIVE_MANAGEMENT, "Neo.Native.Management", 0},
    {NEOC_INTEROP_NEO_NATIVE_CRYPTO, "Neo.Native.Crypto", 0},
    {NEOC_INTEROP_NEO_NATIVE_STD, "Neo.Native.Std", 0},
};

static int interop_table_initialized = 0;

static void initialize_interop_table(void) {
    if (interop_table_initialized) {
        return;
    }
    
    // Calculate hashes for all interop services
    for (size_t i = 0; i < sizeof(interop_table) / sizeof(interop_table[0]); i++) {
        neoc_interop_descriptor_t* desc = (neoc_interop_descriptor_t*)&interop_table[i];
        uint8_t sha256[NEOC_SHA256_DIGEST_LENGTH];
        neoc_error_t err = neoc_sha256((const uint8_t*)desc->name, strlen(desc->name), sha256);
        if (err != NEOC_SUCCESS) {
            desc->hash = 0;
            continue;
        }

        desc->hash =
            (uint32_t)sha256[0] |
            ((uint32_t)sha256[1] << 8) |
            ((uint32_t)sha256[2] << 16) |
            ((uint32_t)sha256[3] << 24);
    }
    
    interop_table_initialized = 1;
}

const neoc_interop_descriptor_t* neoc_interop_get_descriptor(neoc_interop_service_t service) {
    initialize_interop_table();
    
    if (service >= 0 && service < NEOC_INTEROP_COUNT) {
        return &interop_table[service];
    }
    return NULL;
}

uint32_t neoc_interop_get_hash(neoc_interop_service_t service) {
    const neoc_interop_descriptor_t* desc = neoc_interop_get_descriptor(service);
    return desc ? desc->hash : 0;
}

const char* neoc_interop_get_name(neoc_interop_service_t service) {
    const neoc_interop_descriptor_t* desc = neoc_interop_get_descriptor(service);
    return desc ? desc->name : NULL;
}

neoc_interop_service_t neoc_interop_find_by_name(const char* name) {
    if (!name) {
        return NEOC_INTEROP_COUNT;
    }
    
    initialize_interop_table();
    
    for (size_t i = 0; i < sizeof(interop_table) / sizeof(interop_table[0]); i++) {
        if (strcmp(interop_table[i].name, name) == 0) {
            return interop_table[i].id;
        }
    }
    
    return NEOC_INTEROP_COUNT;
}

neoc_interop_service_t neoc_interop_find_by_hash(uint32_t hash) {
    initialize_interop_table();
    
    for (size_t i = 0; i < sizeof(interop_table) / sizeof(interop_table[0]); i++) {
        if (interop_table[i].hash == hash) {
            return interop_table[i].id;
        }
    }
    
    return NEOC_INTEROP_COUNT;
}
