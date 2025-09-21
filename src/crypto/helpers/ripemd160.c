/**
 * @file ripemd160.c
 * @brief RIPEMD160 hash implementation
 * 
 * Converted from Swift source: crypto/helpers/RIPEMD160.swift
 * Based on RIPEMD160 specification
 */

#include "neoc/neoc.h"
#include "neoc/crypto/helpers/ripemd160.h"
#include <string.h>

// RIPEMD160 constants
#define RIPEMD160_BLOCK_SIZE 64
#define RIPEMD160_DIGEST_SIZE 20

// Initial hash values
#define H0 0x67452301UL
#define H1 0xEFCDAB89UL
#define H2 0x98BADCFEUL
#define H3 0x10325476UL
#define H4 0xC3D2E1F0UL

// Constants for rounds
#define K1 0x00000000UL
#define K2 0x5A827999UL
#define K3 0x6ED9EBA1UL
#define K4 0x8F1BBCDCUL
#define K5 0xA953FD4EUL

#define KK1 0x50A28BE6UL
#define KK2 0x5C4DD124UL
#define KK3 0x6D703EF3UL
#define KK4 0x7A6D76E9UL
#define KK5 0x00000000UL

// Rotate left
#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// Basic functions
#define F(x, y, z) ((x) ^ (y) ^ (z))
#define G(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z) (((x) | ~(y)) ^ (z))
#define I(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z) ((x) ^ ((y) | ~(z)))

// Round operations
#define FF(a, b, c, d, e, x, s) \
    (a) += F((b), (c), (d)) + (x) + K1; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define GG(a, b, c, d, e, x, s) \
    (a) += G((b), (c), (d)) + (x) + K2; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define HH(a, b, c, d, e, x, s) \
    (a) += H((b), (c), (d)) + (x) + K3; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define II(a, b, c, d, e, x, s) \
    (a) += I((b), (c), (d)) + (x) + K4; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define JJ(a, b, c, d, e, x, s) \
    (a) += J((b), (c), (d)) + (x) + K5; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define FFF(a, b, c, d, e, x, s) \
    (a) += F((b), (c), (d)) + (x) + KK1; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define GGG(a, b, c, d, e, x, s) \
    (a) += G((b), (c), (d)) + (x) + KK2; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define HHH(a, b, c, d, e, x, s) \
    (a) += H((b), (c), (d)) + (x) + KK3; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define III(a, b, c, d, e, x, s) \
    (a) += I((b), (c), (d)) + (x) + KK4; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

#define JJJ(a, b, c, d, e, x, s) \
    (a) += J((b), (c), (d)) + (x) + KK5; \
    (a) = ROL((a), (s)) + (e); \
    (c) = ROL((c), 10)

