/**
 * @file nef_file.h
 * @brief Neo Executable Format (NEF) file support
 */

#ifndef NEOC_NEF_FILE_H
#define NEOC_NEF_FILE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "neoc/neoc_error.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"

#ifdef __cplusplus
extern "C" {
#endif

// NEF magic number
#define NEF_MAGIC 0x3346454E  // "NEF3" in little-endian

// Maximum sizes
#define NEF_MAX_SCRIPT_SIZE 512000     // 512 KB max script size
#define NEF_MAX_SOURCE_SIZE 255         // Max source URL length
#define NEF_MAX_TOKENS 128              // Max method tokens

/**
 * @brief NEF file header structure
 */
typedef struct {
    uint32_t magic;             // Magic number (0x3346454E)
    char compiler[64];          // Compiler name and version
    char source[256];           // Source URL or identifier
    uint8_t reserved[2];        // Reserved bytes (must be 0)
    uint8_t method_tokens_count; // Number of method tokens
    uint8_t reserved2;          // Reserved byte (must be 0)
} neoc_nef_header_t;

/**
 * @brief Method token for external contract calls
 */
typedef struct {
    neoc_hash160_t hash;        // Contract script hash
    char method[32];            // Method name
    uint16_t parameters_count;  // Number of parameters
    bool has_return_value;      // Whether method returns a value
    uint8_t call_flags;         // Call flags
} neoc_nef_method_token_t;

/**
 * @brief NEF file structure
 */
typedef struct neoc_nef_file_t neoc_nef_file_t;

/**
 * @brief Create a new NEF file
 * 
 * @param compiler Compiler name and version
 * @param source Source URL or identifier (can be NULL)
 * @param script Contract script bytecode
 * @param script_size Size of script in bytes
 * @param nef Output NEF file handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_create(const char *compiler,
                                   const char *source,
                                   const uint8_t *script,
                                   size_t script_size,
                                   neoc_nef_file_t **nef);

/**
 * @brief Load NEF file from bytes
 * 
 * @param data NEF file data
 * @param data_size Size of data
 * @param nef Output NEF file handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_from_bytes(const uint8_t *data,
                                       size_t data_size,
                                       neoc_nef_file_t **nef);

/**
 * @brief Load NEF file from file
 * 
 * @param filename Path to NEF file
 * @param nef Output NEF file handle
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_from_file(const char *filename,
                                      neoc_nef_file_t **nef);

/**
 * @brief Serialize NEF file to bytes
 * 
 * @param nef NEF file handle
 * @param data Output data buffer (caller must free)
 * @param data_size Output data size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_to_bytes(const neoc_nef_file_t *nef,
                                     uint8_t **data,
                                     size_t *data_size);

/**
 * @brief Save NEF file to file
 * 
 * @param nef NEF file handle
 * @param filename Path to save NEF file
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_to_file(const neoc_nef_file_t *nef,
                                    const char *filename);

/**
 * @brief Add a method token to NEF file
 * 
 * @param nef NEF file handle
 * @param hash Contract script hash
 * @param method Method name
 * @param parameters_count Number of parameters
 * @param has_return_value Whether method returns a value
 * @param call_flags Call flags
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_add_method_token(neoc_nef_file_t *nef,
                                             const neoc_hash160_t *hash,
                                             const char *method,
                                             uint16_t parameters_count,
                                             bool has_return_value,
                                             uint8_t call_flags);

/**
 * @brief Get script from NEF file
 * 
 * @param nef NEF file handle
 * @param script Output script pointer (do not free)
 * @param script_size Output script size
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_get_script(const neoc_nef_file_t *nef,
                                       const uint8_t **script,
                                       size_t *script_size);

/**
 * @brief Get compiler information
 * 
 * @param nef NEF file handle
 * @return Compiler string (do not free)
 */
const char* neoc_nef_file_get_compiler(const neoc_nef_file_t *nef);

/**
 * @brief Get source information
 * 
 * @param nef NEF file handle
 * @return Source string (do not free), or NULL if not set
 */
const char* neoc_nef_file_get_source(const neoc_nef_file_t *nef);

/**
 * @brief Get checksum of NEF file
 * 
 * @param nef NEF file handle
 * @param checksum Output checksum (32 bytes)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_get_checksum(const neoc_nef_file_t *nef,
                                         neoc_hash256_t *checksum);

/**
 * @brief Verify NEF file checksum
 * 
 * @param nef NEF file handle
 * @return true if checksum is valid, false otherwise
 */
bool neoc_nef_file_verify_checksum(const neoc_nef_file_t *nef);

/**
 * @brief Get number of method tokens
 * 
 * @param nef NEF file handle
 * @return Number of method tokens
 */
size_t neoc_nef_file_get_token_count(const neoc_nef_file_t *nef);

/**
 * @brief Get method token by index
 * 
 * @param nef NEF file handle
 * @param index Token index
 * @param token Output token pointer (do not free)
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_nef_file_get_token(const neoc_nef_file_t *nef,
                                      size_t index,
                                      const neoc_nef_method_token_t **token);

/**
 * @brief Free NEF file
 * 
 * @param nef NEF file handle
 */
void neoc_nef_file_free(neoc_nef_file_t *nef);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NEF_FILE_H
