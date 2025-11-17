/**
 * @file test_r_i_p_e_m_d160.c
 * @brief Unit tests converted from RIPEMD160Tests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/crypto/neoc_hash.h"

static void assert_ripemd_hex(const char *message, const char *expected_hex) {
    uint8_t digest[NEOC_RIPEMD160_DIGEST_LENGTH];
    neoc_error_t err = neoc_ripemd160((const uint8_t *)message,
                                      strlen(message),
                                      digest);
    assert(err == NEOC_SUCCESS);

    char hex_output[NEOC_RIPEMD160_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < NEOC_RIPEMD160_DIGEST_LENGTH; ++i) {
        sprintf(hex_output + i * 2, "%02x", digest[i]);
    }
    hex_output[sizeof(hex_output) - 1] = '\0';
    assert(strcmp(hex_output, expected_hex) == 0);
}

static void test_known_vectors(void) {
    assert_ripemd_hex("",
                      "9c1185a5c5e9fc54612808977ee8f548b2258d31");
    assert_ripemd_hex("a",
                      "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe");
    assert_ripemd_hex("abc",
                      "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
    assert_ripemd_hex("message digest",
                      "5d0689ef49d2fae572b881b123a85ffa21595f36");
    assert_ripemd_hex("abcdefghijklmnopqrstuvwxyz",
                      "f71c27109c692c1b56bbdceb5b9d2865b3708dbc");
}

static void test_million_a(void) {
    const size_t len = 1000000;
    char *buffer = malloc(len + 1);
    assert(buffer);
    memset(buffer, 'a', len);
    buffer[len] = '\0';

    uint8_t digest[NEOC_RIPEMD160_DIGEST_LENGTH];
    neoc_error_t err = neoc_ripemd160((const uint8_t *)buffer, len, digest);
    assert(err == NEOC_SUCCESS);
    free(buffer);

    char hex_output[NEOC_RIPEMD160_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < NEOC_RIPEMD160_DIGEST_LENGTH; ++i) {
        sprintf(hex_output + i * 2, "%02x", digest[i]);
    }
    hex_output[sizeof(hex_output) - 1] = '\0';
    assert(strcmp(hex_output, "52783243c1697bdbe16d37f97f68f08325dc1528") == 0);
}

int main(void) {
    printf("\n=== RIPEMD160 Helper Tests ===\n\n");
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);

    test_known_vectors();
    test_million_a();

    neoc_cleanup();
    printf("\n✅ RIPEMD160 helper tests passed!\n\n");
    return 0;
}
