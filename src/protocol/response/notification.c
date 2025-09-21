#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../../include/neoc/protocol/response/notification.h"
#include "../../../include/neoc/neoc_memory.h"
#include "../../../include/neoc/utils/json.h"

// Create notification
neoc_notification_t* neoc_notification_create(
    const neoc_hash160_t* contract,
    const char* event_name,
    neoc_stack_item_t* state) {
    
    if (!contract || !event_name) {
        return NULL;
    }
    
    neoc_notification_t* notification = neoc_malloc(sizeof(neoc_notification_t));
    if (!notification) {
        return NULL;
    }
    
    // Copy contract hash
    memcpy(&notification->contract, contract, sizeof(neoc_hash160_t));
    
    // Copy event name
    notification->event_name = neoc_strdup(event_name);
    if (!notification->event_name) {
        neoc_free(notification);
        return NULL;
    }
    
    // Store state (can be NULL)
    notification->state = state;
    
    return notification;
}

// Free notification
void neoc_notification_free(neoc_notification_t* notification) {
    if (!notification) {
        return;
    }
    
    if (notification->event_name) {
        neoc_free(notification->event_name);
    }
    
    if (notification->state) {
        neoc_stack_item_free(notification->state);
    }
    
    neoc_free(notification);
}

// Clone notification
neoc_notification_t* neoc_notification_clone(const neoc_notification_t* notification) {
    if (!notification) {
        return NULL;
    }
    
    neoc_stack_item_t* cloned_state = NULL;
    if (notification->state) {
        cloned_state = neoc_stack_item_clone(notification->state);
        if (!cloned_state) {
            return NULL;
        }
    }
    
    neoc_notification_t* clone = neoc_notification_create(
        &notification->contract,
        notification->event_name,
        cloned_state
    );
    
    if (!clone && cloned_state) {
        neoc_stack_item_free(cloned_state);
    }
    
    return clone;
}

// Compare notifications
bool neoc_notification_equals(
    const neoc_notification_t* a,
    const neoc_notification_t* b) {
    
    if (!a && !b) return true;
    if (!a || !b) return false;
    
    // Compare contract hash
    if (memcmp(&a->contract, &b->contract, sizeof(neoc_hash160_t)) != 0) {
        return false;
    }
    
    // Compare event name
    if (strcmp(a->event_name, b->event_name) != 0) {
        return false;
    }
    
    // Compare state
    if (!a->state && !b->state) return true;
    if (!a->state || !b->state) return false;
    
    return neoc_stack_item_equals(a->state, b->state);
}

// Parse from JSON
neoc_notification_t* neoc_notification_from_json(const char* json_str) {
    if (!json_str) {
        return NULL;
    }
    
    // Parse JSON
    neoc_json_t* json = neoc_json_parse(json_str);
    if (!json) {
        return NULL;
    }
    
    neoc_notification_t* notification = NULL;
    
    // Get contract hash
    const char* contract_str = neoc_json_get_string(json, "contract");
    if (!contract_str) {
        neoc_json_free(json);
        return NULL;
    }
    
    neoc_hash160_t contract;
    if (neoc_hash160_from_string(contract_str, &contract) != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NULL;
    }
    
    // Get event name
    const char* event_name = neoc_json_get_string(json, "eventname");
    if (!event_name) {
        neoc_json_free(json);
        return NULL;
    }
    
    // Create notification
    notification = neoc_notification_create(&contract, event_name, NULL);
    
    neoc_json_free(json);
    return notification;
}

// Convert to JSON
char* neoc_notification_to_json(const neoc_notification_t* notification) {
    if (!notification) {
        return NULL;
    }
    
    // Get contract hash string
    char contract_str[41];
    neoc_hash160_to_string(&notification->contract, contract_str, sizeof(contract_str));
    
    // Get state JSON
    char* state_json = NULL;
    if (notification->state) {
        state_json = neoc_stack_item_to_json(notification->state);
    }
    
    // Calculate required buffer size
    size_t json_size = strlen("{\"contract\":\"\",\"eventname\":\"\",\"state\":}") +
                       strlen(contract_str) +
                       strlen(notification->event_name) +
                       (state_json ? strlen(state_json) : 4) + // "null"
                       100;
    
    char* json = neoc_malloc(json_size);
    if (!json) {
        if (state_json) neoc_free(state_json);
        return NULL;
    }
    
    snprintf(json, json_size,
             "{\"contract\":\"%s\",\"eventname\":\"%s\",\"state\":%s}",
             contract_str,
             notification->event_name,
             state_json ? state_json : "null");
    
    if (state_json) {
        neoc_free(state_json);
    }
    
    return json;
}



// Get contract hash string
char* neoc_notification_get_contract_string(const neoc_notification_t* notification) {
    if (!notification) {
        return NULL;
    }
    
    char* contract_str = neoc_malloc(41);
    if (!contract_str) {
        return NULL;
    }
    
    neoc_hash160_to_string(&notification->contract, contract_str, 41);
    return contract_str;
}

// Check if notification is for specific event
bool neoc_notification_is_event(const neoc_notification_t* notification, const char* event_name) {
    if (!notification || !notification->event_name || !event_name) {
        return false;
    }
    
    return strcmp(notification->event_name, event_name) == 0;
}
