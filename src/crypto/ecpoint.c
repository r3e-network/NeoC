/**
 * @file ecpoint.c
 * @brief Elliptic Curve Point implementation
 * 
 * Based on Swift source: crypto/ECPoint.swift
 * Implements SECP256R1 curve point operations for Neo blockchain
 */

#include "../../include/neoc/neoc_error.h"
#include "../../include/neoc/neoc_memory.h"
#include "../../include/neoc/crypto/ecpoint.h"
#include "../../include/neoc/utils/neoc_hex.h"
#include <string.h>
#include <stdio.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/evp.h>

// SECP256R1 constants
#define EC_CURVE_NAME NID_X9_62_prime256v1
#define EC_POINT_COMPRESSED_SIZE 33
#define EC_POINT_UNCOMPRESSED_SIZE 65
#define EC_SCALAR_SIZE 32

// Additional error codes for crypto operations
#define NEOC_ERROR_INVALID_POINT NEOC_ERROR_INVALID_PARAM
#define NEOC_ERROR_CRYPTO_INIT NEOC_ERROR_INVALID_PARAM
#define NEOC_ERROR_CRYPTO_OPERATION NEOC_ERROR_INVALID_PARAM

/**
 * @brief Get the SECP256R1 EC_GROUP
 */
static EC_GROUP* get_secp256r1_group(void) {
    static EC_GROUP *group = NULL;
    if (!group) {
        group = EC_GROUP_new_by_curve_name(EC_CURVE_NAME);
    }
    return group;
}

/**
 * @brief Create an EC point from encoded bytes
 */
