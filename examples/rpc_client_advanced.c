/**
 * @file rpc_client_advanced.c
 * @brief Advanced RPC client example demonstrating all Neo RPC capabilities
 * 
 * This example shows:
 * - Connecting to Neo nodes
 * - Querying blockchain state
 * - Getting account balances
 * - Sending transactions
 * - Monitoring blocks
 * - NEP-17 token operations
 * - Smart contract interactions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "neoc/neoc.h"

// Configuration
#define MAINNET_URL "http://seed1.neo.org:10332"
#define TESTNET_URL "http://seed1.ngd.network:20332"
#define LOCALHOST_URL "http://localhost:10332"

// Well-known contract hashes
#define NEO_CONTRACT "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"
#define GAS_CONTRACT "0xd2a4cff31913016155e38e474a2c06d08be276cf"

/**
 * Print formatted JSON response
 */
void print_json_response(const char* title, const char* json) {
    printf("\n=== %s ===\n", title);
    if (json) {
        printf("%s\n", json);
    } else {
        printf("(null response)\n");
    }
}

/**
 * Example: Get blockchain information
 */
void example_blockchain_info(rpc_client_t* client) {
    printf("\n========================================\n");
    printf("Blockchain Information\n");
    printf("========================================\n");
    
    // Get version
    rpc_version_response_t* version = rpc_get_version(client);
    if (version) {
        printf("\nNode Version:\n");
        printf("  Protocol: %d\n", version->protocol);
        printf("  Network: %d\n", version->network);
        printf("  User Agent: %s\n", version->user_agent);
        rpc_version_response_free(version);
    }
    
    // Get block count
    uint32_t block_count = 0;
    if (rpc_get_block_count(client, &block_count) == NEOC_SUCCESS) {
        printf("\nBlock Count: %u\n", block_count);
    }
    
    // Get best block hash
    char best_hash[65];
    if (rpc_get_best_block_hash(client, best_hash, sizeof(best_hash)) == NEOC_SUCCESS) {
        printf("Best Block Hash: %s\n", best_hash);
    }
    
    // Get connection count
    uint32_t connections = 0;
    if (rpc_get_connection_count(client, &connections) == NEOC_SUCCESS) {
        printf("Connected Peers: %u\n", connections);
    }
    
    // Get mempool info
    rpc_mempool_response_t* mempool = rpc_get_raw_mempool(client);
    if (mempool) {
        printf("\nMempool:\n");
        printf("  Verified Transactions: %zu\n", mempool->verified_count);
        printf("  Unverified Transactions: %zu\n", mempool->unverified_count);
        rpc_mempool_response_free(mempool);
    }
}

/**
 * Example: Get block information
 */
void example_block_info(rpc_client_t* client, uint32_t height) {
    printf("\n========================================\n");
    printf("Block Information (Height: %u)\n", height);
    printf("========================================\n");
    
    // Get block by height
    rpc_block_response_t* block = rpc_get_block(client, height);
    if (block) {
        printf("\nBlock #%u:\n", block->index);
        printf("  Hash: %s\n", block->hash);
        printf("  Time: %llu\n", (unsigned long long)block->time);
        printf("  Version: %u\n", block->version);
        printf("  Previous Hash: %s\n", block->prev_hash);
        printf("  Merkle Root: %s\n", block->merkle_root);
        printf("  Next Consensus: %s\n", block->next_consensus);
        printf("  Transactions: %zu\n", block->tx_count);
        printf("  Size: %u bytes\n", block->size);
        
        // Show first few transactions
        if (block->tx_count > 0) {
            printf("\n  First Transaction:\n");
            printf("    Hash: %s\n", block->transactions[0]);
        }
        
        rpc_block_response_free(block);
    } else {
        printf("Failed to get block information\n");
    }
}

/**
 * Example: Get account balance
 */
