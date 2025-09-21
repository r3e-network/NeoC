/**
 * @file encoding_example.c
 * @brief Encoding utilities example for NeoC SDK
 * 
 * Demonstrates encoding/decoding functionality:
 * - Hexadecimal encoding/decoding
 * - Base58 encoding/decoding
 * - Base64 encoding/decoding
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/neoc.h"

void print_bytes(const uint8_t* data, size_t length, const char* label) {
    printf("%s (%zu bytes): ", label, length);
    for (size_t i = 0; i < length && i < 16; i++) {
        printf("%02x", data[i]);
        if (i < length - 1 && i < 15) printf(" ");
    }
    if (length > 16) printf("...");
    printf("\n");
}

int main(void) {
    printf("NeoC SDK Encoding Example\n");
    printf("=========================\n\n");
    
    /* Initialize NeoC SDK */
    neoc_error_t result = neoc_init();
    if (result != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC SDK: %s\n", neoc_error_string(result));
        return 1;
    }
    
    /* Test data */
    const char* test_string = "Hello, Neo blockchain world!";
    const uint8_t* test_data = (const uint8_t*)test_string;
    size_t test_length = strlen(test_string);
    
    printf("Original data: %s\n", test_string);
    print_bytes(test_data, test_length, "Raw bytes");
    printf("\n");
    
    /* Example 1: Hexadecimal encoding/decoding */
    printf("Example 1: Hexadecimal Encoding\n");
    printf("===============================\n");
    
    /* Encode to hex */
    char* hex_encoded = neoc_hex_encode_alloc(test_data, test_length, false, false);
    if (hex_encoded) {
        printf("Hex encoded (lowercase): %s\n", hex_encoded);
    } else {
        printf("Failed to encode to hex\n");
    }
    
    char* hex_encoded_upper = neoc_hex_encode_alloc(test_data, test_length, true, true);
    if (hex_encoded_upper) {
        printf("Hex encoded (uppercase with prefix): %s\n", hex_encoded_upper);
    }
    
    /* Decode from hex */
    if (hex_encoded) {
        size_t decoded_length;
        uint8_t* hex_decoded = neoc_hex_decode_alloc(hex_encoded, &decoded_length);
        if (hex_decoded) {
            printf("Hex decoded: %.*s\n", (int)decoded_length, hex_decoded);
            printf("Roundtrip successful: %s\n", 
                   (decoded_length == test_length && memcmp(hex_decoded, test_data, test_length) == 0) ? "Yes" : "No");
            neoc_free(hex_decoded);
        } else {
            printf("Failed to decode hex\n");
        }
        neoc_free(hex_encoded);
    }
    
    if (hex_encoded_upper) {
        neoc_free(hex_encoded_upper);
    }
    printf("\n");
    
    /* Example 2: Base58 encoding/decoding */
    printf("Example 2: Base58 Encoding\n");
    printf("==========================\n");
    
    /* Encode to Base58 */
    char* base58_encoded = neoc_base58_encode_alloc(test_data, test_length);
    if (base58_encoded) {
        printf("Base58 encoded: %s\n", base58_encoded);
    } else {
        printf("Failed to encode to Base58\n");
    }
    
    /* Decode from Base58 */
    if (base58_encoded) {
        size_t decoded_length;
        uint8_t* base58_decoded = neoc_base58_decode_alloc(base58_encoded, &decoded_length);
        if (base58_decoded) {
            printf("Base58 decoded: %.*s\n", (int)decoded_length, base58_decoded);
            printf("Roundtrip successful: %s\n",
                   (decoded_length == test_length && memcmp(base58_decoded, test_data, test_length) == 0) ? "Yes" : "No");
            neoc_free(base58_decoded);
        } else {
            printf("Failed to decode Base58\n");
        }
        neoc_free(base58_encoded);
    }
    printf("\n");
    
    /* Example 3: Base58Check encoding/decoding */
    printf("Example 3: Base58Check Encoding\n");
    printf("===============================\n");
    
    /* Encode to Base58Check */
    char* base58check_encoded = neoc_base58_check_encode_alloc(test_data, test_length);
    if (base58check_encoded) {
        printf("Base58Check encoded: %s\n", base58check_encoded);
    } else {
        printf("Failed to encode to Base58Check\n");
    }
    
    /* Decode from Base58Check */
    if (base58check_encoded) {
        size_t decoded_length;
        uint8_t* base58check_decoded = neoc_base58_check_decode_alloc(base58check_encoded, &decoded_length);
        if (base58check_decoded) {
            printf("Base58Check decoded: %.*s\n", (int)decoded_length, base58check_decoded);
            printf("Roundtrip successful: %s\n",
                   (decoded_length == test_length && memcmp(base58check_decoded, test_data, test_length) == 0) ? "Yes" : "No");
            neoc_free(base58check_decoded);
        } else {
            printf("Failed to decode Base58Check\n");
        }
        neoc_free(base58check_encoded);
    }
    printf("\n");
    
    /* Example 4: Base64 encoding/decoding */
    printf("Example 4: Base64 Encoding\n");
    printf("==========================\n");
    
    /* Encode to Base64 */
    char* base64_encoded = neoc_base64_encode_alloc(test_data, test_length);
    if (base64_encoded) {
        printf("Base64 encoded: %s\n", base64_encoded);
    } else {
        printf("Failed to encode to Base64\n");
    }
    
    /* Decode from Base64 */
    if (base64_encoded) {
        size_t decoded_length;
        uint8_t* base64_decoded = neoc_base64_decode_alloc(base64_encoded, &decoded_length);
        if (base64_decoded) {
            printf("Base64 decoded: %.*s\n", (int)decoded_length, base64_decoded);
            printf("Roundtrip successful: %s\n",
                   (decoded_length == test_length && memcmp(base64_decoded, test_data, test_length) == 0) ? "Yes" : "No");
            neoc_free(base64_decoded);
        } else {
            printf("Failed to decode Base64\n");
        }
        neoc_free(base64_encoded);
    }
    printf("\n");
    
    /* Example 5: Binary data encoding */
    printf("Example 5: Binary Data Encoding\n");
    printf("===============================\n");
    
    /* Create some binary data */
    uint8_t binary_data[16];
    for (int i = 0; i < 16; i++) {
        binary_data[i] = (uint8_t)i * 17;  /* Create pattern */
    }
    
    print_bytes(binary_data, 16, "Binary data");
    
    /* Encode with different methods */
    char* bin_hex = neoc_hex_encode_alloc(binary_data, 16, false, false);
    char* bin_base58 = neoc_base58_encode_alloc(binary_data, 16);
    char* bin_base64 = neoc_base64_encode_alloc(binary_data, 16);
    
    if (bin_hex) {
        printf("As hex: %s\n", bin_hex);
        neoc_free(bin_hex);
    }
    if (bin_base58) {
        printf("As Base58: %s\n", bin_base58);
        neoc_free(bin_base58);
    }
    if (bin_base64) {
        printf("As Base64: %s\n", bin_base64);
        neoc_free(bin_base64);
    }
    printf("\n");
    
    /* Example 6: Validation */
    printf("Example 6: Input Validation\n");
    printf("===========================\n");
    
    const char* valid_hex = "deadbeef";
    const char* invalid_hex = "xyz123";
    const char* valid_base58 = "3mJr7AoUCHxNqd";
    const char* invalid_base58 = "O0Il";  /* Contains invalid chars */
    const char* valid_base64 = "SGVsbG8=";
    const char* invalid_base64 = "SGVs#bG8=";  /* Contains invalid char */
    
    printf("'%s' is valid hex: %s\n", valid_hex, 
           neoc_hex_is_valid_string(valid_hex, false) ? "Yes" : "No");
    printf("'%s' is valid hex: %s\n", invalid_hex,
           neoc_hex_is_valid_string(invalid_hex, false) ? "Yes" : "No");
    
    printf("'%s' is valid Base58: %s\n", valid_base58,
           neoc_base58_is_valid_string(valid_base58) ? "Yes" : "No");
    printf("'%s' is valid Base58: %s\n", invalid_base58,
           neoc_base58_is_valid_string(invalid_base58) ? "Yes" : "No");
    
    printf("'%s' is valid Base64: %s\n", valid_base64,
           neoc_base64_is_valid_string(valid_base64) ? "Yes" : "No");
    printf("'%s' is valid Base64: %s\n", invalid_base64,
           neoc_base64_is_valid_string(invalid_base64) ? "Yes" : "No");
    printf("\n");
    
    printf("Encoding example completed successfully!\n");
    
    /* Cleanup NeoC SDK */
    neoc_cleanup();
    
    return 0;
}
