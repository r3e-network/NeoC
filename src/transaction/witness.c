#include "neoc/transaction/witness.h"
#include "neoc/utils/neoc_hex.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

neoc_error_t neoc_witness_create(const uint8_t *invocation_script,
                                  size_t invocation_len,
                                  const uint8_t *verification_script,
                                  size_t verification_len,
                                  neoc_witness_t **witness) {
    if (!witness) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness pointer");
    }
    
    *witness = calloc(1, sizeof(neoc_witness_t));
    if (!*witness) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness");
    }
    
    // Copy invocation script
    if (invocation_script && invocation_len > 0) {
        (*witness)->invocation_script = malloc(invocation_len);
        if (!(*witness)->invocation_script) {
            free(*witness);
            *witness = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate invocation script");
        }
        memcpy((*witness)->invocation_script, invocation_script, invocation_len);
        (*witness)->invocation_script_len = invocation_len;
    }
    
    // Copy verification script
    if (verification_script && verification_len > 0) {
        (*witness)->verification_script = malloc(verification_len);
        if (!(*witness)->verification_script) {
            if ((*witness)->invocation_script) {
                free((*witness)->invocation_script);
            }
            free(*witness);
            *witness = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate verification script");
        }
        memcpy((*witness)->verification_script, verification_script, verification_len);
        (*witness)->verification_script_len = verification_len;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_create_from_signature(const uint8_t *signature,
                                                 size_t signature_len,
                                                 const uint8_t *public_key,
                                                 size_t public_key_len,
                                                 neoc_witness_t **witness) {
    if (!signature || !public_key || !witness) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Create invocation script (push signature)
    size_t invocation_len = 1 + signature_len;  // OpCode + signature
    uint8_t *invocation_script = malloc(invocation_len);
    if (!invocation_script) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate invocation script");
    }
    
    // Push signature opcode (0x40 + length for 64 bytes)
    invocation_script[0] = 0x40;
    memcpy(invocation_script + 1, signature, signature_len);
    
    // Create verification script (push public key + CHECKSIG)
    size_t verification_len = 1 + public_key_len + 1;  // OpCode + pubkey + CHECKSIG
    uint8_t *verification_script = malloc(verification_len);
    if (!verification_script) {
        free(invocation_script);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate verification script");
    }
    
    // Push public key opcode (0x21 for 33 bytes compressed key)
    verification_script[0] = 0x21;
    memcpy(verification_script + 1, public_key, public_key_len);
    verification_script[1 + public_key_len] = 0xAC;  // CHECKSIG opcode
    
    // Create witness
    neoc_error_t err = neoc_witness_create(invocation_script, invocation_len,
                                            verification_script, verification_len,
                                            witness);
    
    free(invocation_script);
    free(verification_script);
    
    return err;
}

size_t neoc_witness_get_size(const neoc_witness_t *witness) {
    if (!witness) return 0;
    
    size_t size = 0;
    
    // Invocation script: varint length + data
    size += 1;  // Assuming length fits in 1 byte
    size += witness->invocation_script_len;
    
    // Verification script: varint length + data
    size += 1;  // Assuming length fits in 1 byte
    size += witness->verification_script_len;
    
    return size;
}

neoc_error_t neoc_witness_serialize(const neoc_witness_t *witness,
                                     uint8_t **bytes,
                                     size_t *bytes_len) {
    if (!witness || !bytes || !bytes_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *bytes_len = neoc_witness_get_size(witness);
    *bytes = malloc(*bytes_len);
    if (!*bytes) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate bytes");
    }
    
    size_t offset = 0;
    
    // Write invocation script
    (*bytes)[offset++] = (uint8_t)witness->invocation_script_len;
    if (witness->invocation_script_len > 0) {
        memcpy(*bytes + offset, witness->invocation_script, witness->invocation_script_len);
        offset += witness->invocation_script_len;
    }
    
    // Write verification script
    (*bytes)[offset++] = (uint8_t)witness->verification_script_len;
    if (witness->verification_script_len > 0) {
        memcpy(*bytes + offset, witness->verification_script, witness->verification_script_len);
        offset += witness->verification_script_len;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_deserialize(const uint8_t *bytes,
                                       size_t bytes_len,
                                       neoc_witness_t **witness) {
    if (!bytes || !witness) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (bytes_len < 2) {  // Minimum: two length bytes
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Witness data too short");
    }
    
    size_t offset = 0;
    
    // Read invocation script length
    size_t invocation_len = bytes[offset++];
    if (offset + invocation_len > bytes_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Invalid invocation script length");
    }
    
    const uint8_t *invocation_script = (invocation_len > 0) ? bytes + offset : NULL;
    offset += invocation_len;
    
    // Read verification script length
    if (offset >= bytes_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Missing verification script");
    }
    size_t verification_len = bytes[offset++];
    if (offset + verification_len > bytes_len) {
        return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Invalid verification script length");
    }
    
    const uint8_t *verification_script = (verification_len > 0) ? bytes + offset : NULL;
    
    return neoc_witness_create(invocation_script, invocation_len,
                               verification_script, verification_len,
                               witness);
}

void neoc_witness_free(neoc_witness_t *witness) {
    if (!witness) return;
    
    if (witness->invocation_script) {
        free(witness->invocation_script);
    }
    
    if (witness->verification_script) {
        free(witness->verification_script);
    }
    
    free(witness);
}

char* neoc_witness_to_json(const neoc_witness_t *witness) {
    if (!witness) {
        return NULL;
    }
    
    // Convert scripts to hex strings
    char* invocation_hex = "";
    char* verification_hex = "";
    
    if (witness->invocation_script && witness->invocation_script_len > 0) {
        invocation_hex = neoc_hex_encode_alloc(witness->invocation_script, witness->invocation_script_len, false, false);
        if (!invocation_hex) invocation_hex = "";
    }
    
    if (witness->verification_script && witness->verification_script_len > 0) {
        verification_hex = neoc_hex_encode_alloc(witness->verification_script, witness->verification_script_len, false, false);
        if (!verification_hex) verification_hex = "";
    }
    
    // Create JSON string
    size_t json_size = 256 + strlen(invocation_hex) + strlen(verification_hex);
    char* json = malloc(json_size);
    if (!json) {
        if (invocation_hex && strlen(invocation_hex) > 0) free(invocation_hex);
        if (verification_hex && strlen(verification_hex) > 0) free(verification_hex);
        return NULL;
    }
    
    snprintf(json, json_size,
             "{\"invocation\":\"%s\",\"verification\":\"%s\"}",
             invocation_hex, verification_hex);
    
    // Free hex strings if they were allocated
    if (invocation_hex && strlen(invocation_hex) > 0) free(invocation_hex);
    if (verification_hex && strlen(verification_hex) > 0) free(verification_hex);
    
    return json;
}

neoc_error_t neoc_witness_clone(const neoc_witness_t *source, neoc_witness_t **dest) {
    if (!source || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    return neoc_witness_create(source->invocation_script,
                              source->invocation_script_len,
                              source->verification_script,
                              source->verification_script_len,
                              dest);
}
