#include "unity.h"

#include <neoc/neoc_memory.h>
#include <neoc/protocol/core/record_type.h>
#include <neoc/protocol/core/response/neo_get_token_transfers.h>
#include <neoc/protocol/core/response/neo_get_unspents.h>
#include <neoc/protocol/core/response/neo_list_plugins.h>
#include <neoc/protocol/core/response/neo_network_fee.h>
#include <neoc/protocol/core/response/neo_send_raw_transaction.h>
#include <neoc/protocol/core/response/neo_witness.h>
#include <neoc/protocol/core/response/nep17_contract.h>
#include <neoc/protocol/core/response/oracle_request.h>
#include <neoc/protocol/core/response/populated_blocks.h>
#include <neoc/protocol/core/response/record_state.h>
#include <neoc/protocol/core/response/transaction_send_token.h>
#include <neoc/protocol/core/response/transaction_signer.h>
#include <neoc/types/neoc_hash160.h>
#include <neoc/types/neoc_hash256.h>
#include <neoc/transaction/witness_scope.h>

#include <stdio.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static neoc_hash160_t make_hash160(const char *hex) {
    neoc_hash160_t hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash160_from_hex(&hash, hex));
    return hash;
}

static neoc_hash256_t make_hash256(const char *hex) {
    neoc_hash256_t hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_hash256_from_hex(&hash, hex));
    return hash;
}

void test_transaction_send_token_roundtrip(void) {
    const char *address = "NSampleNeoAddress000";
    neoc_hash160_t token_hash = make_hash160("0x11223344556677889900aabbccddeeff00112233");

    neoc_transaction_send_token_t *token = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_transaction_send_token_create_with_values(&token_hash, 42, address, &token));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_send_token_to_json(token, &json));
    TEST_ASSERT_NOT_NULL(json);

    neoc_transaction_send_token_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_send_token_from_json(json, &parsed));
    TEST_ASSERT_NOT_NULL(parsed);
    TEST_ASSERT_EQUAL_INT(42, parsed->value);
    TEST_ASSERT_TRUE(neoc_hash160_equal(token->token, parsed->token));
    TEST_ASSERT_EQUAL_STRING(address, parsed->address);

    neoc_transaction_send_token_free(parsed);
    neoc_transaction_send_token_free(token);
    neoc_free(json);
}

void test_response_alias_helpers(void) {
    int value = 7;
    neoc_neo_block_count_t *int_resp = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_create_int_response(1, &value, NULL, 0, &int_resp));
    TEST_ASSERT_NOT_NULL(int_resp);
    TEST_ASSERT_NOT_NULL(int_resp->result);
    TEST_ASSERT_EQUAL_INT(7, *int_resp->result);
    TEST_ASSERT_NULL(int_resp->error);
    neoc_neo_response_free(int_resp);

    bool bval = false;
    neoc_neo_boolean_response_t *bool_resp = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_create_bool_response(2, &bval, "err", -1, &bool_resp));
    TEST_ASSERT_NOT_NULL(bool_resp);
    TEST_ASSERT_NOT_NULL(bool_resp->error);
    TEST_ASSERT_EQUAL_STRING("err", bool_resp->error);
    neoc_neo_response_free(bool_resp);

    neoc_neo_string_response_t *str_resp = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_create_string_response(3, "ok", NULL, 0, &str_resp));
    TEST_ASSERT_NOT_NULL(str_resp);
    TEST_ASSERT_EQUAL_STRING("ok", str_resp->result);
    neoc_neo_response_free(str_resp);
}

