/**
 * @file wallet_error.h
 * @brief Wallet-specific error types and utilities
 * 
 * Based on Swift source: wallet/WalletError.swift
 * Provides wallet-specific error handling functionality
 */

#ifndef NEOC_WALLET_WALLET_ERROR_H
#define NEOC_WALLET_WALLET_ERROR_H

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wallet-specific error types
 * 
 * Equivalent to Swift WalletError enum cases
 */
typedef enum {
    NEOC_WALLET_ERROR_ACCOUNT_STATE = 0x1000,    /**< Account state error */
    NEOC_WALLET_ERROR_INVALID_PASSWORD,          /**< Invalid password */
    NEOC_WALLET_ERROR_ENCRYPTION_FAILED,         /**< Encryption operation failed */
    NEOC_WALLET_ERROR_DECRYPTION_FAILED,         /**< Decryption operation failed */
    NEOC_WALLET_ERROR_INVALID_MNEMONIC,          /**< Invalid BIP-39 mnemonic */
    NEOC_WALLET_ERROR_INVALID_WALLET_FILE,       /**< Invalid NEP-6 wallet file */
    NEOC_WALLET_ERROR_WALLET_LOCKED,             /**< Wallet is locked */
    NEOC_WALLET_ERROR_ACCOUNT_NOT_FOUND,         /**< Account not found in wallet */
    NEOC_WALLET_ERROR_INSUFFICIENT_FUNDS,        /**< Insufficient funds for operation */
    NEOC_WALLET_ERROR_MULTISIG_THRESHOLD,        /**< Multi-signature threshold error */
    NEOC_WALLET_ERROR_INVALID_ADDRESS,           /**< Invalid address format */
    NEOC_WALLET_ERROR_KEY_DERIVATION,            /**< Key derivation error */
    NEOC_WALLET_ERROR_BACKUP_FAILED,             /**< Wallet backup failed */
    NEOC_WALLET_ERROR_RESTORE_FAILED             /**< Wallet restore failed */
} neoc_wallet_error_type_t;

/**
 * @brief Structured wallet error payload
 */
typedef struct {
    neoc_wallet_error_type_t type;                       /**< Wallet error classification */
    char message[NEOC_MAX_ERROR_MESSAGE_LENGTH];         /**< Human-readable message */
    uint64_t required_amount;                            /**< Optional required amount */
    uint64_t available_amount;                           /**< Optional available amount */
    int threshold;                                       /**< Optional multisig threshold */
    int provided;                                        /**< Optional provided signatures */
} neoc_wallet_error_t;

neoc_wallet_error_t *neoc_wallet_error_create(void);
void neoc_wallet_error_free(neoc_wallet_error_t *obj);
char *neoc_wallet_error_to_json(const neoc_wallet_error_t *obj);
neoc_wallet_error_t *neoc_wallet_error_from_json(const char *json);

/**
 * @brief Set a wallet-specific error with account state message
 * 
 * Equivalent to Swift WalletError.accountState(_ message:)
 * 
 * @param message Error message describing the account state issue
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_account_state(const char *message);

/**
 * @brief Set a wallet error for invalid password
 * 
 * @param message Optional additional message
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_invalid_password(const char *message);

/**
 * @brief Set a wallet error for encryption failure
 * 
 * @param message Optional additional message
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_encryption_failed(const char *message);

/**
 * @brief Set a wallet error for decryption failure
 * 
 * @param message Optional additional message
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_decryption_failed(const char *message);

/**
 * @brief Set a wallet error for invalid mnemonic
 * 
 * @param message Optional additional message
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_invalid_mnemonic(const char *message);

/**
 * @brief Set a wallet error for invalid wallet file
 * 
 * @param message Optional additional message
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_invalid_wallet_file(const char *message);

/**
 * @brief Set a wallet error for locked wallet
 * 
 * @param message Optional additional message
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_wallet_locked(const char *message);

/**
 * @brief Set a wallet error for account not found
 * 
 * @param address Address of the account that was not found
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_account_not_found(const char *address);

/**
 * @brief Set a wallet error for insufficient funds
 * 
 * @param required_amount Required amount
 * @param available_amount Available amount
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_insufficient_funds(uint64_t required_amount, uint64_t available_amount);

/**
 * @brief Set a wallet error for multi-signature threshold issues
 * 
 * @param threshold Required threshold
 * @param provided Number of signatures provided
 * @return Wallet error code
 */
neoc_error_t neoc_wallet_error_multisig_threshold(int threshold, int provided);

/**
 * @brief Get wallet error type from error code
 * 
 * @param error_code Error code to check
 * @return Wallet error type, or -1 if not a wallet error
 */
int neoc_wallet_error_get_type(neoc_error_t error_code);

/**
 * @brief Check if error code is a wallet error
 * 
 * @param error_code Error code to check
 * @return True if it's a wallet error
 */
bool neoc_wallet_error_is_wallet_error(neoc_error_t error_code);

/**
 * @brief Get human-readable description of wallet error
 * 
 * Equivalent to Swift WalletError.errorDescription
 * 
 * @param error_code Wallet error code
 * @return Error description string (do not free)
 */
const char *neoc_wallet_error_get_description(neoc_error_t error_code);

/**
 * @brief Get detailed error message with context
 * 
 * @param error_code Wallet error code
 * @param buffer Output buffer for detailed message
 * @param buffer_size Size of output buffer
 * @return NEOC_SUCCESS on success, error code otherwise
 */
neoc_error_t neoc_wallet_error_get_detailed_message(neoc_error_t error_code,
                                                     char *buffer,
                                                     size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* NEOC_WALLET_WALLET_ERROR_H */
