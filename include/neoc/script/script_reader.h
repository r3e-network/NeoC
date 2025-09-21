/**
 * @file script_reader.h
 * @brief NeoVM script reader and analyzer
 * 
 * Provides functionality to read and analyze NeoVM scripts, converting
 * them to human-readable representations and extracting interop service calls.
 */

#ifndef NEOC_SCRIPT_READER_H
#define NEOC_SCRIPT_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "neoc/neoc_error.h"
#include "neoc/script/interop_service.h"
#include "neoc/script/opcode.h"
#include <stdint.h>
#include <stddef.h>

#define NEOC_SCRIPT_MAX_OUTPUT_SIZE 4096
#define NEOC_SCRIPT_MAX_LINE_SIZE 256

/**
 * @brief Script reader context structure
 */
typedef struct {
    const uint8_t* script;          ///< Script data pointer
    size_t script_length;           ///< Total script length
    size_t position;                ///< Current read position
    char* output_buffer;            ///< Output buffer for string representation
    size_t output_size;             ///< Size of output buffer
    size_t output_length;           ///< Current output length
} neoc_script_reader_t;

/**
 * @brief Initialize script reader
 * 
 * @param reader Script reader structure to initialize
 * @param script Script data to read
 * @param script_length Length of script data
 * @param output_buffer Buffer for output string
 * @param output_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_init(
    neoc_script_reader_t* reader,
    const uint8_t* script,
    size_t script_length,
    char* output_buffer,
    size_t output_size
);

/**
 * @brief Get InteropService by hash string
 * 
 * @param hash_string Hash string to lookup
 * @param interop_service Output interop service
 * @return NEOC_SUCCESS on success, NEOC_ERROR_NOT_FOUND if not found
 */
neoc_error_t neoc_script_reader_get_interop_service(
    const char* hash_string,
    neoc_interop_service_t* interop_service
);

/**
 * @brief Convert script to OpCode string representation
 * 
 * @param script Script data as hex string
 * @param output_buffer Buffer for OpCode string representation
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_to_opcode_string_hex(
    const char* script_hex,
    char* output_buffer,
    size_t buffer_size
);

/**
 * @brief Convert script bytes to OpCode string representation
 * 
 * @param script Script data as bytes
 * @param script_length Length of script data
 * @param output_buffer Buffer for OpCode string representation
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_to_opcode_string(
    const uint8_t* script,
    size_t script_length,
    char* output_buffer,
    size_t buffer_size
);

/**
 * @brief Read next byte from script
 * 
 * @param reader Script reader context
 * @param byte Output byte value
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_read_byte(neoc_script_reader_t* reader, uint8_t* byte);

/**
 * @brief Read multiple bytes from script
 * 
 * @param reader Script reader context
 * @param buffer Output buffer for bytes
 * @param count Number of bytes to read
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_read_bytes(neoc_script_reader_t* reader, uint8_t* buffer, size_t count);

/**
 * @brief Check if reader has more data
 * 
 * @param reader Script reader context
 * @return true if more data available, false otherwise
 */
bool neoc_script_reader_has_more(const neoc_script_reader_t* reader);

/**
 * @brief Get current position in script
 * 
 * @param reader Script reader context
 * @return Current position
 */
size_t neoc_script_reader_get_position(const neoc_script_reader_t* reader);

/**
 * @brief Append string to output buffer
 * 
 * @param reader Script reader context
 * @param str String to append
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_append_output(neoc_script_reader_t* reader, const char* str);

/**
 * @brief Append formatted string to output buffer
 * 
 * @param reader Script reader context
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_script_reader_append_formatted(neoc_script_reader_t* reader, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_SCRIPT_READER_H */
