/**
 * @file neoc_error.c
 * @brief Error handling implementation
 */

#include "neoc/neoc_error.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    neoc_error_t code;
    const char *message;
} neoc_error_entry_t;

static const neoc_error_entry_t k_error_entries[] = {
    { NEOC_SUCCESS, "Success" },
    { NEOC_ERROR_NULL_POINTER, "Null pointer" },
    { NEOC_ERROR_INVALID_ARGUMENT, "Invalid argument" },
    { NEOC_ERROR_OUT_OF_MEMORY, "Out of memory" },
    { NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small" },
    { NEOC_ERROR_INVALID_STATE, "Invalid state" },
    { NEOC_ERROR_NOT_IMPLEMENTED, "Not implemented" },
    { NEOC_ERROR_INVALID_LENGTH, "Invalid length" },
    { NEOC_ERROR_END_OF_STREAM, "End of stream" },
    { NEOC_ERROR_BUFFER_OVERFLOW, "Buffer overflow" },
    { NEOC_ERROR_INVALID_FORMAT, "Invalid format" },
    { NEOC_ERROR_INVALID_HEX, "Invalid hexadecimal string" },
    { NEOC_ERROR_INVALID_BASE58, "Invalid Base58 string" },
    { NEOC_ERROR_INVALID_BASE64, "Invalid Base64 string" },
    { NEOC_ERROR_DESERIALIZE, "Deserialization failed" },
    { NEOC_ERROR_SERIALIZE, "Serialization failed" },
    { NEOC_ERROR_OUT_OF_BOUNDS, "Index out of bounds" },
    { NEOC_ERROR_NOT_FOUND, "Not found" },
    { NEOC_ERROR_INVALID_SIZE, "Invalid size" },
    { NEOC_ERROR_INVALID_PASSWORD, "Invalid password" },
    { NEOC_ERROR_NOT_SUPPORTED, "Operation not supported" },
    { NEOC_ERROR_CRYPTO, "Cryptographic error" },
    { NEOC_ERROR_CRYPTO_INIT, "Cryptographic initialization failed" },
    { NEOC_ERROR_CRYPTO_INVALID_KEY, "Invalid cryptographic key" },
    { NEOC_ERROR_CRYPTO_SIGN, "Signing operation failed" },
    { NEOC_ERROR_CRYPTO_VERIFY, "Verification failed" },
    { NEOC_ERROR_CRYPTO_HASH, "Hash operation failed" },
    { NEOC_ERROR_CRYPTO_RANDOM, "Random generation failed" },
    { NEOC_ERROR_NETWORK, "Network error" },
    { NEOC_ERROR_PROTOCOL, "Protocol error" },
    { NEOC_ERROR_RPC, "RPC call failed" },
    { NEOC_ERROR_HTTP, "HTTP request failed" },
    { NEOC_ERROR_TX_INVALID, "Invalid transaction" },
    { NEOC_ERROR_TX_SIZE, "Transaction size exceeded" },
    { NEOC_ERROR_TX_SCRIPT, "Transaction script error" },
    { NEOC_ERROR_TX_WITNESS, "Transaction witness error" },
    { NEOC_ERROR_CONTRACT_INVALID, "Invalid contract" },
    { NEOC_ERROR_CONTRACT_INVOKE, "Contract invocation failed" },
    { NEOC_ERROR_CONTRACT_MANIFEST, "Contract manifest error" },
    { NEOC_ERROR_WALLET_INVALID, "Invalid wallet" },
    { NEOC_ERROR_WALLET_LOCKED, "Wallet is locked" },
    { NEOC_ERROR_WALLET_DECRYPT, "Wallet decryption failed" },
    { NEOC_ERROR_WALLET_ACCOUNT, "Wallet account error" },
    { NEOC_ERROR_SYSTEM, "System error" },
    { NEOC_ERROR_IO, "I/O error" },
    { NEOC_ERROR_TIMEOUT, "Operation timed out" },
    { NEOC_ERROR_CANCELLED, "Operation cancelled" },
    { NEOC_ERROR_FILE, "File operation failed" },
    { NEOC_ERROR_FILE_NOT_FOUND, "File not found" },
    { NEOC_ERROR_INTERNAL, "Internal error" }
};

static __thread neoc_error_info_t last_error_info = {
    .code = NEOC_SUCCESS,
    .message = {0},
    .context = {0},
    .file = NULL,
    .line = 0,
    .function = NULL
};

static __thread bool last_error_has_value = false;

static void set_last_error_internal(neoc_error_t code,
                                    const char *message,
                                    const char *context,
                                    const char *file,
                                    int line,
                                    const char *function) {
    last_error_info.code = code;
    if (message) {
        strncpy(last_error_info.message, message, sizeof(last_error_info.message) - 1);
        last_error_info.message[sizeof(last_error_info.message) - 1] = '\0';
    } else {
        last_error_info.message[0] = '\0';
    }

    if (context) {
        strncpy(last_error_info.context, context, sizeof(last_error_info.context) - 1);
        last_error_info.context[sizeof(last_error_info.context) - 1] = '\0';
    } else {
        last_error_info.context[0] = '\0';
    }

    last_error_info.file = file;
    last_error_info.line = line;
    last_error_info.function = function;

    last_error_has_value = (code != NEOC_SUCCESS);
}

static const char* lookup_error_message(neoc_error_t code) {
    for (size_t i = 0; i < sizeof(k_error_entries) / sizeof(k_error_entries[0]); ++i) {
        if (k_error_entries[i].code == code) {
            return k_error_entries[i].message;
        }
    }
    return "Unknown error";
}

const char* neoc_error_string(neoc_error_t error_code) {
    return lookup_error_message(error_code);
}

bool neoc_is_success(neoc_error_t error_code) {
    return error_code == NEOC_SUCCESS;
}

bool neoc_is_error(neoc_error_t error_code) {
    return !neoc_is_success(error_code);
}

neoc_error_t neoc_set_error_info(neoc_error_info_t *info,
                                 neoc_error_t code,
                                 const char *message,
                                 const char *context,
                                 const char *file,
                                 int line,
                                 const char *function) {
    if (!info) {
        set_last_error_internal(NEOC_ERROR_NULL_POINTER,
                                "neoc_set_error_info: info pointer is NULL",
                                NULL,
                                __FILE__,
                                __LINE__,
                                __func__);
        return NEOC_ERROR_NULL_POINTER;
    }

    info->code = code;

    if (message) {
        strncpy(info->message, message, sizeof(info->message) - 1);
        info->message[sizeof(info->message) - 1] = '\0';
    } else {
        info->message[0] = '\0';
    }

    if (context) {
        strncpy(info->context, context, sizeof(info->context) - 1);
        info->context[sizeof(info->context) - 1] = '\0';
    } else {
        info->context[0] = '\0';
    }

    info->file = file;
    info->line = line;
    info->function = function;

    set_last_error_internal(code, message, context, file, line, function);
    return code;
}

const neoc_error_info_t* neoc_get_last_error(void) {
    return last_error_has_value ? &last_error_info : NULL;
}

neoc_error_t neoc_error_set(neoc_error_t code, const char *message) {
    set_last_error_internal(code, message, NULL, NULL, 0, NULL);
    return code;
}

void neoc_clear_last_error(void) {
    last_error_info.code = NEOC_SUCCESS;
    last_error_info.message[0] = '\0';
    last_error_info.context[0] = '\0';
    last_error_info.file = NULL;
    last_error_info.function = NULL;
    last_error_info.line = 0;
    last_error_has_value = false;
}

size_t neoc_format_error(const neoc_error_info_t *info, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return 0;
    }

    if (!info) {
        return (size_t)snprintf(buffer, buffer_size, "No error information available");
    }

    const char *message = info->message[0] != '\0'
                              ? info->message
                              : lookup_error_message(info->code);

    if (info->file && info->function) {
        return (size_t)snprintf(buffer,
                                buffer_size,
                                "%s (code=%d, context=%s, file=%s, line=%d, function=%s)",
                                message,
                                info->code,
                                info->context[0] ? info->context : "n/a",
                                info->file,
                                info->line,
                                info->function);
    }

    return (size_t)snprintf(buffer,
                            buffer_size,
                            "%s (code=%d, context=%s)",
                            message,
                            info->code,
                            info->context[0] ? info->context : "n/a");
}
