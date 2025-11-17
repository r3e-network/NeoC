#include "unity.h"
#include "neoc/neoc.h"
#include "neoc/contract/role_management.h"
#include "neoc/crypto/ecpoint.h"

static const char *ROLE_PUBKEY_ONE = "033a4d051b04b7fc0230d2b1aaedfd5a84be279a5361a7358db665ad7857787f1b";
static const char *ROLE_PUBKEY_TWO = "0265bf906bf385fbf3f777832e55a87991bcfbe19b097fb7c5ca2e4025a4d5e5d6";

static neoc_role_management_t *role_mgmt = NULL;

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_role_management_create(&role_mgmt));
    TEST_ASSERT_NOT_NULL(role_mgmt);
}

void tearDown(void) {
    neoc_role_management_free(role_mgmt);
    role_mgmt = NULL;
    neoc_cleanup();
}

static neoc_ec_point_t *create_point_from_hex(const char *hex) {
    neoc_ec_point_t *point = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_ec_point_from_hex(hex, &point));
    TEST_ASSERT_NOT_NULL(point);
    return point;
}

void test_role_management_creation(void) {
    TEST_ASSERT_NOT_NULL(role_mgmt);
}

void test_get_designated_by_role_returns_empty_without_rpc(void) {
    neoc_ec_point_t **nodes = (neoc_ec_point_t **)0x1; // sentinel to ensure function resets it
    size_t count = 42;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_role_management_get_designated_by_role(role_mgmt,
                                                                      NEOC_ROLE_STATE_VALIDATOR,
                                                                      0,
                                                                      &nodes,
                                                                      &count));
    TEST_ASSERT_EQUAL_PTR(NULL, nodes);
    TEST_ASSERT_EQUAL_UINT(0, count);
}

void test_designate_as_role_accepts_valid_points(void) {
    neoc_ec_point_t *points[2] = {
        create_point_from_hex(ROLE_PUBKEY_ONE),
        create_point_from_hex(ROLE_PUBKEY_TWO)
    };

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_role_management_designate_as_role(role_mgmt,
                                                                 NEOC_ROLE_ORACLE,
                                                                 points,
                                                                 2));

    neoc_ec_point_free(points[0]);
    neoc_ec_point_free(points[1]);
}

void test_role_management_argument_validation(void) {
    neoc_ec_point_t *points[1] = {create_point_from_hex(ROLE_PUBKEY_ONE)};
    size_t count = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_role_management_get_designated_by_role(NULL,
                                                                      NEOC_ROLE_STATE_VALIDATOR,
                                                                      0,
                                                                      NULL,
                                                                      &count));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_role_management_designate_as_role(NULL,
                                                                 NEOC_ROLE_NEOFS_ALPHABET_NODE,
                                                                 points,
                                                                 1));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_role_management_designate_as_role(role_mgmt,
                                                                 NEOC_ROLE_NEOFS_ALPHABET_NODE,
                                                                 NULL,
                                                                 1));

    neoc_ec_point_free(points[0]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_role_management_creation);
    RUN_TEST(test_get_designated_by_role_returns_empty_without_rpc);
    RUN_TEST(test_designate_as_role_accepts_valid_points);
    RUN_TEST(test_role_management_argument_validation);
    UNITY_END();
}