void example_account_balance(rpc_client_t* client, const char* address) {
    printf("\n========================================\n");
    printf("Account Balance\n");
    printf("========================================\n");
    
    printf("\nAddress: %s\n", address);
    
    // Get NEO balance
    char* neo_balance = rpc_get_nep17_balance(client, address, NEO_CONTRACT);
    if (neo_balance) {
        printf("NEO Balance: %s\n", neo_balance);
        free(neo_balance);
    }
    
    // Get GAS balance
    char* gas_balance = rpc_get_nep17_balance(client, address, GAS_CONTRACT);
    if (gas_balance) {
        printf("GAS Balance: %s\n", gas_balance);
        free(gas_balance);
    }
    
    // Get all NEP-17 balances
    rpc_nep17_balances_t* balances = rpc_get_nep17_balances(client, address);
    if (balances) {
        printf("\nAll NEP-17 Token Balances:\n");
        for (size_t i = 0; i < balances->balance_count; i++) {
            printf("  Token: %s\n", balances->balances[i].asset_hash);
            printf("    Amount: %s\n", balances->balances[i].amount);
            printf("    Last Updated: %u\n", balances->balances[i].last_updated_block);
        }
        rpc_nep17_balances_free(balances);
    }
}

/**
 * Example: Get transaction information
 */
void example_transaction_info(rpc_client_t* client, const char* tx_hash) {
    printf("\n========================================\n");
    printf("Transaction Information\n");
    printf("========================================\n");
    
    printf("\nTransaction Hash: %s\n", tx_hash);
    
    // Get raw transaction
    char* raw_tx = rpc_get_raw_transaction(client, tx_hash);
    if (raw_tx) {
        printf("\nRaw Transaction (hex):\n");
        printf("%.100s...\n", raw_tx); // Show first 100 chars
        free(raw_tx);
    }
    
    // Get application log
    char* app_log = rpc_get_application_log(client, tx_hash);
    if (app_log) {
        print_json_response("Application Log", app_log);
        free(app_log);
    }
}

/**
 * Example: Invoke smart contract (read-only)
 */
void example_invoke_contract(rpc_client_t* client) {
    printf("\n========================================\n");
    printf("Smart Contract Invocation (Read-Only)\n");
    printf("========================================\n");
    
    // Example: Get NEO token symbol
    script_builder_t* builder = script_builder_create();
    if (!builder) {
        printf("Failed to create script builder\n");
        return;
    }
    
    // Build script to call "symbol" method
    script_builder_emit_push(builder, "symbol");
    hash160_t* neo_hash = hash160_from_string(NEO_CONTRACT);
    script_builder_emit_app_call(builder, neo_hash);
    
    // Get script bytes
    uint8_t* script = NULL;
    size_t script_len = 0;
    if (script_builder_to_array(builder, &script, &script_len) != NEOC_SUCCESS) {
        printf("Failed to build script\n");
        hash160_free(neo_hash);
        script_builder_free(builder);
        return;
    }
    
    // Convert to base64 for RPC
    char* script_base64 = base64_encode(script, script_len);
    if (!script_base64) {
        printf("Failed to encode script\n");
        free(script);
        hash160_free(neo_hash);
        script_builder_free(builder);
        return;
    }
    
    // Invoke script
    printf("\nInvoking NEO contract 'symbol' method...\n");
    char* result = rpc_invoke_script(client, script_base64, NULL);
    if (result) {
        print_json_response("Invocation Result", result);
        free(result);
    }
    
    // Cleanup
    free(script_base64);
    free(script);
    hash160_free(neo_hash);
    script_builder_free(builder);
}

/**
 * Example: Build and send transaction
 */
