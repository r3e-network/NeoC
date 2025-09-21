/**
 * @file neo_application_log.c
 * @brief Application log helpers aligned with NeoSwift NeoApplicationLog
 */

#include "neoc/protocol/core/response/neo_application_log.h"

#include "neoc/protocol/core/response/notification.h"
#include "neoc/protocol/stack_item.h"
#include "neoc/types/hash256.h"

#include <string.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static void neoc_application_execution_reset(neoc_application_execution_t *execution) {
    if (!execution) {
        return;
    }
    if (execution->trigger) {
        neoc_free(execution->trigger);
        execution->trigger = NULL;
    }
    if (execution->exception) {
        neoc_free(execution->exception);
        execution->exception = NULL;
    }
    if (execution->gas_consumed) {
        neoc_free(execution->gas_consumed);
        execution->gas_consumed = NULL;
    }
    if (execution->stack) {
        for (size_t i = 0; i < execution->stack_count; i++) {
            neoc_stack_item_free(execution->stack[i]);
        }
        neoc_free(execution->stack);
        execution->stack = NULL;
        execution->stack_count = 0;
    }
    if (execution->notifications) {
        for (size_t i = 0; i < execution->notifications_count; i++) {
            neoc_notification_free(execution->notifications[i]);
        }
        neoc_free(execution->notifications);
        execution->notifications = NULL;
        execution->notifications_count = 0;
    }
}

void neoc_application_execution_free(neoc_application_execution_t *execution) {
    if (!execution) {
        return;
    }
    neoc_application_execution_reset(execution);
    neoc_free(execution);
}

static neoc_error_t neoc_application_execution_allocate(neoc_application_execution_t **execution) {
    if (!execution) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_execution: output pointer missing");
    }

    neoc_application_execution_t *result = neoc_calloc(1, sizeof(neoc_application_execution_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: allocation failed");
    }

    *execution = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_application_execution_create(neoc_application_execution_t **execution) {
    return neoc_application_execution_allocate(execution);
}

static void neoc_application_log_reset(neoc_application_log_t *log) {
    if (!log) {
        return;
    }
    if (log->transaction_id) {
        neoc_free(log->transaction_id);
        log->transaction_id = NULL;
    }
    if (log->executions) {
        for (size_t i = 0; i < log->executions_count; i++) {
            neoc_application_execution_free(log->executions[i]);
        }
        neoc_free(log->executions);
        log->executions = NULL;
        log->executions_count = 0;
    }
}

void neoc_application_log_free(neoc_application_log_t *log) {
    if (!log) {
        return;
    }
    neoc_application_log_reset(log);
    neoc_free(log);
}

static neoc_error_t neoc_application_log_allocate(neoc_application_log_t **log) {
    if (!log) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_log: output pointer missing");
    }

    neoc_application_log_t *result = neoc_calloc(1, sizeof(neoc_application_log_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log: allocation failed");
    }

    *log = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_application_log_create(neoc_application_log_t **log) {
    return neoc_application_log_allocate(log);
}

static void neoc_get_application_log_response_reset(neoc_get_application_log_response_t *response) {
    if (!response) {
        return;
    }
    if (response->jsonrpc) {
        neoc_free(response->jsonrpc);
        response->jsonrpc = NULL;
    }
    if (response->error_message) {
        neoc_free(response->error_message);
        response->error_message = NULL;
    }
    if (response->result) {
        neoc_application_log_free(response->result);
        response->result = NULL;
    }
}

void neoc_get_application_log_response_free(neoc_get_application_log_response_t *response) {
    if (!response) {
        return;
    }
    neoc_get_application_log_response_reset(response);
    neoc_free(response);
}

static neoc_error_t neoc_get_application_log_response_allocate(neoc_get_application_log_response_t **response) {
    if (!response) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_log_response: output pointer missing");
    }

    neoc_get_application_log_response_t *result = neoc_calloc(1, sizeof(neoc_get_application_log_response_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: allocation failed");
    }

    *response = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_get_application_log_response_create(neoc_get_application_log_response_t **response) {
    return neoc_get_application_log_response_allocate(response);
}

neoc_error_t neoc_application_log_add_execution(neoc_application_log_t *log,
                                                neoc_application_execution_t *execution) {
    if (!log || !execution) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_log: add_execution arguments invalid");
    }

    size_t new_count = log->executions_count + 1;
    neoc_application_execution_t **expanded = neoc_realloc(log->executions,
                                                           new_count * sizeof(neoc_application_execution_t *));
    if (!expanded) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log: executions realloc failed");
    }

    log->executions = expanded;
    log->executions[new_count - 1] = execution;
    log->executions_count = new_count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_application_execution_add_stack_item(neoc_application_execution_t *execution,
                                                       neoc_stack_item_t *stack_item) {
    if (!execution || !stack_item) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_execution: add_stack_item arguments invalid");
    }

    size_t new_count = execution->stack_count + 1;
    neoc_stack_item_t **expanded = neoc_realloc(execution->stack,
                                               new_count * sizeof(neoc_stack_item_t *));
    if (!expanded) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: stack realloc failed");
    }

    execution->stack = expanded;
    execution->stack[new_count - 1] = stack_item;
    execution->stack_count = new_count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_application_execution_add_notification(neoc_application_execution_t *execution,
                                                         neoc_notification_t *notification) {
    if (!execution || !notification) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_execution: add_notification arguments invalid");
    }

    size_t new_count = execution->notifications_count + 1;
    neoc_notification_t **expanded = neoc_realloc(execution->notifications,
                                                 new_count * sizeof(neoc_notification_t *));
    if (!expanded) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: notifications realloc failed");
    }

    execution->notifications = expanded;
    execution->notifications[new_count - 1] = notification;
    execution->notifications_count = new_count;
    return NEOC_SUCCESS;
}

