/**
 * @file test_signer.c
 * @brief Unit tests converted from SignerTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness_scope.h"
#include "neoc/wallet/account.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/types/hash160.h"
#include "neoc/utils/hex.h"
#include "neoc/serialization/binary_writer.h"

// Test data
static const char *ACCOUNT_WIF = "Kzt94tAAiZSgH7Yt4i25DW6jJFprZFPSqTgLr5dWmWgKDKCjXMfZ";
static const char *CONTRACT1_SCRIPT = "d802a401";
static const char *CONTRACT2_SCRIPT = "c503b112";
static const char *GROUP_PUBKEY1 = "0306d3e7f18e6dd477d34ce3cfeca172a877f3c907cc6c2b66c295d1fcc76ff8f7";
static const char *GROUP_PUBKEY2 = "02958ab88e4cea7ae1848047daeb8883daf5fdf5c1301dbbfe973f0a29fe75de60";

// Test objects
static neoc_account_t *account = NULL;
static neoc_hash160_t *account_script_hash = NULL;
static neoc_hash160_t *contract1 = NULL;
static neoc_hash160_t *contract2 = NULL;
static neoc_ec_public_key_t *group_pubkey1 = NULL;
static neoc_ec_public_key_t *group_pubkey2 = NULL;

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
    
    // Create account from WIF
    err = neoc_account_from_wif(ACCOUNT_WIF, &account);
    assert(err == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Get account script hash
    err = neoc_account_get_script_hash(account, &account_script_hash);
    assert(err == NEOC_SUCCESS);
    assert(account_script_hash != NULL);
    
    // Create contract hashes from scripts
    uint8_t script1[4];
    size_t script1_len = 0;
    err = neoc_hex_decode(CONTRACT1_SCRIPT, script1, sizeof(script1), &script1_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_hash160_from_script(script1, script1_len, &contract1);
    assert(err == NEOC_SUCCESS);
    
    uint8_t script2[4];
    size_t script2_len = 0;
    err = neoc_hex_decode(CONTRACT2_SCRIPT, script2, sizeof(script2), &script2_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_hash160_from_script(script2, script2_len, &contract2);
    assert(err == NEOC_SUCCESS);
    
    // Create group public keys
    uint8_t pubkey1_bytes[33];
    size_t pubkey1_len = 0;
    err = neoc_hex_decode(GROUP_PUBKEY1, pubkey1_bytes, sizeof(pubkey1_bytes), &pubkey1_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_ec_public_key_from_bytes(pubkey1_bytes, pubkey1_len, &group_pubkey1);
    assert(err == NEOC_SUCCESS);
    
    uint8_t pubkey2_bytes[33];
    size_t pubkey2_len = 0;
    err = neoc_hex_decode(GROUP_PUBKEY2, pubkey2_bytes, sizeof(pubkey2_bytes), &pubkey2_len);
    assert(err == NEOC_SUCCESS);
    err = neoc_ec_public_key_from_bytes(pubkey2_bytes, pubkey2_len, &group_pubkey2);
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    if (account) {
        neoc_account_free(account);
        account = NULL;
    }
    if (account_script_hash) {
        neoc_hash160_free(account_script_hash);
        account_script_hash = NULL;
    }
    if (contract1) {
        neoc_hash160_free(contract1);
        contract1 = NULL;
    }
    if (contract2) {
        neoc_hash160_free(contract2);
        contract2 = NULL;
    }
    if (group_pubkey1) {
        neoc_ec_public_key_free(group_pubkey1);
        group_pubkey1 = NULL;
    }
    if (group_pubkey2) {
        neoc_ec_public_key_free(group_pubkey2);
        group_pubkey2 = NULL;
    }
    neoc_cleanup();
}

// Test create signer with called by entry witness scope
static void test_create_signer_with_called_by_entry_scope(void) {
    printf("Testing create signer with called by entry scope...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_called_by_entry(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    assert(signer != NULL);
    
    // Verify signer properties
    neoc_hash160_t *signer_hash = neoc_signer_get_account(signer);
    assert(neoc_hash160_equals(signer_hash, account_script_hash));
    
    neoc_witness_scope_t scope = neoc_signer_get_scope(signer);
    assert(scope == NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY);
    
    size_t num_contracts = neoc_signer_get_allowed_contracts_count(signer);
    assert(num_contracts == 0);
    
    size_t num_groups = neoc_signer_get_allowed_groups_count(signer);
    assert(num_groups == 0);
    
    neoc_signer_free(signer);
    printf("  ✅ Create signer with called by entry scope test passed\n");
}

// Test create signer with global witness scope
static void test_create_signer_with_global_scope(void) {
    printf("Testing create signer with global scope...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_global(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    assert(signer != NULL);
    
    // Verify signer properties
    neoc_hash160_t *signer_hash = neoc_signer_get_account(signer);
    assert(neoc_hash160_equals(signer_hash, account_script_hash));
    
    neoc_witness_scope_t scope = neoc_signer_get_scope(signer);
    assert(scope == NEOC_WITNESS_SCOPE_GLOBAL);
    
    size_t num_contracts = neoc_signer_get_allowed_contracts_count(signer);
    assert(num_contracts == 0);
    
    size_t num_groups = neoc_signer_get_allowed_groups_count(signer);
    assert(num_groups == 0);
    
    neoc_signer_free(signer);
    printf("  ✅ Create signer with global scope test passed\n");
}

// Test build valid signer with allowed contracts
static void test_build_valid_signer_with_contracts(void) {
    printf("Testing build valid signer with allowed contracts...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_called_by_entry(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    
    // Set allowed contracts
    neoc_hash160_t *contracts[2] = {contract1, contract2};
    err = neoc_signer_set_allowed_contracts(signer, contracts, 2);
    assert(err == NEOC_SUCCESS);
    
    // Verify scope includes custom contracts
    neoc_witness_scope_t scope = neoc_signer_get_scope(signer);
    assert((scope & NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS) != 0);
    assert((scope & NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY) != 0);
    
    // Verify allowed contracts
    size_t num_contracts = neoc_signer_get_allowed_contracts_count(signer);
    assert(num_contracts == 2);
    
    neoc_signer_free(signer);
    printf("  ✅ Build valid signer with allowed contracts test passed\n");
}

// Test build valid signer with allowed groups
static void test_build_valid_signer_with_groups(void) {
    printf("Testing build valid signer with allowed groups...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_none(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    
    // Set allowed groups
    neoc_ec_public_key_t *groups[2] = {group_pubkey1, group_pubkey2};
    err = neoc_signer_set_allowed_groups(signer, groups, 2);
    assert(err == NEOC_SUCCESS);
    
    // Verify scope includes custom groups
    neoc_witness_scope_t scope = neoc_signer_get_scope(signer);
    assert(scope == NEOC_WITNESS_SCOPE_CUSTOM_GROUPS);
    
    // Verify allowed groups
    size_t num_groups = neoc_signer_get_allowed_groups_count(signer);
    assert(num_groups == 2);
    
    neoc_signer_free(signer);
    printf("  ✅ Build valid signer with allowed groups test passed\n");
}

// Test fail building signer with global scope and custom contracts
static void test_fail_global_scope_with_contracts(void) {
    printf("Testing fail building signer with global scope and contracts...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_global(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    
    // Try to set allowed contracts on global scope signer - should fail
    neoc_hash160_t *contracts[2] = {contract1, contract2};
    err = neoc_signer_set_allowed_contracts(signer, contracts, 2);
    assert(err != NEOC_SUCCESS);
    
    neoc_signer_free(signer);
    printf("  ✅ Fail global scope with contracts test passed\n");
}

// Test fail building signer with too many contracts
static void test_fail_too_many_contracts(void) {
    printf("Testing fail building signer with too many contracts...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_called_by_entry(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    
    // Create 17 contracts (exceeds MAX_SIGNER_SUBITEMS which is 16)
    neoc_hash160_t *contracts[17];
    for (int i = 0; i < 17; i++) {
        err = neoc_hash160_from_string("3ab0be8672e25cf475219d018ded961ec684ca88", &contracts[i]);
        assert(err == NEOC_SUCCESS);
    }
    
    // Try to set too many contracts - should fail
    err = neoc_signer_set_allowed_contracts(signer, contracts, 17);
    assert(err != NEOC_SUCCESS);
    
    // Clean up
    for (int i = 0; i < 17; i++) {
        neoc_hash160_free(contracts[i]);
    }
    neoc_signer_free(signer);
    
    printf("  ✅ Fail too many contracts test passed\n");
}

// Test serialize signer with global scope
static void test_serialize_global_scope(void) {
    printf("Testing serialize signer with global scope...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_global(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    
    // Serialize signer
    neoc_binary_writer_t *writer = NULL;
    err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_signer_serialize(signer, writer);
    assert(err == NEOC_SUCCESS);
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_len);
    assert(err == NEOC_SUCCESS);
    
    // Verify serialization (account hash + global scope byte)
    assert(data_len == 21); // 20 bytes hash + 1 byte scope
    assert(data[20] == NEOC_WITNESS_SCOPE_GLOBAL);
    
    free(data);
    neoc_binary_writer_free(writer);
    neoc_signer_free(signer);
    
    printf("  ✅ Serialize global scope test passed\n");
}

// Test serialize signer with custom contracts
static void test_serialize_custom_contracts(void) {
    printf("Testing serialize signer with custom contracts...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t err = neoc_signer_create_none(account_script_hash, &signer);
    assert(err == NEOC_SUCCESS);
    
    // Set allowed contracts
    neoc_hash160_t *contracts[2] = {contract1, contract2};
    err = neoc_signer_set_allowed_contracts(signer, contracts, 2);
    assert(err == NEOC_SUCCESS);
    
    // Serialize signer
    neoc_binary_writer_t *writer = NULL;
    err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_signer_serialize(signer, writer);
    assert(err == NEOC_SUCCESS);
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_len);
    assert(err == NEOC_SUCCESS);
    
    // Verify serialization includes contracts
    assert(data_len > 21); // Should include contract data
    assert(data[20] == NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS);
    assert(data[21] == 2); // Number of contracts
    
    free(data);
    neoc_binary_writer_free(writer);
    neoc_signer_free(signer);
    
    printf("  ✅ Serialize custom contracts test passed\n");
}

int main(void) {
    printf("\n=== SignerTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_create_signer_with_called_by_entry_scope();
    test_create_signer_with_global_scope();
    test_build_valid_signer_with_contracts();
    test_build_valid_signer_with_groups();
    test_fail_global_scope_with_contracts();
    test_fail_too_many_contracts();
    test_serialize_global_scope();
    test_serialize_custom_contracts();
    
    tearDown();
    
    printf("\n✅ All SignerTests tests passed!\n\n");
    return 0;
}
