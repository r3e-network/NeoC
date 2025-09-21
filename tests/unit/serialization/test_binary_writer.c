/**
 * @file test_binary_writer.c
 * @brief Unit tests converted from BinaryWriterTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "neoc/neoc.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/utils/hex.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    neoc_cleanup();
}

// Helper function to verify writer output
static void verify_writer_output(neoc_binary_writer_t *writer, const char *expected_hex) {
    uint8_t *data = NULL;
    size_t data_len = 0;
    neoc_error_t err = neoc_binary_writer_to_array(writer, &data, &data_len);
    assert(err == NEOC_SUCCESS);
    
    // Convert expected hex to bytes
    size_t expected_len = strlen(expected_hex) / 2;
    uint8_t *expected = malloc(expected_len);
    assert(expected != NULL);
    
    size_t actual_expected_len = 0;
    err = neoc_hex_decode(expected_hex, expected, expected_len, &actual_expected_len);
    assert(err == NEOC_SUCCESS);
    assert(actual_expected_len == expected_len);
    
    // Compare
    assert(data_len == expected_len);
    assert(memcmp(data, expected, expected_len) == 0);
    
    free(data);
    free(expected);
}

// Test write uint32
static void test_write_uint32(void) {
    printf("Testing write uint32...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Test max uint32 value (2^32 - 1)
    err = neoc_binary_writer_write_uint32(writer, UINT32_MAX);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "ffffffff");
    neoc_binary_writer_reset(writer);
    
    // Test 0
    err = neoc_binary_writer_write_uint32(writer, 0);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "00000000");
    neoc_binary_writer_reset(writer);
    
    // Test 12345
    err = neoc_binary_writer_write_uint32(writer, 12345);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "39300000");
    neoc_binary_writer_reset(writer);
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Write uint32 test passed\n");
}

// Test write int64
static void test_write_int64(void) {
    printf("Testing write int64...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Test INT64_MAX
    err = neoc_binary_writer_write_int64(writer, INT64_MAX);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "ffffffffffffff7f");
    neoc_binary_writer_reset(writer);
    
    // Test INT64_MIN
    err = neoc_binary_writer_write_int64(writer, INT64_MIN);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "0000000000000080");
    neoc_binary_writer_reset(writer);
    
    // Test 0
    err = neoc_binary_writer_write_int64(writer, 0);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "0000000000000000");
    neoc_binary_writer_reset(writer);
    
    // Test 1234567890
    err = neoc_binary_writer_write_int64(writer, 1234567890);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "d202964900000000");
    neoc_binary_writer_reset(writer);
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Write int64 test passed\n");
}

// Test write uint16
static void test_write_uint16(void) {
    printf("Testing write uint16...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Test max uint16 value (2^16 - 1)
    err = neoc_binary_writer_write_uint16(writer, UINT16_MAX);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "ffff");
    neoc_binary_writer_reset(writer);
    
    // Test 0
    err = neoc_binary_writer_write_uint16(writer, 0);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "0000");
    neoc_binary_writer_reset(writer);
    
    // Test 12345
    err = neoc_binary_writer_write_uint16(writer, 12345);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "3930");
    neoc_binary_writer_reset(writer);
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Write uint16 test passed\n");
}

// Test write var int
static void test_write_var_int(void) {
    printf("Testing write var int...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Test v == 0, encode with one byte
    err = neoc_binary_writer_write_var_int(writer, 0);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "00");
    neoc_binary_writer_reset(writer);
    
    // Test v == 252 (0xfc), encode with one byte
    err = neoc_binary_writer_write_var_int(writer, 252);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fc");
    neoc_binary_writer_reset(writer);
    
    // Test v == 253 (0xfd), encode with uint16
    err = neoc_binary_writer_write_var_int(writer, 253);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fdfd00");
    neoc_binary_writer_reset(writer);
    
    // Test v == 254, encode with uint16
    err = neoc_binary_writer_write_var_int(writer, 254);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fdfe00");
    neoc_binary_writer_reset(writer);
    
    // Test v == 65534, encode with uint16
    err = neoc_binary_writer_write_var_int(writer, 65534);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fdfeff");
    neoc_binary_writer_reset(writer);
    
    // Test v == 65535 (0xffff), encode with uint16
    err = neoc_binary_writer_write_var_int(writer, 65535);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fdffff");
    neoc_binary_writer_reset(writer);
    
    // Test v == 65536 (0x10000), encode with uint32
    err = neoc_binary_writer_write_var_int(writer, 65536);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fe00000100");
    neoc_binary_writer_reset(writer);
    
    // Test v == 4294967294 (0xfffffffe), encode with uint32
    err = neoc_binary_writer_write_var_int(writer, 4294967294ULL);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "fefeffffff");
    neoc_binary_writer_reset(writer);
    
    // Test v == 4294967295 (0xffffffff), encode with uint32
    err = neoc_binary_writer_write_var_int(writer, 4294967295ULL);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "feffffffff");
    neoc_binary_writer_reset(writer);
    
    // Test v == 4294967296 (0x100000000), encode with uint64
    err = neoc_binary_writer_write_var_int(writer, 4294967296ULL);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "ff0000000001000000");
    neoc_binary_writer_reset(writer);
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Write var int test passed\n");
}

// Test write var bytes
static void test_write_var_bytes(void) {
    printf("Testing write var bytes...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Test small byte array
    uint8_t small_data[] = {0x01, 0x02, 0x03};
    err = neoc_binary_writer_write_var_bytes(writer, small_data, sizeof(small_data));
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "03010203");
    neoc_binary_writer_reset(writer);
    
    // Test larger byte array (262 bytes)
    const char *hex_input = "00102030102030102030102030102030102030102030102030102030102030102031020301020301020301020301020301020301020301020301020301020301020310203010203010203010203010203010203010203010203010203010203010203102030102030102030102030102030102030102030102030102030102030102030010203010203010203010203010203010203010203010203010203010203010203102030102030102030102030102030102030102030102030102030102030102031020301020301020301020301020301020301020301020301020301020301020310203010203010203010203010203010203010203010203010203010203010203";
    size_t input_len = strlen(hex_input) / 2;
    uint8_t *large_data = malloc(input_len);
    assert(large_data != NULL);
    
    size_t actual_len = 0;
    err = neoc_hex_decode(hex_input, large_data, input_len, &actual_len);
    assert(err == NEOC_SUCCESS);
    
    err = neoc_binary_writer_write_var_bytes(writer, large_data, actual_len);
    assert(err == NEOC_SUCCESS);
    
    char expected[1024];
    snprintf(expected, sizeof(expected), "fd0601%s", hex_input);
    verify_writer_output(writer, expected);
    
    free(large_data);
    neoc_binary_writer_free(writer);
    printf("  ✅ Write var bytes test passed\n");
}

// Test write var string
static void test_write_var_string(void) {
    printf("Testing write var string...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Test "hello, world!"
    err = neoc_binary_writer_write_var_string(writer, "hello, world!");
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "0d68656c6c6f2c20776f726c6421");
    neoc_binary_writer_reset(writer);
    
    // Test longer string (533 bytes)
    const char *long_str = "hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!hello, world!";
    
    err = neoc_binary_writer_write_var_string(writer, long_str);
    assert(err == NEOC_SUCCESS);
    
    // Construct expected output
    char expected[2048] = "fd1502";  // var int for length 533
    size_t str_len = strlen(long_str);
    for (size_t i = 0; i < str_len; i++) {
        char hex_byte[3];
        snprintf(hex_byte, sizeof(hex_byte), "%02x", (unsigned char)long_str[i]);
        strcat(expected, hex_byte);
    }
    
    verify_writer_output(writer, expected);
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Write var string test passed\n");
}

// Test write bytes
static void test_write_bytes(void) {
    printf("Testing write bytes...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Write some bytes
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    err = neoc_binary_writer_write_bytes(writer, data, sizeof(data));
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "0102030405");
    
    // Write more bytes
    uint8_t data2[] = {0x06, 0x07, 0x08};
    err = neoc_binary_writer_write_bytes(writer, data2, sizeof(data2));
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "0102030405060708");
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Write bytes test passed\n");
}

// Test writer reset
static void test_writer_reset(void) {
    printf("Testing writer reset...\n");
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Write some data
    err = neoc_binary_writer_write_uint32(writer, 12345);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "39300000");
    
    // Reset writer
    err = neoc_binary_writer_reset(writer);
    assert(err == NEOC_SUCCESS);
    
    // Verify writer is empty
    uint8_t *data = NULL;
    size_t data_len = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_len);
    assert(err == NEOC_SUCCESS);
    assert(data_len == 0);
    free(data);
    
    // Write new data
    err = neoc_binary_writer_write_uint16(writer, 999);
    assert(err == NEOC_SUCCESS);
    verify_writer_output(writer, "e703");
    
    neoc_binary_writer_free(writer);
    printf("  ✅ Writer reset test passed\n");
}

int main(void) {
    printf("\n=== BinaryWriterTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_write_uint32();
    test_write_int64();
    test_write_uint16();
    test_write_var_int();
    test_write_var_bytes();
    test_write_var_string();
    test_write_bytes();
    test_writer_reset();
    
    tearDown();
    
    printf("\n✅ All BinaryWriterTests tests passed!\n\n");
    return 0;
}