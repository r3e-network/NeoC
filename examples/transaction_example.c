/**
 * @file transaction_example.c
 * @brief Example application demonstrating NeoC transaction functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <neoc/neoc.h>
#include <neoc/transaction/transaction_builder.h>
#include <neoc/transaction/transaction.h>
#include <neoc/wallet/account.h>
#include <neoc/contract/neo_token.h>
#include <neoc/contract/gas_token.h>
#include <neoc/script/script_builder.h>
#include <neoc/types/uint256.h>
#include <neoc/protocol/rpc_client.h>

/**
 * Example 1: Build a simple transfer transaction
 */
int example_simple_transfer() {
    printf("\n=== Example 1: Simple NEO Transfer Transaction ===\n\n");
    
    neoc_error_t err;
    
    // Create sender account (in real scenario, import from WIF or wallet)
    neoc_account_t *sender = NULL;
    err = neoc_account_create(&sender);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create sender account: %s\n", neoc_error_string(err));
        return -1;
    }
    
    printf("Sender address: %s\n", neoc_account_get_address(sender));
    
    // Create receiver account
    neoc_account_t *receiver = NULL;
    err = neoc_account_create(&receiver);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create receiver account: %s\n", neoc_error_string(err));
        neoc_account_free(sender);
        return -1;
    }
    
    printf("Receiver address: %s\n\n", neoc_account_get_address(receiver));
    
    // Create transaction builder
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create transaction builder: %s\n", neoc_error_string(err));
        neoc_account_free(sender);
        neoc_account_free(receiver);
        return -1;
    }
    
    // Get NEO token contract
    neoc_smart_contract_t *neo_token = NULL;
    err = neoc_neo_token_create(&neo_token);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create NEO token contract: %s\n", neoc_error_string(err));
        neoc_transaction_builder_free(builder);
        neoc_account_free(sender);
        neoc_account_free(receiver);
        return -1;
    }
    
    // Build transfer parameters
    int64_t amount = 10; // 10 NEO
    neoc_hash160_t *sender_hash = neoc_account_get_script_hash(sender);
    neoc_hash160_t *receiver_hash = neoc_account_get_script_hash(receiver);
    
    // Create contract parameters
    neoc_contract_parameter_t *params[4];
    err = neoc_contract_parameter_create_hash160(sender_hash, &params[0]);
    err |= neoc_contract_parameter_create_hash160(receiver_hash, &params[1]);
    err |= neoc_contract_parameter_create_integer(amount, &params[2]);
    err |= neoc_contract_parameter_create_null(&params[3]); // data parameter
    
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create contract parameters\n");
        // Clean up
        neoc_smart_contract_free(neo_token);
        neoc_transaction_builder_free(builder);
        neoc_account_free(sender);
        neoc_account_free(receiver);
        return -1;
    }
    
    // Add contract invocation to transaction
    err = neoc_transaction_builder_invoke_contract(
        builder,
        neoc_smart_contract_get_script_hash(neo_token),
        "transfer",
        params,
        4
    );
    
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to add contract invocation: %s\n", neoc_error_string(err));
        // Clean up parameters
        for (int i = 0; i < 4; i++) {
            neoc_contract_parameter_free(params[i]);
        }
        neoc_smart_contract_free(neo_token);
        neoc_transaction_builder_free(builder);
        neoc_account_free(sender);
        neoc_account_free(receiver);
        return -1;
    }
    
    printf("✓ Transfer script added to transaction\n");
    printf("  From: %s\n", neoc_account_get_address(sender));
    printf("  To: %s\n", neoc_account_get_address(receiver));
    printf("  Amount: %lld NEO\n\n", (long long)amount);
    
    // Set transaction attributes (derive expiry from RPC if available)
    neoc_rpc_client_t *rpc = NULL;
    if (neoc_rpc_client_create("http://localhost:10332", &rpc) == NEOC_SUCCESS) {
        err = neoc_transaction_builder_set_valid_until_block_from_rpc(builder, rpc, 1000);
        neoc_rpc_client_free(rpc);
    } else {
        err = neoc_transaction_builder_set_valid_until_block(builder, TX_DEFAULT_VALID_UNTIL_BLOCK);
    }
    err |= neoc_transaction_builder_set_system_fee(builder, 1000000); // 0.01 GAS
    err |= neoc_transaction_builder_set_network_fee(builder, 1000000); // 0.01 GAS
    
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to set transaction attributes\n");
    }
    
    // Add signer
    err = neoc_transaction_builder_add_signer(builder, sender);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to add signer: %s\n", neoc_error_string(err));
    } else {
        printf("✓ Signer added to transaction\n");
    }
    
    // Build the transaction
    neoc_transaction_t *transaction = NULL;
    err = neoc_transaction_builder_build(builder, &transaction);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to build transaction: %s\n", neoc_error_string(err));
    } else {
        printf("✓ Transaction built successfully\n");
        
        // Get transaction hash
        neoc_uint256_t *tx_hash = neoc_transaction_get_hash(transaction);
        char hash_str[65];
        neoc_uint256_to_string(tx_hash, hash_str, sizeof(hash_str));
        printf("  Transaction hash: %s\n", hash_str);
        
        // Get transaction size
        size_t tx_size = neoc_transaction_get_size(transaction);
        printf("  Transaction size: %zu bytes\n", tx_size);
    }
    
    // Sign transaction
    neoc_transaction_t *signed_tx = NULL;
    err = neoc_transaction_builder_sign(builder, sender, &signed_tx);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to sign transaction: %s\n", neoc_error_string(err));
    } else {
        printf("✓ Transaction signed successfully\n");
        
        // Verify witness
        size_t witness_count = neoc_transaction_get_witness_count(signed_tx);
        printf("  Witnesses: %zu\n", witness_count);
    }
    
    // Clean up
    for (int i = 0; i < 4; i++) {
        neoc_contract_parameter_free(params[i]);
    }
    if (transaction) neoc_transaction_free(transaction);
    if (signed_tx) neoc_transaction_free(signed_tx);
    neoc_smart_contract_free(neo_token);
    neoc_transaction_builder_free(builder);
    neoc_account_free(sender);
    neoc_account_free(receiver);
    
    return 0;
}

