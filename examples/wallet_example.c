/**
 * @file wallet_example.c
 * @brief Example application demonstrating NeoC wallet functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <neoc/neoc.h>
#include <neoc/wallet/wallet.h>
#include <neoc/wallet/account.h>
#include <neoc/wallet/nep6_wallet.h>
#include <neoc/crypto/keys.h>
#include <neoc/types/address.h>

/**
 * Print account information
 */
void print_account_info(neoc_account_t *account) {
    const char *address = neoc_account_get_address(account);
    neoc_ec_public_key_t *public_key = neoc_account_get_public_key(account);
    
    printf("Account Information:\n");
    printf("  Address: %s\n", address);
    
    // Get public key hex
    char pub_hex[132];
    neoc_ec_public_key_to_hex(public_key, pub_hex, sizeof(pub_hex));
    printf("  Public Key: %s\n", pub_hex);
    
    // Check if multi-sig
    if (neoc_account_is_multisig(account)) {
        printf("  Type: Multi-signature\n");
    } else {
        printf("  Type: Standard\n");
    }
}

/**
 * Example 1: Create a new wallet with accounts
 */
int example_create_wallet() {
    printf("\n=== Example 1: Creating a New Wallet ===\n\n");
    
    neoc_error_t err;
    neoc_wallet_t *wallet = NULL;
    
    // Create a new wallet
    err = neoc_wallet_create("MyWallet", &wallet);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create wallet: %s\n", neoc_error_string(err));
        return -1;
    }
    
    printf("✓ Wallet 'MyWallet' created successfully\n\n");
    
    // Create multiple accounts
    for (int i = 0; i < 3; i++) {
        neoc_account_t *account = NULL;
        err = neoc_account_create(&account);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to create account: %s\n", neoc_error_string(err));
            neoc_wallet_free(wallet);
            return -1;
        }
        
        // Add account to wallet
        err = neoc_wallet_add_account(wallet, account);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to add account: %s\n", neoc_error_string(err));
            neoc_account_free(account);
            neoc_wallet_free(wallet);
            return -1;
        }
        
        printf("Account %d:\n", i + 1);
        print_account_info(account);
        printf("\n");
    }
    
    // Get wallet statistics
    size_t account_count = neoc_wallet_get_account_count(wallet);
    printf("Total accounts in wallet: %zu\n", account_count);
    
    // Set default account
    neoc_account_t *default_account = neoc_wallet_get_account(wallet, 0);
    err = neoc_wallet_set_default_account(wallet, default_account);
    if (err == NEOC_SUCCESS) {
        printf("✓ Default account set\n");
    }
    
    // Clean up
    neoc_wallet_free(wallet);
    
    return 0;
}

/**
 * Example 2: Import and export wallet (NEP-6)
 */
int example_import_export_wallet() {
    printf("\n=== Example 2: Import/Export NEP-6 Wallet ===\n\n");
    
    neoc_error_t err;
    neoc_wallet_t *wallet = NULL;
    const char *wallet_path = "/tmp/example_wallet.json";
    const char *password = "SecurePassword123!";
    
    // Create a wallet with an account
    err = neoc_wallet_create("ExportWallet", &wallet);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create wallet: %s\n", neoc_error_string(err));
        return -1;
    }
    
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    if (err != NEOC_SUCCESS) {
        neoc_wallet_free(wallet);
        return -1;
    }
    
    err = neoc_wallet_add_account(wallet, account);
    if (err != NEOC_SUCCESS) {
        neoc_account_free(account);
        neoc_wallet_free(wallet);
        return -1;
    }
    
    printf("Original wallet created with account:\n");
    print_account_info(account);
    printf("\n");
    
    // Export wallet to NEP-6 format
    err = neoc_nep6_wallet_export(wallet, wallet_path, password);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to export wallet: %s\n", neoc_error_string(err));
        neoc_wallet_free(wallet);
        return -1;
    }
    
    printf("✓ Wallet exported to: %s\n\n", wallet_path);
    
    // Free original wallet
    const char *original_address = strdup(neoc_account_get_address(account));
    neoc_wallet_free(wallet);
    wallet = NULL;
    
    // Import wallet back
    err = neoc_nep6_wallet_import(wallet_path, password, &wallet);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to import wallet: %s\n", neoc_error_string(err));
        free((void*)original_address);
        return -1;
    }
    
    printf("✓ Wallet imported successfully\n\n");
    
    // Verify imported account
    neoc_account_t *imported_account = neoc_wallet_get_account(wallet, 0);
    if (imported_account != NULL) {
        const char *imported_address = neoc_account_get_address(imported_account);
        if (strcmp(original_address, imported_address) == 0) {
            printf("✓ Account verification successful\n");
            printf("  Original address: %s\n", original_address);
            printf("  Imported address: %s\n", imported_address);
        } else {
            printf("✗ Address mismatch after import\n");
        }
    }
    
    // Clean up
    free((void*)original_address);
    neoc_wallet_free(wallet);
    remove(wallet_path);
    
    return 0;
}

/**
 * Example 3: Create multi-signature account
 */
