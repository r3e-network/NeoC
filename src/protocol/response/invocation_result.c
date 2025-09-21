#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../../include/neoc/protocol/response/invocation_result.h"
#include "../../../include/neoc/neoc_memory.h"
#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

// Create invocation result
neoc_invocation_result_t* neoc_invocation_result_create(void) {
    neoc_invocation_result_t* result = neoc_malloc(sizeof(neoc_invocation_result_t));
    if (!result) {
        return NULL;
    }
    
    // Initialize all fields
    result->script = NULL;
    result->state = NEO_VM_STATE_NONE;
    result->gas_consumed = 0;
    result->exception = NULL;
    result->stack = NULL;
    result->stack_count = 0;
    result->notifications = NULL;
    result->notifications_count = 0;
    result->diagnostics = NULL;
    result->session_id = NULL;
    result->storage_changes = NULL;
    result->storage_changes_count = 0;
    
    return result;
}

// Free invocation result
void neoc_invocation_result_free(neoc_invocation_result_t* result) {
    if (!result) {
        return;
    }
    
    // Free script
    if (result->script) {
        neoc_free(result->script);
    }
    
    // Free exception
    if (result->exception) {
        neoc_free(result->exception);
    }
    
    // Free stack items
    if (result->stack) {
        for (size_t i = 0; i < result->stack_count; i++) {
            if (result->stack[i]) {
                neoc_stack_item_free(result->stack[i]);
            }
        }
        neoc_free(result->stack);
    }
    
    // Free notifications
    if (result->notifications) {
        for (size_t i = 0; i < result->notifications_count; i++) {
            if (result->notifications[i]) {
                neoc_notification_free(result->notifications[i]);
            }
        }
        neoc_free(result->notifications);
    }
    
    // Free diagnostics
    if (result->diagnostics) {
        neoc_diagnostics_free(result->diagnostics);
    }
    
    // Free session ID
    if (result->session_id) {
        neoc_free(result->session_id);
    }
    
    // Free storage changes
    if (result->storage_changes) {
        for (size_t i = 0; i < result->storage_changes_count; i++) {
            if (result->storage_changes[i]) {
                neoc_free(result->storage_changes[i]);
            }
        }
        neoc_free(result->storage_changes);
    }
    
    neoc_free(result);
}

// Clone invocation result
neoc_invocation_result_t* neoc_invocation_result_clone(const neoc_invocation_result_t* result) {
    if (!result) {
        return NULL;
    }
    
    neoc_invocation_result_t* clone = neoc_invocation_result_create();
    if (!clone) {
        return NULL;
    }
    
    // Clone script
    if (result->script) {
        clone->script = neoc_strdup(result->script);
        if (!clone->script) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
    }
    
    // Copy state and gas consumed
    clone->state = result->state;
    clone->gas_consumed = result->gas_consumed;
    
    // Clone exception
    if (result->exception) {
        clone->exception = neoc_strdup(result->exception);
        if (!clone->exception) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
    }
    
    // Clone stack items
    if (result->stack_count > 0 && result->stack) {
        clone->stack = neoc_malloc(sizeof(neoc_stack_item_t*) * result->stack_count);
        if (!clone->stack) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
        
        clone->stack_count = result->stack_count;
        for (size_t i = 0; i < result->stack_count; i++) {
            clone->stack[i] = neoc_stack_item_clone(result->stack[i]);
            if (!clone->stack[i]) {
                neoc_invocation_result_free(clone);
                return NULL;
            }
        }
    }
    
    // Clone notifications
    if (result->notifications_count > 0 && result->notifications) {
        clone->notifications = neoc_malloc(sizeof(neoc_notification_t*) * result->notifications_count);
        if (!clone->notifications) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
        
        clone->notifications_count = result->notifications_count;
        for (size_t i = 0; i < result->notifications_count; i++) {
            clone->notifications[i] = neoc_notification_clone(result->notifications[i]);
            if (!clone->notifications[i]) {
                neoc_invocation_result_free(clone);
                return NULL;
            }
        }
    }
    
    // Clone diagnostics
    if (result->diagnostics) {
        clone->diagnostics = neoc_diagnostics_clone(result->diagnostics);
        if (!clone->diagnostics) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
    }
    
    // Clone session ID
    if (result->session_id) {
        clone->session_id = neoc_strdup(result->session_id);
        if (!clone->session_id) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
    }
    
    // Clone storage changes
    if (result->storage_changes_count > 0 && result->storage_changes) {
        clone->storage_changes = neoc_malloc(sizeof(char*) * result->storage_changes_count);
        if (!clone->storage_changes) {
            neoc_invocation_result_free(clone);
            return NULL;
        }
        
        clone->storage_changes_count = result->storage_changes_count;
        for (size_t i = 0; i < result->storage_changes_count; i++) {
            clone->storage_changes[i] = neoc_strdup(result->storage_changes[i]);
            if (!clone->storage_changes[i]) {
                neoc_invocation_result_free(clone);
                return NULL;
            }
        }
    }
    
    return clone;
}

