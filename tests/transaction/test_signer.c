/**
 * @file test_signer.c
 * @brief Unit tests for transaction signer functionality
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "neoc/transaction/signer.h"
#include "neoc/wallet/account.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/crypto/ec_public_key.h"
#include "neoc/utils/hex.h"
#include "neoc/neoc_memory.h"

#define TEST_WIF "Kzt94tAAiZSgH7Yt4i25DW6jJFprZFPSqTgLr5dWmWgKDKCjXMfZ"
#define CONTRACT1_SCRIPT "d802a401"
#define CONTRACT2_SCRIPT "c503b112"
#define GROUP_PUBKEY1 "0306d3e7f18e6dd477d34ce3cfeca172a877f3c907cc6c2b66c295d1fcc76ff8f7"
#define GROUP_PUBKEY2 "02958ab88e4cea7ae1848047daeb8883daf5fdf5c1301dbbfe973f0a29fe75de60"

static neoc_account_t *account = NULL;
static neoc_hash160_t *account_script_hash = NULL;
static neoc_hash160_t *contract1 = NULL;
static neoc_hash160_t *contract2 = NULL;
static neoc_ec_public_key_t *group_pubkey1 = NULL;
static neoc_ec_public_key_t *group_pubkey2 = NULL;

static void setup_test_data(void) {
    // Create account from WIF
    neoc_error_t result = neoc_account_from_wif(&account, TEST_WIF);
    assert(result == NEOC_SUCCESS);
    assert(account != NULL);
    
    // Get account script hash
    result = neoc_account_get_script_hash(account, &account_script_hash);
    assert(result == NEOC_SUCCESS);
    assert(account_script_hash != NULL);
    
    // Create contract hashes from scripts
    uint8_t script1[4];
    size_t script1_len = sizeof(script1);
    result = neoc_hex_decode(CONTRACT1_SCRIPT, strlen(CONTRACT1_SCRIPT), script1, &script1_len);
    assert(result == NEOC_SUCCESS);
    
    contract1 = (neoc_hash160_t*)neoc_malloc(sizeof(neoc_hash160_t));
    assert(contract1 != NULL);
    result = neoc_hash160_from_script(contract1, script1, script1_len);
    assert(result == NEOC_SUCCESS);
    
    uint8_t script2[4];
    size_t script2_len = sizeof(script2);
    result = neoc_hex_decode(CONTRACT2_SCRIPT, strlen(CONTRACT2_SCRIPT), script2, &script2_len);
    assert(result == NEOC_SUCCESS);
    
    contract2 = (neoc_hash160_t*)neoc_malloc(sizeof(neoc_hash160_t));
    assert(contract2 != NULL);
    result = neoc_hash160_from_script(contract2, script2, script2_len);
    assert(result == NEOC_SUCCESS);
    
    // Create group public keys
    result = neoc_ec_public_key_from_hex(&group_pubkey1, GROUP_PUBKEY1);
    assert(result == NEOC_SUCCESS);
    assert(group_pubkey1 != NULL);
    
    result = neoc_ec_public_key_from_hex(&group_pubkey2, GROUP_PUBKEY2);
    assert(result == NEOC_SUCCESS);
    assert(group_pubkey2 != NULL);
}

static void cleanup_test_data(void) {
    if (account) {
        neoc_account_free(account);
        account = NULL;
    }
    if (contract1) {
        neoc_free(contract1);
        contract1 = NULL;
    }
    if (contract2) {
        neoc_free(contract2);
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
}

static void test_create_signer_with_called_by_entry_witness_scope(void) {
    printf("Testing create signer with CalledByEntry witness scope...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t result = neoc_signer_create_called_by_entry(&signer, account_script_hash);
    assert(result == NEOC_SUCCESS);
    assert(signer != NULL);
    
    // Verify signer properties
    neoc_hash160_t *signer_hash = NULL;
    result = neoc_signer_get_account(signer, &signer_hash);
    assert(result == NEOC_SUCCESS);
    assert(neoc_hash160_equal(signer_hash, account_script_hash));
    
    neoc_witness_scope_t scope;
    result = neoc_signer_get_scope(signer, &scope);
    assert(result == NEOC_SUCCESS);
    assert(scope == NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY);
    
    size_t contract_count, group_count;
    result = neoc_signer_get_allowed_contracts_count(signer, &contract_count);
    assert(result == NEOC_SUCCESS);
    assert(contract_count == 0);
    
    result = neoc_signer_get_allowed_groups_count(signer, &group_count);
    assert(result == NEOC_SUCCESS);
    assert(group_count == 0);
    
    neoc_signer_free(signer);
    printf("✓ test_create_signer_with_called_by_entry_witness_scope passed\n");
}

static void test_create_signer_with_global_witness_scope(void) {
    printf("Testing create signer with Global witness scope...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t result = neoc_signer_create_global(&signer, account_script_hash);
    assert(result == NEOC_SUCCESS);
    assert(signer != NULL);
    
    // Verify signer properties
    neoc_hash160_t *signer_hash = NULL;
    result = neoc_signer_get_account(signer, &signer_hash);
    assert(result == NEOC_SUCCESS);
    assert(neoc_hash160_equal(signer_hash, account_script_hash));
    
    neoc_witness_scope_t scope;
    result = neoc_signer_get_scope(signer, &scope);
    assert(result == NEOC_SUCCESS);
    assert(scope == NEOC_WITNESS_SCOPE_GLOBAL);
    
    size_t contract_count, group_count;
    result = neoc_signer_get_allowed_contracts_count(signer, &contract_count);
    assert(result == NEOC_SUCCESS);
    assert(contract_count == 0);
    
    result = neoc_signer_get_allowed_groups_count(signer, &group_count);
    assert(result == NEOC_SUCCESS);
    assert(group_count == 0);
    
    neoc_signer_free(signer);
    printf("✓ test_create_signer_with_global_witness_scope passed\n");
}

static void test_build_valid_signer_with_contracts(void) {
    printf("Testing build valid signer with allowed contracts...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t result = neoc_signer_create_called_by_entry(&signer, account_script_hash);
    assert(result == NEOC_SUCCESS);
    
    // Set allowed contracts
    neoc_hash160_t *contracts[] = {contract1, contract2};
    result = neoc_signer_set_allowed_contracts(signer, contracts, 2);
    assert(result == NEOC_SUCCESS);
    
    // Verify signer properties
    neoc_hash160_t *signer_hash = NULL;
    result = neoc_signer_get_account(signer, &signer_hash);
    assert(result == NEOC_SUCCESS);
    assert(neoc_hash160_equal(signer_hash, account_script_hash));
    
    neoc_witness_scope_t scope;
    result = neoc_signer_get_scope(signer, &scope);
    assert(result == NEOC_SUCCESS);
    assert(scope == (NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY | NEOC_WITNESS_SCOPE_CUSTOM_CONTRACTS));
    
    size_t contract_count;
    result = neoc_signer_get_allowed_contracts_count(signer, &contract_count);
    assert(result == NEOC_SUCCESS);
    assert(contract_count == 2);
    
    // Verify contracts
    neoc_hash160_t **allowed_contracts = NULL;
    result = neoc_signer_get_allowed_contracts(signer, &allowed_contracts, &contract_count);
    assert(result == NEOC_SUCCESS);
    assert(contract_count == 2);
    assert(neoc_hash160_equal(allowed_contracts[0], contract1));
    assert(neoc_hash160_equal(allowed_contracts[1], contract2));
    
    size_t group_count;
    result = neoc_signer_get_allowed_groups_count(signer, &group_count);
    assert(result == NEOC_SUCCESS);
    assert(group_count == 0);
    
    neoc_signer_free(signer);
    printf("✓ test_build_valid_signer_with_contracts passed\n");
}

static void test_build_valid_signer_with_groups(void) {
    printf("Testing build valid signer with allowed groups...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t result = neoc_signer_create_none(&signer, account_script_hash);
    assert(result == NEOC_SUCCESS);
    
    // Set allowed groups
    neoc_ec_public_key_t *groups[] = {group_pubkey1, group_pubkey2};
    result = neoc_signer_set_allowed_groups(signer, groups, 2);
    assert(result == NEOC_SUCCESS);
    
    // Verify signer properties
    neoc_witness_scope_t scope;
    result = neoc_signer_get_scope(signer, &scope);
    assert(result == NEOC_SUCCESS);
    assert(scope == NEOC_WITNESS_SCOPE_CUSTOM_GROUPS);
    
    size_t group_count;
    result = neoc_signer_get_allowed_groups_count(signer, &group_count);
    assert(result == NEOC_SUCCESS);
    assert(group_count == 2);
    
    // Verify groups
    neoc_ec_public_key_t **allowed_groups = NULL;
    result = neoc_signer_get_allowed_groups(signer, &allowed_groups, &group_count);
    assert(result == NEOC_SUCCESS);
    assert(group_count == 2);
    
    bool is_equal;
    result = neoc_ec_public_key_equals(allowed_groups[0], group_pubkey1, &is_equal);
    assert(result == NEOC_SUCCESS);
    assert(is_equal);
    
    result = neoc_ec_public_key_equals(allowed_groups[1], group_pubkey2, &is_equal);
    assert(result == NEOC_SUCCESS);
    assert(is_equal);
    
    size_t contract_count;
    result = neoc_signer_get_allowed_contracts_count(signer, &contract_count);
    assert(result == NEOC_SUCCESS);
    assert(contract_count == 0);
    
    neoc_signer_free(signer);
    printf("✓ test_build_valid_signer_with_groups passed\n");
}

static void test_signer_serialization(void) {
    printf("Testing signer serialization...\n");
    
    neoc_signer_t *signer = NULL;
    neoc_error_t result = neoc_signer_create_called_by_entry(&signer, account_script_hash);
    assert(result == NEOC_SUCCESS);
    
    // Serialize signer
    uint8_t buffer[1024];
    size_t written;
    result = neoc_signer_serialize(signer, buffer, sizeof(buffer), &written);
    assert(result == NEOC_SUCCESS);
    assert(written > 0);
    
    // Deserialize signer
    neoc_signer_t *deserialized = NULL;
    result = neoc_signer_deserialize(&deserialized, buffer, written);
    assert(result == NEOC_SUCCESS);
    assert(deserialized != NULL);
    
    // Compare signers
    neoc_hash160_t *original_hash = NULL;
    neoc_hash160_t *deserialized_hash = NULL;
    
    result = neoc_signer_get_account(signer, &original_hash);
    assert(result == NEOC_SUCCESS);
    
    result = neoc_signer_get_account(deserialized, &deserialized_hash);
    assert(result == NEOC_SUCCESS);
    
    assert(neoc_hash160_equal(original_hash, deserialized_hash));
    
    neoc_witness_scope_t original_scope, deserialized_scope;
    result = neoc_signer_get_scope(signer, &original_scope);
    assert(result == NEOC_SUCCESS);
    
    result = neoc_signer_get_scope(deserialized, &deserialized_scope);
    assert(result == NEOC_SUCCESS);
    
    assert(original_scope == deserialized_scope);
    
    neoc_signer_free(signer);
    neoc_signer_free(deserialized);
    printf("✓ test_signer_serialization passed\n");
}

int main(void) {
    printf("\n=== Signer Tests ===\n\n");
    
    // Initialize NeoC library
    neoc_error_t result = neoc_init();
    if (result != NEOC_SUCCESS) {
        printf("Failed to initialize NeoC library: %d\n", result);
        return 1;
    }
    
    // Setup test data
    setup_test_data();
    
    // Run tests
    test_create_signer_with_called_by_entry_witness_scope();
    test_create_signer_with_global_witness_scope();
    test_build_valid_signer_with_contracts();
    test_build_valid_signer_with_groups();
    test_signer_serialization();
    
    // Cleanup test data
    cleanup_test_data();
    
    // Cleanup library
    neoc_cleanup();
    
    printf("\n✅ All Signer tests passed!\n\n");
    return 0;
}