int example_multisig_account() {
    printf("\n=== Example 3: Multi-Signature Account ===\n\n");
    
    neoc_error_t err;
    neoc_account_t *accounts[3];
    neoc_ec_public_key_t *pub_keys[3];
    
    // Create 3 accounts for multi-sig
    printf("Creating 3 accounts for multi-signature setup:\n\n");
    for (int i = 0; i < 3; i++) {
        err = neoc_account_create(&accounts[i]);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to create account %d: %s\n", 
                    i, neoc_error_string(err));
            // Clean up previously created accounts
            for (int j = 0; j < i; j++) {
                neoc_account_free(accounts[j]);
            }
            return -1;
        }
        
        pub_keys[i] = neoc_account_get_public_key(accounts[i]);
        printf("Account %d: %s\n", i + 1, neoc_account_get_address(accounts[i]));
    }
    
    // Create 2-of-3 multi-signature account
    printf("\nCreating 2-of-3 multi-signature account...\n");
    
    neoc_account_t *multisig = NULL;
    err = neoc_account_create_multisig(2, pub_keys, 3, &multisig);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create multi-sig account: %s\n", 
                neoc_error_string(err));
        for (int i = 0; i < 3; i++) {
            neoc_account_free(accounts[i]);
        }
        return -1;
    }
    
    printf("\n✓ Multi-signature account created:\n");
    print_account_info(multisig);
    printf("  Threshold: 2 of 3\n");
    printf("  Required signatures: 2\n");
    
    // Get multi-sig script
    neoc_script_t *script = neoc_account_get_script(multisig);
    if (script != NULL) {
        size_t script_size = neoc_script_get_size(script);
        printf("  Script size: %zu bytes\n", script_size);
    }
    
    // Clean up
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
    }
    neoc_account_free(multisig);
    
    return 0;
}

/**
 * Example 4: Private key operations
 */
int example_private_key_operations() {
    printf("\n=== Example 4: Private Key Operations ===\n\n");
    
    neoc_error_t err;
    neoc_account_t *account = NULL;
    
    // Create an account
    err = neoc_account_create(&account);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create account: %s\n", neoc_error_string(err));
        return -1;
    }
    
    printf("Account created: %s\n\n", neoc_account_get_address(account));
    
    // Export private key as WIF
    char wif[256];
    err = neoc_account_export_wif(account, wif, sizeof(wif));
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to export WIF: %s\n", neoc_error_string(err));
        neoc_account_free(account);
        return -1;
    }
    
    printf("Private key (WIF format):\n");
    printf("  %s\n\n", wif);
    printf("  ⚠️  Keep this private key secure!\n\n");
    
    // Export encrypted private key (NEP-2)
    const char *password = "MySecretPassword";
    char *nep2 = NULL;
    err = neoc_account_export_encrypted(account, password, &nep2);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to export NEP-2: %s\n", neoc_error_string(err));
        neoc_account_free(account);
        return -1;
    }
    
    printf("Encrypted private key (NEP-2 format):\n");
    printf("  %s\n\n", nep2);
    
    // Free original account
    const char *original_address = strdup(neoc_account_get_address(account));
    neoc_account_free(account);
    account = NULL;
    
    // Import from WIF
    printf("Importing account from WIF...\n");
    err = neoc_account_from_wif(wif, &account);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to import from WIF: %s\n", neoc_error_string(err));
        free((void*)original_address);
        free(nep2);
        return -1;
    }
    
    const char *imported_address = neoc_account_get_address(account);
    if (strcmp(original_address, imported_address) == 0) {
        printf("✓ Successfully imported from WIF\n");
        printf("  Address: %s\n\n", imported_address);
    }
    
    neoc_account_free(account);
    account = NULL;
    
    // Import from NEP-2
    printf("Importing account from NEP-2 (encrypted)...\n");
    err = neoc_account_import_encrypted(nep2, password, &account);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to import from NEP-2: %s\n", neoc_error_string(err));
        free((void*)original_address);
        free(nep2);
        return -1;
    }
    
    imported_address = neoc_account_get_address(account);
    if (strcmp(original_address, imported_address) == 0) {
        printf("✓ Successfully imported from NEP-2\n");
        printf("  Address: %s\n", imported_address);
    }
    
    // Clean up
    free((void*)original_address);
    free(nep2);
    neoc_account_free(account);
    
    // Clear sensitive data from memory
    memset(wif, 0, sizeof(wif));
    
    return 0;
}

/**
 * Main function - run all examples
 */
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("     NeoC SDK - Wallet Examples\n");
    printf("========================================\n");
    
    // Initialize NeoC SDK
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC: %s\n", neoc_error_string(err));
        return 1;
    }
    
    // Check command line arguments
    int example_num = 0;
    if (argc > 1) {
        example_num = atoi(argv[1]);
    }
    
    int result = 0;
    
    // Run examples
    if (example_num == 0 || example_num == 1) {
        result = example_create_wallet();
        if (result != 0) goto cleanup;
    }
    
    if (example_num == 0 || example_num == 2) {
        result = example_import_export_wallet();
        if (result != 0) goto cleanup;
    }
    
    if (example_num == 0 || example_num == 3) {
        result = example_multisig_account();
        if (result != 0) goto cleanup;
    }
    
    if (example_num == 0 || example_num == 4) {
        result = example_private_key_operations();
        if (result != 0) goto cleanup;
    }
    
    printf("\n========================================\n");
    printf("     All examples completed successfully!\n");
    printf("========================================\n\n");
    
cleanup:
    // Clean up NeoC SDK
    neoc_cleanup();
    
    return result;
}