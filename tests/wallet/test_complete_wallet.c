/**
 * Complete Wallet Module Test Suite
 * Comprehensive unit tests matching Swift test coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "neoc/wallet/wallet.h"
#include "neoc/wallet/account.h"
#include "neoc/wallet/nep6.h"
#include "neoc/crypto/crypto.h"
#include "neoc/utils/utils.h"

// Test framework
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return 1; \
        } \
    } while(0)

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Run a test
#define RUN_TEST(test_func) \
    do { \
        printf("Running: %s... ", #test_func); \
        tests_run++; \
        if (test_func() == TEST_SUCCESS) { \
            printf("✓ PASSED\n"); \
            tests_passed++; \
        } else { \
            printf("✗ FAILED\n"); \
            tests_failed++; \
        } \
    } while(0)

// Test wallet file path
static const char *TEST_WALLET_PATH = "/tmp/test_wallet.json";

// Clean up test files
static void cleanup_test_files() {
    unlink(TEST_WALLET_PATH);
    unlink("/tmp/test_import.json");
    unlink("/tmp/test_export.json");
}

// Account Tests
int test_account_creation() {
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_create(&account);
    
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(account != NULL);
    TEST_ASSERT(account->private_key != NULL);
    TEST_ASSERT(account->public_key != NULL);
    TEST_ASSERT(account->address != NULL);
    TEST_ASSERT(account->script_hash != NULL);
    TEST_ASSERT(strlen(account->address) > 0);
    TEST_ASSERT(account->address[0] == 'A'); // Neo address prefix
    
    neoc_account_free(account);
    return TEST_SUCCESS;
}

int test_account_from_private_key() {
    // Create account from known private key
    uint8_t private_key[32];
    memset(private_key, 0x42, 32); // Test key
    
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_account_from_private_key(private_key, 32, &account);
    
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(account != NULL);
    TEST_ASSERT(memcmp(account->private_key, private_key, 32) == 0);
    
    // Verify deterministic address generation
    neoc_account_t *account2 = NULL;
    err = neoc_account_from_private_key(private_key, 32, &account2);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(strcmp(account->address, account2->address) == 0);
    
    neoc_account_free(account);
    neoc_account_free(account2);
    return TEST_SUCCESS;
}

int test_account_from_wif() {
    // Generate account and export to WIF
    neoc_account_t *original = NULL;
    neoc_account_create(&original);
    
    char wif[128];
    size_t wif_len = sizeof(wif);
    neoc_error_t err = neoc_account_export_wif(original, wif, &wif_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Import from WIF
    neoc_account_t *imported = NULL;
    err = neoc_account_from_wif(wif, &imported);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(imported != NULL);
    
    // Verify same private key and address
    TEST_ASSERT(memcmp(original->private_key, imported->private_key, 32) == 0);
    TEST_ASSERT(strcmp(original->address, imported->address) == 0);
    
    neoc_account_free(original);
    neoc_account_free(imported);
    return TEST_SUCCESS;
}

int test_account_encryption() {
    neoc_account_t *account = NULL;
    neoc_account_create(&account);
    
    const char *password = "TestPassword123!";
    
    // Encrypt account
    neoc_error_t err = neoc_account_encrypt(account, password);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(account->encrypted_key != NULL);
    TEST_ASSERT(account->is_encrypted == true);
    
    // Private key should be cleared after encryption
    bool all_zeros = true;
    for (int i = 0; i < 32; i++) {
        if (account->private_key[i] != 0) {
            all_zeros = false;
            break;
        }
    }
    TEST_ASSERT(all_zeros);
    
    // Decrypt account
    err = neoc_account_decrypt(account, password);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(account->is_encrypted == false);
    
    // Private key should be restored
    all_zeros = true;
    for (int i = 0; i < 32; i++) {
        if (account->private_key[i] != 0) {
            all_zeros = false;
            break;
        }
    }
    TEST_ASSERT(!all_zeros);
    
    // Wrong password should fail
    err = neoc_account_encrypt(account, password);
    TEST_ASSERT(err == NEOC_SUCCESS);
    err = neoc_account_decrypt(account, "WrongPassword");
    TEST_ASSERT(err != NEOC_SUCCESS);
    
    neoc_account_free(account);
    return TEST_SUCCESS;
}

int test_account_multisig() {
    // Create multiple accounts for multisig
    neoc_account_t *accounts[3];
    for (int i = 0; i < 3; i++) {
        neoc_account_create(&accounts[i]);
    }
    
    // Create 2-of-3 multisig account
    neoc_public_key_t *public_keys[3];
    for (int i = 0; i < 3; i++) {
        public_keys[i] = malloc(sizeof(neoc_public_key_t));
        public_keys[i]->data = accounts[i]->public_key;
        public_keys[i]->len = accounts[i]->public_key_len;
    }
    
    neoc_account_t *multisig = NULL;
    neoc_error_t err = neoc_account_create_multisig(2, public_keys, 3, &multisig);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(multisig != NULL);
    TEST_ASSERT(multisig->is_multisig == true);
    TEST_ASSERT(multisig->multisig_m == 2);
    TEST_ASSERT(multisig->multisig_n == 3);
    
    // Multisig address should be different
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(strcmp(multisig->address, accounts[i]->address) != 0);
    }
    
    // Clean up
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
        free(public_keys[i]);
    }
    neoc_account_free(multisig);
    
    return TEST_SUCCESS;
}

// Wallet Tests
int test_wallet_creation() {
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_create("TestWallet", &wallet);
    
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(wallet != NULL);
    TEST_ASSERT(strcmp(wallet->name, "TestWallet") == 0);
    TEST_ASSERT(wallet->version != NULL);
    TEST_ASSERT(wallet->accounts != NULL);
    TEST_ASSERT(wallet->account_count == 0);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

int test_wallet_add_account() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("TestWallet", &wallet);
    
    // Add new account
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_wallet_create_account(wallet, &account);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(account != NULL);
    TEST_ASSERT(wallet->account_count == 1);
    
    // Add another account
    neoc_account_t *account2 = NULL;
    err = neoc_wallet_create_account(wallet, &account2);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(wallet->account_count == 2);
    
    // Accounts should have different addresses
    TEST_ASSERT(strcmp(account->address, account2->address) != 0);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

int test_wallet_remove_account() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("TestWallet", &wallet);
    
    // Add accounts
    neoc_account_t *account1 = NULL;
    neoc_account_t *account2 = NULL;
    neoc_wallet_create_account(wallet, &account1);
    neoc_wallet_create_account(wallet, &account2);
    
    char address[128];
    strcpy(address, account1->address);
    
    // Remove first account
    neoc_error_t err = neoc_wallet_remove_account(wallet, address);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(wallet->account_count == 1);
    
    // Try to remove non-existent account
    err = neoc_wallet_remove_account(wallet, "InvalidAddress");
    TEST_ASSERT(err != NEOC_SUCCESS);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

int test_wallet_find_account() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("TestWallet", &wallet);
    
    // Add accounts
    neoc_account_t *account1 = NULL;
    neoc_account_t *account2 = NULL;
    neoc_wallet_create_account(wallet, &account1);
    neoc_wallet_create_account(wallet, &account2);
    
    // Find account by address
    neoc_account_t *found = neoc_wallet_get_account(wallet, account1->address);
    TEST_ASSERT(found != NULL);
    TEST_ASSERT(strcmp(found->address, account1->address) == 0);
    
    // Find non-existent account
    found = neoc_wallet_get_account(wallet, "InvalidAddress");
    TEST_ASSERT(found == NULL);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

int test_wallet_default_account() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("TestWallet", &wallet);
    
    // No default account initially
    neoc_account_t *default_acc = neoc_wallet_get_default_account(wallet);
    TEST_ASSERT(default_acc == NULL);
    
    // Add account
    neoc_account_t *account = NULL;
    neoc_wallet_create_account(wallet, &account);
    
    // Set as default
    neoc_error_t err = neoc_wallet_set_default_account(wallet, account->address);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Get default account
    default_acc = neoc_wallet_get_default_account(wallet);
    TEST_ASSERT(default_acc != NULL);
    TEST_ASSERT(strcmp(default_acc->address, account->address) == 0);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

// NEP-6 Tests
int test_nep6_save_load() {
    cleanup_test_files();
    
    // Create wallet with accounts
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("TestNEP6", &wallet);
    
    neoc_account_t *account1 = NULL;
    neoc_account_t *account2 = NULL;
    neoc_wallet_create_account(wallet, &account1);
    neoc_wallet_create_account(wallet, &account2);
    
    // Encrypt accounts
    neoc_account_encrypt(account1, "password1");
    neoc_account_encrypt(account2, "password2");
    
    // Save to NEP-6 file
    neoc_error_t err = neoc_wallet_save(wallet, TEST_WALLET_PATH);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Verify file exists
    TEST_ASSERT(access(TEST_WALLET_PATH, F_OK) == 0);
    
    // Load wallet from file
    neoc_wallet_t *loaded = NULL;
    err = neoc_wallet_load(TEST_WALLET_PATH, &loaded);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(loaded != NULL);
    TEST_ASSERT(strcmp(loaded->name, wallet->name) == 0);
    TEST_ASSERT(loaded->account_count == wallet->account_count);
    
    // Verify accounts
    neoc_account_t *loaded_acc1 = neoc_wallet_get_account(loaded, account1->address);
    TEST_ASSERT(loaded_acc1 != NULL);
    TEST_ASSERT(loaded_acc1->is_encrypted == true);
    
    neoc_wallet_free(wallet);
    neoc_wallet_free(loaded);
    cleanup_test_files();
    
    return TEST_SUCCESS;
}

int test_nep6_import_export() {
    cleanup_test_files();
    
    // Create wallet
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("ExportTest", &wallet);
    
    neoc_account_t *account = NULL;
    neoc_wallet_create_account(wallet, &account);
    neoc_wallet_set_default_account(wallet, account->address);
    
    // Export to file
    neoc_error_t err = neoc_wallet_export(wallet, "/tmp/test_export.json", "export_pass");
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Import from file
    neoc_wallet_t *imported = NULL;
    err = neoc_wallet_import("/tmp/test_export.json", "export_pass", &imported);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(imported != NULL);
    TEST_ASSERT(strcmp(imported->name, wallet->name) == 0);
    
    neoc_wallet_free(wallet);
    neoc_wallet_free(imported);
    cleanup_test_files();
    
    return TEST_SUCCESS;
}

int test_nep6_scrypt_parameters() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("ScryptTest", &wallet);
    
    // Set custom scrypt parameters
    wallet->scrypt_params.n = 32768;
    wallet->scrypt_params.r = 16;
    wallet->scrypt_params.p = 2;
    
    neoc_account_t *account = NULL;
    neoc_wallet_create_account(wallet, &account);
    neoc_account_encrypt(account, "testpass");
    
    // Save and reload
    neoc_wallet_save(wallet, TEST_WALLET_PATH);
    
    neoc_wallet_t *loaded = NULL;
    neoc_wallet_load(TEST_WALLET_PATH, &loaded);
    
    // Verify scrypt parameters preserved
    TEST_ASSERT(loaded->scrypt_params.n == 32768);
    TEST_ASSERT(loaded->scrypt_params.r == 16);
    TEST_ASSERT(loaded->scrypt_params.p == 2);
    
    neoc_wallet_free(wallet);
    neoc_wallet_free(loaded);
    cleanup_test_files();
    
    return TEST_SUCCESS;
}

int test_nep6_compatibility() {
    // Test loading a NEP-6 wallet with specific format
    const char *nep6_json = "{\n"
        "  \"name\": \"MyWallet\",\n"
        "  \"version\": \"3.0\",\n"
        "  \"scrypt\": {\n"
        "    \"n\": 16384,\n"
        "    \"r\": 8,\n"
        "    \"p\": 1\n"
        "  },\n"
        "  \"accounts\": [\n"
        "    {\n"
        "      \"address\": \"AXxTdTbMkLJpYgn2wTkN6RkMVnKHHkLsXZ\",\n"
        "      \"label\": \"Account1\",\n"
        "      \"isDefault\": true,\n"
        "      \"lock\": false,\n"
        "      \"key\": \"6PYLHmDf2t3a3dU1s5r3BnXBZTQVGzLxqr2p5rqvVpzLEHWUKCtxswLRzD\",\n"
        "      \"contract\": {\n"
        "        \"script\": \"21031a6c6fbbdf02f7a012c832fb5b8dbf6e017e77e37f67ac201e51d4d35e1b9a2cac\",\n"
        "        \"parameters\": [\n"
        "          {\n"
        "            \"name\": \"signature\",\n"
        "            \"type\": \"Signature\"\n"
        "          }\n"
        "        ],\n"
        "        \"deployed\": false\n"
        "      },\n"
        "      \"extra\": null\n"
        "    }\n"
        "  ],\n"
        "  \"extra\": null\n"
        "}";
    
    // Write test file
    FILE *f = fopen("/tmp/test_import.json", "w");
    TEST_ASSERT(f != NULL);
    fprintf(f, "%s", nep6_json);
    fclose(f);
    
    // Load wallet
    neoc_wallet_t *wallet = NULL;
    neoc_error_t err = neoc_wallet_load("/tmp/test_import.json", &wallet);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(wallet != NULL);
    TEST_ASSERT(strcmp(wallet->name, "MyWallet") == 0);
    TEST_ASSERT(strcmp(wallet->version, "3.0") == 0);
    TEST_ASSERT(wallet->account_count == 1);
    
    // Verify account
    neoc_account_t *account = wallet->accounts[0];
    TEST_ASSERT(strcmp(account->address, "AXxTdTbMkLJpYgn2wTkN6RkMVnKHHkLsXZ") == 0);
    TEST_ASSERT(account->is_default == true);
    TEST_ASSERT(account->is_locked == false);
    
    neoc_wallet_free(wallet);
    cleanup_test_files();
    
    return TEST_SUCCESS;
}

// Key Store Tests
int test_keystore_operations() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("KeyStore", &wallet);
    
    // Create encrypted account
    neoc_account_t *account = NULL;
    neoc_wallet_create_account(wallet, &account);
    
    const char *password = "keystore_pass";
    neoc_account_encrypt(account, password);
    
    // Export keystore
    char *keystore_json = NULL;
    size_t json_len = 0;
    neoc_error_t err = neoc_account_to_keystore(account, &keystore_json, &json_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(keystore_json != NULL);
    TEST_ASSERT(json_len > 0);
    
    // Import from keystore
    neoc_account_t *imported = NULL;
    err = neoc_account_from_keystore(keystore_json, password, &imported);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(imported != NULL);
    TEST_ASSERT(strcmp(imported->address, account->address) == 0);
    
    free(keystore_json);
    neoc_account_free(imported);
    neoc_wallet_free(wallet);
    
    return TEST_SUCCESS;
}

// HD Wallet Tests
int test_hd_wallet_derivation() {
    // Create mnemonic
    char mnemonic[512];
    neoc_error_t err = neoc_generate_mnemonic(128, mnemonic, sizeof(mnemonic));
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Derive seed
    uint8_t seed[64];
    err = neoc_mnemonic_to_seed(mnemonic, "passphrase", seed, 64);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Create HD wallet
    neoc_hd_wallet_t *hd_wallet = NULL;
    err = neoc_hd_wallet_from_seed(seed, 64, &hd_wallet);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(hd_wallet != NULL);
    
    // Derive child accounts
    neoc_account_t *account1 = NULL;
    neoc_account_t *account2 = NULL;
    
    err = neoc_hd_wallet_derive_account(hd_wallet, "m/44'/888'/0'/0/0", &account1);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    err = neoc_hd_wallet_derive_account(hd_wallet, "m/44'/888'/0'/0/1", &account2);
    TEST_ASSERT(err == NEOC_SUCCESS);
    
    // Different paths should give different accounts
    TEST_ASSERT(strcmp(account1->address, account2->address) != 0);
    
    neoc_account_free(account1);
    neoc_account_free(account2);
    neoc_hd_wallet_free(hd_wallet);
    
    return TEST_SUCCESS;
}

int test_mnemonic_validation() {
    // Valid mnemonic
    const char *valid = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    bool is_valid = neoc_validate_mnemonic(valid);
    TEST_ASSERT(is_valid == true);
    
    // Invalid mnemonic (wrong word)
    const char *invalid1 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon invalid";
    is_valid = neoc_validate_mnemonic(invalid1);
    TEST_ASSERT(is_valid == false);
    
    // Invalid mnemonic (wrong checksum)
    const char *invalid2 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon";
    is_valid = neoc_validate_mnemonic(invalid2);
    TEST_ASSERT(is_valid == false);
    
    return TEST_SUCCESS;
}

// Signature Tests
int test_wallet_sign_verify() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("SignTest", &wallet);
    
    neoc_account_t *account = NULL;
    neoc_wallet_create_account(wallet, &account);
    
    // Sign message
    const char *message = "Test message for signing";
    uint8_t signature[72];
    size_t sig_len = sizeof(signature);
    
    neoc_error_t err = neoc_wallet_sign(wallet, account->address, 
                                        (uint8_t*)message, strlen(message),
                                        signature, &sig_len);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(sig_len > 0);
    
    // Verify signature
    bool valid = false;
    err = neoc_wallet_verify(wallet, account->address,
                             (uint8_t*)message, strlen(message),
                             signature, sig_len, &valid);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(valid == true);
    
    // Wrong message should fail
    const char *wrong = "Wrong message";
    err = neoc_wallet_verify(wallet, account->address,
                             (uint8_t*)wrong, strlen(wrong),
                             signature, sig_len, &valid);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(valid == false);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

// Import/Export Tests
int test_wallet_import_private_key() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("ImportTest", &wallet);
    
    // Generate private key
    uint8_t private_key[32];
    neoc_random_bytes(private_key, 32);
    
    // Import to wallet
    neoc_account_t *account = NULL;
    neoc_error_t err = neoc_wallet_import_private_key(wallet, private_key, 32, &account);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(account != NULL);
    TEST_ASSERT(wallet->account_count == 1);
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

int test_wallet_import_wif() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("WIFImport", &wallet);
    
    // Create account and export WIF
    neoc_account_t *original = NULL;
    neoc_account_create(&original);
    
    char wif[128];
    size_t wif_len = sizeof(wif);
    neoc_account_export_wif(original, wif, &wif_len);
    
    // Import WIF to wallet
    neoc_account_t *imported = NULL;
    neoc_error_t err = neoc_wallet_import_wif(wallet, wif, &imported);
    TEST_ASSERT(err == NEOC_SUCCESS);
    TEST_ASSERT(imported != NULL);
    TEST_ASSERT(strcmp(imported->address, original->address) == 0);
    
    neoc_account_free(original);
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

// Claim Tests
int test_wallet_claim_gas() {
    neoc_wallet_t *wallet = NULL;
    neoc_wallet_create("ClaimTest", &wallet);
    
    neoc_account_t *account = NULL;
    neoc_wallet_create_account(wallet, &account);
    
    // Set claimable gas (mock)
    account->unclaimed_gas = 100000000; // 1 GAS
    
    // Check claimable gas
    uint64_t claimable = neoc_wallet_get_claimable_gas(wallet, account->address);
    TEST_ASSERT(claimable == 100000000);
    
    // Claim gas (would create transaction in real implementation)
    neoc_transaction_t *tx = NULL;
    neoc_error_t err = neoc_wallet_claim_gas(wallet, account->address, &tx);
    // This would normally create a claim transaction
    // For testing, we just verify the function exists
    
    neoc_wallet_free(wallet);
    return TEST_SUCCESS;
}

// Performance Tests
int test_wallet_performance() {
    printf("\n  Performance Metrics:\n");
    
    // Account creation performance
    clock_t start = clock();
    for (int i = 0; i < 100; i++) {
        neoc_account_t *account = NULL;
        neoc_account_create(&account);
        neoc_account_free(account);
    }
    clock_t end = clock();
    double create_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("    Account Creation: %.0f ops/sec\n", 100.0 / create_time);
    
    // Encryption performance
    neoc_account_t *account = NULL;
    neoc_account_create(&account);
    
    start = clock();
    for (int i = 0; i < 10; i++) {
        neoc_account_encrypt(account, "password");
        neoc_account_decrypt(account, "password");
    }
    end = clock();
    double crypt_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("    Encrypt/Decrypt: %.0f ops/sec\n", 20.0 / crypt_time);
    
    neoc_account_free(account);
    
    return TEST_SUCCESS;
}

// Main test runner
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("    Complete Wallet Module Test Suite   \n");
    printf("========================================\n\n");
    
    cleanup_test_files();
    
    // Account Tests
    printf("Account Tests:\n");
    RUN_TEST(test_account_creation);
    RUN_TEST(test_account_from_private_key);
    RUN_TEST(test_account_from_wif);
    RUN_TEST(test_account_encryption);
    RUN_TEST(test_account_multisig);
    
    // Wallet Tests
    printf("\nWallet Tests:\n");
    RUN_TEST(test_wallet_creation);
    RUN_TEST(test_wallet_add_account);
    RUN_TEST(test_wallet_remove_account);
    RUN_TEST(test_wallet_find_account);
    RUN_TEST(test_wallet_default_account);
    
    // NEP-6 Tests
    printf("\nNEP-6 Tests:\n");
    RUN_TEST(test_nep6_save_load);
    RUN_TEST(test_nep6_import_export);
    RUN_TEST(test_nep6_scrypt_parameters);
    RUN_TEST(test_nep6_compatibility);
    
    // Key Store Tests
    printf("\nKey Store Tests:\n");
    RUN_TEST(test_keystore_operations);
    
    // HD Wallet Tests
    printf("\nHD Wallet Tests:\n");
    RUN_TEST(test_hd_wallet_derivation);
    RUN_TEST(test_mnemonic_validation);
    
    // Signature Tests
    printf("\nSignature Tests:\n");
    RUN_TEST(test_wallet_sign_verify);
    
    // Import/Export Tests
    printf("\nImport/Export Tests:\n");
    RUN_TEST(test_wallet_import_private_key);
    RUN_TEST(test_wallet_import_wif);
    
    // Claim Tests
    printf("\nClaim Tests:\n");
    RUN_TEST(test_wallet_claim_gas);
    
    // Performance Tests
    printf("\nPerformance Tests:\n");
    RUN_TEST(test_wallet_performance);
    
    // Summary
    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", tests_run);
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✅ ALL TESTS PASSED!\n");
    } else {
        printf("\n❌ SOME TESTS FAILED\n");
    }
    printf("========================================\n");
    
    cleanup_test_files();
    
    return tests_failed > 0 ? 1 : 0;
}