/**
 * Example 2: Build a multi-signature transaction
 */
int example_multisig_transaction() {
    printf("\n=== Example 2: Multi-Signature Transaction ===\n\n");
    
    neoc_error_t err;
    
    // Create 3 accounts for multi-sig
    neoc_account_t *accounts[3];
    neoc_ec_public_key_t *pub_keys[3];
    
    printf("Creating 3 accounts for 2-of-3 multi-sig:\n");
    for (int i = 0; i < 3; i++) {
        err = neoc_account_create(&accounts[i]);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to create account %d\n", i);
            for (int j = 0; j < i; j++) {
                neoc_account_free(accounts[j]);
            }
            return -1;
        }
        pub_keys[i] = neoc_account_get_public_key(accounts[i]);
        printf("  Account %d: %s\n", i + 1, neoc_account_get_address(accounts[i]));
    }
    
    // Create 2-of-3 multi-sig account
    neoc_account_t *multisig = NULL;
    err = neoc_account_create_multisig(2, pub_keys, 3, &multisig);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create multi-sig account\n");
        for (int i = 0; i < 3; i++) {
            neoc_account_free(accounts[i]);
        }
        return -1;
    }
    
    printf("\nMulti-sig account: %s\n\n", neoc_account_get_address(multisig));
    
    // Create transaction builder
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create transaction builder\n");
        neoc_account_free(multisig);
        for (int i = 0; i < 3; i++) {
            neoc_account_free(accounts[i]);
        }
        return -1;
    }
    
    // Create a simple script (get NEO balance)
    neoc_smart_contract_t *neo_token = NULL;
    err = neoc_neo_token_create(&neo_token);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create NEO token contract\n");
        neoc_transaction_builder_free(builder);
        neoc_account_free(multisig);
        for (int i = 0; i < 3; i++) {
            neoc_account_free(accounts[i]);
        }
        return -1;
    }
    
    // Add balance check invocation
    neoc_hash160_t *multisig_hash = neoc_account_get_script_hash(multisig);
    neoc_contract_parameter_t *param = NULL;
    err = neoc_contract_parameter_create_hash160(multisig_hash, &param);
    
    err = neoc_transaction_builder_invoke_contract(
        builder,
        neoc_smart_contract_get_script_hash(neo_token),
        "balanceOf",
        &param,
        1
    );
    
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to add contract invocation\n");
    } else {
        printf("✓ Balance check script added\n");
    }
    
    // Set transaction attributes
    neoc_transaction_builder_set_valid_until_block(builder, 1000000);
    neoc_transaction_builder_add_signer(builder, multisig);
    
    // Build transaction
    neoc_transaction_t *transaction = NULL;
    err = neoc_transaction_builder_build(builder, &transaction);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to build transaction\n");
    } else {
        printf("✓ Transaction built\n");
    }
    
    // Sign with first two accounts (2-of-3)
    printf("\nSigning with 2 out of 3 accounts:\n");
    
    neoc_witness_t *witnesses[2];
    for (int i = 0; i < 2; i++) {
        err = neoc_transaction_sign_multisig(transaction, accounts[i], multisig, &witnesses[i]);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to sign with account %d\n", i);
        } else {
            printf("  ✓ Signed with account %d\n", i + 1);
        }
    }
    
    // Combine witnesses
    err = neoc_transaction_add_multisig_witness(transaction, witnesses, 2, multisig);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to add multi-sig witness\n");
    } else {
        printf("✓ Multi-signature witness added\n");
        printf("  Threshold met: 2 of 3 signatures\n");
    }
    
    // Clean up
    if (param) neoc_contract_parameter_free(param);
    if (transaction) neoc_transaction_free(transaction);
    neoc_smart_contract_free(neo_token);
    neoc_transaction_builder_free(builder);
    neoc_account_free(multisig);
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
    }
    
    return 0;
}

/**
 * Example 3: Transaction serialization and deserialization
 */