// Set script
void neoc_invocation_result_set_script(neoc_invocation_result_t* result, const char* script) {
    if (!result || !script) {
        return;
    }
    
    if (result->script) {
        neoc_free(result->script);
    }
    
    result->script = neoc_strdup(script);
}

// Set state
void neoc_invocation_result_set_state(neoc_invocation_result_t* result, neoc_vm_state_t state) {
    if (result) {
        result->state = state;
    }
}

// Set gas consumed
void neoc_invocation_result_set_gas_consumed(neoc_invocation_result_t* result, uint64_t gas) {
    if (result) {
        result->gas_consumed = gas;
    }
}

// Set exception
void neoc_invocation_result_set_exception(neoc_invocation_result_t* result, const char* exception) {
    if (!result) {
        return;
    }
    
    if (result->exception) {
        neoc_free(result->exception);
    }
    
    result->exception = exception ? neoc_strdup(exception) : NULL;
}

// Add stack item
void neoc_invocation_result_add_stack_item(neoc_invocation_result_t* result, neoc_stack_item_t* item) {
    if (!result || !item) {
        return;
    }
    
    // Resize stack array
    neoc_stack_item_t** new_stack = neoc_realloc(
        result->stack,
        sizeof(neoc_stack_item_t*) * (result->stack_count + 1)
    );
    
    if (!new_stack) {
        return;
    }
    
    result->stack = new_stack;
    result->stack[result->stack_count] = item;
    result->stack_count++;
}

// Add notification
void neoc_invocation_result_add_notification(neoc_invocation_result_t* result, neoc_notification_t* notification) {
    if (!result || !notification) {
        return;
    }
    
    // Resize notifications array
    neoc_notification_t** new_notifications = neoc_realloc(
        result->notifications,
        sizeof(neoc_notification_t*) * (result->notifications_count + 1)
    );
    
    if (!new_notifications) {
        return;
    }
    
    result->notifications = new_notifications;
    result->notifications[result->notifications_count] = notification;
    result->notifications_count++;
}

// Parse from JSON
neoc_invocation_result_t* neoc_invocation_result_from_json(const char* json_str) {
    if (!json_str) {
        return NULL;
    }
    
#ifndef HAVE_CJSON
    return NULL; // cJSON not available
#else
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return NULL;
    }
    
    neoc_invocation_result_t* result = neoc_invocation_result_create();
    if (!result) {
        cJSON_Delete(root);
        return NULL;
    }
    
    // Parse script
    cJSON *script = cJSON_GetObjectItem(root, "script");
    if (script && cJSON_IsString(script)) {
        neoc_invocation_result_set_script(result, script->valuestring);
    }
    
    // Parse state
    cJSON *state = cJSON_GetObjectItem(root, "state");
    if (state && cJSON_IsString(state)) {
        if (strcmp(state->valuestring, "HALT") == 0) {
            neoc_invocation_result_set_state(result, NEO_VM_STATE_HALT);
        } else if (strcmp(state->valuestring, "FAULT") == 0) {
            neoc_invocation_result_set_state(result, NEO_VM_STATE_FAULT);
        } else if (strcmp(state->valuestring, "BREAK") == 0) {
            neoc_invocation_result_set_state(result, NEO_VM_STATE_BREAK);
        }
    }
    
    // Parse gas consumed
    cJSON *gas = cJSON_GetObjectItem(root, "gasconsumed");
    if (gas) {
        if (cJSON_IsString(gas)) {
            uint64_t gas_value = strtoull(gas->valuestring, NULL, 10);
            neoc_invocation_result_set_gas_consumed(result, gas_value);
        } else if (cJSON_IsNumber(gas)) {
            neoc_invocation_result_set_gas_consumed(result, (uint64_t)gas->valueint);
        }
    }
    
    // Parse exception
    cJSON *exception = cJSON_GetObjectItem(root, "exception");
    if (exception && cJSON_IsString(exception)) {
        neoc_invocation_result_set_exception(result, exception->valuestring);
    }
    
    // Parse stack items
    cJSON *stack = cJSON_GetObjectItem(root, "stack");
    if (stack && cJSON_IsArray(stack)) {
        size_t stack_size = cJSON_GetArraySize(stack);
        for (size_t i = 0; i < stack_size; i++) {
            cJSON *item = cJSON_GetArrayItem(stack, i);
            if (item) {
                char *item_str = cJSON_Print(item);
                if (item_str) {
                    neoc_stack_item_t *stack_item = neoc_stack_item_from_json(item_str);
                    if (stack_item) {
                        neoc_invocation_result_add_stack_item(result, stack_item);
                    }
                    free(item_str);
                }
            }
        }
    }
    
    cJSON_Delete(root);
    return result;
