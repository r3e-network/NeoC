/**
 * @file basic_example.c
 * @brief Basic usage example for NeoC SDK
 * 
 * Demonstrates core functionality including:
 * - SDK initialization
 * - Hash160 and Hash256 operations
 * - Memory management
 * - Error handling
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/neoc.h"

void print_hash160(const neoc_hash160_t* hash, const char* label) {
    char hex_string[41];  // 40 chars + null terminator
    neoc_error_t result = neoc_hash160_to_hex(hash, hex_string, sizeof(hex_string), false);
    if (result == NEOC_SUCCESS) {
        printf("%s: %s\n", label, hex_string);
    } else {
        printf("%s: Error converting to hex (%d)\n", label, result);
    }
}

void print_hash256(const neoc_hash256_t* hash, const char* label) {
    char hex_string[65];  // 64 chars + null terminator
    neoc_error_t result = neoc_hash256_to_hex(hash, hex_string, sizeof(hex_string), false);
    if (result == NEOC_SUCCESS) {
        printf("%s: %s\n", label, hex_string);
    } else {
        printf("%s: Error converting to hex (%d)\n", label, result);
    }
}

int main(void) {
    printf("NeoC SDK Basic Example\n");
    printf("======================\n\n");
    
    /* Initialize NeoC SDK */
    neoc_error_t result = neoc_init();
    if (result != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC SDK: %s\n", neoc_error_string(result));
        return 1;
    }
    
    printf("NeoC SDK Version: %s\n", neoc_get_version());
    printf("Build Info: %s\n\n", neoc_get_build_info());
    
    /* Example 1: Create Hash160 from hex string */
    printf("Example 1: Hash160 from hex string\n");
    printf("----------------------------------\n");
    
    const char* hash160_hex = "17694821c6e3ea8b7a7d770952e7de86c73d94c3";
    neoc_hash160_t hash160_1;
    
    result = neoc_hash160_from_hex(&hash160_1, hash160_hex);
    if (result == NEOC_SUCCESS) {
        printf("Input hex: %s\n", hash160_hex);
        print_hash160(&hash160_1, "Hash160");
        
        /* Convert to address */
        char address[64];
        result = neoc_hash160_to_address(&hash160_1, address, sizeof(address));
        if (result == NEOC_SUCCESS) {
            printf("Address: %s\n", address);
        } else {
            printf("Address conversion failed: %s\n", neoc_error_string(result));
        }
    } else {
        printf("Failed to create Hash160 from hex: %s\n", neoc_error_string(result));
    }
    printf("\n");
    
    /* Example 2: Create Hash160 from script */
    printf("Example 2: Hash160 from script\n");
    printf("------------------------------\n");
    
    /* Sample verification script (push public key + CheckSig) */
    const char* script_hex = "0c2102b53b2dd3e1be3dfb4bb8e5a56e3215db0b6b45e4c2b1b56ed9f4f6f3a52b6b4e2b41687e51";
    neoc_hash160_t hash160_2;
    
    result = neoc_hash160_from_script_hex(&hash160_2, script_hex);
    if (result == NEOC_SUCCESS) {
        printf("Script hex: %s\n", script_hex);
        print_hash160(&hash160_2, "Script hash");
    } else if (result == NEOC_ERROR_NOT_IMPLEMENTED) {
        printf("Script hash calculation not yet implemented\n");
    } else {
        printf("Failed to create Hash160 from script: %s\n", neoc_error_string(result));
    }
    printf("\n");
    
    /* Example 3: Hash256 operations */
    printf("Example 3: Hash256 operations\n");
    printf("-----------------------------\n");
    
    const char* data = "Hello, Neo blockchain!";
    neoc_hash256_t hash256_1;
    
    result = neoc_hash256_from_data_hash(&hash256_1, (const uint8_t*)data, strlen(data));
    if (result == NEOC_SUCCESS) {
        printf("Input data: %s\n", data);
        print_hash256(&hash256_1, "SHA-256 hash");
        
        /* Double hash */
        neoc_hash256_t hash256_double;
        result = neoc_hash256_from_data_double_hash(&hash256_double, (const uint8_t*)data, strlen(data));
        if (result == NEOC_SUCCESS) {
            print_hash256(&hash256_double, "Double SHA-256");
        }
    } else {
        printf("Failed to hash data: %s\n", neoc_error_string(result));
    }
    printf("\n");
    
    /* Example 4: Hash comparison */
    printf("Example 4: Hash comparison\n");
    printf("-------------------------\n");
    
    neoc_hash160_t zero_hash;
    neoc_hash160_init_zero(&zero_hash);
    
    printf("Is hash160_1 zero? %s\n", neoc_hash160_is_zero(&hash160_1) ? "Yes" : "No");
    printf("Is zero_hash zero? %s\n", neoc_hash160_is_zero(&zero_hash) ? "Yes" : "No");
    printf("Are hash160_1 and zero_hash equal? %s\n", 
           neoc_hash160_equal(&hash160_1, &zero_hash) ? "Yes" : "No");
    
    /* Copy hash */
    neoc_hash160_t hash160_copy;
    result = neoc_hash160_copy(&hash160_copy, &hash160_1);
    if (result == NEOC_SUCCESS) {
        printf("Are hash160_1 and hash160_copy equal? %s\n",
               neoc_hash160_equal(&hash160_1, &hash160_copy) ? "Yes" : "No");
    }
    printf("\n");
    
    /* Example 5: Error handling */
    printf("Example 5: Error handling\n");
    printf("------------------------\n");
    
    /* Try to create hash from invalid hex */
    neoc_hash160_t invalid_hash;
    result = neoc_hash160_from_hex(&invalid_hash, "invalid_hex_string");
    printf("Creating Hash160 from invalid hex: %s\n", neoc_error_string(result));
    
    /* Try to create hash from wrong-length hex */
    result = neoc_hash160_from_hex(&invalid_hash, "1234");  /* Too short */
    printf("Creating Hash160 from short hex: %s\n", neoc_error_string(result));
    printf("\n");
    
    /* Example 6: Memory operations */
    printf("Example 6: Memory operations\n");
    printf("---------------------------\n");
    
    /* Get raw bytes */
    uint8_t hash_bytes[20];
    result = neoc_hash160_to_bytes(&hash160_1, hash_bytes, sizeof(hash_bytes));
    if (result == NEOC_SUCCESS) {
        printf("Raw bytes (first 8): ");
        for (int i = 0; i < 8; i++) {
            printf("%02x ", hash_bytes[i]);
        }
        printf("...\n");
        
        /* Little-endian bytes */
        uint8_t le_bytes[20];
        result = neoc_hash160_to_little_endian_bytes(&hash160_1, le_bytes, sizeof(le_bytes));
        if (result == NEOC_SUCCESS) {
            printf("Little-endian (first 8): ");
            for (int i = 0; i < 8; i++) {
                printf("%02x ", le_bytes[i]);
            }
            printf("...\n");
        }
    }
    printf("\n");
    
    printf("Example completed successfully!\n");
    
    /* Cleanup NeoC SDK */
    neoc_cleanup();
    
    return 0;
}
