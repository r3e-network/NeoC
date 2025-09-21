/**
 * @file transaction_error.h
 * @brief Header converted from TransactionError.swift
 */

#ifndef NEOC_TRANSACTION_ERROR_H
#define NEOC_TRANSACTION_ERROR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Transaction error types
 */
typedef enum {
    NEOC_TX_ERROR_NONE = 0,
    NEOC_TX_ERROR_SCRIPT_FORMAT,
    NEOC_TX_ERROR_SIGNER_CONFIG,
    NEOC_TX_ERROR_TRANSACTION_CONFIG,
    NEOC_TX_ERROR_INVALID_WITNESS,
    NEOC_TX_ERROR_INSUFFICIENT_FUNDS,
    NEOC_TX_ERROR_INVALID_SIGNATURE
} neoc_tx_error_type_t;

/**
 * Transaction error structure
 */
typedef struct neoc_tx_error {
    neoc_tx_error_type_t type;
    char message[256];
} neoc_tx_error_t;

/**
 * Create a transaction error
 * @param type Error type
 * @param message Error message
 * @param error Output error structure
 * @return Error code
 */
neoc_error_t neoc_tx_error_create(neoc_tx_error_type_t type, 
                                   const char *message,
                                   neoc_tx_error_t **error);

/**
 * Free a transaction error
 * @param error Error to free
 */
void neoc_tx_error_free(neoc_tx_error_t *error);

/**
 * Get error message
 * @param error Error structure
 * @return Error message string
 */
const char* neoc_tx_error_get_message(const neoc_tx_error_t *error);

#ifdef __cplusplus
}
#endif

#endif // NEOC_TRANSACTION_ERROR_H