void test_transaction_signer_roundtrip(void) {
    neoc_hash160_t account = make_hash160("0xa1b2c3d4e5f60123456789abcdef0123456789ab");
    neoc_witness_scope_t scopes[] = {NEOC_WITNESS_SCOPE_GLOBAL};
    const char *allowed_contracts[] = {"0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
    const char *allowed_groups[] = {"0234567890abcdef0234567890abcdef0234567890abcdef0234567890abcd"};

    neoc_transaction_signer_t *signer = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_transaction_signer_create_full(&account,
                                                             scopes,
                                                             1,
                                                             allowed_contracts,
                                                             1,
                                                             allowed_groups,
                                                             1,
                                                             NULL,
                                                             0,
                                                             &signer));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_signer_to_json(signer, &json));
    TEST_ASSERT_NOT_NULL(json);

    neoc_transaction_signer_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_transaction_signer_from_json(json, &parsed));
    TEST_ASSERT_NOT_NULL(parsed);
    TEST_ASSERT_TRUE(neoc_hash160_equal(signer->account, parsed->account));
    TEST_ASSERT_TRUE(neoc_transaction_signer_has_scope(parsed, NEOC_WITNESS_SCOPE_GLOBAL));
    TEST_ASSERT_TRUE(neoc_transaction_signer_is_contract_allowed(parsed, allowed_contracts[0]));
    TEST_ASSERT_TRUE(neoc_transaction_signer_is_group_allowed(parsed, allowed_groups[0]));

    neoc_transaction_signer_free(parsed);
    neoc_transaction_signer_free(signer);
    neoc_free(json);
}

void test_oracle_request_response_roundtrip(void) {
    neoc_hash256_t tx_hash = make_hash256("0x11223344556677889900aabbccddeeff0011223344556677889900aabbccddeeff");
    neoc_hash160_t callback_contract = make_hash160("0xbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");

    neoc_oracle_request_t *request = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_oracle_request_create(7,
                                                     &tx_hash,
                                                     10,
                                                     "https://example.com/data",
                                                     "$.price",
                                                     &callback_contract,
                                                     "onOracle",
                                                     "user-data",
                                                     &request));

    char *result_json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_oracle_request_to_json(request, &result_json));

    size_t buffer_len = strlen(result_json) + 64;
    char *response_json = neoc_malloc(buffer_len);
    TEST_ASSERT_NOT_NULL(response_json);
    snprintf(response_json, buffer_len, "{\"jsonrpc\":\"2.0\",\"id\":101,\"result\":%s}", result_json);

    neoc_oracle_request_response_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_oracle_request_response_from_json(response_json, &parsed));
    TEST_ASSERT_NOT_NULL(parsed);
    TEST_ASSERT_NOT_NULL(parsed->result);
    TEST_ASSERT_EQUAL_INT(7, parsed->result->request_id);
    TEST_ASSERT_EQUAL_INT(10, parsed->result->gas_for_response);
    TEST_ASSERT_EQUAL_STRING("https://example.com/data", parsed->result->url);
    TEST_ASSERT_EQUAL_STRING("onOracle", parsed->result->callback_method);

    neoc_oracle_request_response_free(parsed);
    neoc_oracle_request_free(request);
    neoc_free(result_json);
    neoc_free(response_json);
}

void test_populated_blocks_roundtrip(void) {
    int blocks[] = {12, 14, 16};
    neoc_populated_blocks_t *populated = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_populated_blocks_create("cache-42", blocks, 3, &populated));

    char *result_json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_populated_blocks_to_json(populated, &result_json));

    size_t buffer_len = strlen(result_json) + 64;
    char *response_json = neoc_malloc(buffer_len);
    TEST_ASSERT_NOT_NULL(response_json);
    snprintf(response_json, buffer_len, "{\"jsonrpc\":\"2.0\",\"id\":3,\"result\":%s}", result_json);

    neoc_populated_blocks_response_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_populated_blocks_response_from_json(response_json, &parsed));
    TEST_ASSERT_NOT_NULL(parsed);
    TEST_ASSERT_NOT_NULL(parsed->result);
    TEST_ASSERT_TRUE(neoc_populated_blocks_contains_block(parsed->result, 12));
    TEST_ASSERT_TRUE(neoc_populated_blocks_contains_block(parsed->result, 16));

    neoc_populated_blocks_response_free(parsed);
    neoc_populated_blocks_free(populated);
    neoc_free(result_json);
    neoc_free(response_json);
}

