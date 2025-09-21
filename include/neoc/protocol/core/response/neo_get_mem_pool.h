/**
 * @file neo_get_mem_pool.h
 * @brief Neo memory pool response types and utilities
 * 
 * Converted from Swift source: protocol/core/response/NeoGetMemPool.swift
 * Provides functionality for retrieving memory pool transaction information.
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_MEM_POOL_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_MEM_POOL_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash256.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents memory pool transaction details
 * 
 * This structure contains information about transactions in the memory pool,
 * categorized by verification status and including the current height.
 */
typedef struct {
    int height;                         /**< Current block height */
    neoc_hash256_t *verified;           /**< Array of verified transaction hashes */
    size_t verified_count;              /**< Number of verified transactions */
    neoc_hash256_t *unverified;         /**< Array of unverified transaction hashes */
    size_t unverified_count;            /**< Number of unverified transactions */
} neoc_mem_pool_details_t;

/**
 * @brief Response structure for getrawmempool RPC call
 * 
 * Contains the memory pool details returned by the Neo node.
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                             /**< Request ID */
    neoc_mem_pool_details_t *result;    /**< Memory pool details (NULL if error) */
    char *error;                        /**< Error message (NULL if success) */
    int error_code;                     /**< Error code (0 if success) */
} neoc_neo_get_mem_pool_t;

/**
 * @brief Create a new memory pool details structure
 * 
 * @param height Current block height
 * @param verified Array of verified transaction hashes
 * @param verified_count Number of verified transactions
 * @param unverified Array of unverified transaction hashes
 * @param unverified_count Number of unverified transactions
 * @param details Pointer to store the created details (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_mem_pool_details_create(
    int height,
    const neoc_hash256_t *verified,
    size_t verified_count,
    const neoc_hash256_t *unverified,
    size_t unverified_count,
    neoc_mem_pool_details_t **details
);

/**
 * @brief Free a memory pool details structure
 * 
 * @param details Details to free
 */
void neoc_mem_pool_details_free(neoc_mem_pool_details_t *details);

/**
 * @brief Create a new Neo get memory pool response
 * 
 * @param id Request ID
 * @param mem_pool_details Memory pool details (can be NULL for error responses)
 * @param error Error message (can be NULL for success responses)
 * @param error_code Error code (0 for success)
 * @param response Pointer to store the created response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_mem_pool_create(
    int id,
    neoc_mem_pool_details_t *mem_pool_details,
    const char *error,
    int error_code,
    neoc_neo_get_mem_pool_t **response
);

/**
 * @brief Free a Neo get memory pool response
 * 
 * @param response Response to free
 */
void neoc_neo_get_mem_pool_free(neoc_neo_get_mem_pool_t *response);

/**
 * @brief Parse JSON response into Neo get memory pool structure
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_mem_pool_from_json(
    const char *json_str,
    neoc_neo_get_mem_pool_t **response
);

/**
 * @brief Convert Neo get memory pool response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (output, must be freed)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_mem_pool_to_json(
    const neoc_neo_get_mem_pool_t *response,
    char **json_str
);

/**
 * @brief Get memory pool details from response (convenience function)
 * 
 * @param response Response to get details from
 * @return Memory pool details pointer, or NULL if error or no result
 */
neoc_mem_pool_details_t *neoc_neo_get_mem_pool_get_details(
    const neoc_neo_get_mem_pool_t *response
);

/**
 * @brief Check if the response contains valid memory pool data
 * 
 * @param response Response to check
 * @return true if response has valid details, false otherwise
 */
bool neoc_neo_get_mem_pool_has_details(
    const neoc_neo_get_mem_pool_t *response
);

/**
 * @brief Get current block height from response
 * 
 * @param response Response to get height from
 * @param height Pointer to store the height (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_mem_pool_get_height(
    const neoc_neo_get_mem_pool_t *response,
    int *height
);

/**
 * @brief Get total number of transactions in memory pool
 * 
 * @param response Response to count transactions from
 * @return Total number of transactions (verified + unverified), or 0 on error
 */
size_t neoc_neo_get_mem_pool_get_total_count(
    const neoc_neo_get_mem_pool_t *response
);

/**
 * @brief Get number of verified transactions
 * 
 * @param response Response to count verified transactions from
 * @return Number of verified transactions, or 0 on error
 */
size_t neoc_neo_get_mem_pool_get_verified_count(
    const neoc_neo_get_mem_pool_t *response
);

/**
 * @brief Get number of unverified transactions
 * 
 * @param response Response to count unverified transactions from
 * @return Number of unverified transactions, or 0 on error
 */
size_t neoc_neo_get_mem_pool_get_unverified_count(
    const neoc_neo_get_mem_pool_t *response
);

/**
 * @brief Get verified transaction by index
 * 
 * @param response Response to get transaction from
 * @param index Index of the verified transaction
 * @param hash Pointer to store the transaction hash (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_mem_pool_get_verified_transaction(
    const neoc_neo_get_mem_pool_t *response,
    size_t index,
    neoc_hash256_t *hash
);

/**
 * @brief Get unverified transaction by index
 * 
 * @param response Response to get transaction from
 * @param index Index of the unverified transaction
 * @param hash Pointer to store the transaction hash (output)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_neo_get_mem_pool_get_unverified_transaction(
    const neoc_neo_get_mem_pool_t *response,
    size_t index,
    neoc_hash256_t *hash
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_GET_MEM_POOL_H */
