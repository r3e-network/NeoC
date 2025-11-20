/**
 * @file transaction_send_token.c
 * @brief TransactionSendToken RPC model implementation
 */

#include "neoc/protocol/core/response/transaction_send_token.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_transaction_send_token_create(neoc_transaction_send_token_t **send_token) {
    if (!send_token) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *send_token = neoc_calloc(1, sizeof(neoc_transaction_send_token_t));
    return *send_token ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

void neoc_transaction_send_token_free(neoc_transaction_send_token_t *send_token) {
    if (!send_token) {
        return;
    }
    if (send_token->token) {
        neoc_free(send_token->token);
    }
    neoc_free(send_token->address);
    neoc_free(send_token);
}

neoc_error_t neoc_transaction_send_token_create_with_values(const neoc_hash160_t *token,
                                                            int64_t value,
                                                            const char *address,
                                                            neoc_transaction_send_token_t **send_token) {
    if (!token || !send_token) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    neoc_error_t err = neoc_transaction_send_token_create(send_token);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    (*send_token)->token = neoc_malloc(sizeof(neoc_hash160_t));
    if (!(*send_token)->token) {
        neoc_transaction_send_token_free(*send_token);
        *send_token = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    *(*send_token)->token = *token;
    (*send_token)->value = value;
    (*send_token)->address = dup_string(address);
    if (address && !(*send_token)->address) {
        neoc_transaction_send_token_free(*send_token);
        *send_token = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_transaction_send_token_from_json(const char *json_str,
                                                   neoc_transaction_send_token_t **send_token) {
    if (!json_str || !send_token) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *send_token = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    const char *token_hex = neoc_json_get_string(json, "token");
    if (!token_hex) {
        token_hex = neoc_json_get_string(json, "asset");
    }
    int64_t value = 0;
    neoc_json_get_int(json, "value", &value);
    const char *address = neoc_json_get_string(json, "address");

    neoc_hash160_t token_hash;
    if (!token_hex || neoc_hash160_from_hex(&token_hash, token_hex) != NEOC_SUCCESS) {
        neoc_json_free(json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_error_t err = neoc_transaction_send_token_create_with_values(&token_hash, value, address, send_token);
    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_transaction_send_token_to_json(const neoc_transaction_send_token_t *send_token,
                                                 char **json_str) {
    if (!send_token || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *json = neoc_json_create_object();
    if (!json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (send_token->token) {
        char hash_hex[NEOC_HASH160_STRING_LENGTH] = {0};
        if (neoc_hash160_to_hex(send_token->token, hash_hex, sizeof(hash_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(json, "token", hash_hex);
        }
    }
    neoc_json_add_int(json, "value", send_token->value);
    if (send_token->address) {
        neoc_json_add_string(json, "address", send_token->address);
    }

    *json_str = neoc_json_to_string(json);
    neoc_json_free(json);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

neoc_error_t neoc_transaction_send_token_copy(const neoc_transaction_send_token_t *src,
                                              neoc_transaction_send_token_t **dest) {
    if (!src || !dest) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    return neoc_transaction_send_token_create_with_values(src->token, src->value, src->address, dest);
}

bool neoc_transaction_send_token_equals(const neoc_transaction_send_token_t *token1,
                                        const neoc_transaction_send_token_t *token2) {
    if (token1 == token2) {
        return true;
    }
    if (!token1 || !token2 || !token1->token || !token2->token) {
        return false;
    }
    if (token1->value != token2->value) {
        return false;
    }
    const char *addr1 = token1->address ? token1->address : "";
    const char *addr2 = token2->address ? token2->address : "";
    if (strcmp(addr1, addr2) != 0) {
        return false;
    }
    return memcmp(token1->token->data, token2->token->data, NEOC_HASH160_SIZE) == 0;
}
