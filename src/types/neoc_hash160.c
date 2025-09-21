/**
 * @file neoc_hash160.c
 * @brief Implementation of Hash160 type for NeoC SDK
 */

#include "neoc/types/neoc_hash160.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/utils/neoc_base58.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/neoc_memory.h"
#include <string.h>

/* Constant zero Hash160 */
const neoc_hash160_t NEOC_HASH160_ZERO_VALUE = {{0}};

neoc_error_t neoc_hash160_init_zero(neoc_hash160_t* hash) {
    if (!hash) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memset(hash->data, 0, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_bytes(neoc_hash160_t* hash, const uint8_t data[NEOC_HASH160_SIZE]) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memcpy(hash->data, data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_data(neoc_hash160_t* hash, const uint8_t* data, size_t length) {
    if (!hash || !data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (length != NEOC_HASH160_SIZE) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    memcpy(hash->data, data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_hex(neoc_hash160_t* hash, const char* hex_string) {
    if (!hash || !hex_string) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (!neoc_hex_is_valid_string(hex_string, true)) {
        return NEOC_ERROR_INVALID_HEX;
    }
    
    size_t decoded_length;
    neoc_error_t result = neoc_hex_decode(hex_string, hash->data, NEOC_HASH160_SIZE, &decoded_length);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    if (decoded_length != NEOC_HASH160_SIZE) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_address(neoc_hash160_t* hash, const char* address) {
    if (!hash || !address) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Decode Base58Check address */
    size_t decoded_length;
    uint8_t* decoded = neoc_base58_check_decode_alloc(address, &decoded_length);
    if (!decoded) {
        return NEOC_ERROR_INVALID_BASE58;
    }
    
    /* Address should be version byte + 20 byte hash */
    if (decoded_length != 1 + NEOC_HASH160_SIZE) {
        neoc_free(decoded);
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    /* Check version byte */
    if (decoded[0] != NEOC_ADDRESS_VERSION) {
        neoc_free(decoded);
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    /* Extract hash160 (skip version byte) */
    memcpy(hash->data, decoded + 1, NEOC_HASH160_SIZE);
    
    neoc_secure_free(decoded, decoded_length);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_script(neoc_hash160_t* hash, const uint8_t* script, size_t script_length) {
    if (!hash || !script) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Calculate hash160 of script */
    uint8_t hash_result[NEOC_RIPEMD160_DIGEST_LENGTH];
    neoc_error_t result = neoc_hash160(script, script_length, hash_result);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    /* Neo stores script hash in little-endian format */
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        hash->data[i] = hash_result[NEOC_HASH160_SIZE - 1 - i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_from_script_hex(neoc_hash160_t* hash, const char* script_hex) {
    if (!hash || !script_hex) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t script_length;
    uint8_t* script = neoc_hex_decode_alloc(script_hex, &script_length);
    if (!script) {
        return NEOC_ERROR_INVALID_HEX;
    }
    
    neoc_error_t result = neoc_hash160_from_script(hash, script, script_length);
    neoc_free(script);
    
    return result;
}

neoc_error_t neoc_hash160_from_public_key(neoc_hash160_t* hash, const uint8_t public_key_data[NEOC_PUBLIC_KEY_SIZE_COMPRESSED]) {
    if (!hash || !public_key_data) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Build verification script: PUSH<pubkey> SYSCALL System.Crypto.CheckSig */
    uint8_t script[67];
    size_t offset = 0;
    
    /* OpCode.PUSH1 + 33 bytes of public key */
    script[offset++] = 0x21; /* PUSHDATA1 with 33 bytes */
    memcpy(script + offset, public_key_data, NEOC_PUBLIC_KEY_SIZE_COMPRESSED);
    offset += NEOC_PUBLIC_KEY_SIZE_COMPRESSED;
    
    /* OpCode.SYSCALL + interop service hash (System.Crypto.CheckSig) */
    script[offset++] = 0x41; /* SYSCALL */
    /* System.Crypto.CheckSig hash: 0x41c40f64 */
    script[offset++] = 0x41;
    script[offset++] = 0xc4;
    script[offset++] = 0x0f;
    script[offset++] = 0x64;
    
    /* Calculate hash160 of the script */
    return neoc_hash160_from_script(hash, script, offset);
}

neoc_error_t neoc_hash160_from_public_keys(neoc_hash160_t* hash, 
                                           const uint8_t public_keys[][NEOC_PUBLIC_KEY_SIZE_COMPRESSED],
                                           size_t num_keys,
                                           size_t signing_threshold) {
    if (!hash || !public_keys) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (num_keys == 0 || signing_threshold == 0 || signing_threshold > num_keys) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    if (num_keys > NEOC_MAX_PUBLIC_KEYS_PER_MULTISIG_ACCOUNT) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    /* Build multi-sig verification script */
    /* Script format: PUSH<m> <pubkey1> <pubkey2> ... <pubkeyn> PUSH<n> SYSCALL System.Crypto.CheckMultiSig */
    uint8_t script[1024]; /* Max size for multi-sig script */
    size_t offset = 0;
    
    /* Push signing threshold (m) */
    if (signing_threshold <= 16) {
        script[offset++] = 0x51 + (uint8_t)(signing_threshold - 1); /* PUSH1-PUSH16 */
    } else {
        script[offset++] = 0x00; /* PUSHINT8 */
        script[offset++] = (uint8_t)signing_threshold;
    }
    
    /* Push all public keys */
    for (size_t i = 0; i < num_keys; i++) {
        script[offset++] = 0x21; /* PUSHDATA1 with 33 bytes */
        memcpy(script + offset, public_keys[i], NEOC_PUBLIC_KEY_SIZE_COMPRESSED);
        offset += NEOC_PUBLIC_KEY_SIZE_COMPRESSED;
    }
    
    /* Push number of keys (n) */
    if (num_keys <= 16) {
        script[offset++] = 0x51 + (uint8_t)(num_keys - 1); /* PUSH1-PUSH16 */
    } else {
        script[offset++] = 0x00; /* PUSHINT8 */
        script[offset++] = (uint8_t)num_keys;
    }
    
    /* OpCode.SYSCALL + interop service hash (System.Crypto.CheckMultiSig) */
    script[offset++] = 0x41; /* SYSCALL */
    /* System.Crypto.CheckMultiSig hash: 0xbb3b1f6f */
    script[offset++] = 0xbb;
    script[offset++] = 0x3b;
    script[offset++] = 0x1f;
    script[offset++] = 0x6f;
    
    /* Calculate hash160 of the script */
    return neoc_hash160_from_script(hash, script, offset);
}

neoc_error_t neoc_hash160_copy(neoc_hash160_t* dest, const neoc_hash160_t* src) {
    if (!dest || !src) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    memcpy(dest->data, src->data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_to_bytes(const neoc_hash160_t* hash, uint8_t* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < NEOC_HASH160_SIZE) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    memcpy(buffer, hash->data, NEOC_HASH160_SIZE);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_to_little_endian_bytes(const neoc_hash160_t* hash, uint8_t* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (buffer_size < NEOC_HASH160_SIZE) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    /* Reverse byte order */
    for (size_t i = 0; i < NEOC_HASH160_SIZE; i++) {
        buffer[i] = hash->data[NEOC_HASH160_SIZE - 1 - i];
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_hash160_to_hex(const neoc_hash160_t* hash, char* buffer, size_t buffer_size, bool uppercase) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t required_size = neoc_hex_encode_buffer_size(NEOC_HASH160_SIZE, false);
    if (buffer_size < required_size) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    return neoc_hex_encode(hash->data, NEOC_HASH160_SIZE, buffer, buffer_size, uppercase, false);
}

neoc_error_t neoc_hash160_to_address(const neoc_hash160_t* hash, char* buffer, size_t buffer_size) {
    if (!hash || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Create versioned hash (version byte + hash) */
    uint8_t versioned[1 + NEOC_HASH160_SIZE];
    versioned[0] = NEOC_ADDRESS_VERSION;
    memcpy(versioned + 1, hash->data, NEOC_HASH160_SIZE);
    
    /* Encode as Base58Check */
    char* address = neoc_base58_check_encode_alloc(versioned, sizeof(versioned));
    if (!address) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    size_t address_len = strlen(address);
    if (buffer_size <= address_len) {
        neoc_free(address);
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    strcpy(buffer, address);
    neoc_free(address);
    
    return NEOC_SUCCESS;
}

int neoc_hash160_compare(const neoc_hash160_t* a, const neoc_hash160_t* b) {
    if (!a || !b) {
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    
    return memcmp(a->data, b->data, NEOC_HASH160_SIZE);
}

bool neoc_hash160_equal(const neoc_hash160_t* a, const neoc_hash160_t* b) {
    return neoc_hash160_compare(a, b) == 0;
}

bool neoc_hash160_is_zero(const neoc_hash160_t* hash) {
    if (!hash) {
        return false;
    }
    
    return neoc_hash160_equal(hash, &NEOC_HASH160_ZERO_VALUE);
}

neoc_error_t neoc_hash160_serialize(const neoc_hash160_t* hash, neoc_binary_writer_t* writer) {
    if (!hash || !writer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Write hash bytes directly */
    return neoc_binary_writer_write_bytes(writer, hash->data, NEOC_HASH160_SIZE);
}

neoc_error_t neoc_hash160_deserialize(neoc_hash160_t* hash, neoc_binary_reader_t* reader) {
    if (!hash || !reader) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Read hash bytes directly */
    return neoc_binary_reader_read_bytes(reader, hash->data, NEOC_HASH160_SIZE);
}

size_t neoc_hash160_serialized_size(void) {
    return NEOC_HASH160_SIZE;
}

neoc_error_t neoc_hash160_from_string(const char *str, neoc_hash160_t *hash) {
    if (!str || !hash) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    const char *hex_str = str;
    
    /* Skip '0x' prefix if present */
    if (strlen(str) >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        hex_str = str + 2;
    }
    
    /* Check if length is correct for Hash160 (40 hex characters) */
    size_t hex_len = strlen(hex_str);
    if (hex_len != 40) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    
    return neoc_hash160_from_hex(hash, hex_str);
}

neoc_error_t neoc_hash160_to_string(const neoc_hash160_t* hash, char* buffer, size_t buffer_size) {
    return neoc_hash160_to_hex(hash, buffer, buffer_size, false);
}
