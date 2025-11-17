#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/contract/policy_contract.h"
#include "neoc/types/neoc_hash160.h"

static neoc_policy_contract_t *policy = NULL;

static void fill_hash160(neoc_hash160_t *hash, uint8_t seed) {
    for (size_t i = 0; i < sizeof(hash->data); ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_policy_contract_create(&policy));
    TEST_ASSERT_NOT_NULL(policy);
}

void tearDown(void) {
    neoc_policy_contract_free(policy);
    policy = NULL;
    neoc_cleanup();
}

void test_policy_contract_defaults(void) {
    uint64_t fee = 0;
    uint32_t factor = 0;
    uint32_t price = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_policy_get_fee_per_byte(policy, &fee));
    TEST_ASSERT_EQUAL_UINT64(1000ULL, fee);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_policy_get_exec_fee_factor(policy, &factor));
    TEST_ASSERT_EQUAL_UINT32(30U, factor);

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_policy_get_storage_price(policy, &price));
    TEST_ASSERT_EQUAL_UINT32(100000U, price);
}

void test_policy_contract_set_fee_per_byte(void) {
    uint64_t fee = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_policy_set_fee_per_byte(policy, 2048ULL));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_policy_get_fee_per_byte(policy, &fee));
    TEST_ASSERT_EQUAL_UINT64(2048ULL, fee);
}

void test_policy_contract_is_blocked_defaults_to_false(void) {
    neoc_hash160_t account;
    bool blocked = true;

    fill_hash160(&account, 0x42);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_policy_is_blocked(policy, &account, &blocked));
    TEST_ASSERT_FALSE(blocked);
}

void test_policy_contract_invalid_arguments(void) {
    uint64_t fee = 0;
    uint32_t factor = 0;
    uint32_t price = 0;
    bool blocked = false;
    neoc_hash160_t account;

    fill_hash160(&account, 0x10);

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, neoc_policy_contract_create(NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_get_fee_per_byte(NULL, &fee));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_get_fee_per_byte(policy, NULL));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_get_exec_fee_factor(NULL, &factor));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_get_exec_fee_factor(policy, NULL));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_get_storage_price(NULL, &price));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_get_storage_price(policy, NULL));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_is_blocked(NULL, &account, &blocked));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_is_blocked(policy, NULL, &blocked));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_is_blocked(policy, &account, NULL));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_policy_set_fee_per_byte(NULL, 10));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_policy_contract_defaults);
    RUN_TEST(test_policy_contract_set_fee_per_byte);
    RUN_TEST(test_policy_contract_is_blocked_defaults_to_false);
    RUN_TEST(test_policy_contract_invalid_arguments);
    return UNITY_END();
}
