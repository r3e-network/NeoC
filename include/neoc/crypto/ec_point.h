/**
 * @file e_c_point.h
 * @brief Elliptic curve point operations
 */

#ifndef NEOC_E_C_POINT_H
#define NEOC_E_C_POINT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Elliptic curve point structure
 */
typedef struct neoc_ec_point {
    uint8_t x[32];           // X coordinate
    uint8_t y[32];           // Y coordinate
    bool is_infinity;        // Point at infinity flag
    bool compressed;         // Whether point is in compressed form
} neoc_ec_point_t;

/**
 * Create EC point from coordinates
 */
neoc_error_t neoc_ec_point_create(const uint8_t *x, const uint8_t *y,
                                   neoc_ec_point_t **point);

/**
 * Create EC point from encoded bytes
 */
neoc_error_t neoc_ec_point_from_bytes(const uint8_t *encoded, size_t len,
                                       neoc_ec_point_t **point);

/**
 * Create point at infinity
 */
neoc_error_t neoc_ec_point_infinity(neoc_ec_point_t **point);

/**
 * Encode point to bytes
 */
neoc_error_t neoc_ec_point_encode(neoc_ec_point_t *point, bool compressed,
                                   uint8_t **encoded, size_t *len);

/**
 * Add two points
 */
neoc_error_t neoc_ec_point_add(neoc_ec_point_t *p1, neoc_ec_point_t *p2,
                                neoc_ec_point_t **result);

/**
 * Multiply point by scalar
 */
neoc_error_t neoc_ec_point_multiply(neoc_ec_point_t *point, const uint8_t *scalar,
                                     neoc_ec_point_t **result);

/**
 * Check if point is valid on curve
 */
bool neoc_ec_point_is_valid(neoc_ec_point_t *point);

/**
 * Check if point is at infinity
 */
bool neoc_ec_point_is_infinity(neoc_ec_point_t *point);

/**
 * Compare two points
 */
bool neoc_ec_point_equals(neoc_ec_point_t *p1, neoc_ec_point_t *p2);

/**
 * Negate point
 */
neoc_error_t neoc_ec_point_negate(neoc_ec_point_t *point,
                                   neoc_ec_point_t **result);

/**
 * Get X coordinate
 */
neoc_error_t neoc_ec_point_get_x(neoc_ec_point_t *point, uint8_t *x);

/**
 * Get Y coordinate
 */
neoc_error_t neoc_ec_point_get_y(neoc_ec_point_t *point, uint8_t *y);

/**
 * Free EC point
 */
void neoc_ec_point_free(neoc_ec_point_t *point);

#ifdef __cplusplus
}
#endif

#endif // NEOC_E_C_POINT_H
