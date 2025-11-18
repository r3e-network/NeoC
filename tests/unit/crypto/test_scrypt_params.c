/**
 * @file test_scrypt_params.c
 * @brief Unit tests for scrypt parameter helpers
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <neoc/crypto/scrypt_params.h>

static neoc_scrypt_params_t *params = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_scrypt_params_create(16384, 8, 1, 64, &params));
    TEST_ASSERT_NOT_NULL(params);
}

void tearDown(void) {
    neoc_scrypt_params_free(params);
    params = NULL;
    neoc_cleanup();
}

static void test_scrypt_params_create(void) {
    TEST_ASSERT_EQUAL_UINT32(16384, params->n);
    TEST_ASSERT_EQUAL_UINT32(8, params->r);
    TEST_ASSERT_EQUAL_UINT32(1, params->p);
    TEST_ASSERT_EQUAL_UINT32(64, params->dk_len);
    TEST_ASSERT_TRUE(neoc_scrypt_params_is_valid(params));
}

static void test_scrypt_params_invalid_values(void) {
    neoc_scrypt_params_t *tmp = NULL;
    TEST_ASSERT_TRUE(neoc_scrypt_params_create(0, 8, 1, 64, &tmp) != NEOC_SUCCESS);
    TEST_ASSERT_TRUE(neoc_scrypt_params_create(3, 8, 1, 64, &tmp) != NEOC_SUCCESS);
    TEST_ASSERT_TRUE(neoc_scrypt_params_create(16, 0, 1, 64, &tmp) != NEOC_SUCCESS);
    TEST_ASSERT_TRUE(neoc_scrypt_params_create(16, 8, 0, 64, &tmp) != NEOC_SUCCESS);
    TEST_ASSERT_TRUE(neoc_scrypt_params_create(16, 8, 1, 0, &tmp) != NEOC_SUCCESS);
}

static void test_scrypt_params_constants(void) {
    TEST_ASSERT_TRUE(NEOC_SCRYPT_PARAMS_DEFAULT.n > 0);
    TEST_ASSERT_TRUE(NEOC_SCRYPT_PARAMS_LIGHT.dk_len > 0);
    TEST_ASSERT_TRUE(NEOC_SCRYPT_PARAMS_STANDARD.p > 0);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_scrypt_params_create);
    RUN_TEST(test_scrypt_params_invalid_values);
    RUN_TEST(test_scrypt_params_constants);
    return UnityEnd();
}
