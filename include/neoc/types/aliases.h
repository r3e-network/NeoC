/**
 * @file aliases.h
 * @brief Type aliases for NEO
 */

#ifndef NEOC_ALIASES_H
#define NEOC_ALIASES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic type aliases
typedef uint8_t neoc_byte_t;
typedef neoc_byte_t* neoc_bytes_t;
typedef int64_t neoc_int_t;
typedef uint64_t neoc_uint_t;

// Size type aliases
typedef size_t neoc_size_t;
typedef ptrdiff_t neoc_ssize_t;

// String type
typedef char* neoc_string_t;
typedef const char* neoc_const_string_t;

// Boolean type
typedef uint8_t neoc_bool_t;
#define NEOC_TRUE 1
#define NEOC_FALSE 0

// Result types
typedef int neoc_result_t;
#define NEOC_OK 0
#define NEOC_ERROR -1

// Block and transaction types
typedef uint32_t neoc_block_index_t;
typedef uint32_t neoc_nonce_t;
typedef int64_t neoc_fee_t;

// Time types
typedef uint64_t neoc_timestamp_t;
typedef uint32_t neoc_duration_t;

#ifdef __cplusplus
}
#endif

#endif // NEOC_ALIASES_H