#endif
}

// Convert to JSON
char* neoc_invocation_result_to_json(const neoc_invocation_result_t* result) {
    if (!result) {
        return NULL;
    }
    
    // Build JSON string
    size_t json_size = 4096;  // Initial size
    char* json = neoc_malloc(json_size);
    if (!json) {
        return NULL;
    }
    
    // Convert VM state to string
    const char* state_str = "NONE";
    switch (result->state) {
        case NEO_VM_STATE_HALT: state_str = "HALT"; break;
        case NEO_VM_STATE_FAULT: state_str = "FAULT"; break;
        case NEO_VM_STATE_BREAK: state_str = "BREAK"; break;
        default: break;
    }
    
    // Start building JSON
    int offset = snprintf(json, json_size,
                          "{\"script\":\"%s\",\"state\":\"%s\",\"gasconsumed\":\"%llu\"",
                          result->script ? result->script : "",
                          state_str,
                          (unsigned long long)result->gas_consumed);
    
    // Add exception if present
    if (result->exception) {
        offset += snprintf(json + offset, json_size - offset,
                          ",\"exception\":\"%s\"", result->exception);
    }
    
    // Add stack items
    if (result->stack_count > 0) {
        offset += snprintf(json + offset, json_size - offset, ",\"stack\":[");
        for (size_t i = 0; i < result->stack_count; i++) {
            if (i > 0) {
                offset += snprintf(json + offset, json_size - offset, ",");
            }
            char* item_json = neoc_stack_item_to_json(result->stack[i]);
            if (item_json) {
                offset += snprintf(json + offset, json_size - offset, "%s", item_json);
                neoc_free(item_json);
            }
        }
        offset += snprintf(json + offset, json_size - offset, "]");
    } else {
        offset += snprintf(json + offset, json_size - offset, ",\"stack\":[]");
    }
    
    // Add notifications
    if (result->notifications_count > 0) {
        offset += snprintf(json + offset, json_size - offset, ",\"notifications\":[");
        for (size_t i = 0; i < result->notifications_count; i++) {
            if (i > 0) {
                offset += snprintf(json + offset, json_size - offset, ",");
            }
            char* notification_json = neoc_notification_to_json(result->notifications[i]);
            if (notification_json) {
                offset += snprintf(json + offset, json_size - offset, "%s", notification_json);
                neoc_free(notification_json);
            }
        }
        offset += snprintf(json + offset, json_size - offset, "]");
    } else {
        offset += snprintf(json + offset, json_size - offset, ",\"notifications\":[]");
    }
    
    // Close JSON object
    snprintf(json + offset, json_size - offset, "}");
    
    return json;
}


// Check if execution was successful
bool neoc_invocation_result_is_successful(const neoc_invocation_result_t* result) {
    return result && result->state == NEO_VM_STATE_HALT && !result->exception;
}

// Get first stack item
neoc_stack_item_t* neoc_invocation_result_get_first_stack_item(const neoc_invocation_result_t* result) {
    if (!result || result->stack_count == 0 || !result->stack) {
        return NULL;
    }
    
    return result->stack[0];
}
