/**
 * @file test_wif.c
 * @brief Unit tests for WIF (Wallet Import Format) functionality
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/crypto/wif.h"
#include "neoc/crypto/ec_private_key.h"
#include "neoc/utils/hex.h"
#include "neoc/utils/base58.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"

#define VALID_WIF "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13A"
#define PRIVATE_KEY_HEX "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3a3"

static void test_valid_wif_to_private_key(void) {
    printf("Testing valid WIF to private key...\n");
    
    neoc_ec_private_key_t *private_key = NULL;
    neoc_error_t result = neoc_wif_to_private_key(VALID_WIF, &private_key);
    assert(result == NEOC_SUCCESS);
    assert(private_key != NULL);
    
    // Get private key bytes
    uint8_t key_bytes[32];
    size_t key_len;
    result = neoc_ec_private_key_to_bytes(private_key, key_bytes, sizeof(key_bytes), &key_len);
    assert(result == NEOC_SUCCESS);
    assert(key_len == 32);
    
    // Convert to hex
    char hex_buffer[65];
    neoc_hex_encode(key_bytes, key_len, hex_buffer, sizeof(hex_buffer));
    assert(strcmp(hex_buffer, PRIVATE_KEY_HEX) == 0);
    
    neoc_ec_private_key_free(private_key);
    printf("✓ test_valid_wif_to_private_key passed\n");
}

static void test_wrongly_sized_wifs(void) {
    printf("Testing wrongly sized WIFs...\n");
    
    const char *too_large = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13Ahc7S";
    const char *too_small = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWML";
    
    neoc_ec_private_key_t *private_key = NULL;
    
    // Test too large
    neoc_error_t result = neoc_wif_to_private_key(too_large, &private_key);
    assert(result == NEOC_ERROR_INVALID_WIF_FORMAT);
    assert(private_key == NULL);
    
    // Test too small
    result = neoc_wif_to_private_key(too_small, &private_key);
    assert(result == NEOC_ERROR_INVALID_WIF_FORMAT);
    assert(private_key == NULL);
    
    printf("✓ test_wrongly_sized_wifs passed\n");
}

static void test_wrong_first_byte_wif(void) {
    printf("Testing wrong first byte WIF...\n");
    
    // Decode valid WIF
    uint8_t decoded[256];
    size_t decoded_len = sizeof(decoded);
    neoc_error_t result = neoc_base58_decode(VALID_WIF, strlen(VALID_WIF), decoded, &decoded_len);
    assert(result == NEOC_SUCCESS);
    
    // Modify first byte
    decoded[0] = 0x81;
    
    // Re-encode
    char wrong_wif[256];
    result = neoc_base58_encode(decoded, decoded_len, wrong_wif, sizeof(wrong_wif));
    assert(result == NEOC_SUCCESS);
    
    // Try to convert to private key
    neoc_ec_private_key_t *private_key = NULL;
    result = neoc_wif_to_private_key(wrong_wif, &private_key);
    assert(result == NEOC_ERROR_INVALID_WIF_FORMAT);
    assert(private_key == NULL);
    
    printf("✓ test_wrong_first_byte_wif passed\n");
}

static void test_wrong_byte33_wif(void) {
    printf("Testing wrong byte 33 WIF...\n");
    
    // Decode valid WIF
    uint8_t decoded[256];
    size_t decoded_len = sizeof(decoded);
    neoc_error_t result = neoc_base58_decode(VALID_WIF, strlen(VALID_WIF), decoded, &decoded_len);
    assert(result == NEOC_SUCCESS);
    
    // Modify byte 33
    decoded[33] = 0x00;
    
    // Re-encode
    char wrong_wif[256];
    result = neoc_base58_encode(decoded, decoded_len, wrong_wif, sizeof(wrong_wif));
    assert(result == NEOC_SUCCESS);
    
    // Try to convert to private key
    neoc_ec_private_key_t *private_key = NULL;
    result = neoc_wif_to_private_key(wrong_wif, &private_key);
    assert(result == NEOC_ERROR_INVALID_WIF_FORMAT);
    assert(private_key == NULL);
    
    printf("✓ test_wrong_byte33_wif passed\n");
}

static void test_valid_private_key_to_wif(void) {
    printf("Testing valid private key to WIF...\n");
    
    // Convert hex to bytes
    uint8_t key_bytes[32];
    size_t key_len = sizeof(key_bytes);
    neoc_error_t result = neoc_hex_decode(PRIVATE_KEY_HEX, strlen(PRIVATE_KEY_HEX), 
                                           key_bytes, &key_len);
    assert(result == NEOC_SUCCESS);
    assert(key_len == 32);
    
    // Create private key
    neoc_ec_private_key_t *private_key = NULL;
    result = neoc_ec_private_key_from_bytes(&private_key, key_bytes, key_len);
    assert(result == NEOC_SUCCESS);
    
    // Convert to WIF
    char wif_buffer[256];
    result = neoc_private_key_to_wif(private_key, wif_buffer, sizeof(wif_buffer));
    assert(result == NEOC_SUCCESS);
    assert(strcmp(wif_buffer, VALID_WIF) == 0);
    
    neoc_ec_private_key_free(private_key);
    printf("✓ test_valid_private_key_to_wif passed\n");
}

static void test_wrongly_sized_private_key(void) {
    printf("Testing wrongly sized private key...\n");
    
    // Create wrong sized key (31 bytes instead of 32)
    const char *wrong_hex = "9117f4bf9be717c9a90994326897f4243503accd06712162267e77f18b49c3";
    uint8_t key_bytes[31];
    size_t key_len = sizeof(key_bytes);
    
    neoc_error_t result = neoc_hex_decode(wrong_hex, strlen(wrong_hex), key_bytes, &key_len);
    assert(result == NEOC_SUCCESS);
    assert(key_len == 31);
    
    // Try to create private key
    neoc_ec_private_key_t *private_key = NULL;
    result = neoc_ec_private_key_from_bytes(&private_key, key_bytes, key_len);
    assert(result == NEOC_ERROR_INVALID_KEY_SIZE);
    assert(private_key == NULL);
    
    printf("✓ test_wrongly_sized_private_key passed\n");
}

static void test_wif_checksum_validation(void) {
    printf("Testing WIF checksum validation...\n");
    
    // Create WIF with invalid checksum
    const char *invalid_checksum_wif = "L25kgAQJXNHnhc7Sx9bomxxwVSMsZdkaNQ3m2VfHrnLzKWMLP13B"; // Changed last char
    
    neoc_ec_private_key_t *private_key = NULL;
    neoc_error_t result = neoc_wif_to_private_key(invalid_checksum_wif, &private_key);
    assert(result == NEOC_ERROR_INVALID_WIF_CHECKSUM);
    assert(private_key == NULL);
    
    printf("✓ test_wif_checksum_validation passed\n");
}

static void test_wif_round_trip(void) {
    printf("Testing WIF round trip conversion...\n");
    
    // Start with WIF
    neoc_ec_private_key_t *private_key1 = NULL;
    neoc_error_t result = neoc_wif_to_private_key(VALID_WIF, &private_key1);
    assert(result == NEOC_SUCCESS);
    
    // Convert back to WIF
    char wif_buffer[256];
    result = neoc_private_key_to_wif(private_key1, wif_buffer, sizeof(wif_buffer));
    assert(result == NEOC_SUCCESS);
    assert(strcmp(wif_buffer, VALID_WIF) == 0);
    
    // Convert WIF back to private key
    neoc_ec_private_key_t *private_key2 = NULL;
    result = neoc_wif_to_private_key(wif_buffer, &private_key2);
    assert(result == NEOC_SUCCESS);
    
    // Compare private keys
    uint8_t key1_bytes[32], key2_bytes[32];
    size_t key1_len, key2_len;
    
    result = neoc_ec_private_key_to_bytes(private_key1, key1_bytes, sizeof(key1_bytes), &key1_len);
    assert(result == NEOC_SUCCESS);
    
    result = neoc_ec_private_key_to_bytes(private_key2, key2_bytes, sizeof(key2_bytes), &key2_len);
    assert(result == NEOC_SUCCESS);
    
    assert(key1_len == key2_len);
    assert(memcmp(key1_bytes, key2_bytes, key1_len) == 0);
    
    neoc_ec_private_key_free(private_key1);
    neoc_ec_private_key_free(private_key2);
    printf("✓ test_wif_round_trip passed\n");
}

int main(void) {
    printf("\n=== WIF Tests ===\n\n");
    
    // Initialize NeoC library
    neoc_error_t result = neoc_init();
    if (result != NEOC_SUCCESS) {
        printf("Failed to initialize NeoC library: %d\n", result);
        return 1;
    }
    
    // Run tests
    test_valid_wif_to_private_key();
    test_wrongly_sized_wifs();
    test_wrong_first_byte_wif();
    test_wrong_byte33_wif();
    test_valid_private_key_to_wif();
    test_wrongly_sized_private_key();
    test_wif_checksum_validation();
    test_wif_round_trip();
    
    // Cleanup
    neoc_cleanup();
    
    printf("\n✅ All WIF tests passed!\n\n");
    return 0;
}