neoc_error_t neoc_ec_point_create(const uint8_t *encoded, size_t encoded_len,
                                   neoc_ec_point_t **point) {
    if (!encoded || !point || 
        (encoded_len != EC_POINT_COMPRESSED_SIZE && encoded_len != EC_POINT_UNCOMPRESSED_SIZE)) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *point = NULL;
    
    // Validate point format
    if (encoded_len == EC_POINT_COMPRESSED_SIZE) {
        if (encoded[0] != 0x02 && encoded[0] != 0x03) {
            return NEOC_ERROR_INVALID_POINT;
        }
    } else if (encoded_len == EC_POINT_UNCOMPRESSED_SIZE) {
        if (encoded[0] != 0x04) {
            return NEOC_ERROR_INVALID_POINT;
        }
    }
    
    // Create EC point structure
    neoc_ec_point_t *new_point = neoc_malloc(sizeof(neoc_ec_point_t));
    if (!new_point) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Copy encoded data
    new_point->encoded = neoc_malloc(encoded_len);
    if (!new_point->encoded) {
        neoc_free(new_point);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(new_point->encoded, encoded, encoded_len);
    new_point->encoded_len = encoded_len;
    new_point->is_infinity = false;
    
    // Validate that the point is on the curve
    EC_GROUP *group = get_secp256r1_group();
    if (!group) {
        neoc_ec_point_free(new_point);
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    EC_POINT *ec_point = EC_POINT_new(group);
    if (!ec_point) {
        neoc_ec_point_free(new_point);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) {
        EC_POINT_free(ec_point);
        neoc_ec_point_free(new_point);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    int result = EC_POINT_oct2point(group, ec_point, encoded, encoded_len, ctx);
    
    BN_CTX_free(ctx);
    EC_POINT_free(ec_point);
    
    if (result != 1) {
        neoc_ec_point_free(new_point);
        return NEOC_ERROR_INVALID_POINT;
    }
    
    *point = new_point;
    return NEOC_SUCCESS;
}

/**
 * @brief Create an EC point from hex string
 */
neoc_error_t neoc_ec_point_from_hex(const char *hex_string, neoc_ec_point_t **point) {
    if (!hex_string || !point) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    size_t bytes_len = 0;
    uint8_t *bytes = neoc_hex_decode_alloc(hex_string, &bytes_len);
    if (!bytes) {
        return NEOC_ERROR_INVALID_HEX;
    }
    
    neoc_error_t error = neoc_ec_point_create(bytes, bytes_len, point);
    neoc_free(bytes);
    
    return error;
}

/**
 * @brief Create the point at infinity
 */
neoc_error_t neoc_ec_point_create_infinity(neoc_ec_point_t **point) {
    if (!point) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *point = neoc_malloc(sizeof(neoc_ec_point_t));
    if (!*point) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    (*point)->encoded = NULL;
    (*point)->encoded_len = 0;
    (*point)->is_infinity = true;
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get the encoded point data
 */
neoc_error_t neoc_ec_point_get_encoded(const neoc_ec_point_t *point, bool compressed,
                                        uint8_t **encoded, size_t *encoded_len) {
    if (!point || !encoded || !encoded_len) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *encoded = NULL;
    *encoded_len = 0;
    
    if (point->is_infinity) {
        return NEOC_ERROR_INVALID_POINT;
    }
    
    // If requested format matches stored format, return copy
    bool stored_compressed = (point->encoded_len == EC_POINT_COMPRESSED_SIZE);
    if (compressed == stored_compressed) {
        *encoded = neoc_malloc(point->encoded_len);
        if (!*encoded) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(*encoded, point->encoded, point->encoded_len);
        *encoded_len = point->encoded_len;
        return NEOC_SUCCESS;
    }
    
    // Need to convert format
    EC_GROUP *group = get_secp256r1_group();
    if (!group) {
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    EC_POINT *ec_point = EC_POINT_new(group);
    if (!ec_point) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) {
        EC_POINT_free(ec_point);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Convert stored point to EC_POINT
    int result = EC_POINT_oct2point(group, ec_point, point->encoded, point->encoded_len, ctx);
    if (result != 1) {
        BN_CTX_free(ctx);
        EC_POINT_free(ec_point);
        return NEOC_ERROR_INVALID_POINT;
    }
    
    // Convert to requested format
    point_conversion_form_t form = compressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED;
    size_t output_len = compressed ? EC_POINT_COMPRESSED_SIZE : EC_POINT_UNCOMPRESSED_SIZE;
    
    *encoded = neoc_malloc(output_len);
    if (!*encoded) {
        BN_CTX_free(ctx);
        EC_POINT_free(ec_point);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    size_t actual_len = EC_POINT_point2oct(group, ec_point, form, *encoded, output_len, ctx);
    
    BN_CTX_free(ctx);
    EC_POINT_free(ec_point);
    
    if (actual_len != output_len) {
        neoc_free(*encoded);
        *encoded = NULL;
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    *encoded_len = output_len;
    return NEOC_SUCCESS;
}

/**
 * @brief Multiply the point by a scalar (k * point)
 */
neoc_error_t neoc_ec_point_multiply(const neoc_ec_point_t *point, const uint8_t *k,
                                     neoc_ec_point_t **result) {
    if (!point || !k || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *result = NULL;
    
    if (point->is_infinity) {
        return neoc_ec_point_create_infinity(result);
    }
    
    EC_GROUP *group = get_secp256r1_group();
    if (!group) {
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    EC_POINT *ec_point = EC_POINT_new(group);
    EC_POINT *result_point = EC_POINT_new(group);
    BIGNUM *scalar = BN_new();
    BN_CTX *ctx = BN_CTX_new();
    
    if (!ec_point || !result_point || !scalar || !ctx) {
        EC_POINT_free(ec_point);
        EC_POINT_free(result_point);
        BN_free(scalar);
        BN_CTX_free(ctx);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Convert stored point to EC_POINT
    int ec_result = EC_POINT_oct2point(group, ec_point, point->encoded, point->encoded_len, ctx);
    if (ec_result != 1) {
        EC_POINT_free(ec_point);
        EC_POINT_free(result_point);
        BN_free(scalar);
        BN_CTX_free(ctx);
        return NEOC_ERROR_INVALID_POINT;
    }
    
    // Convert scalar bytes to BIGNUM
    BN_bin2bn(k, EC_SCALAR_SIZE, scalar);
    
    // Perform scalar multiplication
    ec_result = EC_POINT_mul(group, result_point, NULL, ec_point, scalar, ctx);
    if (ec_result != 1) {
        EC_POINT_free(ec_point);
        EC_POINT_free(result_point);
        BN_free(scalar);
        BN_CTX_free(ctx);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Convert result back to encoded form (use same compression as input)
    bool compressed = (point->encoded_len == EC_POINT_COMPRESSED_SIZE);
    point_conversion_form_t form = compressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED;
    size_t output_len = compressed ? EC_POINT_COMPRESSED_SIZE : EC_POINT_UNCOMPRESSED_SIZE;
    
    uint8_t *encoded_result = neoc_malloc(output_len);
    if (!encoded_result) {
        EC_POINT_free(ec_point);
        EC_POINT_free(result_point);
        BN_free(scalar);
        BN_CTX_free(ctx);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    size_t actual_len = EC_POINT_point2oct(group, result_point, form, encoded_result, output_len, ctx);
    
    EC_POINT_free(ec_point);
    EC_POINT_free(result_point);
    BN_free(scalar);
    BN_CTX_free(ctx);
    
    if (actual_len != output_len) {
        neoc_free(encoded_result);
        return NEOC_ERROR_CRYPTO_OPERATION;
    }
    
    // Create result point
    neoc_error_t error = neoc_ec_point_create(encoded_result, output_len, result);
    neoc_free(encoded_result);
    
    return error;
}

/**
 * @brief Check if the point is at infinity
 */
neoc_error_t neoc_ec_point_is_infinity(const neoc_ec_point_t *point, bool *is_infinity) {
    if (!point || !is_infinity) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *is_infinity = point->is_infinity;
    return NEOC_SUCCESS;
}

/**
 * @brief Check if the point is valid (on the curve)
 */
neoc_error_t neoc_ec_point_is_valid(const neoc_ec_point_t *point, bool *is_valid) {
    if (!point || !is_valid) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *is_valid = false;
    
    if (point->is_infinity) {
        *is_valid = true;
        return NEOC_SUCCESS;
    }
    
    if (!point->encoded) {
        return NEOC_SUCCESS;
    }
    
    EC_GROUP *group = get_secp256r1_group();
    if (!group) {
        return NEOC_ERROR_CRYPTO_INIT;
    }
    
    EC_POINT *ec_point = EC_POINT_new(group);
    if (!ec_point) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    BN_CTX *ctx = BN_CTX_new();
    if (!ctx) {
        EC_POINT_free(ec_point);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    int result = EC_POINT_oct2point(group, ec_point, point->encoded, point->encoded_len, ctx);
    if (result == 1) {
        result = EC_POINT_is_on_curve(group, ec_point, ctx);
        *is_valid = (result == 1);
    }
    
    BN_CTX_free(ctx);
    EC_POINT_free(ec_point);
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get the X coordinate of the point
 */
neoc_error_t neoc_ec_point_get_x(const neoc_ec_point_t *point, uint8_t *x) {
    if (!point || !x || point->is_infinity) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (point->encoded_len == EC_POINT_COMPRESSED_SIZE) {
        // X coordinate is bytes 1-32 for compressed format
        memcpy(x, point->encoded + 1, EC_SCALAR_SIZE);
        return NEOC_SUCCESS;
    } else if (point->encoded_len == EC_POINT_UNCOMPRESSED_SIZE) {
        // X coordinate is bytes 1-32 for uncompressed format
        memcpy(x, point->encoded + 1, EC_SCALAR_SIZE);
        return NEOC_SUCCESS;
    }
    
    return NEOC_ERROR_INVALID_POINT;
}

/**
 * @brief Get the Y coordinate of the point
 */
neoc_error_t neoc_ec_point_get_y(const neoc_ec_point_t *point, uint8_t *y) {
    if (!point || !y || point->is_infinity) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (point->encoded_len == EC_POINT_UNCOMPRESSED_SIZE) {
        // Y coordinate is bytes 33-64 for uncompressed format
        memcpy(y, point->encoded + 1 + EC_SCALAR_SIZE, EC_SCALAR_SIZE);
        return NEOC_SUCCESS;
    } else if (point->encoded_len == EC_POINT_COMPRESSED_SIZE) {
        // Need to decompress to get Y coordinate
        uint8_t *uncompressed = NULL;
        size_t uncompressed_len = 0;
        
        neoc_error_t error = neoc_ec_point_get_encoded(point, false, &uncompressed, &uncompressed_len);
        if (error != NEOC_SUCCESS) {
            return error;
        }
        
        memcpy(y, uncompressed + 1 + EC_SCALAR_SIZE, EC_SCALAR_SIZE);
        neoc_free(uncompressed);
        return NEOC_SUCCESS;
    }
    
    return NEOC_ERROR_INVALID_POINT;
}

/**
 * @brief Compare two EC points for equality
 */
neoc_error_t neoc_ec_point_equals(const neoc_ec_point_t *point1, const neoc_ec_point_t *point2,
                                   bool *are_equal) {
    if (!point1 || !point2 || !are_equal) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *are_equal = false;
    
    // Both at infinity
    if (point1->is_infinity && point2->is_infinity) {
        *are_equal = true;
        return NEOC_SUCCESS;
    }
    
    // One at infinity, other not
    if (point1->is_infinity || point2->is_infinity) {
        return NEOC_SUCCESS;
    }
    
    // Compare X coordinates (both points should have valid X)
    uint8_t x1[EC_SCALAR_SIZE], x2[EC_SCALAR_SIZE];
    
    neoc_error_t error = neoc_ec_point_get_x(point1, x1);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    error = neoc_ec_point_get_x(point2, x2);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    if (memcmp(x1, x2, EC_SCALAR_SIZE) != 0) {
        return NEOC_SUCCESS; // X coordinates different
    }
    
    // X coordinates match, compare Y coordinates
    uint8_t y1[EC_SCALAR_SIZE], y2[EC_SCALAR_SIZE];
    
    error = neoc_ec_point_get_y(point1, y1);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    error = neoc_ec_point_get_y(point2, y2);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    *are_equal = (memcmp(y1, y2, EC_SCALAR_SIZE) == 0);
    return NEOC_SUCCESS;
}

/**
 * @brief Convert EC point to hex string
 */
neoc_error_t neoc_ec_point_to_hex(const neoc_ec_point_t *point, bool compressed,
                                   char **hex_string) {
    if (!point || !hex_string) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *hex_string = NULL;
    
    if (point->is_infinity) {
        return NEOC_ERROR_INVALID_POINT;
    }
    
    uint8_t *encoded = NULL;
    size_t encoded_len = 0;
    
    neoc_error_t error = neoc_ec_point_get_encoded(point, compressed, &encoded, &encoded_len);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    *hex_string = neoc_hex_encode_alloc(encoded, encoded_len, false, false);
    neoc_free(encoded);
    
    return *hex_string ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

/**
 * @brief Free an EC point
 */
void neoc_ec_point_free(neoc_ec_point_t *point) {
    if (!point) {
        return;
    }
    
    neoc_free(point->encoded);
    neoc_free(point);
}

