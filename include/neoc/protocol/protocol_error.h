/**
 * @file protocol_error.h
 * @brief Protocol-specific error definitions
 * 
 * Based on Swift source: protocol/ProtocolError.swift
 * Defines errors that can occur during protocol operations
 */

#ifndef NEOC_PROTOCOL_PROTOCOL_ERROR_H
#define NEOC_PROTOCOL_PROTOCOL_ERROR_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Protocol error types
 * 
 * These correspond to different types of protocol-level errors
 */
typedef enum {
    NEOC_PROTOCOL_ERROR_RPC_RESPONSE = 1000,    /**< RPC response error */
    NEOC_PROTOCOL_ERROR_INVOCATION_FAULT,       /**< Invocation fault state */
    NEOC_PROTOCOL_ERROR_CLIENT_CONNECTION,      /**< Client connection error */
    NEOC_PROTOCOL_ERROR_STACK_ITEM_CAST         /**< Stack item cast error */
} neoc_protocol_error_type_t;

/**
 * @brief Protocol error structure
 * 
 * Contains detailed error information for protocol operations
 */
typedef struct {
    neoc_protocol_error_type_t type;    /**< Error type */
    char *message;                      /**< Error message */
    char *details;                      /**< Additional error details */
} neoc_protocol_error_t;

/**
 * @brief Create a protocol error
 * 
 * @param type Error type
 * @param message Error message
 * @param details Additional details (can be NULL)
 * @param error Pointer to store the created error
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_protocol_error_create(
    neoc_protocol_error_type_t type,
    const char *message,
    const char *details,
    neoc_protocol_error_t **error
);

/**
 * @brief Free a protocol error
 * 
 * @param error Error to free
 */
void neoc_protocol_error_free(
    neoc_protocol_error_t *error
);

/**
 * @brief Create an RPC response error
 * 
 * @param rpc_error RPC error message
 * @param error Pointer to store the created error
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_protocol_error_create_rpc_response(
    const char *rpc_error,
    neoc_protocol_error_t **error
);

/**
 * @brief Create an invocation fault state error
 * 
 * @param fault_message Fault message
 * @param error Pointer to store the created error
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_protocol_error_create_invocation_fault(
    const char *fault_message,
    neoc_protocol_error_t **error
);

/**
 * @brief Create a client connection error
 * 
 * @param connection_message Connection error message
 * @param error Pointer to store the created error
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_protocol_error_create_client_connection(
    const char *connection_message,
    neoc_protocol_error_t **error
);

/**
 * @brief Create a stack item cast error
 * 
 * @param item_type Actual item type
 * @param target_type Target type
 * @param error Pointer to store the created error
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_protocol_error_create_stack_item_cast(
    const char *item_type,
    const char *target_type,
    neoc_protocol_error_t **error
);

/**
 * @brief Get error message from protocol error
 * 
 * @param error Protocol error
 * @return Error message (do not free)
 */
const char* neoc_protocol_error_get_message(
    const neoc_protocol_error_t *error
);

/**
 * @brief Get error type from protocol error
 * 
 * @param error Protocol error
 * @return Error type
 */
neoc_protocol_error_type_t neoc_protocol_error_get_type(
    const neoc_protocol_error_t *error
);

/**
 * @brief Get error details from protocol error
 * 
 * @param error Protocol error
 * @return Error details (do not free, can be NULL)
 */
const char* neoc_protocol_error_get_details(
    const neoc_protocol_error_t *error
);

/**
 * @brief Convert protocol error to string
 * 
 * @param error Protocol error
 * @param error_str Pointer to store the error string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_protocol_error_to_string(
    const neoc_protocol_error_t *error,
    char **error_str
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_PROTOCOL_ERROR_H */
