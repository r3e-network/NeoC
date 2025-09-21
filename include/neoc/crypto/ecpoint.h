/**
 * @file ecpoint.h
 * @brief Elliptic Curve Point implementation for Neo C SDK
 * 
 * Based on Swift ECPoint extension and SwiftECC Point type
 */

#ifndef NEOC_CRYPTO_ECPOINT_H
#define NEOC_CRYPTO_ECPOINT_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Elliptic Curve Point structure
 */
typedef struct {
    uint8_t *encoded;     /**< Encoded point data (compressed or uncompressed) */
    size_t encoded_len;   /**< Length of encoded data (33 or 65 bytes) */
    bool is_infinity;     /**< True if this is the point at infinity */
} neoc_ec_point_t;

/**
 * @brief Create an EC point from encoded bytes
 * 
 * @param encoded The encoded point (33 bytes compressed or 65 bytes uncompressed)
 * @param encoded_len Length of encoded data
 * @param point Output EC point (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_create(const uint8_t *encoded, size_t encoded_len,
                                   neoc_ec_point_t **point);

/**
 * @brief Create an EC point from hex string
 * 
 * @param hex_string Hex-encoded point data
 * @param point Output EC point (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_from_hex(const char *hex_string, neoc_ec_point_t **point);

/**
 * @brief Create the point at infinity
 * 
 * @param point Output EC point (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_create_infinity(neoc_ec_point_t **point);

/**
 * @brief Get the encoded point data
 * 
 * @param point The EC point
 * @param compressed Whether to return compressed format
 * @param encoded Output encoded data (caller must free)
 * @param encoded_len Output length of encoded data
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_get_encoded(const neoc_ec_point_t *point, bool compressed,
                                        uint8_t **encoded, size_t *encoded_len);

/**
 * @brief Multiply the point by a scalar (k * point)
 * 
 * @param point The EC point
 * @param k The scalar (32 bytes, big endian)
 * @param result Output result point (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_multiply(const neoc_ec_point_t *point, const uint8_t *k,
                                     neoc_ec_point_t **result);

/**
 * @brief Check if the point is at infinity
 * 
 * @param point The EC point
 * @param is_infinity Output infinity status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_is_infinity(const neoc_ec_point_t *point, bool *is_infinity);

/**
 * @brief Check if the point is valid (on the curve)
 * 
 * @param point The EC point
 * @param is_valid Output validity status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_is_valid(const neoc_ec_point_t *point, bool *is_valid);

/**
 * @brief Get the X coordinate of the point
 * 
 * @param point The EC point
 * @param x Output X coordinate (32 bytes, caller must provide buffer)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_get_x(const neoc_ec_point_t *point, uint8_t *x);

/**
 * @brief Get the Y coordinate of the point
 * 
 * @param point The EC point
 * @param y Output Y coordinate (32 bytes, caller must provide buffer)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_get_y(const neoc_ec_point_t *point, uint8_t *y);

/**
 * @brief Compare two EC points for equality
 * 
 * @param point1 First EC point
 * @param point2 Second EC point
 * @param are_equal Output equality status
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_equals(const neoc_ec_point_t *point1, const neoc_ec_point_t *point2,
                                   bool *are_equal);

/**
 * @brief Convert EC point to hex string
 * 
 * @param point The EC point
 * @param compressed Whether to use compressed format
 * @param hex_string Output hex string (caller must free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_ec_point_to_hex(const neoc_ec_point_t *point, bool compressed,
                                   char **hex_string);

/**
 * @brief Free an EC point
 * 
 * @param point The EC point to free
 */
void neoc_ec_point_free(neoc_ec_point_t *point);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_CRYPTO_ECPOINT_H */