void test_record_state_roundtrip(void) {
    neoc_record_state_t *record = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_record_state_create("neo.com", NEOC_RECORD_TYPE_A, "1.1.1.1", &record));

    char *record_json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_record_state_to_json(record, &record_json));

    neoc_record_state_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_record_state_from_json(record_json, &parsed));
    TEST_ASSERT_TRUE(neoc_record_state_equals(record, parsed));

    size_t buffer_len = strlen(record_json) + 64;
    char *response_json = neoc_malloc(buffer_len);
    TEST_ASSERT_NOT_NULL(response_json);
    snprintf(response_json, buffer_len, "{\"jsonrpc\":\"2.0\",\"id\":9,\"result\":%s}", record_json);

    neoc_record_state_response_t *resp = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_record_state_response_from_json(response_json, &resp));
    TEST_ASSERT_NOT_NULL(resp);
    TEST_ASSERT_NOT_NULL(resp->result);
    TEST_ASSERT_EQUAL_STRING("neo.com", resp->result->name);

    neoc_record_state_response_free(resp);
    neoc_record_state_free(parsed);
    neoc_record_state_free(record);
    neoc_free(record_json);
    neoc_free(response_json);
}

void test_neo_list_plugins_roundtrip(void) {
    const char *interfaces[] = {"IWalletPlugin", "IRpcPlugin"};
    neoc_neo_plugin_t *plugin = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_plugin_create("TestPlugin", "1.0.0", interfaces, 2, &plugin));

    neoc_neo_list_plugins_t *response = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_list_plugins_create(7, plugin, 1, NULL, 0, &response));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_list_plugins_to_json(response, &json));

    neoc_neo_list_plugins_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_list_plugins_from_json(json, &parsed));
    TEST_ASSERT_TRUE(neoc_neo_list_plugins_has_plugins(parsed));
    TEST_ASSERT_EQUAL_UINT(1, neoc_neo_list_plugins_get_count(parsed));
    neoc_neo_plugin_t *first = neoc_neo_list_plugins_get_plugin(parsed, 0);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_STRING("TestPlugin", neoc_neo_plugin_get_name(first));
    TEST_ASSERT_TRUE(neoc_neo_plugin_supports_interface(first, "IRpcPlugin"));

    neoc_neo_list_plugins_free(parsed);
    neoc_neo_list_plugins_free(response);
    neoc_free(json);
}

void test_neo_witness_roundtrip(void) {
    neoc_neo_witness_t *witness = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_witness_create("aGVsbG8=", "d29ybGQ=", &witness));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_witness_to_json(witness, &json));

    neoc_neo_witness_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_witness_from_json(json, &parsed));
    TEST_ASSERT_TRUE(neoc_neo_witness_equals(witness, parsed));

    neoc_neo_witness_free(parsed);
    neoc_neo_witness_free(witness);
    neoc_free(json);
}

void test_nep17_contract_roundtrip(void) {
    neoc_hash160_t script_hash = make_hash160("0xabcdefabcdefabcdefabcdefabcdefabcdefabcd");
    neoc_nep17_contract_t *contract = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_nep17_contract_create(&script_hash, "GAS", 8, &contract));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nep17_contract_to_json(contract, &json));

    neoc_nep17_contract_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_nep17_contract_from_json(json, &parsed));
    TEST_ASSERT_TRUE(neoc_nep17_contract_equals(contract, parsed));

    neoc_nep17_contract_free(parsed);
    neoc_nep17_contract_free(contract);
    neoc_free(json);
}

void test_network_fee_roundtrip(void) {
    neoc_neo_network_fee_t *fee = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_network_fee_create(12345, &fee));

    neoc_neo_calculate_network_fee_t *response = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_calculate_network_fee_create(5, fee, NULL, 0, &response));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_calculate_network_fee_to_json(response, &json));

    neoc_neo_calculate_network_fee_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_calculate_network_fee_from_json(json, &parsed));
    int64_t parsed_fee = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_calculate_network_fee_get_fee_value(parsed, &parsed_fee));
    TEST_ASSERT_EQUAL_INT64(12345, parsed_fee);

    neoc_neo_calculate_network_fee_free(parsed);
    neoc_neo_calculate_network_fee_free(response);
    neoc_free(json);
}

