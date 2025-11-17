/**
 * @file neo_get_nep11_transfers.c
 * @brief Implementation of the getnep11transfers RPC response.
 */

#include "neoc/protocol/core/response/neo_get_nep11_transfers.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/decode.h"
#include "neoc/types/neoc_hash256.h"

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static char *neoc_strdup_checked(const char *value, neoc_error_t *err_out) {
    if (!value) {
        return NULL;
    }
    char *copy = neoc_strdup(value);
    if (!copy && err_out) {
        *err_out = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers: string duplication failed");
    }
    return copy;
}

neoc_error_t neoc_nep11_transfer_create(const char *token_id,
                                        int64_t timestamp,
                                        const neoc_hash160_t *asset_hash,
                                        const char *transfer_address,
                                        int64_t amount,
                                        int block_index,
                                        int transfer_notify_index,
                                        const neoc_hash256_t *tx_hash,
                                        neoc_nep11_transfer_t **transfer_out) {
    if (!asset_hash || !tx_hash || !transfer_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_transfer: invalid arguments");
    }

    *transfer_out = NULL;

    neoc_nep11_transfer_t *transfer = neoc_calloc(1, sizeof(neoc_nep11_transfer_t));
    if (!transfer) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfer: allocation failed");
    }

    transfer->timestamp = timestamp;
    transfer->amount = amount;
    transfer->block_index = block_index;
    transfer->transfer_notify_index = transfer_notify_index;

    neoc_error_t err = neoc_hash160_copy(&transfer->asset_hash, asset_hash);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_transfer_free(transfer);
        return err;
    }

    err = neoc_hash256_copy(&transfer->tx_hash, tx_hash);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_transfer_free(transfer);
        return err;
    }

    transfer->token_id = neoc_strdup_checked(token_id, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_transfer_free(transfer);
        return err;
    }

    transfer->transfer_address = neoc_strdup_checked(transfer_address, &err);
    if (err != NEOC_SUCCESS) {
        neoc_nep11_transfer_free(transfer);
        return err;
    }

    *transfer_out = transfer;
    return NEOC_SUCCESS;
}

void neoc_nep11_transfer_free(neoc_nep11_transfer_t *transfer) {
    if (!transfer) {
        return;
    }
    neoc_free(transfer->token_id);
    neoc_free(transfer->transfer_address);
    neoc_free(transfer);
}

static void neoc_nep11_transfer_array_free(neoc_nep11_transfer_t *transfers, size_t count) {
    if (!transfers) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        neoc_free(transfers[i].token_id);
        neoc_free(transfers[i].transfer_address);
    }
    neoc_free(transfers);
}

neoc_error_t neoc_nep11_transfers_create(neoc_nep11_transfer_t *sent,
                                         size_t sent_count,
                                         neoc_nep11_transfer_t *received,
                                         size_t received_count,
                                         const char *transfer_address,
                                         neoc_nep11_transfers_t **transfers_out) {
    if (!transfers_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_transfers: output pointer invalid");
    }

    *transfers_out = NULL;

    neoc_nep11_transfers_t *transfers = neoc_calloc(1, sizeof(neoc_nep11_transfers_t));
    if (!transfers) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers: allocation failed");
    }

    transfers->sent = sent;
    transfers->sent_count = sent_count;
    transfers->received = received;
    transfers->received_count = received_count;
    transfers->transfer_address = neoc_strdup(transfer_address);
    if (transfer_address && !transfers->transfer_address) {
        neoc_nep11_transfers_free(transfers);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers: address duplication failed");
    }

    *transfers_out = transfers;
    return NEOC_SUCCESS;
}

void neoc_nep11_transfers_free(neoc_nep11_transfers_t *transfers) {
    if (!transfers) {
        return;
    }

    neoc_nep11_transfer_array_free(transfers->sent, transfers->sent_count);
    neoc_nep11_transfer_array_free(transfers->received, transfers->received_count);
    neoc_free(transfers->transfer_address);
    neoc_free(transfers);
}

