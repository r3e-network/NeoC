#include "unity.h"
#include <string.h>
#include "neoc/neoc_error.h"
#include "neoc/utils/neoc_bytes.h"

void setUp(void) {}
void tearDown(void) {}

void test_var_size_length_boundaries(void) {
    TEST_ASSERT_EQUAL_UINT(1, neoc_bytes_var_size(0xfc));
    TEST_ASSERT_EQUAL_UINT(3, neoc_bytes_var_size(0xfd));
    TEST_ASSERT_EQUAL_UINT(3, neoc_bytes_var_size(0xffff));
    TEST_ASSERT_EQUAL_UINT(5, neoc_bytes_var_size(0x10000));
    TEST_ASSERT_EQUAL_UINT(9, neoc_bytes_var_size((size_t)0x1ULL << 40));
}

static void assert_round_trip(size_t value) {
    uint8_t buffer[16] = {0};
    size_t written = 0;
    size_t read_value = 0;
    size_t bytes_read = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bytes_encode_var_size(value, buffer, sizeof(buffer), &written));
    TEST_ASSERT_TRUE(written <= sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(NEOC_SUCCESS,
                          neoc_bytes_decode_var_size(buffer, written, &read_value, &bytes_read));
    TEST_ASSERT_EQUAL_UINT(written, bytes_read);
    TEST_ASSERT_EQUAL_UINT(value, read_value);
}

void test_var_size_encode_decode_roundtrip(void) {
    const size_t values[] = {
        0, 0xfc, 0xfd, 0xffff, 0x10000, 0xffffffffu, ((size_t)1 << 48) + 123
    };
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        assert_round_trip(values[i]);
    }
}

void test_var_size_buffer_errors(void) {
    uint8_t buffer[2] = {0};
    size_t written = 0;
    size_t value = 0;
    size_t read = 0;

    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL,
                          neoc_bytes_encode_var_size(0xffff, buffer, sizeof(buffer), &written));
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL,
                          neoc_bytes_decode_var_size(buffer, 0, &value, &read));

    buffer[0] = 0xfe;
    TEST_ASSERT_EQUAL_INT(NEOC_ERROR_BUFFER_TOO_SMALL,
                          neoc_bytes_decode_var_size(buffer, 2, &value, &read));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_var_size_length_boundaries);
    RUN_TEST(test_var_size_encode_decode_roundtrip);
    RUN_TEST(test_var_size_buffer_errors);
    UNITY_END();
}