#ifdef HAVE_CJSON
static neoc_error_t neoc_application_execution_state_from_string(const char *state_str,
                                                                 neoc_vm_state_t *state) {
    if (!state_str || !state) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_execution: vm state parse arguments invalid");
    }

    if (strcmp(state_str, "HALT") == 0) {
        *state = NEOC_VM_STATE_HALT;
    } else if (strcmp(state_str, "FAULT") == 0) {
        *state = NEOC_VM_STATE_FAULT;
    } else if (strcmp(state_str, "NONE") == 0) {
        *state = NEOC_VM_STATE_NONE;
    } else {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: unknown VM state");
    }
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_application_execution_from_json_object(const cJSON *exec_obj,
                                                                neoc_application_execution_t **execution_out) {
    neoc_application_execution_t *execution = NULL;
    neoc_error_t err = neoc_application_execution_allocate(&execution);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    const cJSON *trigger_item = cJSON_GetObjectItemCaseSensitive(exec_obj, "trigger");
    if (!trigger_item || !cJSON_IsString(trigger_item) || !trigger_item->valuestring) {
        neoc_application_execution_free(execution);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: missing trigger");
    }
    execution->trigger = neoc_strdup(trigger_item->valuestring);
    if (!execution->trigger) {
        neoc_application_execution_free(execution);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: trigger copy failed");
    }

    const cJSON *state_item = cJSON_GetObjectItemCaseSensitive(exec_obj, "vmstate");
    if (!state_item || !cJSON_IsString(state_item) || !state_item->valuestring) {
        neoc_application_execution_free(execution);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: missing vmstate");
    }
    err = neoc_application_execution_state_from_string(state_item->valuestring, &execution->state);
    if (err != NEOC_SUCCESS) {
        neoc_application_execution_free(execution);
        return err;
    }

    const cJSON *exception_item = cJSON_GetObjectItemCaseSensitive(exec_obj, "exception");
    if (exception_item && cJSON_IsString(exception_item) && exception_item->valuestring) {
        execution->exception = neoc_strdup(exception_item->valuestring);
        if (!execution->exception) {
            neoc_application_execution_free(execution);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: exception copy failed");
        }
    }

    const cJSON *gas_item = cJSON_GetObjectItemCaseSensitive(exec_obj, "gasconsumed");
    if (!gas_item || !cJSON_IsString(gas_item) || !gas_item->valuestring) {
        neoc_application_execution_free(execution);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: missing gasconsumed");
    }
    execution->gas_consumed = neoc_strdup(gas_item->valuestring);
    if (!execution->gas_consumed) {
        neoc_application_execution_free(execution);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: gasconsumed copy failed");
    }

    const cJSON *stack_array = cJSON_GetObjectItemCaseSensitive(exec_obj, "stack");
    if (stack_array && cJSON_IsArray(stack_array)) {
        size_t count = (size_t)cJSON_GetArraySize(stack_array);
        for (size_t i = 0; i < count; i++) {
            cJSON *stack_elem = cJSON_GetArrayItem(stack_array, (int)i);
            if (!stack_elem) {
                neoc_application_execution_free(execution);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: invalid stack entry");
            }
            char *stack_str = cJSON_PrintUnformatted(stack_elem);
            if (!stack_str) {
                neoc_application_execution_free(execution);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: stack item stringify failed");
            }
            neoc_stack_item_t *item = neoc_stack_item_from_json(stack_str);
            cJSON_free(stack_str);
            if (!item) {
                neoc_application_execution_free(execution);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: stack item parse failed");
            }
            neoc_error_t append_err = neoc_application_execution_add_stack_item(execution, item);
            if (append_err != NEOC_SUCCESS) {
                neoc_stack_item_free(item);
                neoc_application_execution_free(execution);
                return append_err;
            }
        }
    }

    const cJSON *notify_array = cJSON_GetObjectItemCaseSensitive(exec_obj, "notifications");
    if (notify_array && cJSON_IsArray(notify_array)) {
        size_t count = (size_t)cJSON_GetArraySize(notify_array);
        for (size_t i = 0; i < count; i++) {
            cJSON *notification_elem = cJSON_GetArrayItem(notify_array, (int)i);
            if (!notification_elem) {
                neoc_application_execution_free(execution);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: notification entry missing");
            }
            char *notification_str = cJSON_PrintUnformatted(notification_elem);
            if (!notification_str) {
                neoc_application_execution_free(execution);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_execution: notification stringify failed");
            }
            neoc_notification_t *notification = neoc_notification_from_json(notification_str);
            cJSON_free(notification_str);
            if (!notification) {
                neoc_application_execution_free(execution);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_execution: notification parse failed");
            }
            neoc_error_t append_err = neoc_application_execution_add_notification(execution, notification);
            if (append_err != NEOC_SUCCESS) {
                neoc_notification_free(notification);
                neoc_application_execution_free(execution);
                return append_err;
            }
        }
    }

    *execution_out = execution;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_application_log_from_json_object(const cJSON *log_obj,
                                                          neoc_application_log_t **log_out) {
    neoc_application_log_t *log = NULL;
    neoc_error_t err = neoc_application_log_allocate(&log);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    const cJSON *txid_item = cJSON_GetObjectItemCaseSensitive(log_obj, "txid");
    if (!txid_item || !cJSON_IsString(txid_item) || !txid_item->valuestring) {
        neoc_application_log_free(log);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_log: missing txid");
    }

    neoc_hash256_t *hash = neoc_malloc(sizeof(neoc_hash256_t));
    if (!hash) {
        neoc_application_log_free(log);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log: transaction hash allocation failed");
    }
    if (neoc_hash256_from_string(txid_item->valuestring, hash) != NEOC_SUCCESS) {
        neoc_free(hash);
        neoc_application_log_free(log);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_log: invalid txid");
    }
    log->transaction_id = hash;

    const cJSON *executions_array = cJSON_GetObjectItemCaseSensitive(log_obj, "executions");
    if (!executions_array || !cJSON_IsArray(executions_array)) {
        neoc_application_log_free(log);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_log: executions missing or invalid");
    }

    size_t count = (size_t)cJSON_GetArraySize(executions_array);
    for (size_t i = 0; i < count; i++) {
        cJSON *exec_obj = cJSON_GetArrayItem(executions_array, (int)i);
        neoc_application_execution_t *execution = NULL;
        err = neoc_application_execution_from_json_object(exec_obj, &execution);
        if (err != NEOC_SUCCESS) {
            neoc_application_log_free(log);
            return err;
        }
        err = neoc_application_log_add_execution(log, execution);
        if (err != NEOC_SUCCESS) {
            neoc_application_execution_free(execution);
            neoc_application_log_free(log);
            return err;
        }
    }

    *log_out = log;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_get_application_log_response_parse_error(const cJSON *root,
                                                                  neoc_get_application_log_response_t *response) {
    const cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (!error_obj || !cJSON_IsObject(error_obj)) {
        return NEOC_SUCCESS;
    }

    const cJSON *code_item = cJSON_GetObjectItemCaseSensitive(error_obj, "code");
    if (code_item && cJSON_IsNumber(code_item)) {
        response->error_code = code_item->valueint;
    }

    const cJSON *message_item = cJSON_GetObjectItemCaseSensitive(error_obj, "message");
    if (message_item && cJSON_IsString(message_item) && message_item->valuestring) {
        response->error_message = neoc_strdup(message_item->valuestring);
        if (!response->error_message) {
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: error message copy failed");
        }
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_get_application_log_response_from_json(const char *json_str,
                                                         neoc_get_application_log_response_t **response_out) {
    if (!json_str || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_log_response: from_json arguments invalid");
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_log_response: JSON is not an object");
    }

    neoc_get_application_log_response_t *response = NULL;
    neoc_error_t err = neoc_get_application_log_response_allocate(&response);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(root);
        return err;
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_get_application_log_response_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: jsonrpc copy failed");
        }
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (id_item && cJSON_IsNumber(id_item)) {
        response->id = id_item->valueint;
    }

    err = neoc_get_application_log_response_parse_error(root, response);
    if (err != NEOC_SUCCESS) {
        neoc_get_application_log_response_free(response);
        cJSON_Delete(root);
        return err;
    }

    const cJSON *result_item = cJSON_GetObjectItemCaseSensitive(root, "result");
    if (result_item && cJSON_IsObject(result_item)) {
        neoc_application_log_t *log = NULL;
        err = neoc_application_log_from_json_object(result_item, &log);
        if (err != NEOC_SUCCESS) {
            neoc_get_application_log_response_free(response);
            cJSON_Delete(root);
            return err;
        }
        response->result = log;
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

#endif

neoc_error_t neoc_get_application_log_response_to_json(const neoc_get_application_log_response_t *response,
                                                       char **json_str) {
    if (!response || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "application_log_response: to_json arguments invalid");
    }

#ifndef HAVE_CJSON
    (void)response;
    (void)json_str;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "application_log_response: serialization requires cJSON");
#else
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: JSON object allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_message || response->error_code) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: error object allocation failed");
        }
        cJSON_AddItemToObject(root, "error", error_obj);
        cJSON_AddNumberToObject(error_obj, "code", response->error_code);
        if (response->error_message) {
            cJSON_AddStringToObject(error_obj, "message", response->error_message);
        }
    } else if (response->result) {
        cJSON *result_obj = cJSON_CreateObject();
        if (!result_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: result object allocation failed");
        }
        cJSON_AddItemToObject(root, "result", result_obj);

        char hash_str[67];
        if (neoc_hash256_to_string(response->result->transaction_id, hash_str, sizeof(hash_str)) == NEOC_SUCCESS) {
            cJSON_AddStringToObject(result_obj, "txid", hash_str);
        }

        cJSON *executions_array = cJSON_CreateArray();
        if (!executions_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: executions array allocation failed");
        }
        cJSON_AddItemToObject(result_obj, "executions", executions_array);

        for (size_t i = 0; i < response->result->executions_count; i++) {
            const neoc_application_execution_t *execution = response->result->executions[i];
            if (!execution) {
                continue;
            }

            cJSON *exec_obj = cJSON_CreateObject();
            if (!exec_obj) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: execution object allocation failed");
            }
            cJSON_AddItemToArray(executions_array, exec_obj);

            cJSON_AddStringToObject(exec_obj, "trigger", execution->trigger ? execution->trigger : "");

            const char *state_str = "NONE";
            switch (execution->state) {
                case NEOC_VM_STATE_HALT: state_str = "HALT"; break;
                case NEOC_VM_STATE_FAULT: state_str = "FAULT"; break;
                case NEOC_VM_STATE_NONE: default: state_str = "NONE"; break;
            }
            cJSON_AddStringToObject(exec_obj, "vmstate", state_str);

            if (execution->exception) {
                cJSON_AddStringToObject(exec_obj, "exception", execution->exception);
            }

            cJSON_AddStringToObject(exec_obj, "gasconsumed", execution->gas_consumed ? execution->gas_consumed : "0");

            cJSON *stack_array = cJSON_CreateArray();
            if (!stack_array) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: stack array allocation failed");
            }
            cJSON_AddItemToObject(exec_obj, "stack", stack_array);

            for (size_t j = 0; j < execution->stack_count; j++) {
                neoc_stack_item_t *item = execution->stack[j];
                if (!item) {
                    continue;
                }
                char *item_json = neoc_stack_item_to_json(item);
                if (!item_json) {
                    cJSON_Delete(root);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: stack item serialization failed");
                }
                cJSON *item_obj = cJSON_Parse(item_json);
                cJSON_free(item_json);
                if (!item_obj) {
                    cJSON_Delete(root);
                    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_log_response: stack item json invalid");
                }
                cJSON_AddItemToArray(stack_array, item_obj);
            }

            cJSON *notification_array = cJSON_CreateArray();
            if (!notification_array) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: notification array allocation failed");
            }
            cJSON_AddItemToObject(exec_obj, "notifications", notification_array);

            for (size_t j = 0; j < execution->notifications_count; j++) {
                neoc_notification_t *notification = execution->notifications[j];
                if (!notification) {
                    continue;
                }
                char *notification_json = neoc_notification_to_json(notification);
                if (!notification_json) {
                    cJSON_Delete(root);
                    return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: notification serialization failed");
                }
                cJSON *notification_obj = cJSON_Parse(notification_json);
                cJSON_free(notification_json);
                if (!notification_obj) {
                    cJSON_Delete(root);
                    return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "application_log_response: notification json invalid");
                }
                cJSON_AddItemToArray(notification_array, notification_obj);
            }
        }
    }

    *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!*json_str) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "application_log_response: JSON print failed");
    }
    return NEOC_SUCCESS;
#endif
}
