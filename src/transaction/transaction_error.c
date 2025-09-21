/**
 * @file transaction_error.c
 * @brief Transaction error handling implementation
 */

#include "neoc/transaction/transaction_error.h"
#include "neoc/neoc_memory.h"
#include <string.h>

static const char* transaction_error_messages[] = {
    [NEOC_TX_ERROR_NONE] = "No error",
    [NEOC_TX_ERROR_SCRIPT_FORMAT] = "Invalid script format",
    [NEOC_TX_ERROR_SIGNER_CONFIG] = "Invalid signer configuration",
    [NEOC_TX_ERROR_TRANSACTION_CONFIG] = "Invalid transaction configuration",
    [NEOC_TX_ERROR_INVALID_WITNESS] = "Invalid transaction witness",
    [NEOC_TX_ERROR_INSUFFICIENT_FUNDS] = "Insufficient funds",
    [NEOC_TX_ERROR_INVALID_SIGNATURE] = "Invalid signature"
};

neoc_error_t neoc_tx_error_create(neoc_tx_error_type_t type, 
                                   const char *message,
                                   neoc_tx_error_t **error) {
    if (!error) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid error pointer");
    }
    
    *error = neoc_malloc(sizeof(neoc_tx_error_t));
    if (!*error) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate transaction error");
    }
    
    (*error)->type = type;
    if (message) {
        strncpy((*error)->message, message, sizeof((*error)->message) - 1);
        (*error)->message[sizeof((*error)->message) - 1] = '\0';
    } else {
        const char *default_msg = (type < sizeof(transaction_error_messages) / sizeof(transaction_error_messages[0])) 
                                 ? transaction_error_messages[type] 
                                 : "Unknown error";
        strncpy((*error)->message, default_msg, sizeof((*error)->message) - 1);
        (*error)->message[sizeof((*error)->message) - 1] = '\0';
    }
    
    return NEOC_SUCCESS;
}

void neoc_tx_error_free(neoc_tx_error_t *error) {
    if (error) {
        neoc_free(error);
    }
}

const char* neoc_tx_error_get_message(const neoc_tx_error_t *error) {
    if (!error) {
        return "NULL error";
    }
    return error->message;
}
