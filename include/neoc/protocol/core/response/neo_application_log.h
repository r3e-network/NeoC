/**
 * @file neo_application_log.h
 * @brief Neo application execution log response
 * 
 * Based on Swift source: protocol/core/response/NeoApplicationLog.swift
 * Contains transaction execution logs including notifications and stack traces
 */

#ifndef NEOC_PROTOCOL_CORE_RESPONSE_NEO_APPLICATION_LOG_H
#define NEOC_PROTOCOL_CORE_RESPONSE_NEO_APPLICATION_LOG_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/hash256.h"
#include "neoc/types/neoc_vm_state_type.h"
#include "neoc/protocol/stack_item.h"
#include "neoc/protocol/core/response/notification.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Execution details for a transaction
 * 
 * Contains the execution state, gas consumed, stack result, and notifications
 */
typedef struct {
    char *trigger;                      /**< Execution trigger type */
    neoc_vm_state_t state;              /**< VM execution state */
    char *exception;                    /**< Exception message (nullable) */
    char *gas_consumed;                 /**< Gas consumed as string */
    neoc_stack_item_t **stack;          /**< Stack items array */
    size_t stack_count;                 /**< Number of stack items */
    neoc_notification_t **notifications; /**< Notifications array */
    size_t notifications_count;         /**< Number of notifications */
} neoc_application_execution_t;

/**
 * @brief Neo application log containing transaction execution details
 * 
 * Represents the complete execution log for a transaction including all executions
 */
typedef struct {
    neoc_hash256_t *transaction_id;     /**< Transaction hash */
    neoc_application_execution_t **executions; /**< Array of executions */
    size_t executions_count;            /**< Number of executions */
} neoc_application_log_t;

/**
 * @brief GetApplicationLog JSON-RPC response
 * 
 * Response for getapplicationlog RPC method
 */
typedef struct {
    char *jsonrpc;                      /**< JSON-RPC version ("2.0") */
    int id;                            /**< Request ID */
    neoc_application_log_t *result;     /**< Application log result */
    
    /* Error fields */
    int error_code;                     /**< Error code if request failed */
    char *error_message;                /**< Error message if request failed */
} neoc_get_application_log_response_t;

/**
 * @brief Create a new application execution
 * 
 * @param execution Pointer to store the created execution
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_application_execution_create(
    neoc_application_execution_t **execution
);

/**
 * @brief Free an application execution and its resources
 * 
 * @param execution Execution to free
 */
void neoc_application_execution_free(
    neoc_application_execution_t *execution
);

/**
 * @brief Create a new application log
 * 
 * @param log Pointer to store the created log
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_application_log_create(
    neoc_application_log_t **log
);

/**
 * @brief Free an application log and its resources
 * 
 * @param log Application log to free
 */
void neoc_application_log_free(
    neoc_application_log_t *log
);

/**
 * @brief Create a new GetApplicationLog response
 * 
 * @param response Pointer to store the created response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_application_log_response_create(
    neoc_get_application_log_response_t **response
);

/**
 * @brief Free a GetApplicationLog response and its resources
 * 
 * @param response Response to free
 */
void neoc_get_application_log_response_free(
    neoc_get_application_log_response_t *response
);

/**
 * @brief Parse JSON into GetApplicationLog response
 * 
 * @param json_str JSON string to parse
 * @param response Pointer to store the parsed response
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_application_log_response_from_json(
    const char *json_str,
    neoc_get_application_log_response_t **response
);

/**
 * @brief Convert GetApplicationLog response to JSON string
 * 
 * @param response Response to convert
 * @param json_str Pointer to store the JSON string (caller must free)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_get_application_log_response_to_json(
    const neoc_get_application_log_response_t *response,
    char **json_str
);

/**
 * @brief Add an execution to the application log
 * 
 * @param log Application log to modify
 * @param execution Execution to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_application_log_add_execution(
    neoc_application_log_t *log,
    neoc_application_execution_t *execution
);

/**
 * @brief Add a stack item to the execution
 * 
 * @param execution Execution to modify
 * @param stack_item Stack item to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_application_execution_add_stack_item(
    neoc_application_execution_t *execution,
    neoc_stack_item_t *stack_item
);

/**
 * @brief Add a notification to the execution
 * 
 * @param execution Execution to modify
 * @param notification Notification to add (takes ownership)
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_application_execution_add_notification(
    neoc_application_execution_t *execution,
    neoc_notification_t *notification
);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_PROTOCOL_CORE_RESPONSE_NEO_APPLICATION_LOG_H */