static void ripemd160_compress(uint32_t state[5], const uint8_t block[64]) {
    uint32_t X[16];
    uint32_t aa, bb, cc, dd, ee;
    uint32_t aaa, bbb, ccc, ddd, eee;
    
    // Convert block to 16 32-bit words (little-endian)
    for (int i = 0; i < 16; i++) {
        X[i] = (uint32_t)block[i*4] |
               ((uint32_t)block[i*4+1] << 8) |
               ((uint32_t)block[i*4+2] << 16) |
               ((uint32_t)block[i*4+3] << 24);
    }
    
    // Initialize working variables
    aa = aaa = state[0];
    bb = bbb = state[1];
    cc = ccc = state[2];
    dd = ddd = state[3];
    ee = eee = state[4];
    
    // Left line - 5 rounds of 16 operations each
    // Round 1
    FF(aa, bb, cc, dd, ee, X[ 0], 11); FF(ee, aa, bb, cc, dd, X[ 1], 14);
    FF(dd, ee, aa, bb, cc, X[ 2], 15); FF(cc, dd, ee, aa, bb, X[ 3], 12);
    FF(bb, cc, dd, ee, aa, X[ 4],  5); FF(aa, bb, cc, dd, ee, X[ 5],  8);
    FF(ee, aa, bb, cc, dd, X[ 6],  7); FF(dd, ee, aa, bb, cc, X[ 7],  9);
    FF(cc, dd, ee, aa, bb, X[ 8], 11); FF(bb, cc, dd, ee, aa, X[ 9], 13);
    FF(aa, bb, cc, dd, ee, X[10], 14); FF(ee, aa, bb, cc, dd, X[11], 15);
    FF(dd, ee, aa, bb, cc, X[12],  6); FF(cc, dd, ee, aa, bb, X[13],  7);
    FF(bb, cc, dd, ee, aa, X[14],  9); FF(aa, bb, cc, dd, ee, X[15],  8);
    
    // Round 2
    GG(ee, aa, bb, cc, dd, X[ 7],  7); GG(dd, ee, aa, bb, cc, X[ 4],  6);
    GG(cc, dd, ee, aa, bb, X[13],  8); GG(bb, cc, dd, ee, aa, X[ 1], 13);
    GG(aa, bb, cc, dd, ee, X[10], 11); GG(ee, aa, bb, cc, dd, X[ 6],  9);
    GG(dd, ee, aa, bb, cc, X[15],  7); GG(cc, dd, ee, aa, bb, X[ 3], 15);
    GG(bb, cc, dd, ee, aa, X[12],  7); GG(aa, bb, cc, dd, ee, X[ 0], 12);
    GG(ee, aa, bb, cc, dd, X[ 9], 15); GG(dd, ee, aa, bb, cc, X[ 5],  9);
    GG(cc, dd, ee, aa, bb, X[ 2], 11); GG(bb, cc, dd, ee, aa, X[14],  7);
    GG(aa, bb, cc, dd, ee, X[11], 13); GG(ee, aa, bb, cc, dd, X[ 8], 12);
    
    // Round 3
    HH(dd, ee, aa, bb, cc, X[ 3], 11); HH(cc, dd, ee, aa, bb, X[10], 13);
    HH(bb, cc, dd, ee, aa, X[14],  6); HH(aa, bb, cc, dd, ee, X[ 4],  7);
    HH(ee, aa, bb, cc, dd, X[ 9], 14); HH(dd, ee, aa, bb, cc, X[15],  9);
    HH(cc, dd, ee, aa, bb, X[ 8], 13); HH(bb, cc, dd, ee, aa, X[ 1], 15);
    HH(aa, bb, cc, dd, ee, X[ 2], 14); HH(ee, aa, bb, cc, dd, X[ 7],  8);
    HH(dd, ee, aa, bb, cc, X[ 0], 13); HH(cc, dd, ee, aa, bb, X[ 6],  6);
    HH(bb, cc, dd, ee, aa, X[13],  5); HH(aa, bb, cc, dd, ee, X[11], 12);
    HH(ee, aa, bb, cc, dd, X[ 5],  7); HH(dd, ee, aa, bb, cc, X[12],  5);
    
    // Round 4
    II(cc, dd, ee, aa, bb, X[ 1], 11); II(bb, cc, dd, ee, aa, X[ 9], 12);
    II(aa, bb, cc, dd, ee, X[11], 14); II(ee, aa, bb, cc, dd, X[10], 15);
    II(dd, ee, aa, bb, cc, X[ 0], 14); II(cc, dd, ee, aa, bb, X[ 8], 15);
    II(bb, cc, dd, ee, aa, X[12],  9); II(aa, bb, cc, dd, ee, X[ 4],  8);
    II(ee, aa, bb, cc, dd, X[13],  9); II(dd, ee, aa, bb, cc, X[ 3], 14);
    II(cc, dd, ee, aa, bb, X[ 7],  5); II(bb, cc, dd, ee, aa, X[15],  6);
    II(aa, bb, cc, dd, ee, X[14],  8); II(ee, aa, bb, cc, dd, X[ 5],  6);
    II(dd, ee, aa, bb, cc, X[ 6],  5); II(cc, dd, ee, aa, bb, X[ 2], 12);
    
    // Round 5
    JJ(bb, cc, dd, ee, aa, X[ 4],  9); JJ(aa, bb, cc, dd, ee, X[ 0], 15);
    JJ(ee, aa, bb, cc, dd, X[ 5],  5); JJ(dd, ee, aa, bb, cc, X[ 9], 11);
    JJ(cc, dd, ee, aa, bb, X[ 7],  6); JJ(bb, cc, dd, ee, aa, X[12],  8);
    JJ(aa, bb, cc, dd, ee, X[ 2], 13); JJ(ee, aa, bb, cc, dd, X[10], 12);
    JJ(dd, ee, aa, bb, cc, X[14],  5); JJ(cc, dd, ee, aa, bb, X[ 1], 12);
    JJ(bb, cc, dd, ee, aa, X[ 3], 13); JJ(aa, bb, cc, dd, ee, X[ 8], 14);
    JJ(ee, aa, bb, cc, dd, X[11], 11); JJ(dd, ee, aa, bb, cc, X[ 6],  8);
    JJ(cc, dd, ee, aa, bb, X[15],  5); JJ(bb, cc, dd, ee, aa, X[13],  6);
    
    // Right line - 5 rounds of 16 operations each
    // Round 1
    JJJ(aaa, bbb, ccc, ddd, eee, X[ 5],  8); JJJ(eee, aaa, bbb, ccc, ddd, X[14],  9);
    JJJ(ddd, eee, aaa, bbb, ccc, X[ 7],  9); JJJ(ccc, ddd, eee, aaa, bbb, X[ 0], 11);
    JJJ(bbb, ccc, ddd, eee, aaa, X[ 9], 13); JJJ(aaa, bbb, ccc, ddd, eee, X[ 2], 15);
    JJJ(eee, aaa, bbb, ccc, ddd, X[11], 15); JJJ(ddd, eee, aaa, bbb, ccc, X[ 4],  5);
    JJJ(ccc, ddd, eee, aaa, bbb, X[13],  7); JJJ(bbb, ccc, ddd, eee, aaa, X[ 6],  7);
    JJJ(aaa, bbb, ccc, ddd, eee, X[15],  8); JJJ(eee, aaa, bbb, ccc, ddd, X[ 8], 11);
    JJJ(ddd, eee, aaa, bbb, ccc, X[ 1], 14); JJJ(ccc, ddd, eee, aaa, bbb, X[10], 14);
    JJJ(bbb, ccc, ddd, eee, aaa, X[ 3], 12); JJJ(aaa, bbb, ccc, ddd, eee, X[12],  6);
    
    // Round 2
    III(eee, aaa, bbb, ccc, ddd, X[ 6],  9); III(ddd, eee, aaa, bbb, ccc, X[11], 13);
    III(ccc, ddd, eee, aaa, bbb, X[ 3], 15); III(bbb, ccc, ddd, eee, aaa, X[ 7],  7);
    III(aaa, bbb, ccc, ddd, eee, X[ 0], 12); III(eee, aaa, bbb, ccc, ddd, X[13],  8);
    III(ddd, eee, aaa, bbb, ccc, X[ 5],  9); III(ccc, ddd, eee, aaa, bbb, X[10], 11);
    III(bbb, ccc, ddd, eee, aaa, X[14],  7); III(aaa, bbb, ccc, ddd, eee, X[15],  7);
    III(eee, aaa, bbb, ccc, ddd, X[ 8], 12); III(ddd, eee, aaa, bbb, ccc, X[12],  7);
    III(ccc, ddd, eee, aaa, bbb, X[ 4],  6); III(bbb, ccc, ddd, eee, aaa, X[ 9], 15);
    III(aaa, bbb, ccc, ddd, eee, X[ 1], 13); III(eee, aaa, bbb, ccc, ddd, X[ 2], 11);
    
    // Round 3
    HHH(ddd, eee, aaa, bbb, ccc, X[15],  9); HHH(ccc, ddd, eee, aaa, bbb, X[ 5],  7);
    HHH(bbb, ccc, ddd, eee, aaa, X[ 1], 15); HHH(aaa, bbb, ccc, ddd, eee, X[ 3], 11);
    HHH(eee, aaa, bbb, ccc, ddd, X[ 7],  8); HHH(ddd, eee, aaa, bbb, ccc, X[14],  6);
    HHH(ccc, ddd, eee, aaa, bbb, X[ 6],  6); HHH(bbb, ccc, ddd, eee, aaa, X[ 9], 14);
    HHH(aaa, bbb, ccc, ddd, eee, X[11], 12); HHH(eee, aaa, bbb, ccc, ddd, X[ 8], 13);
    HHH(ddd, eee, aaa, bbb, ccc, X[12],  5); HHH(ccc, ddd, eee, aaa, bbb, X[ 2], 14);
    HHH(bbb, ccc, ddd, eee, aaa, X[10], 13); HHH(aaa, bbb, ccc, ddd, eee, X[ 0], 13);
    HHH(eee, aaa, bbb, ccc, ddd, X[ 4],  7); HHH(ddd, eee, aaa, bbb, ccc, X[13],  5);
    
    // Round 4
    GGG(ccc, ddd, eee, aaa, bbb, X[ 8], 15); GGG(bbb, ccc, ddd, eee, aaa, X[ 6],  5);
    GGG(aaa, bbb, ccc, ddd, eee, X[ 4],  8); GGG(eee, aaa, bbb, ccc, ddd, X[ 1], 11);
    GGG(ddd, eee, aaa, bbb, ccc, X[ 3], 14); GGG(ccc, ddd, eee, aaa, bbb, X[11], 14);
    GGG(bbb, ccc, ddd, eee, aaa, X[15],  6); GGG(aaa, bbb, ccc, ddd, eee, X[ 0], 14);
    GGG(eee, aaa, bbb, ccc, ddd, X[ 5],  6); GGG(ddd, eee, aaa, bbb, ccc, X[12],  9);
    GGG(ccc, ddd, eee, aaa, bbb, X[ 2], 12); GGG(bbb, ccc, ddd, eee, aaa, X[13],  9);
    GGG(aaa, bbb, ccc, ddd, eee, X[ 9], 12); GGG(eee, aaa, bbb, ccc, ddd, X[ 7],  5);
    GGG(ddd, eee, aaa, bbb, ccc, X[10], 15); GGG(ccc, ddd, eee, aaa, bbb, X[14],  8);
    
    // Round 5
    FFF(bbb, ccc, ddd, eee, aaa, X[12],  8); FFF(aaa, bbb, ccc, ddd, eee, X[15],  5);
    FFF(eee, aaa, bbb, ccc, ddd, X[10], 12); FFF(ddd, eee, aaa, bbb, ccc, X[ 4],  9);
    FFF(ccc, ddd, eee, aaa, bbb, X[ 1], 12); FFF(bbb, ccc, ddd, eee, aaa, X[ 5],  5);
    FFF(aaa, bbb, ccc, ddd, eee, X[ 8], 14); FFF(eee, aaa, bbb, ccc, ddd, X[ 7],  6);
    FFF(ddd, eee, aaa, bbb, ccc, X[ 6],  8); FFF(ccc, ddd, eee, aaa, bbb, X[ 2], 13);
    FFF(bbb, ccc, ddd, eee, aaa, X[13],  6); FFF(aaa, bbb, ccc, ddd, eee, X[14],  5);
    FFF(eee, aaa, bbb, ccc, ddd, X[ 0], 15); FFF(ddd, eee, aaa, bbb, ccc, X[ 3], 13);
    FFF(ccc, ddd, eee, aaa, bbb, X[ 9], 11); FFF(bbb, ccc, ddd, eee, aaa, X[11], 11);
    
    // Combine results
    ddd += cc + state[1];
    state[1] = state[2] + dd + eee;
    state[2] = state[3] + ee + aaa;
    state[3] = state[4] + aa + bbb;
    state[4] = state[0] + bb + ccc;
    state[0] = ddd;
}