int example_transaction_serialization() {
    printf("\n=== Example 3: Transaction Serialization ===\n\n");
    
    neoc_error_t err;
    
    // Create a simple transaction
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create transaction builder\n");
        return -1;
    }
    
    // Add a simple script
    neoc_script_builder_t *script_builder = NULL;
    err = neoc_script_builder_create(&script_builder);
    if (err != NEOC_SUCCESS) {
        neoc_transaction_builder_free(builder);
        return -1;
    }
    
    // Build script: PUSH1 PUSH2 ADD
    neoc_script_builder_push_integer(script_builder, 1);
    neoc_script_builder_push_integer(script_builder, 2);
    neoc_script_builder_emit_opcode(script_builder, NEOC_OPCODE_ADD);
    
    size_t script_size;
    uint8_t *script = neoc_script_builder_to_array(script_builder, &script_size);
    
    err = neoc_transaction_builder_set_script(builder, script, script_size);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to set script\n");
    } else {
        printf("✓ Script added: PUSH1 PUSH2 ADD\n");
        printf("  Script size: %zu bytes\n\n", script_size);
    }
    
    // Build transaction
    neoc_transaction_t *original_tx = NULL;
    err = neoc_transaction_builder_build(builder, &original_tx);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to build transaction\n");
        free(script);
        neoc_script_builder_free(script_builder);
        neoc_transaction_builder_free(builder);
        return -1;
    }
    
    // Get transaction hash
    neoc_uint256_t *original_hash = neoc_transaction_get_hash(original_tx);
    char original_hash_str[65];
    neoc_uint256_to_string(original_hash, original_hash_str, sizeof(original_hash_str));
    printf("Original transaction hash: %s\n", original_hash_str);
    
    // Serialize transaction to binary
    neoc_binary_writer_t *writer = NULL;
    err = neoc_binary_writer_create(&writer);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to create binary writer\n");
        neoc_transaction_free(original_tx);
        free(script);
        neoc_script_builder_free(script_builder);
        neoc_transaction_builder_free(builder);
        return -1;
    }
    
    err = neoc_transaction_serialize(original_tx, writer);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to serialize transaction\n");
    } else {
        size_t serialized_size = neoc_binary_writer_get_size(writer);
        printf("✓ Transaction serialized\n");
        printf("  Serialized size: %zu bytes\n\n", serialized_size);
        
        // Get serialized data
        const uint8_t *serialized_data = neoc_binary_writer_get_data(writer);
        
        // Print hex representation (first 32 bytes)
        printf("Serialized data (first 32 bytes):\n  ");
        for (size_t i = 0; i < 32 && i < serialized_size; i++) {
            printf("%02x", serialized_data[i]);
        }
        printf("...\n\n");
        
        // Deserialize transaction
        neoc_binary_reader_t *reader = NULL;
        err = neoc_binary_reader_create(serialized_data, serialized_size, &reader);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to create binary reader\n");
        } else {
            neoc_transaction_t *deserialized_tx = NULL;
            err = neoc_transaction_deserialize(reader, &deserialized_tx);
            if (err != NEOC_SUCCESS) {
                fprintf(stderr, "Failed to deserialize transaction\n");
            } else {
                printf("✓ Transaction deserialized\n");
                
                // Verify hash matches
                neoc_uint256_t *deserialized_hash = neoc_transaction_get_hash(deserialized_tx);
                char deserialized_hash_str[65];
                neoc_uint256_to_string(deserialized_hash, deserialized_hash_str, sizeof(deserialized_hash_str));
                
                if (strcmp(original_hash_str, deserialized_hash_str) == 0) {
                    printf("✓ Hash verification successful\n");
                    printf("  Original hash:     %s\n", original_hash_str);
                    printf("  Deserialized hash: %s\n", deserialized_hash_str);
                } else {
                    printf("✗ Hash mismatch!\n");
                }
                
                neoc_transaction_free(deserialized_tx);
            }
            
            neoc_binary_reader_free(reader);
        }
    }
    
    // Serialize to JSON
    char *json = NULL;
    err = neoc_transaction_to_json(original_tx, &json);
    if (err == NEOC_SUCCESS && json != NULL) {
        printf("\n✓ Transaction serialized to JSON:\n");
        printf("%.500s...\n", json); // Print first 500 chars
        free(json);
    }
    
    // Clean up
    neoc_binary_writer_free(writer);
    neoc_transaction_free(original_tx);
    free(script);
    neoc_script_builder_free(script_builder);
    neoc_transaction_builder_free(builder);
    
    return 0;
}

/**
 * Main function - run all examples
 */
int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("   NeoC SDK - Transaction Examples\n");
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
        result = example_simple_transfer();
        if (result != 0) goto cleanup;
    }
    
    if (example_num == 0 || example_num == 2) {
        result = example_multisig_transaction();
        if (result != 0) goto cleanup;
    }
    
    if (example_num == 0 || example_num == 3) {
        result = example_transaction_serialization();
        if (result != 0) goto cleanup;
    }
    
    printf("\n========================================\n");
    printf("  All examples completed successfully!\n");
    printf("========================================\n\n");
    
cleanup:
    // Clean up NeoC SDK
    neoc_cleanup();
    
    return result;
}
