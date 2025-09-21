#include <neoc/neoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("NeoC Cryptography Example\n");
    printf("========================\n\n");
    
    // Initialize NeoC SDK
    if (neoc_init() != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC\n");
        return 1;
    }
    
    // 1. Create a random key pair
    printf("1. Creating random EC key pair...\n");
    neoc_ec_key_pair_t *key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create key pair: %d\n", err);
        neoc_cleanup();
        return 1;
    }
    
    // Get the public key
    uint8_t *public_key = NULL;
    size_t pub_key_len = 0;
    err = neoc_ec_public_key_get_encoded(key_pair->public_key, true, &public_key, &pub_key_len);
    if (err == NEOC_SUCCESS) {
        print_hex("Public Key (compressed)", public_key, pub_key_len);
        free(public_key);
    }
    
    // Get the address
    char *address = NULL;
    err = neoc_ec_key_pair_get_address(key_pair, &address);
    if (err == NEOC_SUCCESS) {
        printf("NEO Address: %s\n", address);
        free(address);
    }
    
    // Export as WIF
    char *wif = NULL;
    err = neoc_ec_key_pair_export_as_wif(key_pair, &wif);
    if (err == NEOC_SUCCESS) {
        printf("WIF: %s\n\n", wif);
    }
    
    // 2. Sign a message
    printf("2. Signing a message...\n");
    const char *message = "Hello, Neo blockchain!";
    printf("Message: %s\n", message);
    
    neoc_signature_data_t *sig_data = NULL;
    err = neoc_sign_message((const uint8_t *)message, strlen(message), key_pair, &sig_data);
    if (err == NEOC_SUCCESS) {
        printf("Signature created:\n");
        print_hex("  R", sig_data->r, 32);
        print_hex("  S", sig_data->s, 32);
        printf("  V (recovery ID): %d\n\n", sig_data->v);
        
        // 3. Verify the signature
        printf("3. Verifying signature...\n");
        bool valid = neoc_verify_signature((const uint8_t *)message, strlen(message), 
                                            sig_data, key_pair->public_key);
        printf("Signature is %s\n\n", valid ? "VALID" : "INVALID");
        
        // 4. Recover public key from signature
        printf("4. Recovering public key from signature...\n");
        neoc_ec_public_key_t *recovered_pub = NULL;
        err = neoc_signed_message_to_key((const uint8_t *)message, strlen(message), 
                                          sig_data, &recovered_pub);
        if (err == NEOC_SUCCESS) {
            uint8_t *recovered_bytes = NULL;
            size_t recovered_len = 0;
            err = neoc_ec_public_key_get_encoded(recovered_pub, true, &recovered_bytes, &recovered_len);
            if (err == NEOC_SUCCESS) {
                print_hex("Recovered Public Key", recovered_bytes, recovered_len);
                
                // Compare with original
                if (memcmp(key_pair->public_key->compressed, recovered_bytes, 33) == 0) {
                    printf("✓ Recovered public key matches original!\n\n");
                } else {
                    printf("✗ Recovered public key does not match!\n\n");
                }
                free(recovered_bytes);
            }
            neoc_ec_public_key_free(recovered_pub);
        }
        
        neoc_signature_data_free(sig_data);
    }
    
    // 5. Import key pair from WIF
    if (wif) {
        printf("5. Importing key pair from WIF...\n");
        neoc_ec_key_pair_t *imported_pair = NULL;
        err = neoc_ec_key_pair_import_from_wif(wif, &imported_pair);
        if (err == NEOC_SUCCESS) {
            // Get address from imported key
            char *imported_address = NULL;
            err = neoc_ec_key_pair_get_address(imported_pair, &imported_address);
            if (err == NEOC_SUCCESS) {
                printf("Imported Address: %s\n", imported_address);
                
                // Get original address for comparison
                char *original_address = NULL;
                err = neoc_ec_key_pair_get_address(key_pair, &original_address);
                if (err == NEOC_SUCCESS) {
                    if (strcmp(original_address, imported_address) == 0) {
                        printf("✓ Imported key pair matches original!\n\n");
                    } else {
                        printf("✗ Imported key pair does not match!\n\n");
                    }
                    free(original_address);
                }
                free(imported_address);
            }
            neoc_ec_key_pair_free(imported_pair);
        }
        free(wif);
    }
    
    // 6. Create key pair from known private key
    printf("6. Creating key pair from known private key...\n");
    uint8_t private_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
    };
    
    neoc_ec_key_pair_t *known_pair = NULL;
    err = neoc_ec_key_pair_create_from_private_key(private_key, &known_pair);
    if (err == NEOC_SUCCESS) {
        char *known_address = NULL;
        err = neoc_ec_key_pair_get_address(known_pair, &known_address);
        if (err == NEOC_SUCCESS) {
            printf("Address from known key: %s\n", known_address);
            free(known_address);
        }
        
        // Get script hash
        neoc_hash160_t script_hash;
        err = neoc_ec_key_pair_get_script_hash(known_pair, &script_hash);
        if (err == NEOC_SUCCESS) {
            print_hex("Script Hash", script_hash.data, 20);
        }
        
        neoc_ec_key_pair_free(known_pair);
    }
    
    // Cleanup
    neoc_ec_key_pair_free(key_pair);
    neoc_cleanup();
    
    printf("\nCryptography example completed successfully!\n");
    return 0;
}

