#include <stdio.h>
#include <neoc/neoc.h>
#include <neoc/crypto/bip32.h>

int main(void) {
    printf("Starting BIP-32 simple test...\n");
    
    // Initialize SDK
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        printf("Failed to initialize SDK: %d\n", err);
        return 1;
    }
    printf("SDK initialized\n");
    
    // Simple seed
    uint8_t seed[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    
    printf("Creating master key...\n");
    neoc_bip32_key_t master_key;
    err = neoc_bip32_from_seed(seed, sizeof(seed), &master_key);
    if (err != NEOC_SUCCESS) {
        printf("Failed to create master key: %d\n", err);
        neoc_cleanup();
        return 1;
    }
    
    printf("Master key created successfully!\n");
    printf("Depth: %d\n", master_key.depth);
    printf("Is private: %d\n", master_key.is_private);
    
    neoc_cleanup();
    printf("Test completed successfully!\n");
    return 0;
}
