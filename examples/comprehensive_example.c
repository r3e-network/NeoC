#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/crypto/sign.h"
#include "neoc/wallet/account.h"
#include "neoc/wallet/wallet.h"
#include "neoc/transaction/transaction.h"
#include "neoc/contract/smart_contract.h"
#include "neoc/protocol/json_rpc.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"

void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("NeoC SDK Comprehensive Example\n");
    printf("==============================\n\n");
    
    // Initialize SDK
    if (neoc_init() != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC SDK\n");
        return 1;
    }
    
    printf("SDK Version: %s\n", neoc_get_version());
    printf("Build Info: %s\n\n", neoc_get_build_info());
    
    // 1. Cryptography
    printf("1. Cryptography Operations\n");
    printf("--------------------------\n");
    
    // Create a key pair
    neoc_ec_key_pair_t* key_pair = NULL;
    neoc_error_t err = neoc_ec_key_pair_create_random(&key_pair);
    if (err == NEOC_SUCCESS) {
        printf("✓ Created random EC key pair\n");
        
        // Get private key
        uint8_t private_key[32];
        size_t priv_len = 32;
        err = neoc_ec_key_pair_get_private_key(key_pair, private_key, &priv_len);
        if (err == NEOC_SUCCESS) {
            print_hex("  Private Key", private_key, priv_len);
        }
        
        // Get public key
        uint8_t public_key[65];
        size_t pub_len = 65;
        err = neoc_ec_key_pair_get_public_key(key_pair, public_key, &pub_len);
        if (err == NEOC_SUCCESS) {
            print_hex("  Public Key", public_key, pub_len);
        }
        
        // Get address
        char* address = NULL;
        err = neoc_ec_key_pair_get_address(key_pair, &address);
        if (err == NEOC_SUCCESS && address) {
            printf("  Address: %s\n", address);
            neoc_free(address);
        }
        
        // Sign a message
        const char* message = "Hello, Neo!";
        neoc_signature_data_t* signature = NULL;
        err = neoc_sign_message((const uint8_t*)message, strlen(message), key_pair, &signature);
        if (err == NEOC_SUCCESS) {
            printf("✓ Signed message\n");
            
            // Verify signature
            bool verified = false;
            err = neoc_verify_message((const uint8_t*)message, strlen(message), signature, key_pair, &verified);
            if (err == NEOC_SUCCESS && verified) {
                printf("✓ Signature verified\n");
            }
            
            neoc_signature_data_free(signature);
        }
        
        neoc_ec_key_pair_free(key_pair);
    }
    
    printf("\n");
    
    // 2. Wallet Operations
    printf("2. Wallet Operations\n");
    printf("--------------------\n");
    
    neoc_wallet_t* wallet = NULL;
    err = neoc_wallet_create("MyWallet", &wallet);
    if (err == NEOC_SUCCESS) {
        printf("✓ Created wallet: %s\n", neoc_wallet_get_name(wallet));
        
        // Create an account
        neoc_account_t* account = NULL;
        err = neoc_account_create("Default Account", &account);
        if (err == NEOC_SUCCESS) {
            printf("✓ Created account\n");
            
            // Get account address
            const char* acc_address = neoc_account_get_address(account);
            if (acc_address) {
                printf("  Account Address: %s\n", acc_address);
            }
            
            // Add account to wallet
            err = neoc_wallet_add_account(wallet, account);
            if (err == NEOC_SUCCESS) {
                printf("✓ Added account to wallet\n");
                printf("  Wallet now has %zu account(s)\n", neoc_wallet_get_account_count(wallet));
            }
            
            // Note: Account is now owned by wallet, don't free it separately
        }
        
        neoc_wallet_free(wallet);
    }
    
    printf("\n");
    
    // 3. Transaction Building
    printf("3. Transaction Building\n");
    printf("-----------------------\n");
    
    neoc_transaction_t* tx = NULL;
    err = neoc_transaction_create(&tx);
    if (err == NEOC_SUCCESS) {
        printf("✓ Created transaction\n");
        
        // Set transaction properties
        neoc_transaction_set_version(tx, 0);
        neoc_transaction_set_system_fee(tx, 100000);
        neoc_transaction_set_network_fee(tx, 200000);
        neoc_transaction_set_valid_until_block(tx, 1000000);
        
        // Add a signer
        neoc_signer_t* signer = NULL;
        neoc_hash160_t signer_hash;
        neoc_hash160_init_zero(&signer_hash);
        err = neoc_signer_create(&signer_hash, NEOC_WITNESS_SCOPE_GLOBAL, &signer);
        if (err == NEOC_SUCCESS) {
            err = neoc_transaction_add_signer(tx, signer);
            if (err == NEOC_SUCCESS) {
                printf("✓ Added signer to transaction\n");
            }
            // Transaction takes ownership of signer, don't free it
        }
        
        // Get transaction hash
        neoc_hash256_t tx_hash;
        err = neoc_transaction_get_hash(tx, &tx_hash);
        if (err == NEOC_SUCCESS) {
            char hash_str[NEOC_HASH256_STRING_LENGTH];
            neoc_hash256_to_hex(&tx_hash, hash_str, sizeof(hash_str), false);
            printf("  Transaction Hash: 0x%s\n", hash_str);
        }
        
        neoc_transaction_free(tx);
    }
    
    printf("\n");
    
    // 4. Smart Contract Operations
    printf("4. Smart Contract Operations\n");
    printf("----------------------------\n");
    
    // Get native contract hashes (hardcoded for now)
    neoc_hash160_t neo_hash;
    neoc_hash160_t gas_hash;
    neoc_hash160_from_hex(&neo_hash, "ef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
    neoc_hash160_from_hex(&gas_hash, "d2a4cff31913016155e38e474a2c06d08be276cf");
    
    char neo_str[NEOC_HASH160_STRING_LENGTH];
    char gas_str[NEOC_HASH160_STRING_LENGTH];
    neoc_hash160_to_hex(&neo_hash, neo_str, sizeof(neo_str), false);
    neoc_hash160_to_hex(&gas_hash, gas_str, sizeof(gas_str), false);
    
    printf("  NEO Contract: 0x%s\n", neo_str);
    printf("  GAS Contract: 0x%s\n", gas_str);
    
    printf("\n");
    
    // 5. Serialization
    printf("5. Serialization\n");
    printf("----------------\n");
    
    // Binary writer
    neoc_binary_writer_t* writer = NULL;
    err = neoc_binary_writer_create(256, true, &writer);
    if (err == NEOC_SUCCESS) {
        // Write some data
        neoc_binary_writer_write_byte(writer, 0x42);
        neoc_binary_writer_write_uint32(writer, 0x12345678);
        neoc_binary_writer_write_var_string(writer, "NeoC");
        
        // Get the serialized data
        const uint8_t* data;
        size_t data_len;
        err = neoc_binary_writer_get_data(writer, &data, &data_len);
        if (err == NEOC_SUCCESS) {
            printf("✓ Serialized %zu bytes\n", data_len);
            print_hex("  Data", data, data_len);
            
            // Binary reader
            neoc_binary_reader_t* reader = NULL;
            err = neoc_binary_reader_create(data, data_len, &reader);
            if (err == NEOC_SUCCESS) {
                uint8_t byte_val;
                uint32_t uint_val;
                char* str_val;
                
                neoc_binary_reader_read_byte(reader, &byte_val);
                neoc_binary_reader_read_uint32(reader, &uint_val);
                neoc_binary_reader_read_var_string(reader, &str_val);
                
                printf("✓ Deserialized:\n");
                printf("  Byte: 0x%02x\n", byte_val);
                printf("  UInt32: 0x%08x\n", uint_val);
                printf("  String: %s\n", str_val);
                
                free(str_val);
                neoc_binary_reader_free(reader);
            }
        }
        
        neoc_binary_writer_free(writer);
    }
    
    printf("\n");
    
    // 6. RPC Client (stub implementation)
    printf("6. RPC Client\n");
    printf("-------------\n");
    
    neoc_rpc_config_t rpc_config = {
        .url = "http://localhost:10332",
        .timeout_ms = 30000,
        .max_retries = 3,
        .network_magic = 0x334F454E  // NEO3 TestNet
    };
    
    neoc_rpc_client_t* rpc_client = NULL;
    err = neoc_rpc_client_create(&rpc_client, &rpc_config);
    if (err == NEOC_SUCCESS) {
        printf("✓ Created RPC client for: %s\n", rpc_config.url);
        
        // Try to get block count (will fail with NOT_IMPLEMENTED if no cURL/cJSON)
        uint32_t block_count = 0;
        err = neoc_rpc_get_block_count(rpc_client, &block_count);
        if (err == NEOC_ERROR_NOT_IMPLEMENTED) {
            printf("  RPC operations not available (cURL/cJSON not compiled)\n");
        } else if (err == NEOC_SUCCESS) {
            printf("  Current block height: %u\n", block_count);
        }
        
        neoc_rpc_client_free(rpc_client);
    }
    
    printf("\n");
    
    // 7. Type Operations
    printf("7. Type Operations\n");
    printf("------------------\n");
    
    // Hash160
    neoc_hash160_t hash160;
    err = neoc_hash160_from_hex(&hash160, "17694821c6e3ea8b7a7d770952e7de86c73d94c3");
    if (err == NEOC_SUCCESS) {
        char hash160_str[NEOC_HASH160_STRING_LENGTH];
        neoc_hash160_to_hex(&hash160, hash160_str, sizeof(hash160_str), false);
        printf("  Hash160: 0x%s\n", hash160_str);
        
        // Convert to address
        char addr[NEOC_ADDRESS_LENGTH];
        neoc_hash160_to_address(&hash160, addr, sizeof(addr));
        printf("  Address: %s\n", addr);
    }
    
    // Hash256
    neoc_hash256_t hash256;
    const char* test_data = "Hello, Neo!";
    err = neoc_hash256_from_data_hash(&hash256, (const uint8_t*)test_data, strlen(test_data));
    if (err == NEOC_SUCCESS) {
        char hash256_str[NEOC_HASH256_STRING_LENGTH];
        neoc_hash256_to_hex(&hash256, hash256_str, sizeof(hash256_str), false);
        printf("  Hash256 of '%s': 0x%s\n", test_data, hash256_str);
    }
    
    printf("\n");
    
    // Cleanup
    neoc_cleanup();
    
    printf("==============================\n");
    printf("Comprehensive example completed successfully!\n");
    
    return 0;
}