neoc_error_t neoc_ripemd160_hash(const uint8_t *data, size_t len, uint8_t hash[20]) {
    if (!data || !hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint32_t state[5] = {H0, H1, H2, H3, H4};
    uint8_t buffer[64];
    size_t buffer_len = 0;
    uint64_t total_len = 0;
    
    // Process input data
    while (len > 0) {
        size_t copy_len = (len > 64 - buffer_len) ? 64 - buffer_len : len;
        memcpy(buffer + buffer_len, data, copy_len);
        buffer_len += copy_len;
        data += copy_len;
        len -= copy_len;
        total_len += copy_len;
        
        if (buffer_len == 64) {
            ripemd160_compress(state, buffer);
            buffer_len = 0;
        }
    }
    
    // Add padding
    buffer[buffer_len++] = 0x80;
    if (buffer_len > 56) {
        while (buffer_len < 64) {
            buffer[buffer_len++] = 0;
        }
        ripemd160_compress(state, buffer);
        buffer_len = 0;
    }
    
    while (buffer_len < 56) {
        buffer[buffer_len++] = 0;
    }
    
    // Add length in bits (little-endian)
    uint64_t bit_len = total_len * 8;
    for (int i = 0; i < 8; i++) {
        buffer[56 + i] = (uint8_t)(bit_len >> (i * 8));
    }
    
    ripemd160_compress(state, buffer);
    
    // Output hash (little-endian)
    for (int i = 0; i < 5; i++) {
        hash[i*4] = (uint8_t)(state[i] & 0xFF);
        hash[i*4+1] = (uint8_t)((state[i] >> 8) & 0xFF);
        hash[i*4+2] = (uint8_t)((state[i] >> 16) & 0xFF);
        hash[i*4+3] = (uint8_t)((state[i] >> 24) & 0xFF);
    }
    
    return NEOC_SUCCESS;
}