void example_send_transaction(rpc_client_t* client, ec_key_pair_t* key_pair) {
    printf("\n========================================\n");
    printf("Build and Send Transaction\n");
    printf("========================================\n");
    
    // Get sender address
    char sender_address[64];
    if (ec_key_pair_get_address(key_pair, sender_address, sizeof(sender_address)) != NEOC_SUCCESS) {
        printf("Failed to get sender address\n");
        return;
    }
    
    printf("\nSender Address: %s\n", sender_address);
    
    // Create recipient address (example)
    const char* recipient_address = "NXXnRQ5nMr6D3vwkPufPiH7PTVpUmEXdKf";
    printf("Recipient Address: %s\n", recipient_address);
    
    // Build transaction
    transaction_builder_t* builder = transaction_builder_create();
    if (!builder) {
        printf("Failed to create transaction builder\n");
        return;
    }
    
    // Set transaction properties
    transaction_builder_set_nonce(builder, (uint32_t)time(NULL));
    transaction_builder_set_valid_until_block(builder, 1000000);
    transaction_builder_set_system_fee(builder, 1000000);  // 0.01 GAS
    transaction_builder_set_network_fee(builder, 500000);  // 0.005 GAS
    
    // Build transfer script (1 GAS)
    script_builder_t* script_builder = script_builder_create();
    if (!script_builder) {
        printf("Failed to create script builder\n");
        transaction_builder_free(builder);
        return;
    }
    
    // Push parameters for transfer
    script_builder_push_integer(script_builder, 100000000);  // 1 GAS
    
    hash160_t* to_hash = hash160_from_address(recipient_address);
    uint8_t to_bytes[20];
    hash160_to_bytes(to_hash, to_bytes, sizeof(to_bytes));
    script_builder_push_data(script_builder, to_bytes, 20);
    
    hash160_t* from_hash = hash160_from_address(sender_address);
    uint8_t from_bytes[20];
    hash160_to_bytes(from_hash, from_bytes, sizeof(from_bytes));
    script_builder_push_data(script_builder, from_bytes, 20);
    
    script_builder_push_integer(script_builder, 3);  // Parameter count
    script_builder_emit_push(script_builder, "transfer");
    
    // Add GAS contract
    hash160_t* gas_hash = hash160_from_string(GAS_CONTRACT);
    script_builder_emit_app_call(script_builder, gas_hash);
    
    // Get script bytes
    uint8_t* script = NULL;
    size_t script_len = 0;
    script_builder_to_array(script_builder, &script, &script_len);
    
    // Set script in transaction
    transaction_builder_set_script(builder, script, script_len);
    
    // Add signer
    signer_t* signer = signer_create_with_scope(from_hash, WITNESS_SCOPE_CALLED_BY_ENTRY);
    transaction_builder_add_signer(builder, signer);
    
    // Build transaction
    transaction_t* tx = transaction_builder_build(builder);
    if (!tx) {
        printf("Failed to build transaction\n");
        goto cleanup;
    }
    
    // Sign transaction
    uint8_t* tx_hash_data = NULL;
    size_t hash_len = 0;
    transaction_get_hash_data(tx, &tx_hash_data, &hash_len);
    
    ecdsa_signature_t* signature = sign_message(tx_hash_data, hash_len, key_pair);
    if (!signature) {
        printf("Failed to sign transaction\n");
        free(tx_hash_data);
        transaction_free(tx);
        goto cleanup;
    }
    
    // Create witness
    witness_t* witness = witness_create();
    
    // Build invocation script
    uint8_t* inv_script = NULL;
    size_t inv_len = 0;
    script_builder_build_invocation_script(signature, &inv_script, &inv_len);
    witness_set_invocation_script(witness, inv_script, inv_len);
    
    // Build verification script
    ec_public_key_t* pub_key = ec_key_pair_get_public_key(key_pair);
    uint8_t* ver_script = NULL;
    size_t ver_len = 0;
    script_builder_build_verification_script(pub_key, &ver_script, &ver_len);
    witness_set_verification_script(witness, ver_script, ver_len);
    
    // Add witness to transaction
    transaction_add_witness(tx, witness);
    
    // Serialize transaction
    uint8_t* tx_bytes = NULL;
    size_t tx_size = 0;
    transaction_serialize(tx, &tx_bytes, &tx_size);
    
    // Convert to base64
    char* tx_base64 = base64_encode(tx_bytes, tx_size);
    
    printf("\nTransaction built successfully\n");
    printf("Size: %zu bytes\n", tx_size);
    
    // Send transaction (commented out for safety)
    // char* tx_id = rpc_send_raw_transaction(client, tx_base64);
    // if (tx_id) {
    //     printf("Transaction sent! ID: %s\n", tx_id);
    //     free(tx_id);
    // }
    
    printf("\n(Transaction not sent - uncomment code to enable sending)\n");
    
    // Cleanup
    free(tx_base64);
    free(tx_bytes);
    free(ver_script);
    free(inv_script);
    ec_public_key_free(pub_key);
    witness_free(witness);
    ecdsa_signature_free(signature);
    free(tx_hash_data);
    transaction_free(tx);
    
cleanup:
    signer_free(signer);
    free(script);
    hash160_free(gas_hash);
    hash160_free(from_hash);
    hash160_free(to_hash);
    script_builder_free(script_builder);
    transaction_builder_free(builder);
}

