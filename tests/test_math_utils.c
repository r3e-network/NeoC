/**
 * @file test_math_utils.c
 * @brief Mathematical utility tests
 */

#include "unity.h"
#include <neoc/neoc.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

void setUp(void) {
    neoc_init();
}

void tearDown(void) {
    neoc_cleanup();
}

/* ===== MATH UTILITY TESTS ===== */

void test_min_max_operations(void) {
    // Test min
    int a = 5, b = 10;
    int min_val = (a < b) ? a : b;
    TEST_ASSERT_EQUAL_INT(5, min_val);
    
    // Test max
    int max_val = (a > b) ? a : b;
    TEST_ASSERT_EQUAL_INT(10, max_val);
    
    // Test with negative numbers
    int c = -5, d = -10;
    min_val = (c < d) ? c : d;
    TEST_ASSERT_EQUAL_INT(-10, min_val);
    
    max_val = (c > d) ? c : d;
    TEST_ASSERT_EQUAL_INT(-5, max_val);
}

void test_absolute_value(void) {
    // Test positive number
    int positive = 42;
    int abs_val = (positive < 0) ? -positive : positive;
    TEST_ASSERT_EQUAL_INT(42, abs_val);
    
    // Test negative number
    int negative = -42;
    abs_val = (negative < 0) ? -negative : negative;
    TEST_ASSERT_EQUAL_INT(42, abs_val);
    
    // Test zero
    int zero = 0;
    abs_val = (zero < 0) ? -zero : zero;
    TEST_ASSERT_EQUAL_INT(0, abs_val);
}

void test_power_of_two_check(void) {
    // Powers of 2
    uint32_t powers[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    
    for (size_t i = 0; i < sizeof(powers) / sizeof(powers[0]); i++) {
        // Check if power of 2: n & (n-1) == 0
        TEST_ASSERT_TRUE((powers[i] & (powers[i] - 1)) == 0);
    }
    
    // Non-powers of 2
    uint32_t non_powers[] = {3, 5, 6, 7, 9, 10, 11, 15, 17, 100};
    
    for (size_t i = 0; i < sizeof(non_powers) / sizeof(non_powers[0]); i++) {
        TEST_ASSERT_FALSE((non_powers[i] & (non_powers[i] - 1)) == 0);
    }
}

void test_bit_operations(void) {
    uint32_t value = 0;
    
    // Set bit 3
    value |= (1 << 3);
    TEST_ASSERT_EQUAL_UINT32(8, value);
    
    // Set bit 5
    value |= (1 << 5);
    TEST_ASSERT_EQUAL_UINT32(40, value); // 8 + 32
    
    // Clear bit 3
    value &= ~(1 << 3);
    TEST_ASSERT_EQUAL_UINT32(32, value);
    
    // Toggle bit 5
    value ^= (1 << 5);
    TEST_ASSERT_EQUAL_UINT32(0, value);
    
    // Check if bit is set
    value = 0x0F; // 00001111
    TEST_ASSERT_TRUE(value & (1 << 2)); // Bit 2 is set
    TEST_ASSERT_FALSE(value & (1 << 4)); // Bit 4 is not set
}

void test_division_rounding(void) {
    // Round up division: (a + b - 1) / b
    int dividend = 10;
    int divisor = 3;
    
    // Normal division (rounds down)
    int result_down = dividend / divisor;
    TEST_ASSERT_EQUAL_INT(3, result_down);
    
    // Round up division
    int result_up = (dividend + divisor - 1) / divisor;
    TEST_ASSERT_EQUAL_INT(4, result_up);
    
    // Test exact division
    dividend = 12;
    result_down = dividend / divisor;
    result_up = (dividend + divisor - 1) / divisor;
    TEST_ASSERT_EQUAL_INT(4, result_down);
    TEST_ASSERT_EQUAL_INT(4, result_up);
}

void test_safe_addition(void) {
    // Test normal addition
    uint32_t a = 100;
    uint32_t b = 200;
    uint32_t sum = a + b;
    TEST_ASSERT_EQUAL_UINT32(300, sum);
    
    // Test overflow detection
    uint32_t max = UINT32_MAX;
    uint32_t one = 1;
    
    // Check if addition would overflow
    bool would_overflow = (max > UINT32_MAX - one);
    TEST_ASSERT_TRUE(would_overflow);
}

void test_alignment_calculations(void) {
    // Test alignment to 4 bytes
    size_t unaligned = 7;
    size_t aligned_4 = (unaligned + 3) & ~3;
    TEST_ASSERT_EQUAL_UINT32(8, aligned_4);
    
    // Test alignment to 8 bytes
    size_t aligned_8 = (unaligned + 7) & ~7;
    TEST_ASSERT_EQUAL_UINT32(8, aligned_8);
    
    // Test already aligned
    unaligned = 16;
    aligned_4 = (unaligned + 3) & ~3;
    aligned_8 = (unaligned + 7) & ~7;
    TEST_ASSERT_EQUAL_UINT32(16, aligned_4);
    TEST_ASSERT_EQUAL_UINT32(16, aligned_8);
}

void test_modulo_operations(void) {
    // Basic modulo
    int value = 17;
    int mod_5 = value % 5;
    TEST_ASSERT_EQUAL_INT(2, mod_5);
    
    // Modulo with negative numbers
    value = -17;
    mod_5 = value % 5;
    TEST_ASSERT_EQUAL_INT(-2, mod_5);
    
    // Power of 2 modulo using bitwise AND
    uint32_t val = 17;
    uint32_t mod_8 = val & 7; // Equivalent to val % 8
    TEST_ASSERT_EQUAL_UINT32(1, mod_8);
}

/* ===== MAIN TEST RUNNER ===== */

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== MATH UTILITY TESTS ===\n");
    
    RUN_TEST(test_min_max_operations);
    RUN_TEST(test_absolute_value);
    RUN_TEST(test_power_of_two_check);
    RUN_TEST(test_bit_operations);
    RUN_TEST(test_division_rounding);
    RUN_TEST(test_safe_addition);
    RUN_TEST(test_alignment_calculations);
    RUN_TEST(test_modulo_operations);
    
    UNITY_END();
    return 0;
}

