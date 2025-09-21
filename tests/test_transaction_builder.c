#include <unity.h>
#include <neoc/transaction/transaction_builder.h>
#include <neoc/wallet/account.h>
#include <neoc/types/neoc_hash160.h>
#include <neoc/types/neoc_hash256.h>
#include <string.h>

void setUp(void) {
    // Setup code
}

void tearDown(void) {
    // Cleanup code
}

void test_transaction_builder_create(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(builder);
    
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_set_properties(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set version
    err = neoc_tx_builder_set_version(builder, 0);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set nonce
    err = neoc_tx_builder_set_nonce(builder, 123456);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set valid until block
    err = neoc_tx_builder_set_valid_until_block(builder, 1000000);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add network fee
    err = neoc_tx_builder_add_network_fee(builder, 100000);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add system fee
    err = neoc_tx_builder_add_system_fee(builder, 200000);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_add_signer(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create a signer
    neoc_hash160_t account_hash;
    neoc_hash160_init_zero(&account_hash);
    memset(account_hash.data, 0xAA, 20);  // Test pattern
    
    neoc_signer_t *signer = NULL;
    err = neoc_signer_create_global(&account_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add signer to builder
    err = neoc_tx_builder_add_signer(builder, signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_signer_free(signer);
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_set_script(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create a simple script
    uint8_t script[] = {0x00, 0x01, 0x02, 0x03, 0x04};
    err = neoc_tx_builder_set_script(builder, script, sizeof(script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_build_unsigned(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set basic properties
    err = neoc_tx_builder_set_version(builder, 0);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    err = neoc_tx_builder_set_valid_until_block(builder, 1000000);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add a signer
    neoc_hash160_t account_hash;
    neoc_hash160_init_zero(&account_hash);
    neoc_signer_t *signer = NULL;
    err = neoc_signer_create_global(&account_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    err = neoc_tx_builder_add_signer(builder, signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    neoc_signer_free(signer);
    
    // Set script
    uint8_t script[] = {0x00, 0x01, 0x02};
    err = neoc_tx_builder_set_script(builder, script, sizeof(script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Build unsigned transaction
    neoc_transaction_t *transaction = NULL;
    err = neoc_tx_builder_build_unsigned(builder, &transaction);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(transaction);
    
    // Check transaction properties
    TEST_ASSERT_EQUAL_INT(0, transaction->version);
    TEST_ASSERT_EQUAL_INT(1000000, transaction->valid_until_block);
    TEST_ASSERT_EQUAL_INT(1, transaction->signer_count);
    TEST_ASSERT_EQUAL_INT(sizeof(script), transaction->script_len);
    
    neoc_transaction_free(transaction);
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_with_account(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create an account
    neoc_account_t *account = NULL;
    err = neoc_account_create("test", &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add signer from account
    err = neoc_tx_builder_add_signer_from_account(builder, account, 
                                                   NEOC_WITNESS_SCOPE_CALLED_BY_ENTRY);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set first signer
    err = neoc_tx_builder_set_first_signer(builder, account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_account_free(account);
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_high_priority(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set high priority
    err = neoc_tx_builder_set_high_priority(builder, true);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_build_and_sign(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Create account for signing
    neoc_account_t *account = NULL;
    err = neoc_account_create("test", &account);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Add signer from account
    err = neoc_tx_builder_add_signer_from_account(builder, account, 
                                                   NEOC_WITNESS_SCOPE_GLOBAL);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Set script
    uint8_t script[] = {0x00, 0x01, 0x02};
    err = neoc_tx_builder_set_script(builder, script, sizeof(script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Build and sign
    neoc_account_t *accounts[] = {account};
    neoc_transaction_t *transaction = NULL;
    err = neoc_tx_builder_build_and_sign(builder, accounts, 1, &transaction);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    TEST_ASSERT_NOT_NULL(transaction);
    
    // Check that transaction has witnesses
    TEST_ASSERT_EQUAL_INT(1, transaction->witness_count);
    
    neoc_transaction_free(transaction);
    neoc_account_free(account);
    neoc_tx_builder_free(builder);
}

void test_transaction_builder_get_hash(void) {
    neoc_tx_builder_t *builder = NULL;
    neoc_error_t err = neoc_tx_builder_create(&builder);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Build a simple transaction
    neoc_hash160_t account_hash;
    neoc_hash160_init_zero(&account_hash);
    neoc_signer_t *signer = NULL;
    err = neoc_signer_create_global(&account_hash, &signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    err = neoc_tx_builder_add_signer(builder, signer);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    neoc_signer_free(signer);
    
    uint8_t script[] = {0x00};
    err = neoc_tx_builder_set_script(builder, script, sizeof(script));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Get hash
    neoc_hash256_t hash;
    err = neoc_tx_builder_get_hash(builder, &hash);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, err);
    
    // Verify hash is not zero
    bool is_zero = true;
    for (int i = 0; i < 32; i++) {
        if (hash.data[i] != 0) {
            is_zero = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(is_zero);
    
    neoc_tx_builder_free(builder);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_transaction_builder_create);
    RUN_TEST(test_transaction_builder_set_properties);
    RUN_TEST(test_transaction_builder_add_signer);
    RUN_TEST(test_transaction_builder_set_script);
    RUN_TEST(test_transaction_builder_build_unsigned);
    RUN_TEST(test_transaction_builder_with_account);
    RUN_TEST(test_transaction_builder_high_priority);
    RUN_TEST(test_transaction_builder_build_and_sign);
    RUN_TEST(test_transaction_builder_get_hash);
    
    return UnityEnd();
}