/**
 * Example: Monitor new blocks
 */
void example_monitor_blocks(rpc_client_t* client, int duration_seconds) {
    printf("\n========================================\n");
    printf("Block Monitor (Duration: %d seconds)\n", duration_seconds);
    printf("========================================\n");
    
    time_t start_time = time(NULL);
    uint32_t last_height = 0;
    
    // Get initial block height
    if (rpc_get_block_count(client, &last_height) != NEOC_SUCCESS) {
        printf("Failed to get initial block count\n");
        return;
    }
    
    printf("\nStarting at block height: %u\n", last_height);
    printf("Monitoring for new blocks...\n\n");
    
    while (time(NULL) - start_time < duration_seconds) {
        uint32_t current_height = 0;
        
        if (rpc_get_block_count(client, &current_height) == NEOC_SUCCESS) {
            if (current_height > last_height) {
                // New block(s) detected
                for (uint32_t h = last_height + 1; h <= current_height; h++) {
                    rpc_block_response_t* block = rpc_get_block(client, h);
                    if (block) {
                        time_t block_time = (time_t)block->time;
                        printf("[%s] Block #%u - Hash: %.16s... - Txs: %zu\n",
                               ctime(&block_time), block->index,
                               block->hash, block->tx_count);
                        rpc_block_response_free(block);
                    }
                }
                last_height = current_height;
            }
        }
        
        sleep(1);  // Check every second
    }
    
    printf("\nMonitoring complete.\n");
}

// Main function
int main(int argc, char* argv[]) {
    printf("\n");
    printf("================================================================================\n");
    printf("                      NeoC SDK - Advanced RPC Client Example                   \n");
    printf("================================================================================\n");
    printf("\n");
    
    // Initialize SDK
    if (neoc_init() != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC SDK\n");
        return 1;
    }
    
    // Select network
    const char* rpc_url = TESTNET_URL;  // Default to testnet
    if (argc > 1) {
        if (strcmp(argv[1], "mainnet") == 0) {
            rpc_url = MAINNET_URL;
            printf("Using MainNet: %s\n", rpc_url);
        } else if (strcmp(argv[1], "localhost") == 0) {
            rpc_url = LOCALHOST_URL;
            printf("Using Localhost: %s\n", rpc_url);
        } else {
            rpc_url = argv[1];
            printf("Using custom URL: %s\n", rpc_url);
        }
    } else {
        printf("Using TestNet: %s\n", rpc_url);
        printf("(Use './rpc_client_advanced mainnet' for MainNet)\n");
    }
    
    // Create RPC client
    rpc_client_t* client = rpc_client_create(rpc_url);
    if (!client) {
        fprintf(stderr, "Failed to create RPC client\n");
        neoc_cleanup();
        return 1;
    }
    
    // Example 1: Get blockchain information
    example_blockchain_info(client);
    
    // Example 2: Get specific block information
    example_block_info(client, 1);  // Genesis block
    
    // Example 3: Get account balance (example address)
    example_account_balance(client, "NXXnRQ5nMr6D3vwkPufPiH7PTVpUmEXdKf");
    
    // Example 4: Invoke contract (read-only)
    example_invoke_contract(client);
    
    // Example 5: Build transaction (requires private key)
    printf("\n========================================\n");
    printf("Transaction Building Example\n");
    printf("========================================\n");
    
    // Generate a test key pair
    ec_key_pair_t* test_key = ec_key_pair_create();
    if (test_key) {
        char test_address[64];
        ec_key_pair_get_address(test_key, test_address, sizeof(test_address));
        printf("\nGenerated test address: %s\n");
        printf("(This is just for demonstration - would need funds to send real transaction)\n");
        
        example_send_transaction(client, test_key);
        ec_key_pair_free(test_key);
    }
    
    // Example 6: Monitor blocks (optional - takes time)
    char response[10];
    printf("\nMonitor blocks for 10 seconds? (y/n): ");
    if (fgets(response, sizeof(response), stdin) && response[0] == 'y') {
        example_monitor_blocks(client, 10);
    }
    
    // Cleanup
    rpc_client_free(client);
    neoc_cleanup();
    
    printf("\n");
    printf("================================================================================\n");
    printf("RPC client example complete.\n");
    printf("================================================================================\n");
    printf("\n");
    
    return 0;
}
