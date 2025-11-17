/**
 * @file wallet_error.c
 * @brief wallet_error implementation
 * 
 * Based on Swift source: wallet/WalletError.swift
 */

#include "neoc/wallet/wallet_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include "neoc/utils/json.h"
#include <string.h>
#include <inttypes.h>

static neoc_error_t wallet_error_set(neoc_wallet_error_type_t type, neoc_error_t code, const char *message) {
    (void)type;
    if (message) {
        neoc_error_set(code, message);
    } else {
        neoc_error_set(code, NULL);
    }
    return code;
}

neoc_wallet_error_t *neoc_wallet_error_create(void) {
    neoc_wallet_error_t *obj = neoc_calloc(1, sizeof(neoc_wallet_error_t));
    return obj;
}

void neoc_wallet_error_free(neoc_wallet_error_t *obj) {
    neoc_free(obj);
}

char *neoc_wallet_error_to_json(const neoc_wallet_error_t *obj) {
    if (!obj) {
        return NULL;
    }

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NULL;
    }

    neoc_json_add_number(root, "type", (double)obj->type);
    neoc_json_add_string(root, "message", obj->message);
    if (obj->required_amount || obj->available_amount) {
        neoc_json_add_number(root, "requiredAmount", (double)obj->required_amount);
        neoc_json_add_number(root, "availableAmount", (double)obj->available_amount);
    }
    if (obj->threshold || obj->provided) {
        neoc_json_add_number(root, "threshold", (double)obj->threshold);
        neoc_json_add_number(root, "provided", (double)obj->provided);
    }

    char *json = neoc_json_to_string(root);
    neoc_json_free(root);
    return json;
}

neoc_wallet_error_t *neoc_wallet_error_from_json(const char *json) {
    if (!json) {
        return NULL;
    }

    neoc_json_t *root = neoc_json_parse(json);
    if (!root) {
        return NULL;
    }

    neoc_wallet_error_t *obj = neoc_wallet_error_create();
    if (!obj) {
        neoc_json_free(root);
        return NULL;
    }

    double value = 0.0;
    if (neoc_json_get_number(root, "type", &value) == NEOC_SUCCESS) {
        obj->type = (neoc_wallet_error_type_t)value;
    }

    const char *message = neoc_json_get_string(root, "message");
    if (message) {
        strncpy(obj->message, message, sizeof(obj->message) - 1);
    }

    if (neoc_json_get_number(root, "requiredAmount", &value) == NEOC_SUCCESS) {
        obj->required_amount = (uint64_t)value;
    }
    if (neoc_json_get_number(root, "availableAmount", &value) == NEOC_SUCCESS) {
        obj->available_amount = (uint64_t)value;
    }
    if (neoc_json_get_number(root, "threshold", &value) == NEOC_SUCCESS) {
        obj->threshold = (int)value;
    }
    if (neoc_json_get_number(root, "provided", &value) == NEOC_SUCCESS) {
        obj->provided = (int)value;
    }

    neoc_json_free(root);
    return obj;
}

neoc_error_t neoc_wallet_error_account_state(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_ACCOUNT_STATE, NEOC_ERROR_WALLET_ACCOUNT, message);
}

neoc_error_t neoc_wallet_error_invalid_password(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_INVALID_PASSWORD, NEOC_ERROR_INVALID_PASSWORD, message);
}

neoc_error_t neoc_wallet_error_encryption_failed(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_ENCRYPTION_FAILED, NEOC_ERROR_WALLET_ACCOUNT, message);
}

neoc_error_t neoc_wallet_error_decryption_failed(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_DECRYPTION_FAILED, NEOC_ERROR_WALLET_DECRYPT, message);
}

neoc_error_t neoc_wallet_error_invalid_mnemonic(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_INVALID_MNEMONIC, NEOC_ERROR_INVALID_ARGUMENT, message);
}

neoc_error_t neoc_wallet_error_invalid_wallet_file(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_INVALID_WALLET_FILE, NEOC_ERROR_INVALID_FORMAT, message);
}

neoc_error_t neoc_wallet_error_wallet_locked(const char *message) {
    return wallet_error_set(NEOC_WALLET_ERROR_WALLET_LOCKED, NEOC_ERROR_WALLET_LOCKED, message);
}

neoc_error_t neoc_wallet_error_account_not_found(const char *address) {
    return wallet_error_set(NEOC_WALLET_ERROR_ACCOUNT_NOT_FOUND, NEOC_ERROR_NOT_FOUND, address);
}

