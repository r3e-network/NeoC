/**
 * @file populated_blocks.h
 * @brief Populated blocks structure for caching
 * 
 * Converted from Swift source: protocol/core/response/PopulatedBlocks.swift
 * Represents cached block information with cache ID and block indices.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_POPULATED_BLOCKS_H
#define NEOC_PROTOCOL_CORE_RESPONSE_POPULATED_BLOCKS_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Populated blocks structure
 * 
 * Represents cached block information with a cache ID and array of block indices.
 */
typedef struct {
    char *cache_id;                     /**< Cache identifier string */
    int *blocks;                        /**< Array of block indices */
    size_t blocks_count;                /**< Number of block indices */
} neoc_populated_blocks_t;

/**
 * @brief JSON-RPC response for populated blocks
 * 
 * Complete response structure for populated blocks queries.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_populated_blocks_t *result;    /**< Populated blocks result (NULL on error) */
    char *error;                        /**< Error message (NULL on success) */
    int error_code;                     /**< Error code (0 on success) */
} neoc_populated_blocks_response_t;

/**
 * @brief Create a new populated blocks structure
 * 
 * @param cache_id Cache identifier string
 * @param blocks Array of block indices
 * @param blocks_count Number of block indices
 * @param populated_blocks Output pointer for the created structure (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_populated_blocks_create(
    const char *cache_id,
    const int *blocks,
    size_t blocks_count,
    neoc_populated_blocks_t **populated_blocks
);

/**
 * @brief Free a populated blocks structure
 * 
 * @param populated_blocks Structure to free (can be NULL)
 */
void neoc_populated_blocks_free(
    neoc_populated_blocks_t *populated_blocks
);

/**
 * @brief Create a new populated blocks response
 * 
 * @param jsonrpc JSON-RPC version string
 * @param id Request ID
 * @param result Populated blocks result (can be NULL for error responses)
 * @param error Error message (can be NULL for successful responses)
 * @param error_code Error code (0 for success)
 * @param response Output pointer for the created response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_populated_blocks_response_create(
    const char *jsonrpc,
    int id,
    neoc_populated_blocks_t *result,
    const char *error,
    int error_code,
    neoc_populated_blocks_response_t **response
);

/**
 * @brief Free a populated blocks response
 * 
 * @param response Response to free (can be NULL)
 */
void neoc_populated_blocks_response_free(
    neoc_populated_blocks_response_t *response
);

/**
 * @brief Parse populated blocks from JSON string
 * 
 * @param json_str JSON string containing populated blocks data
 * @param populated_blocks Output pointer for the parsed structure (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_populated_blocks_from_json(
    const char *json_str,
    neoc_populated_blocks_t **populated_blocks
);

/**
 * @brief Parse populated blocks response from JSON string
 * 
 * @param json_str JSON string containing response data
 * @param response Output pointer for the parsed response (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_populated_blocks_response_from_json(
    const char *json_str,
    neoc_populated_blocks_response_t **response
);

/**
 * @brief Convert populated blocks to JSON string
 * 
 * @param populated_blocks Structure to convert
 * @param json_str Output pointer for JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_populated_blocks_to_json(
    const neoc_populated_blocks_t *populated_blocks,
    char **json_str
);

/**
 * @brief Create a copy of populated blocks
 * 
 * @param src Source structure to copy
 * @param dest Output pointer for the copied structure (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_populated_blocks_copy(
    const neoc_populated_blocks_t *src,
    neoc_populated_blocks_t **dest
);

/**
 * @brief Check if a specific block is in the populated blocks list
 * 
 * @param populated_blocks Populated blocks structure
 * @param block_index Block index to search for
 * @return true if block is in the list, false otherwise
 */
bool neoc_populated_blocks_contains_block(
    const neoc_populated_blocks_t *populated_blocks,
    int block_index
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_POPULATED_BLOCKS_H */
