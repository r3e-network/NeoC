#include "unity.h"
#include <string.h>

#include "neoc/neoc.h"
#include "neoc/protocol/neo_c.h"
#include "neoc/protocol/service.h"
#include "neoc/protocol/core/request.h"

static neoc_error_t mock_perform_io(neoc_service_t *service,
                                    const neoc_byte_array_t *payload,
                                    neoc_byte_array_t **result) {
    (void)service;
    TEST_ASSERT_NOT_NULL(payload);
    TEST_ASSERT_NOT_NULL(result);
    *result = neoc_calloc(1, sizeof(neoc_byte_array_t));
    TEST_ASSERT_NOT_NULL(*result);

    const char *response = "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"ok\":true}}";
    size_t len = strlen(response);
    (*result)->data = neoc_malloc(len);
    TEST_ASSERT_NOT_NULL((*result)->data);
    memcpy((*result)->data, response, len);
    (*result)->length = len;
    (*result)->capacity = len;
    return NEOC_SUCCESS;
}

static neoc_service_t *make_mock_service(void) {
    neoc_service_t *service = neoc_calloc(1, sizeof(neoc_service_t));
    TEST_ASSERT_NOT_NULL(service);
    service->type = NEOC_SERVICE_TYPE_HTTP;
    service->config.include_raw_responses = false;

    static neoc_service_vtable_t vtable = {0};
    vtable.perform_io = mock_perform_io;
    service->vtable = &vtable;
    return service;
}

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_neo_c_send_request_uses_service_io(void) {
    neoc_neo_c_config_t *config = neoc_neo_c_config_create();
    neoc_service_t *service = make_mock_service();
    neoc_neo_c_t *client = neoc_neo_c_create(config, service);
    TEST_ASSERT_NOT_NULL(client);

    const char *payload = "{\"jsonrpc\":\"2.0\",\"method\":\"test\",\"params\":[],\"id\":1}";
    neoc_byte_array_t request_bytes = { (uint8_t *)payload, strlen(payload), strlen(payload) };
    neoc_response_t *response = NULL;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_neo_c_send_request(client, &request_bytes, &response));
    TEST_ASSERT_NOT_NULL(response);
    TEST_ASSERT_FALSE(neoc_response_has_error(response));
    TEST_ASSERT_EQUAL_INT(1, response->id);
    TEST_ASSERT_NOT_NULL(response->result);
    TEST_ASSERT_NOT_NULL(strstr((char *)response->result, "\"ok\":true"));

    neoc_response_free(response);
    neoc_neo_c_free(client);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_neo_c_send_request_uses_service_io);
    return UnityEnd();
}
