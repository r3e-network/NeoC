#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/transaction/transaction.h"

static neoc_transaction_t *transaction = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_create(&transaction));
}

void tearDown(void) {
    neoc_transaction_free(transaction);
    transaction = NULL;
    neoc_cleanup();
}

void test_transaction_serialization_and_hash(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_version(transaction, 0x01));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_nonce(transaction, 42));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_system_fee(transaction, 10));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_network_fee(transaction, 1));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_valid_until_block(transaction, 1000));

    const uint8_t script[] = {0x01, 0x02, 0x03};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_transaction_set_script(transaction, script, sizeof(script)));

    uint8_t buffer[512];
    size_t serialized = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_transaction_serialize(transaction, buffer, sizeof(buffer), &serialized));
    TEST_ASSERT_TRUE(serialized > 0);

    neoc_hash256_t hash_direct;
    neoc_hash256_t hash_cached;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_transaction_calculate_hash(transaction, &hash_direct));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_transaction_get_hash(transaction, &hash_cached));
    TEST_ASSERT_EQUAL_MEMORY(hash_direct.data, hash_cached.data, NEOC_HASH256_SIZE);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_transaction_serialization_and_hash);
    UNITY_END();
    return 0;
}
