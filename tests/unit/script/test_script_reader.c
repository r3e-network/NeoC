#include "unity.h"
#include <string.h>
#include "neoc/neoc.h"
#include "neoc/script/script_reader.h"
#include "neoc/script/opcode.h"

static const uint8_t SIMPLE_SCRIPT[] = {
    NEOC_OP_PUSH1,
    NEOC_OP_PUSH2,
    NEOC_OP_SYSCALL, 0x07, 0x5C, 0x1C, 0x52, // random syscall hash
    NEOC_OP_RET
};

void setUp(void) {
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_init());
}

void tearDown(void) {
    neoc_cleanup();
}

void test_script_reader_init_and_position(void) {
    neoc_script_reader_t reader;
    char buffer[128] = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_script_reader_init(&reader,
                                                  SIMPLE_SCRIPT,
                                                  sizeof(SIMPLE_SCRIPT),
                                                  buffer,
                                                  sizeof(buffer)));
    TEST_ASSERT_TRUE(neoc_script_reader_has_more(&reader));
    TEST_ASSERT_EQUAL_UINT(0, neoc_script_reader_get_position(&reader));

    uint8_t opcode = 0;
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_script_reader_read_byte(&reader, &opcode));
    TEST_ASSERT_EQUAL_UINT8(NEOC_OP_PUSH1, opcode);
    TEST_ASSERT_EQUAL_UINT(1, neoc_script_reader_get_position(&reader));

    uint8_t bytes[2] = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS, neoc_script_reader_read_bytes(&reader, bytes, 2));
    TEST_ASSERT_EQUAL_UINT8(NEOC_OP_PUSH2, bytes[0]);
    TEST_ASSERT_EQUAL_UINT8(NEOC_OP_SYSCALL, bytes[1]);
}

void test_script_reader_to_opcode_string(void) {
    char output[512] = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_script_reader_to_opcode_string(SIMPLE_SCRIPT,
                                                              sizeof(SIMPLE_SCRIPT),
                                                              output,
                                                              sizeof(output)));
    TEST_ASSERT_TRUE(strlen(output) > 0);
    TEST_ASSERT_NOT_NULL(strstr(output, "PUSH1"));
    TEST_ASSERT_NOT_NULL(strstr(output, "SYSCALL"));
}

void test_script_reader_output_helpers(void) {
    neoc_script_reader_t reader;
    char buffer[64] = {0};
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_script_reader_init(&reader,
                                                  SIMPLE_SCRIPT,
                                                  sizeof(SIMPLE_SCRIPT),
                                                  buffer,
                                                  sizeof(buffer)));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_script_reader_append_output(&reader, "OP "));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_script_reader_append_formatted(&reader, "%s %d", "PUSH", 1));
    TEST_ASSERT_EQUAL_STRING("OP PUSH 1", buffer);
}

void test_script_reader_get_interop_service_unknown(void) {
    neoc_interop_service_t service;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_NOT_FOUND,
                          neoc_script_reader_get_interop_service("0x00000000", &service));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_script_reader_init_and_position);
    RUN_TEST(test_script_reader_to_opcode_string);
    RUN_TEST(test_script_reader_output_helpers);
    RUN_TEST(test_script_reader_get_interop_service_unknown);
    UNITY_END();
}