void test_send_raw_transaction_roundtrip(void) {
    neoc_hash256_t tx_hash = make_hash256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    neoc_raw_transaction_result_t *result = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_raw_transaction_result_create(&tx_hash, &result));

    neoc_neo_send_raw_transaction_t *response = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_send_raw_transaction_create(17, result, NULL, 0, &response));

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_send_raw_transaction_to_json(response, &json));

    neoc_neo_send_raw_transaction_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_send_raw_transaction_from_json(json, &parsed));
    neoc_hash256_t parsed_hash;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_neo_send_raw_transaction_get_hash(parsed, &parsed_hash));
    TEST_ASSERT_TRUE(neoc_hash256_equal(&tx_hash, &parsed_hash));

    neoc_neo_send_raw_transaction_free(parsed);
    neoc_neo_send_raw_transaction_free(response);
    neoc_free(json);
}

void test_token_transfers_roundtrip(void) {
    neoc_hash160_t asset_hash = make_hash160("0x0101010101010101010101010101010101010101");
    neoc_hash256_t tx_hash = make_hash256("0x0202020202020202020202020202020202020202020202020202020202020202");

    neoc_token_transfer_t *sent = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_transfer_create_full(1700000000,
                                                          &asset_hash,
                                                          "NsentAddress",
                                                          "10",
                                                          12,
                                                          1,
                                                          &tx_hash,
                                                          &sent));

    neoc_token_transfer_t *received = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_token_transfer_create_full(1700000001,
                                                          &asset_hash,
                                                          "NrecvAddress",
                                                          "5",
                                                          13,
                                                          2,
                                                          &tx_hash,
                                                          &received));

    neoc_token_transfers_t *transfers = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_token_transfers_create(&transfers));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_token_transfers_set_address(transfers, "NownerAddress"));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_token_transfers_add_sent(transfers, sent));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_token_transfers_add_received(transfers, received));

    neoc_get_token_transfers_response_t *response = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_get_token_transfers_response_create(&response));
    response->id = 33;
    response->result = transfers;

    char *json = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_get_token_transfers_response_to_json(response, &json));

    neoc_get_token_transfers_response_t *parsed = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_get_token_transfers_response_from_json(json, &parsed));
    TEST_ASSERT_NOT_NULL(parsed);
    TEST_ASSERT_NOT_NULL(parsed->result);
    TEST_ASSERT_EQUAL_UINT(1, parsed->result->sent_count);
    TEST_ASSERT_EQUAL_UINT(1, parsed->result->received_count);
    TEST_ASSERT_EQUAL_STRING("NownerAddress", parsed->result->transfer_address);

    neoc_get_token_transfers_response_free(parsed);
    neoc_get_token_transfers_response_free(response);
    neoc_free(json);
}

void test_unspents_from_json(void) {
    static const char *sample_json =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":11,"
        "\"result\":{"
            "\"address\":\"NTestUnspentsAddr\","
            "\"balance\":["
                "{"
                    "\"assethash\":\"0x0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f\","
                    "\"assetname\":\"NEO\","
                    "\"symbol\":\"NEO\","
                    "\"amount\":\"10\","
                    "\"unspent\":["
                        "{"
                            "\"txid\":\"0xabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabca\","
                            "\"n\":0,"
                            "\"value\":\"10\""
                        "}"
                    "]"
                "}"
            "]"
        "}"
        "}";

    neoc_neo_get_unspents_response_t *response = NULL;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_get_unspents_response_from_json(sample_json, &response));
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_EQUAL_UINT(1, response->result->balances_count);
    double neo_balance = 0.0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_unspents_get_asset_balance(response->result,
                                                          "0x0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f",
                                                          &neo_balance));
    TEST_ASSERT_EQUAL_DOUBLE(10.0, neo_balance);

    neoc_neo_get_unspents_response_free(response);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_transaction_send_token_roundtrip);
    RUN_TEST(test_response_alias_helpers);
    RUN_TEST(test_transaction_signer_roundtrip);
    RUN_TEST(test_oracle_request_response_roundtrip);
    RUN_TEST(test_populated_blocks_roundtrip);
    RUN_TEST(test_record_state_roundtrip);
    RUN_TEST(test_neo_list_plugins_roundtrip);
    RUN_TEST(test_neo_witness_roundtrip);
    RUN_TEST(test_nep17_contract_roundtrip);
    RUN_TEST(test_network_fee_roundtrip);
    RUN_TEST(test_send_raw_transaction_roundtrip);
    RUN_TEST(test_token_transfers_roundtrip);
    RUN_TEST(test_unspents_from_json);
    return UnityEnd();
}
