#ifndef NEOC_NOTIFICATION_H
#define NEOC_NOTIFICATION_H

#include <stddef.h>
#include <stdbool.h>
#include "../stack_item.h"
#include "../../types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

// Notification structure
typedef struct neoc_notification {
    neoc_hash160_t contract;       // Contract hash that sent the notification
    char* event_name;               // Event name
    neoc_stack_item_t* state;       // Event state/data
} neoc_notification_t;

// Create notification
neoc_notification_t* neoc_notification_create(
    const neoc_hash160_t* contract,
    const char* event_name,
    neoc_stack_item_t* state
);

// Free notification
void neoc_notification_free(neoc_notification_t* notification);

// Clone notification
neoc_notification_t* neoc_notification_clone(const neoc_notification_t* notification);

// Compare notifications
bool neoc_notification_equals(
    const neoc_notification_t* a,
    const neoc_notification_t* b
);

// Parse from JSON
neoc_notification_t* neoc_notification_from_json(const char* json_str);

// Convert to JSON
char* neoc_notification_to_json(const neoc_notification_t* notification);

// Get contract hash string
char* neoc_notification_get_contract_string(const neoc_notification_t* notification);

// Check if notification is for specific event
bool neoc_notification_is_event(const neoc_notification_t* notification, const char* event_name);

#ifdef __cplusplus
}
#endif

#endif // NEOC_NOTIFICATION_H