neoc_error_t neoc_neo_get_nep11_transfers_create(int id,
                                                  neoc_nep11_transfers_t *nep11_transfers,
                                                  const char *error,
                                                  int error_code,
                                                  neoc_neo_get_nep11_transfers_t **response_out) {
    if (!response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_transfers_response: output pointer invalid");
    }

    *response_out = NULL;

    neoc_neo_get_nep11_transfers_t *response = neoc_calloc(1, sizeof(neoc_neo_get_nep11_transfers_t));
    if (!response) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: allocation failed");
    }

    response->jsonrpc = neoc_strdup("2.0");
    if (!response->jsonrpc) {
        neoc_neo_get_nep11_transfers_free(response);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: jsonrpc allocation failed");
    }

    response->id = id;
    response->result = nep11_transfers;
    response->error_code = error_code;

    if (error) {
        response->error = neoc_strdup(error);
        if (!response->error) {
            neoc_neo_get_nep11_transfers_free(response);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: error duplication failed");
        }
    }

    *response_out = response;
    return NEOC_SUCCESS;
}

void neoc_neo_get_nep11_transfers_free(neoc_neo_get_nep11_transfers_t *response) {
    if (!response) {
        return;
    }

    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    neoc_nep11_transfers_free(response->result);
    neoc_free(response);
}

neoc_nep11_transfers_t *neoc_neo_get_nep11_transfers_get_transfers(const neoc_neo_get_nep11_transfers_t *response) {
    if (!response) {
        return NULL;
    }
    return response->result;
}

bool neoc_neo_get_nep11_transfers_has_transfers(const neoc_neo_get_nep11_transfers_t *response) {
    return response && response->result != NULL;
}

size_t neoc_neo_get_nep11_transfers_get_sent_count(const neoc_neo_get_nep11_transfers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->sent_count;
}

size_t neoc_neo_get_nep11_transfers_get_received_count(const neoc_neo_get_nep11_transfers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->received_count;
}

size_t neoc_neo_get_nep11_transfers_get_total_count(const neoc_neo_get_nep11_transfers_t *response) {
    if (!response || !response->result) {
        return 0;
    }
    return response->result->sent_count + response->result->received_count;
}

neoc_nep11_transfer_t *neoc_neo_get_nep11_transfers_get_sent(const neoc_neo_get_nep11_transfers_t *response,
                                                             size_t index) {
    if (!response || !response->result || index >= response->result->sent_count) {
        return NULL;
    }
    return &response->result->sent[index];
}

neoc_nep11_transfer_t *neoc_neo_get_nep11_transfers_get_received(const neoc_neo_get_nep11_transfers_t *response,
                                                                 size_t index) {
    if (!response || !response->result || index >= response->result->received_count) {
        return NULL;
    }
    return &response->result->received[index];
}

const char *neoc_neo_get_nep11_transfers_get_address(const neoc_neo_get_nep11_transfers_t *response) {
    if (!response || !response->result) {
        return NULL;
    }
    return response->result->transfer_address;
}

#ifdef HAVE_CJSON

static int64_t neoc_nep11_to_int64(const cJSON *node) {
    if (!node) {
        return 0;
    }
    if (cJSON_IsNumber(node)) {
        return (int64_t)node->valuedouble;
    }
    if (cJSON_IsString(node) && node->valuestring) {
        return (int64_t)strtoll(node->valuestring, NULL, 10);
    }
    return 0;
}

