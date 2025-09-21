/**
 * @file r_i_p_e_m_d160.h
 * @brief RIPEMD-160 hash implementation
 */

#ifndef NEOC_R_I_P_E_M_D160_H
#define NEOC_R_I_P_E_M_D160_H

#include <stdint.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEOC_RIPEMD160_DIGEST_SIZE 20
#define NEOC_RIPEMD160_BLOCK_SIZE 64

/**
 * RIPEMD-160 context
 */
typedef struct neoc_ripemd160_ctx {
    uint32_t state[5];
    uint64_t count;
    uint8_t buffer[NEOC_RIPEMD160_BLOCK_SIZE];
} neoc_ripemd160_ctx_t;

/**
 * Initialize RIPEMD-160 context
 */
neoc_error_t neoc_ripemd160_init(neoc_ripemd160_ctx_t *ctx);

/**
 * Update RIPEMD-160 hash
 */
neoc_error_t neoc_ripemd160_update(neoc_ripemd160_ctx_t *ctx,
                                    const uint8_t *data, size_t len);

/**
 * Finalize RIPEMD-160 hash
 */
neoc_error_t neoc_ripemd160_final(neoc_ripemd160_ctx_t *ctx, uint8_t *hash);

/**
 * One-shot RIPEMD-160 hash
 */
neoc_error_t neoc_ripemd160(const uint8_t *data, size_t len, uint8_t *hash);

#ifdef __cplusplus
}
#endif

#endif // NEOC_R_I_P_E_M_D160_H