neoc_error_t neoc_wallet_error_insufficient_funds(uint64_t required_amount, uint64_t available_amount) {
    char buffer[NEOC_MAX_ERROR_MESSAGE_LENGTH];
    snprintf(buffer, sizeof(buffer), "Required %" PRIu64 " but only %" PRIu64 " available",
             required_amount, available_amount);
    return wallet_error_set(NEOC_WALLET_ERROR_INSUFFICIENT_FUNDS, NEOC_ERROR_WALLET_ACCOUNT, buffer);
}

neoc_error_t neoc_wallet_error_multisig_threshold(int threshold, int provided) {
    char buffer[NEOC_MAX_ERROR_MESSAGE_LENGTH];
    snprintf(buffer, sizeof(buffer), "Threshold %d but only %d signatures provided", threshold, provided);
    return wallet_error_set(NEOC_WALLET_ERROR_MULTISIG_THRESHOLD, NEOC_ERROR_WALLET_ACCOUNT, buffer);
}

int neoc_wallet_error_get_type(neoc_error_t error_code) {
    switch (error_code) {
        case NEOC_ERROR_WALLET_ACCOUNT:
            return NEOC_WALLET_ERROR_ACCOUNT_STATE;
        case NEOC_ERROR_INVALID_PASSWORD:
            return NEOC_WALLET_ERROR_INVALID_PASSWORD;
        case NEOC_ERROR_WALLET_LOCKED:
            return NEOC_WALLET_ERROR_WALLET_LOCKED;
        case NEOC_ERROR_WALLET_DECRYPT:
            return NEOC_WALLET_ERROR_DECRYPTION_FAILED;
        case NEOC_ERROR_INVALID_FORMAT:
            return NEOC_WALLET_ERROR_INVALID_WALLET_FILE;
        default:
            return -1;
    }
}

bool neoc_wallet_error_is_wallet_error(neoc_error_t error_code) {
    return neoc_wallet_error_get_type(error_code) >= 0;
}

const char *neoc_wallet_error_get_description(neoc_error_t error_code) {
    switch (neoc_wallet_error_get_type(error_code)) {
        case NEOC_WALLET_ERROR_ACCOUNT_STATE:
            return "Wallet account state error";
        case NEOC_WALLET_ERROR_INVALID_PASSWORD:
            return "Invalid wallet password";
        case NEOC_WALLET_ERROR_ENCRYPTION_FAILED:
            return "Failed to encrypt account data";
        case NEOC_WALLET_ERROR_DECRYPTION_FAILED:
            return "Failed to decrypt account data";
        case NEOC_WALLET_ERROR_INVALID_MNEMONIC:
            return "Invalid BIP-39 mnemonic";
        case NEOC_WALLET_ERROR_INVALID_WALLET_FILE:
            return "Invalid wallet file";
        case NEOC_WALLET_ERROR_WALLET_LOCKED:
            return "Wallet is locked";
        case NEOC_WALLET_ERROR_ACCOUNT_NOT_FOUND:
            return "Account not found in wallet";
        case NEOC_WALLET_ERROR_INSUFFICIENT_FUNDS:
            return "Insufficient funds";
        case NEOC_WALLET_ERROR_MULTISIG_THRESHOLD:
            return "Insufficient multisig signatures";
        case NEOC_WALLET_ERROR_INVALID_ADDRESS:
            return "Invalid address";
        case NEOC_WALLET_ERROR_KEY_DERIVATION:
            return "Key derivation failed";
        case NEOC_WALLET_ERROR_BACKUP_FAILED:
            return "Wallet backup failed";
        case NEOC_WALLET_ERROR_RESTORE_FAILED:
            return "Wallet restore failed";
        default:
            return "Unknown wallet error";
    }
}

neoc_error_t neoc_wallet_error_get_detailed_message(neoc_error_t error_code,
                                                    char *buffer,
                                                    size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "wallet_error_get_detailed_message: invalid buffer");
    }

    const neoc_error_info_t *info = neoc_get_last_error();
    if (info && info->code == error_code) {
        snprintf(buffer, buffer_size, "%s", info->message);
        return NEOC_SUCCESS;
    }

    snprintf(buffer, buffer_size, "%s", neoc_wallet_error_get_description(error_code));
    return NEOC_SUCCESS;
}