static neoc_error_t neoc_nep11_parse_transfer_array(const cJSON *array,
                                                    neoc_nep11_transfer_t **out_transfers,
                                                    size_t *out_count) {
    if (!out_transfers || !out_count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_transfers: parse array invalid outputs");
    }

    *out_transfers = NULL;
    *out_count = 0;

    if (!array || !cJSON_IsArray(array) || cJSON_GetArraySize(array) == 0) {
        return NEOC_SUCCESS;
    }

    int count = cJSON_GetArraySize(array);
    neoc_nep11_transfer_t *transfers = neoc_calloc((size_t)count, sizeof(neoc_nep11_transfer_t));
    if (!transfers) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers: allocation failed");
    }

    for (int i = 0; i < count; ++i) {
        cJSON *entry = cJSON_GetArrayItem(array, i);
        if (!entry || !cJSON_IsObject(entry)) {
            continue;
        }

        transfers[i].timestamp = neoc_nep11_to_int64(cJSON_GetObjectItemCaseSensitive(entry, "timestamp"));
        transfers[i].amount = neoc_nep11_to_int64(cJSON_GetObjectItemCaseSensitive(entry, "amount"));
        transfers[i].block_index = (int)neoc_nep11_to_int64(cJSON_GetObjectItemCaseSensitive(entry, "blockindex"));
        transfers[i].transfer_notify_index = (int)neoc_nep11_to_int64(cJSON_GetObjectItemCaseSensitive(entry, "transfernotifyindex"));

        const cJSON *token_id_item = cJSON_GetObjectItemCaseSensitive(entry, "tokenid");
        if (token_id_item && cJSON_IsString(token_id_item) && token_id_item->valuestring) {
            transfers[i].token_id = neoc_strdup(token_id_item->valuestring);
            if (!transfers[i].token_id) {
                neoc_nep11_transfer_array_free(transfers, (size_t)count);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers: token id duplication failed");
            }
        }

        const cJSON *asset_hash_item = cJSON_GetObjectItemCaseSensitive(entry, "assethash");
        if (asset_hash_item && cJSON_IsString(asset_hash_item) && asset_hash_item->valuestring) {
            neoc_hash160_from_string(asset_hash_item->valuestring, &transfers[i].asset_hash);
        }

        const cJSON *tx_hash_item = cJSON_GetObjectItemCaseSensitive(entry, "txhash");
        if (tx_hash_item && cJSON_IsString(tx_hash_item) && tx_hash_item->valuestring) {
            neoc_hash256_from_string(tx_hash_item->valuestring, &transfers[i].tx_hash);
        }

        const cJSON *address_item = cJSON_GetObjectItemCaseSensitive(entry, "transferaddress");
        if (address_item && cJSON_IsString(address_item) && address_item->valuestring) {
            transfers[i].transfer_address = neoc_strdup(address_item->valuestring);
            if (!transfers[i].transfer_address) {
                neoc_nep11_transfer_array_free(transfers, (size_t)count);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers: transfer address duplication failed");
            }
        }
    }

    *out_transfers = transfers;
    *out_count = (size_t)count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep11_transfers_from_json(const char *json_str,
                                                     neoc_neo_get_nep11_transfers_t **response_out) {
    if (!json_str || !response_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_transfers_response: from_json invalid arguments");
    }

    *response_out = NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "nep11_transfers_response: root must be object");
    }

    neoc_neo_get_nep11_transfers_t *response = neoc_calloc(1, sizeof(neoc_neo_get_nep11_transfers_t));
    if (!response) {
        cJSON_Delete(root);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: allocation failed");
    }

    const cJSON *jsonrpc_item = cJSON_GetObjectItemCaseSensitive(root, "jsonrpc");
    if (jsonrpc_item && cJSON_IsString(jsonrpc_item) && jsonrpc_item->valuestring) {
        response->jsonrpc = neoc_strdup(jsonrpc_item->valuestring);
        if (!response->jsonrpc) {
            neoc_neo_get_nep11_transfers_free(response);
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: jsonrpc duplication failed");
        }
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (id_item) {
        if (cJSON_IsNumber(id_item)) {
            response->id = id_item->valueint;
        } else if (cJSON_IsString(id_item) && id_item->valuestring) {
            neoc_decode_int_from_string(id_item->valuestring, &response->id);
        }
    }

    const cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(root, "error");
    if (error_obj && cJSON_IsObject(error_obj)) {
        const cJSON *code_item = cJSON_GetObjectItemCaseSensitive(error_obj, "code");
        if (code_item && cJSON_IsNumber(code_item)) {
            response->error_code = code_item->valueint;
        }

        const cJSON *message_item = cJSON_GetObjectItemCaseSensitive(error_obj, "message");
        if (message_item && cJSON_IsString(message_item) && message_item->valuestring) {
            response->error = neoc_strdup(message_item->valuestring);
            if (!response->error) {
                neoc_neo_get_nep11_transfers_free(response);
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: error duplication failed");
            }
        }
    }

    if (!response->error_code) {
        const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(root, "result");
        if (result_obj && cJSON_IsObject(result_obj)) {
            neoc_nep11_transfer_t *sent = NULL;
            size_t sent_count = 0;
            neoc_error_t err = neoc_nep11_parse_transfer_array(
                cJSON_GetObjectItemCaseSensitive(result_obj, "sent"),
                &sent,
                &sent_count);
            if (err != NEOC_SUCCESS) {
                neoc_neo_get_nep11_transfers_free(response);
                cJSON_Delete(root);
                return err;
            }

            neoc_nep11_transfer_t *received = NULL;
            size_t received_count = 0;
            err = neoc_nep11_parse_transfer_array(
                cJSON_GetObjectItemCaseSensitive(result_obj, "received"),
                &received,
                &received_count);
            if (err != NEOC_SUCCESS) {
                neoc_nep11_transfer_array_free(sent, sent_count);
                neoc_neo_get_nep11_transfers_free(response);
                cJSON_Delete(root);
                return err;
            }

            const cJSON *address_item = cJSON_GetObjectItemCaseSensitive(result_obj, "address");
            const char *address_value = (address_item && cJSON_IsString(address_item)) ? address_item->valuestring : NULL;

            neoc_nep11_transfers_t *transfers = NULL;
            err = neoc_nep11_transfers_create(sent, sent_count, received, received_count, address_value, &transfers);
            if (err != NEOC_SUCCESS) {
                neoc_nep11_transfer_array_free(sent, sent_count);
                neoc_nep11_transfer_array_free(received, received_count);
                neoc_neo_get_nep11_transfers_free(response);
                cJSON_Delete(root);
                return err;
            }

            response->result = transfers;
        }
    }

    *response_out = response;
    cJSON_Delete(root);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_neo_get_nep11_transfers_to_json(const neoc_neo_get_nep11_transfers_t *response,
                                                   char **json_str_out) {
    if (!response || !json_str_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "nep11_transfers_response: to_json invalid arguments");
    }

    *json_str_out = NULL;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: JSON object allocation failed");
    }

    if (response->jsonrpc) {
        cJSON_AddStringToObject(root, "jsonrpc", response->jsonrpc);
    }
    cJSON_AddNumberToObject(root, "id", response->id);

    if (response->error_code || response->error) {
        cJSON *error_obj = cJSON_CreateObject();
        if (!error_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: error object allocation failed");
        }
        cJSON_AddItemToObject(root, "error", error_obj);
        cJSON_AddNumberToObject(error_obj, "code", response->error_code);
        if (response->error) {
            cJSON_AddStringToObject(error_obj, "message", response->error);
        }
    } else if (response->result) {
        cJSON *result_obj = cJSON_CreateObject();
        if (!result_obj) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: result object allocation failed");
        }
        cJSON_AddItemToObject(root, "result", result_obj);
        if (response->result->transfer_address) {
            cJSON_AddStringToObject(result_obj, "address", response->result->transfer_address);
        }

        cJSON *sent_array = cJSON_CreateArray();
        if (!sent_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: sent array allocation failed");
        }
        cJSON_AddItemToObject(result_obj, "sent", sent_array);

        for (size_t i = 0; i < response->result->sent_count; ++i) {
            neoc_nep11_transfer_t *transfer = &response->result->sent[i];
            cJSON *entry = cJSON_CreateObject();
            if (!entry) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: sent entry allocation failed");
            }

            if (transfer->token_id) cJSON_AddStringToObject(entry, "tokenid", transfer->token_id);
            cJSON_AddNumberToObject(entry, "timestamp", (double)transfer->timestamp);
            cJSON_AddNumberToObject(entry, "amount", (double)transfer->amount);
            cJSON_AddNumberToObject(entry, "blockindex", transfer->block_index);
            cJSON_AddNumberToObject(entry, "transfernotifyindex", transfer->transfer_notify_index);

            char hash160_buffer[NEOC_HASH160_STRING_LENGTH];
            char prefixed_hash160[NEOC_HASH160_STRING_LENGTH + 2];
            if (neoc_hash160_to_string(&transfer->asset_hash, hash160_buffer, sizeof(hash160_buffer)) == NEOC_SUCCESS) {
                prefixed_hash160[0] = '0';
                prefixed_hash160[1] = 'x';
                memcpy(prefixed_hash160 + 2, hash160_buffer, sizeof(hash160_buffer));
                cJSON_AddStringToObject(entry, "assethash", prefixed_hash160);
            }

            char hash256_buffer[NEOC_HASH256_STRING_LENGTH];
            char prefixed_hash256[NEOC_HASH256_STRING_LENGTH + 2];
            if (neoc_hash256_to_string(&transfer->tx_hash, hash256_buffer, sizeof(hash256_buffer)) == NEOC_SUCCESS) {
                prefixed_hash256[0] = '0';
                prefixed_hash256[1] = 'x';
                memcpy(prefixed_hash256 + 2, hash256_buffer, sizeof(hash256_buffer));
                cJSON_AddStringToObject(entry, "txhash", prefixed_hash256);
            }

            if (transfer->transfer_address) {
                cJSON_AddStringToObject(entry, "transferaddress", transfer->transfer_address);
            }

            cJSON_AddItemToArray(sent_array, entry);
        }

        cJSON *received_array = cJSON_CreateArray();
        if (!received_array) {
            cJSON_Delete(root);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: received array allocation failed");
        }
        cJSON_AddItemToObject(result_obj, "received", received_array);

        for (size_t i = 0; i < response->result->received_count; ++i) {
            neoc_nep11_transfer_t *transfer = &response->result->received[i];
            cJSON *entry = cJSON_CreateObject();
            if (!entry) {
                cJSON_Delete(root);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: received entry allocation failed");
            }

            if (transfer->token_id) cJSON_AddStringToObject(entry, "tokenid", transfer->token_id);
            cJSON_AddNumberToObject(entry, "timestamp", (double)transfer->timestamp);
            cJSON_AddNumberToObject(entry, "amount", (double)transfer->amount);
            cJSON_AddNumberToObject(entry, "blockindex", transfer->block_index);
            cJSON_AddNumberToObject(entry, "transfernotifyindex", transfer->transfer_notify_index);

            char hash160_buffer[NEOC_HASH160_STRING_LENGTH];
            char prefixed_hash160[NEOC_HASH160_STRING_LENGTH + 2];
            if (neoc_hash160_to_string(&transfer->asset_hash, hash160_buffer, sizeof(hash160_buffer)) == NEOC_SUCCESS) {
                prefixed_hash160[0] = '0';
                prefixed_hash160[1] = 'x';
                memcpy(prefixed_hash160 + 2, hash160_buffer, sizeof(hash160_buffer));
                cJSON_AddStringToObject(entry, "assethash", prefixed_hash160);
            }

            char hash256_buffer[NEOC_HASH256_STRING_LENGTH];
            char prefixed_hash256[NEOC_HASH256_STRING_LENGTH + 2];
            if (neoc_hash256_to_string(&transfer->tx_hash, hash256_buffer, sizeof(hash256_buffer)) == NEOC_SUCCESS) {
                prefixed_hash256[0] = '0';
                prefixed_hash256[1] = 'x';
                memcpy(prefixed_hash256 + 2, hash256_buffer, sizeof(hash256_buffer));
                cJSON_AddStringToObject(entry, "txhash", prefixed_hash256);
            }

            if (transfer->transfer_address) {
                cJSON_AddStringToObject(entry, "transferaddress", transfer->transfer_address);
            }

            cJSON_AddItemToArray(received_array, entry);
        }
    }

    char *rendered = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!rendered) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: JSON render failed");
    }

    *json_str_out = neoc_strdup(rendered);
    cJSON_free(rendered);

    if (!*json_str_out) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "nep11_transfers_response: JSON duplication failed");
    }

    return NEOC_SUCCESS;
}

#else

neoc_error_t neoc_neo_get_nep11_transfers_from_json(const char *json_str,
                                                     neoc_neo_get_nep11_transfers_t **response_out) {
    (void)json_str;
    (void)response_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "nep11_transfers_response: JSON parsing requires cJSON");
}

neoc_error_t neoc_neo_get_nep11_transfers_to_json(const neoc_neo_get_nep11_transfers_t *response,
                                                   char **json_str_out) {
    (void)response;
    (void)json_str_out;
    return neoc_error_set(NEOC_ERROR_NOT_SUPPORTED, "nep11_transfers_response: JSON serialization requires cJSON");
}

#endif /* HAVE_CJSON */
