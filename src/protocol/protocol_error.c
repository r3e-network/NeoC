/**
 * @file protocol_error.c
 * @brief Protocol-specific error implementations
 * 
 * Based on Swift source: protocol/ProtocolError.swift
 * Implements error handling for protocol operations
 */

#include "../../include/neoc/neoc_error.h"
#include "../../include/neoc/neoc_memory.h"
#include "../../include/neoc/protocol/protocol_error.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Create a protocol error
 */
neoc_error_t neoc_protocol_error_create(
    neoc_protocol_error_type_t type,
    const char *message,
    const char *details,
    neoc_protocol_error_t **error
) {
    if (!error) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *error = NULL;
    
    if (!message) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    neoc_protocol_error_t *proto_error = neoc_malloc(sizeof(neoc_protocol_error_t));
    if (!proto_error) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    proto_error->type = type;
    
    // Copy message
    size_t message_len = strlen(message) + 1;
    proto_error->message = neoc_malloc(message_len);
    if (!proto_error->message) {
        neoc_free(proto_error);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    strcpy(proto_error->message, message);
    
    // Copy details if provided
    if (details) {
        size_t details_len = strlen(details) + 1;
        proto_error->details = neoc_malloc(details_len);
        if (!proto_error->details) {
            neoc_free(proto_error->message);
            neoc_free(proto_error);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        strcpy(proto_error->details, details);
    } else {
        proto_error->details = NULL;
    }
    
    *error = proto_error;
    return NEOC_SUCCESS;
}

/**
 * @brief Free a protocol error
 */
void neoc_protocol_error_free(neoc_protocol_error_t *error) {
    if (!error) {
        return;
    }
    
    if (error->message) {
        neoc_free(error->message);
    }
    
    if (error->details) {
        neoc_free(error->details);
    }
    
    neoc_free(error);
}

/**
 * @brief Create an RPC response error
 * 
 * Based on Swift: case rpcResponseError(_ error: String)
 */
neoc_error_t neoc_protocol_error_create_rpc_response(
    const char *rpc_error,
    neoc_protocol_error_t **error
) {
    if (!rpc_error || !error) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Create formatted message like Swift: "The Neo node responded with an error: \(error)"
    size_t message_len = strlen("The Neo node responded with an error: ") + strlen(rpc_error) + 1;
    char *message = neoc_malloc(message_len);
    if (!message) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    snprintf(message, message_len, "The Neo node responded with an error: %s", rpc_error);
    
    neoc_error_t result = neoc_protocol_error_create(
        NEOC_PROTOCOL_ERROR_RPC_RESPONSE,
        message,
        rpc_error,  // Store original error as details
        error
    );
    
    neoc_free(message);
    return result;
}

/**
 * @brief Create an invocation fault state error
 * 
 * Based on Swift: case invocationFaultState(_ error: String)
 */
neoc_error_t neoc_protocol_error_create_invocation_fault(
    const char *fault_message,
    neoc_protocol_error_t **error
) {
    if (!fault_message || !error) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Create formatted message like Swift: "The invocation resulted in a FAULT VM state. The VM exited due to the following exception: \(error)"
    size_t message_len = strlen("The invocation resulted in a FAULT VM state. The VM exited due to the following exception: ") + strlen(fault_message) + 1;
    char *message = neoc_malloc(message_len);
    if (!message) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    snprintf(message, message_len, "The invocation resulted in a FAULT VM state. The VM exited due to the following exception: %s", fault_message);
    
    neoc_error_t result = neoc_protocol_error_create(
        NEOC_PROTOCOL_ERROR_INVOCATION_FAULT,
        message,
        fault_message,  // Store original fault message as details
        error
    );
    
    neoc_free(message);
    return result;
}

/**
 * @brief Create a client connection error
 * 
 * Based on Swift: case clientConnection(_ message: String)
 */
neoc_error_t neoc_protocol_error_create_client_connection(
    const char *connection_message,
    neoc_protocol_error_t **error
) {
    if (!connection_message || !error) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Swift returns the message directly for clientConnection
    return neoc_protocol_error_create(
        NEOC_PROTOCOL_ERROR_CLIENT_CONNECTION,
        connection_message,
        NULL,
        error
    );
}

/**
 * @brief Create a stack item cast error
 * 
 * Based on Swift: case stackItemCastError(_ item: StackItem, _ target: String)
 */
neoc_error_t neoc_protocol_error_create_stack_item_cast(
    const char *item_type,
    const char *target_type,
    neoc_protocol_error_t **error
) {
    if (!item_type || !target_type || !error) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Create formatted message like Swift: "Cannot cast stack item \(item.jsonValue) to a \(target)."
    size_t message_len = strlen("Cannot cast stack item ") + strlen(item_type) + strlen(" to a ") + strlen(target_type) + strlen(".") + 1;
    char *message = neoc_malloc(message_len);
    if (!message) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    snprintf(message, message_len, "Cannot cast stack item %s to a %s.", item_type, target_type);
    
    // Create details string with both types
    size_t details_len = strlen("source: ") + strlen(item_type) + strlen(", target: ") + strlen(target_type) + 1;
    char *details = neoc_malloc(details_len);
    if (!details) {
        neoc_free(message);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    snprintf(details, details_len, "source: %s, target: %s", item_type, target_type);
    
    neoc_error_t result = neoc_protocol_error_create(
        NEOC_PROTOCOL_ERROR_STACK_ITEM_CAST,
        message,
        details,
        error
    );
    
    neoc_free(message);
    neoc_free(details);
    return result;
}

/**
 * @brief Get error message from protocol error
 */
const char* neoc_protocol_error_get_message(
    const neoc_protocol_error_t *error
) {
    if (!error) {
        return NULL;
    }
    
    return error->message;
}

/**
 * @brief Get error type from protocol error
 */
neoc_protocol_error_type_t neoc_protocol_error_get_type(
    const neoc_protocol_error_t *error
) {
    if (!error) {
        return 0;  // Invalid type
    }
    
    return error->type;
}

/**
 * @brief Get error details from protocol error
 */
const char* neoc_protocol_error_get_details(
    const neoc_protocol_error_t *error
) {
    if (!error) {
        return NULL;
    }
    
    return error->details;
}

/**
 * @brief Convert protocol error to string
 */
neoc_error_t neoc_protocol_error_to_string(
    const neoc_protocol_error_t *error,
    char **error_str
) {
    if (!error || !error_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *error_str = NULL;
    
    const char *type_name;
    switch (error->type) {
        case NEOC_PROTOCOL_ERROR_RPC_RESPONSE:
            type_name = "RPC Response Error";
            break;
        case NEOC_PROTOCOL_ERROR_INVOCATION_FAULT:
            type_name = "Invocation Fault State";
            break;
        case NEOC_PROTOCOL_ERROR_CLIENT_CONNECTION:
            type_name = "Client Connection Error";
            break;
        case NEOC_PROTOCOL_ERROR_STACK_ITEM_CAST:
            type_name = "Stack Item Cast Error";
            break;
        default:
            type_name = "Unknown Protocol Error";
            break;
    }
    
    const char *message = error->message ? error->message : "(no message)";
    const char *details = error->details ? error->details : "";
    
    // Format: "[Type]: message (details)"
    size_t str_len = strlen("[") + strlen(type_name) + strlen("]: ") + strlen(message);
    if (error->details) {
        str_len += strlen(" (") + strlen(details) + strlen(")");
    }
    str_len += 1;  // null terminator
    
    char *result = neoc_malloc(str_len);
    if (!result) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    if (error->details) {
        snprintf(result, str_len, "[%s]: %s (%s)", type_name, message, details);
    } else {
        snprintf(result, str_len, "[%s]: %s", type_name, message);
    }
    
    *error_str = result;
    return NEOC_SUCCESS;
}

