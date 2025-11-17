/**
 * @file ripemd160.c
 * @brief Portable RIPEMD-160 implementation used for internal hashing.
 */

#include "neoc/crypto/helpers/ripemd160.h"
#include "neoc/neoc_memory.h"
#include <string.h>

/* RIPEMD-160 constants and functions adapted from public domain reference */

typedef struct {
    uint32_t state[5];
    uint64_t count;
    uint8_t buffer[64];
} ripemd160_ctx;

static uint32_t rol(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }

static void init(ripemd160_ctx *ctx) {
    ctx->state[0] = 0x67452301UL;
    ctx->state[1] = 0xEFCDAB89UL;
    ctx->state[2] = 0x98BADCFEUL;
    ctx->state[3] = 0x10325476UL;
    ctx->state[4] = 0xC3D2E1F0UL;
    ctx->count = 0;
}

#define F(x, y, z) ((x) ^ (y) ^ (z))
#define G(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z) (((x) | ~(y)) ^ (z))
#define I(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z) ((x) ^ ((y) | ~(z)))

static const uint32_t K1[5] = {0x00000000UL, 0x5A827999UL, 0x6ED9EBA1UL, 0x8F1BBCDCUL, 0xA953FD4EUL};
static const uint32_t K2[5] = {0x50A28BE6UL, 0x5C4DD124UL, 0x6D703EF3UL, 0x7A6D76E9UL, 0x00000000UL};

static void compress(uint32_t state[5], const uint8_t block[64]) {
    static const uint32_t r1[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8,
        3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12,
        1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2,
        4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
    };
    static const uint32_t r2[] = {
        5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12,
        6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2,
        15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13,
        8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14,
        12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
    };
    static const uint32_t s1[] = {
        11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8,
        7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12,
        11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5,
        11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12,
        9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
    };
    static const uint32_t s2[] = {
        8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6,
        9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11,
        9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5,
        15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8,
        8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
    };

    uint32_t X[16];
    for (int i = 0; i < 16; ++i) {
        X[i] = (uint32_t)block[4 * i] |
               ((uint32_t)block[4 * i + 1] << 8) |
               ((uint32_t)block[4 * i + 2] << 16) |
               ((uint32_t)block[4 * i + 3] << 24);
    }

    uint32_t a1 = state[0], b1 = state[1], c1 = state[2], d1 = state[3], e1 = state[4];
    uint32_t a2 = state[0], b2 = state[1], c2 = state[2], d2 = state[3], e2 = state[4];

    for (int i = 0; i < 80; ++i) {
        uint32_t t;
        uint32_t s;
        uint32_t r;
        if (i < 16) { t = F(b1, c1, d1) + X[r1[i]] + K1[0]; s = s1[i]; }
        else if (i < 32) { t = G(b1, c1, d1) + X[r1[i]] + K1[1]; s = s1[i]; }
        else if (i < 48) { t = H(b1, c1, d1) + X[r1[i]] + K1[2]; s = s1[i]; }
        else if (i < 64) { t = I(b1, c1, d1) + X[r1[i]] + K1[3]; s = s1[i]; }
        else { t = J(b1, c1, d1) + X[r1[i]] + K1[4]; s = s1[i]; }
        t = rol(t + a1, s) + e1; a1 = e1; e1 = d1; d1 = rol(c1, 10); c1 = b1; b1 = t;

        if (i < 16) { t = J(b2, c2, d2) + X[r2[i]] + K2[0]; s = s2[i]; }
        else if (i < 32) { t = I(b2, c2, d2) + X[r2[i]] + K2[1]; s = s2[i]; }
        else if (i < 48) { t = H(b2, c2, d2) + X[r2[i]] + K2[2]; s = s2[i]; }
        else if (i < 64) { t = G(b2, c2, d2) + X[r2[i]] + K2[3]; s = s2[i]; }
        else { t = F(b2, c2, d2) + X[r2[i]] + K2[4]; s = s2[i]; }
        t = rol(t + a2, s) + e2; a2 = e2; e2 = d2; d2 = rol(c2, 10); c2 = b2; b2 = t;
    }

    uint32_t temp = state[1] + c1 + d2;
    state[1] = state[2] + d1 + e2;
    state[2] = state[3] + e1 + a2;
    state[3] = state[4] + a1 + b2;
    state[4] = state[0] + b1 + c2;
    state[0] = temp;
}

static void update(ripemd160_ctx *ctx, const uint8_t *data, size_t len) {
    size_t idx = (size_t)(ctx->count & 63);
    ctx->count += len;
    size_t part_len = 64 - idx;
    size_t i = 0;
    if (len >= part_len) {
        memcpy(ctx->buffer + idx, data, part_len);
        compress(ctx->state, ctx->buffer);
        for (i = part_len; i + 63 < len; i += 64) {
            compress(ctx->state, data + i);
        }
        idx = 0;
    }
    memcpy(ctx->buffer + idx, data + i, len - i);
}

static void final(ripemd160_ctx *ctx, uint8_t digest[20]) {
    uint8_t padding[64] = {0x80};
    uint8_t length[8];
    uint64_t bits = ctx->count * 8;
    for (int i = 0; i < 8; ++i) {
        length[i] = (uint8_t)(bits >> (8 * i));
    }

    size_t idx = (size_t)(ctx->count & 63);
    size_t pad_len = (idx < 56) ? (56 - idx) : (120 - idx);
    update(ctx, padding, pad_len);
    update(ctx, length, 8);

    for (int i = 0; i < 5; ++i) {
        digest[4 * i]     = (uint8_t)(ctx->state[i] & 0xFF);
        digest[4 * i + 1] = (uint8_t)((ctx->state[i] >> 8) & 0xFF);
        digest[4 * i + 2] = (uint8_t)((ctx->state[i] >> 16) & 0xFF);
        digest[4 * i + 3] = (uint8_t)((ctx->state[i] >> 24) & 0xFF);
    }
}

neoc_error_t neoc_ripemd160_hash(const uint8_t *data, size_t len, uint8_t hash[20]) {
    if (!hash || (!data && len > 0)) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid RIPEMD-160 input");
    }

    ripemd160_ctx ctx;
    init(&ctx);
    if (len > 0) {
        update(&ctx, data, len);
    }
    final(&ctx, hash);
    return NEOC_SUCCESS;
}
