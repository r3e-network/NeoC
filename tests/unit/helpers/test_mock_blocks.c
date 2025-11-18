#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/protocol/core/response/neo_block.h"
#include "neoc/transaction/transaction.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"

static neoc_neo_block_t *block = NULL;

static void fill_hash256(neoc_hash256_t *hash, uint8_t seed) {
    for (size_t i = 0; i < sizeof(hash->data); ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

static void fill_hash160(neoc_hash160_t *hash, uint8_t seed) {
    for (size_t i = 0; i < sizeof(hash->data); ++i) {
        hash->data[i] = (uint8_t)(seed + i);
    }
}

static neoc_transaction_t *make_transaction(uint32_t nonce, uint8_t script_tag) {
    neoc_transaction_t *tx = NULL;
    uint8_t script_data[4] = {script_tag, (uint8_t)(script_tag + 1),
                              (uint8_t)(script_tag + 2), (uint8_t)(script_tag + 3)};

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_create(&tx));
    TEST_ASSERT_NOT_NULL(tx);
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_nonce(tx, nonce));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_script(tx, script_data, sizeof(script_data)));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_set_valid_until_block(tx, 1000));
    return tx;
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
    block = neoc_neo_block_create();
    TEST_ASSERT_NOT_NULL(block);
}

void tearDown(void) {
    if (block) {
        neoc_neo_block_free(block);
        block = NULL;
    }
    neoc_cleanup();
}

void test_mock_block_creation_and_setters(void) {
    neoc_hash256_t prev_hash = {{0}};
    neoc_hash256_t merkle_root = {{0}};
    neoc_hash160_t next_consensus = {{0}};

    fill_hash256(&prev_hash, 0x10);
    fill_hash256(&merkle_root, 0x20);
    fill_hash160(&next_consensus, 0x30);

    neoc_neo_block_set_version(block, 3);
    neoc_neo_block_set_prev_hash(block, &prev_hash);
    neoc_neo_block_set_merkle_root(block, &merkle_root);
    neoc_neo_block_set_timestamp(block, 987654321ULL);
    block->header.nonce = 777;
    neoc_neo_block_set_index(block, 42);
    neoc_neo_block_set_primary_index(block, 7);
    neoc_neo_block_set_next_consensus(block, &next_consensus);

    TEST_ASSERT_EQUAL_UINT32(3, block->header.version);
    TEST_ASSERT_EQUAL_MEMORY(prev_hash.data, block->header.prev_hash.data, sizeof(prev_hash.data));
    TEST_ASSERT_EQUAL_MEMORY(merkle_root.data, block->header.merkle_root.data, sizeof(merkle_root.data));
    TEST_ASSERT_EQUAL_UINT64(987654321ULL, block->header.timestamp);
    TEST_ASSERT_EQUAL_UINT64(777ULL, block->header.nonce);
    TEST_ASSERT_EQUAL_UINT32(42, block->header.index);
    TEST_ASSERT_EQUAL_UINT8(7, block->header.primary_index);
    TEST_ASSERT_EQUAL_MEMORY(next_consensus.data, block->header.next_consensus.data, sizeof(next_consensus.data));
    TEST_ASSERT_EQUAL_UINT32(0, block->transaction_count);
}

void test_mock_block_transactions_and_verification(void) {
    neoc_transaction_t *tx1 = make_transaction(1, 0xAA);
    neoc_transaction_t *tx2 = make_transaction(2, 0xBB);

    neoc_neo_block_add_transaction(block, tx1);
    neoc_neo_block_add_transaction(block, tx2);
    TEST_ASSERT_EQUAL_UINT32(2, block->transaction_count);

    neoc_hash256_t merkle = neoc_neo_block_calculate_merkle_root(block);
    TEST_ASSERT_FALSE(neoc_hash256_is_zero(&merkle));
    block->header.merkle_root = merkle;

    block->header.timestamp = 123456789ULL;
    block->header.nonce = 5555;
    block->header.index = 99;
    block->header.primary_index = 2;
    fill_hash256(&block->header.prev_hash, 0x41);
    fill_hash160(&block->header.next_consensus, 0x51);

    neoc_hash256_t block_hash = neoc_neo_block_calculate_hash(block);
    block->hash = block_hash;

    TEST_ASSERT_TRUE(neoc_neo_block_verify(block));

    memset(block->header.merkle_root.data, 0, sizeof(block->header.merkle_root.data));
    TEST_ASSERT_FALSE(neoc_neo_block_verify(block));
    block->header.merkle_root = merkle;
    block->hash = block_hash;
}

void test_mock_block_clone_deep_copy(void) {
    neoc_transaction_t *tx = make_transaction(10, 0x11);
    neoc_neo_block_add_transaction(block, tx);
    block->header.index = 5;
    block->header.timestamp = 222;

    neoc_neo_block_t *clone = neoc_neo_block_clone(block);
    TEST_ASSERT_NOT_NULL(clone);
    TEST_ASSERT_EQUAL_UINT32(block->transaction_count, clone->transaction_count);
    TEST_ASSERT_TRUE(block->transactions != clone->transactions);
    TEST_ASSERT_NOT_NULL(clone->transactions[0]);

    TEST_ASSERT_EQUAL_UINT(block->transactions[0]->script_len,
                           clone->transactions[0]->script_len);
    TEST_ASSERT_EQUAL_MEMORY(block->transactions[0]->script,
                             clone->transactions[0]->script,
                             block->transactions[0]->script_len);

    block->transactions[0]->script[0] ^= 0xFF;
    TEST_ASSERT_TRUE(block->transactions[0]->script[0] !=
                     clone->transactions[0]->script[0]);

    neoc_neo_block_free(clone);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mock_block_creation_and_setters);
    RUN_TEST(test_mock_block_transactions_and_verification);
    RUN_TEST(test_mock_block_clone_deep_copy);
    return UnityEnd();
}
