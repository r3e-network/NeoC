/**
 * @file test_bip32_ec_key_pair.c
 * @brief Unit tests converted from Bip32ECKeyPairTests.swift
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "neoc/neoc.h"
#include "neoc/crypto/bip32_ec_key_pair.h"
#include "neoc/crypto/ec_key_pair.h"
#include "neoc/utils/hex.h"
#include "neoc/encoding/base58.h"
#include "neoc/crypto/helpers/hash.h"

// Test setup
static void setUp(void) {
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
    
    // Set curve to EC256k1 for tests
    err = neoc_constants_set_curve_for_tests(NEOC_CURVE_EC256K1);
    assert(err == NEOC_SUCCESS);
}

// Test teardown
static void tearDown(void) {
    // Reset curve
    neoc_constants_stop_using_other_curve_for_tests();
    neoc_cleanup();
}

// Helper function to add checksum
static void add_checksum(uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len) {
    // Copy input
    memcpy(output, input, input_len);
    
    // Calculate double SHA256
    uint8_t hash[32];
    neoc_hash256(input, input_len, hash);
    neoc_hash256(hash, 32, hash);
    
    // Add first 4 bytes of hash as checksum
    memcpy(output + input_len, hash, 4);
    *output_len = input_len + 4;
}

// Helper function to serialize public key
static void serialize_public(neoc_bip32_ec_key_pair_t *pair, uint8_t *output, size_t *output_len) {
    uint8_t buffer[82];
    size_t offset = 0;
    
    // Version bytes for mainnet public key (0x0488B21E)
    uint32_t header = 0x0488B21E;
    buffer[offset++] = (header >> 24) & 0xFF;
    buffer[offset++] = (header >> 16) & 0xFF;
    buffer[offset++] = (header >> 8) & 0xFF;
    buffer[offset++] = header & 0xFF;
    
    // Depth
    buffer[offset++] = neoc_bip32_ec_key_pair_get_depth(pair);
    
    // Parent fingerprint
    const uint8_t *fingerprint = neoc_bip32_ec_key_pair_get_parent_fingerprint(pair);
    memcpy(buffer + offset, fingerprint, 4);
    offset += 4;
    
    // Child number
    uint32_t child_number = neoc_bip32_ec_key_pair_get_child_number(pair);
    buffer[offset++] = (child_number >> 24) & 0xFF;
    buffer[offset++] = (child_number >> 16) & 0xFF;
    buffer[offset++] = (child_number >> 8) & 0xFF;
    buffer[offset++] = child_number & 0xFF;
    
    // Chain code
    const uint8_t *chain_code = neoc_bip32_ec_key_pair_get_chain_code(pair);
    memcpy(buffer + offset, chain_code, 32);
    offset += 32;
    
    // Public key (compressed)
    neoc_ec_public_key_t *public_key = neoc_bip32_ec_key_pair_get_public_key(pair);
    size_t pub_key_len;
    const uint8_t *pub_key_bytes = neoc_ec_public_key_get_encoded(public_key, true, &pub_key_len);
    memcpy(buffer + offset, pub_key_bytes, 33);
    offset += 33;
    
    add_checksum(buffer, offset, output, output_len);
}

// Helper function to serialize private key
static void serialize_private(neoc_bip32_ec_key_pair_t *pair, uint8_t *output, size_t *output_len) {
    uint8_t buffer[82];
    size_t offset = 0;
    
    // Version bytes for mainnet private key (0x0488ADE4)
    uint32_t header = 0x0488ADE4;
    buffer[offset++] = (header >> 24) & 0xFF;
    buffer[offset++] = (header >> 16) & 0xFF;
    buffer[offset++] = (header >> 8) & 0xFF;
    buffer[offset++] = header & 0xFF;
    
    // Depth
    buffer[offset++] = neoc_bip32_ec_key_pair_get_depth(pair);
    
    // Parent fingerprint
    const uint8_t *fingerprint = neoc_bip32_ec_key_pair_get_parent_fingerprint(pair);
    memcpy(buffer + offset, fingerprint, 4);
    offset += 4;
    
    // Child number
    uint32_t child_number = neoc_bip32_ec_key_pair_get_child_number(pair);
    buffer[offset++] = (child_number >> 24) & 0xFF;
    buffer[offset++] = (child_number >> 16) & 0xFF;
    buffer[offset++] = (child_number >> 8) & 0xFF;
    buffer[offset++] = child_number & 0xFF;
    
    // Chain code
    const uint8_t *chain_code = neoc_bip32_ec_key_pair_get_chain_code(pair);
    memcpy(buffer + offset, chain_code, 32);
    offset += 32;
    
    // Private key (with 0x00 prefix for padding to 33 bytes)
    buffer[offset++] = 0x00;
    neoc_ec_private_key_t *private_key = neoc_bip32_ec_key_pair_get_private_key(pair);
    const uint8_t *priv_key_bytes = neoc_ec_private_key_get_bytes(private_key);
    memcpy(buffer + offset, priv_key_bytes, 32);
    offset += 32;
    
    add_checksum(buffer, offset, output, output_len);
}

// Helper function to test key generation and derivation
static void generate_and_test(const char *seed_hex, const char *expected_private,
                              const char *expected_public, int32_t *path, size_t path_len) {
    // Convert seed from hex
    uint8_t seed[64];
    size_t seed_len = 0;
    neoc_error_t err = neoc_hex_decode(seed_hex, seed, sizeof(seed), &seed_len);
    assert(err == NEOC_SUCCESS);
    
    // Generate master key pair
    neoc_bip32_ec_key_pair_t *key_pair = NULL;
    err = neoc_bip32_ec_key_pair_generate(seed, seed_len, &key_pair);
    assert(err == NEOC_SUCCESS);
    assert(key_pair != NULL);
    
    // Derive child key pair
    neoc_bip32_ec_key_pair_t *child_pair = NULL;
    err = neoc_bip32_ec_key_pair_derive(key_pair, path, path_len, &child_pair);
    assert(err == NEOC_SUCCESS);
    assert(child_pair != NULL);
    
    // Serialize private key
    uint8_t priv_serialized[82];
    size_t priv_len = 0;
    serialize_private(child_pair, priv_serialized, &priv_len);
    
    // Encode to base58
    char priv_base58[120];
    err = neoc_base58_encode(priv_serialized, priv_len, priv_base58, sizeof(priv_base58));
    assert(err == NEOC_SUCCESS);
    assert(strcmp(priv_base58, expected_private) == 0);
    
    // Serialize public key
    uint8_t pub_serialized[82];
    size_t pub_len = 0;
    serialize_public(child_pair, pub_serialized, &pub_len);
    
    // Encode to base58
    char pub_base58[120];
    err = neoc_base58_encode(pub_serialized, pub_len, pub_base58, sizeof(pub_base58));
    assert(err == NEOC_SUCCESS);
    assert(strcmp(pub_base58, expected_public) == 0);
    
    neoc_bip32_ec_key_pair_free(child_pair);
    neoc_bip32_ec_key_pair_free(key_pair);
}

// Test vectors 1
static void test_vectors_1(void) {
    printf("Testing BIP32 test vectors 1...\n");
    
    // Chain m
    int32_t path_m[] = {};
    generate_and_test(
        "000102030405060708090a0b0c0d0e0f",
        "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi",
        "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8",
        path_m, 0
    );
    
    // Chain m/0H
    int32_t path_0h[] = {0 | NEOC_BIP32_HARDENED_BIT};
    generate_and_test(
        "000102030405060708090a0b0c0d0e0f",
        "xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rGL5hj6KCesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7",
        "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw",
        path_0h, 1
    );
    
    // Chain m/0H/1
    int32_t path_0h_1[] = {0 | NEOC_BIP32_HARDENED_BIT, 1};
    generate_and_test(
        "000102030405060708090a0b0c0d0e0f",
        "xprv9wTYmMFdV23N2TdNG573QoEsfRrWKQgWeibmLntzniatZvR9BmLnvSxqu53Kw1UmYPxLgboyZQaXwTCg8MSY3H2EU4pWcQDnRnrVA1xe8fs",
        "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAWbWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ",
        path_0h_1, 2
    );
    
    // Chain m/0H/1/2H
    int32_t path_0h_1_2h[] = {0 | NEOC_BIP32_HARDENED_BIT, 1, 2 | NEOC_BIP32_HARDENED_BIT};
    generate_and_test(
        "000102030405060708090a0b0c0d0e0f",
        "xprv9z4pot5VBttmtdRTWfWQmoH1taj2axGVzFqSb8C9xaxKymcFzXBDptWmT7FwuEzG3ryjH4ktypQSAewRiNMjANTtpgP4mLTj34bhnZX7UiM",
        "xpub6D4BDPcP2GT577Vvch3R8wDkScZWzQzMMUm3PWbmWvVJrZwQY4VUNgqFJPMM3No2dFDFGTsxxpG5uJh7n7epu4trkrX7x7DogT5Uv6fcLW5",
        path_0h_1_2h, 3
    );
    
    // Chain m/0H/1/2H/2
    int32_t path_0h_1_2h_2[] = {0 | NEOC_BIP32_HARDENED_BIT, 1, 2 | NEOC_BIP32_HARDENED_BIT, 2};
    generate_and_test(
        "000102030405060708090a0b0c0d0e0f",
        "xprvA2JDeKCSNNZky6uBCviVfJSKyQ1mDYahRjijr5idH2WwLsEd4Hsb2Tyh8RfQMuPh7f7RtyzTtdrbdqqsunu5Mm3wDvUAKRHSC34sJ7in334",
        "xpub6FHa3pjLCk84BayeJxFW2SP4XRrFd1JYnxeLeU8EqN3vDfZmbqBqaGJAyiLjTAwm6ZLRQUMv1ZACTj37sR62cfN7fe5JnJ7dh8zL4fiyLHV",
        path_0h_1_2h_2, 4
    );
    
    // Chain m/0H/1/2H/2/1000000000
    int32_t path_long[] = {0 | NEOC_BIP32_HARDENED_BIT, 1, 2 | NEOC_BIP32_HARDENED_BIT, 2, 1000000000};
    generate_and_test(
        "000102030405060708090a0b0c0d0e0f",
        "xprvA41z7zogVVwxVSgdKUHDy1SKmdb533PjDz7J6N6mV6uS3ze1ai8FHa8kmHScGpWmj4WggLyQjgPie1rFSruoUihUZREPSL39UNdE3BBDu76",
        "xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFgJS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy",
        path_long, 5
    );
    
    printf("  ✅ BIP32 test vectors 1 passed\n");
}

// Test vectors 2
static void test_vectors_2(void) {
    printf("Testing BIP32 test vectors 2...\n");
    
    // Chain m
    int32_t path_m[] = {};
    generate_and_test(
        "fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542",
        "xprv9s21ZrQH143K31xYSDQpPDxsXRTUcvj2iNHm5NUtrGiGG5e2DtALGdso3pGz6ssrdK4PFmM8NSpSBHNqPqm55Qn3LqFtT2emdEXVYsCzC2U",
        "xpub661MyMwAqRbcFW31YEwpkMuc5THy2PSt5bDMsktWQcFF8syAmRUapSCGu8ED9W6oDMSgv6Zz8idoc4a6mr8BDzTJY47LJhkJ8UB7WEGuduB",
        path_m, 0
    );
    
    // Chain m/0
    int32_t path_0[] = {0};
    generate_and_test(
        "fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542",
        "xprv9vHkqa6EV4sPZHYqZznhT2NPtPCjKuDKGY38FBWLvgaDx45zo9WQRUT3dKYnjwih2yJD9mkrocEZXo1ex8G81dwSM1fwqWpWkeS3v86pgKt",
        "xpub69H7F5d8KSRgmmdJg2KhpAK8SR3DjMwAdkxj3ZuxV27CprR9LgpeyGmXUbC6wb7ERfvrnKZjXoUmmDznezpbZb7ap6r1D3tgFxHmwMkQTPH",
        path_0, 1
    );
    
    // Chain m/0/2147483647H
    int32_t path_0_max_h[] = {0, 2147483647 | NEOC_BIP32_HARDENED_BIT};
    generate_and_test(
        "fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542",
        "xprv9wSp6B7kry3Vj9m1zSnLvN3xH8RdsPP1Mh7fAaR7aRLcQMKTR2vidYEeEg2mUCTAwCd6vnxVrcjfy2kRgVsFawNzmjuHc2YmYRmagcEPdU9",
        "xpub6ASAVgeehLbnwdqV6UKMHVzgqAG8Gr6riv3Fxxpj8ksbH9ebxaEyBLZ85ySDhKiLDBrQSARLq1uNRts8RuJiHjaDMBU4Zn9h8LZNnBC5y4a",
        path_0_max_h, 2
    );
    
    // Chain m/0/2147483647H/1
    int32_t path_0_max_h_1[] = {0, 2147483647 | NEOC_BIP32_HARDENED_BIT, 1};
    generate_and_test(
        "fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542",
        "xprv9zFnWC6h2cLgpmSA46vutJzBcfJ8yaJGg8cX1e5StJh45BBciYTRXSd25UEPVuesF9yog62tGAQtHjXajPPdbRCHuWS6T8XA2ECKADdw4Ef",
        "xpub6DF8uhdarytz3FWdA8TvFSvvAh8dP3283MY7p2V4SeE2wyWmG5mg5EwVvmdMVCQcoNJxGoWaU9DCWh89LojfZ537wTfunKau47EL2dhHKon",
        path_0_max_h_1, 3
    );
    
    printf("  ✅ BIP32 test vectors 2 passed\n");
}

// Test vectors 3
static void test_vectors_3(void) {
    printf("Testing BIP32 test vectors 3...\n");
    
    // Chain m
    int32_t path_m[] = {};
    generate_and_test(
        "4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be",
        "xprv9s21ZrQH143K25QhxbucbDDuQ4naNntJRi4KUfWT7xo4EKsHt2QJDu7KXp1A3u7Bi1j8ph3EGsZ9Xvz9dGuVrtHHs7pXeTzjuxBrCmmhgC6",
        "xpub661MyMwAqRbcEZVB4dScxMAdx6d4nFc9nvyvH3v4gJL378CSRZiYmhRoP7mBy6gSPSCYk6SzXPTf3ND1cZAceL7SfJ1Z3GC8vBgp2epUt13",
        path_m, 0
    );
    
    // Chain m/0H
    int32_t path_0h[] = {0 | NEOC_BIP32_HARDENED_BIT};
    generate_and_test(
        "4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be",
        "xprv9uPDJpEQgRQfDcW7BkF7eTya6RPxXeJCqCJGHuCJ4GiRVLzkTXBAJMu2qaMWPrS7AANYqdq6vcBcBUdJCVVFceUvJFjaPdGZ2y9WACViL4L",
        "xpub68NZiKmJWnxxS6aaHmn81bvJeTESw724CRDs6HbuccFQN9Ku14VQrADWgqbhhTHBaohPX4CjNLf9fq9MYo6oDaPPLPxSb7gwQN3ih19Zm4Y",
        path_0h, 1
    );
    
    printf("  ✅ BIP32 test vectors 3 passed\n");
}

int main(void) {
    printf("\n=== Bip32ECKeyPairTests Tests ===\n\n");
    
    setUp();
    
    // Run all tests
    test_vectors_1();
    test_vectors_2();
    test_vectors_3();
    
    tearDown();
    
    printf("\n✅ All Bip32ECKeyPairTests tests passed!\n\n");
    return 0;
}