#include "unity.h"
#include <string.h>
#include <stdint.h>
#include "neoc/neoc.h"
#include "neoc/contract/contract_management.h"
#include "neoc/neoc_memory.h"

static void fill_hash(neoc_hash160_t *hash, uint8_t seed) {
    for (size_t i = 0; i < NEOC_HASH160_SIZE; ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

static void init_sample_nef(neoc_contract_nef_t *nef) {
    static const uint8_t script_bytes[] = {0x01, 0x51, 0xC3, 0xAA};
    memset(nef, 0, sizeof(*nef));
    nef->magic = 0x3346454E; /* "NEF3" */
    nef->compiler = neoc_strdup("NeoC Unit Compiler");
    nef->source = neoc_strdup("contract.cs");
    nef->script_length = sizeof(script_bytes);
    nef->script = neoc_malloc(nef->script_length);
    memcpy(nef->script, script_bytes, nef->script_length);
    nef->checksum = 0xDEADBEEF;
}

static void init_sample_manifest(neoc_contract_manifest_t *manifest, const char *name) {
    memset(manifest, 0, sizeof(*manifest));
    manifest->name = neoc_strdup(name);
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_contract_management_create_and_free(void) {
    neoc_contract_management_t *mgmt = NULL;

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT, neoc_contract_management_create(NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_contract_management_create(&mgmt));
    TEST_ASSERT_NOT_NULL(mgmt);

    neoc_contract_management_free(mgmt);
}

void test_contract_management_deploy_returns_contract_state(void) {
    neoc_contract_management_t *mgmt = NULL;
    neoc_contract_state_t *contract = NULL;
    neoc_contract_nef_t nef;
    neoc_contract_manifest_t manifest;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_contract_management_create(&mgmt));
    init_sample_nef(&nef);
    init_sample_manifest(&manifest, "UnitContract");

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_management_deploy(mgmt, &nef, &manifest, &contract));
    TEST_ASSERT_NOT_NULL(contract);
    TEST_ASSERT_EQUAL_INT32(0, contract->update_counter);
    TEST_ASSERT_EQUAL_UINT(nef.script_length, contract->nef.script_length);
    TEST_ASSERT_EQUAL_STRING("UnitContract", contract->manifest.name);

    neoc_contract_state_free(contract);
    neoc_contract_management_free(mgmt);
}

void test_contract_management_update_and_destroy(void) {
    neoc_contract_management_t *mgmt = NULL;
    neoc_hash160_t target_hash = {{0}};

    fill_hash(&target_hash, 0x22);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_contract_management_create(&mgmt));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_management_update(mgmt, &target_hash, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_management_destroy(mgmt, &target_hash));

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_update(NULL, &target_hash, NULL, NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_destroy(NULL, &target_hash));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_destroy(mgmt, NULL));

    neoc_contract_management_free(mgmt);
}

void test_contract_management_get_contract_and_exists(void) {
    neoc_contract_management_t *mgmt = NULL;
    neoc_contract_state_t *contract = NULL;
    neoc_hash160_t hash = {{0}};
    bool exists = false;

    fill_hash(&hash, 0x44);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_contract_management_create(&mgmt));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_management_get_contract(mgmt, &hash, &contract));
    TEST_ASSERT_NOT_NULL(contract);
    TEST_ASSERT_EQUAL_MEMORY(hash.data, contract->hash.data, sizeof(hash.data));

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_contract_management_has_contract(mgmt, &hash, &exists));
    TEST_ASSERT_TRUE(exists);

    neoc_contract_state_free(contract);
    neoc_contract_management_free(mgmt);
}

void test_contract_management_invalid_arguments(void) {
    neoc_contract_management_t *mgmt = NULL;
    neoc_contract_state_t *contract = NULL;
    neoc_contract_nef_t nef;
    neoc_contract_manifest_t manifest;
    neoc_hash160_t hash = {{0}};
    bool exists = false;

    init_sample_nef(&nef);
    init_sample_manifest(&manifest, "InvalidArgs");

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_deploy(NULL, &nef, &manifest, &contract));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_contract_management_create(&mgmt));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_deploy(mgmt, NULL, &manifest, &contract));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_deploy(mgmt, &nef, NULL, &contract));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_deploy(mgmt, &nef, &manifest, NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_get_contract(NULL, &hash, &contract));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_get_contract(mgmt, NULL, &contract));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_get_contract(mgmt, &hash, NULL));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_has_contract(NULL, &hash, &exists));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_has_contract(mgmt, NULL, &exists));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_INVALID_ARGUMENT,
                          neoc_contract_management_has_contract(mgmt, &hash, NULL));

    /* Clean up allocations owned by the temporary NEF/manifest */
    neoc_free(nef.compiler);
    neoc_free(nef.source);
    neoc_free(nef.script);
    neoc_free(manifest.name);
    neoc_contract_management_free(mgmt);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_contract_management_create_and_free);
    RUN_TEST(test_contract_management_deploy_returns_contract_state);
    RUN_TEST(test_contract_management_update_and_destroy);
    RUN_TEST(test_contract_management_get_contract_and_exists);
    RUN_TEST(test_contract_management_invalid_arguments);
    return UnityEnd();
}
