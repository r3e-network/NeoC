/**
 * @file neo_get_token_transfers.c
 * @brief GetTokenTransfers RPC response implementation
 */

#include "neoc/protocol/core/response/neo_get_token_transfers.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

neoc_error_t neoc_token_transfer_create(neoc_token_transfer_t **transfer) {
    if (!transfer) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *transfer = neoc_calloc(1, sizeof(neoc_token_transfer_t));
    return *transfer ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

void neoc_token_transfer_free(neoc_token_transfer_t *transfer) {
    if (!transfer) {
        return;
    }
    if (transfer->asset_hash) {
        neoc_free(transfer->asset_hash);
    }
    if (transfer->tx_hash) {
        neoc_free(transfer->tx_hash);
    }
    neoc_free(transfer->transfer_address);
    neoc_free(transfer->amount);
    neoc_free(transfer);
}

neoc_error_t neoc_token_transfer_create_full(uint64_t timestamp,
                                             const neoc_hash160_t *asset_hash,
                                             const char *transfer_address,
                                             const char *amount,
                                             uint32_t block_index,
                                             uint32_t transfer_notify_index,
                                             const neoc_hash256_t *tx_hash,
                                             neoc_token_transfer_t **transfer) {
    neoc_error_t err = neoc_token_transfer_create(transfer);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    (*transfer)->timestamp = timestamp;
    (*transfer)->block_index = block_index;
    (*transfer)->transfer_notify_index = transfer_notify_index;

    if (asset_hash) {
        (*transfer)->asset_hash = neoc_malloc(sizeof(neoc_hash160_t));
        if (!(*transfer)->asset_hash) {
            neoc_token_transfer_free(*transfer);
            *transfer = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        *(*transfer)->asset_hash = *asset_hash;
    }

    if (tx_hash) {
        (*transfer)->tx_hash = neoc_malloc(sizeof(neoc_hash256_t));
        if (!(*transfer)->tx_hash) {
            neoc_token_transfer_free(*transfer);
            *transfer = NULL;
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        *(*transfer)->tx_hash = *tx_hash;
    }

    (*transfer)->transfer_address = dup_string(transfer_address);
    (*transfer)->amount = dup_string(amount);
    if ((transfer_address && !(*transfer)->transfer_address) ||
        (amount && !(*transfer)->amount)) {
        neoc_token_transfer_free(*transfer);
        *transfer = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_token_transfers_create(neoc_token_transfers_t **transfers) {
    if (!transfers) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *transfers = neoc_calloc(1, sizeof(neoc_token_transfers_t));
    return *transfers ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}

static void free_transfer_array(neoc_token_transfer_t **arr, size_t count) {
    if (!arr) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        neoc_token_transfer_free(arr[i]);
    }
    neoc_free(arr);
}

void neoc_token_transfers_free(neoc_token_transfers_t *transfers) {
    if (!transfers) {
        return;
    }
    free_transfer_array(transfers->sent, transfers->sent_count);
    free_transfer_array(transfers->received, transfers->received_count);
    neoc_free(transfers->transfer_address);
    neoc_free(transfers);
}

neoc_error_t neoc_token_transfers_set_address(neoc_token_transfers_t *transfers,
                                              const char *address) {
    if (!transfers || !address) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    char *copy = dup_string(address);
    if (!copy) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    neoc_free(transfers->transfer_address);
    transfers->transfer_address = copy;
    return NEOC_SUCCESS;
}

static neoc_error_t append_transfer(neoc_token_transfer_t ***array,
                                    size_t *count,
                                    neoc_token_transfer_t *transfer) {
    if (!array || !count || !transfer) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    size_t new_count = *count + 1;
    neoc_token_transfer_t **new_arr = neoc_realloc(*array, new_count * sizeof(neoc_token_transfer_t *));
    if (!new_arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    new_arr[*count] = transfer;
    *array = new_arr;
    *count = new_count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_token_transfers_add_sent(neoc_token_transfers_t *transfers,
                                           neoc_token_transfer_t *transfer) {
    return append_transfer(&transfers->sent, &transfers->sent_count, transfer);
}

neoc_error_t neoc_token_transfers_add_received(neoc_token_transfers_t *transfers,
                                               neoc_token_transfer_t *transfer) {
    return append_transfer(&transfers->received, &transfers->received_count, transfer);
}

neoc_error_t neoc_get_token_transfers_response_create(neoc_get_token_transfers_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *response = neoc_calloc(1, sizeof(neoc_get_token_transfers_response_t));
    if (!*response) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    (*response)->jsonrpc = dup_string("2.0");
    if (!(*response)->jsonrpc) {
        neoc_get_token_transfers_response_free(*response);
        *response = NULL;
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    return NEOC_SUCCESS;
}

void neoc_get_token_transfers_response_free(neoc_get_token_transfers_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error_message);
    if (response->result) {
        neoc_token_transfers_free(response->result);
    }
    neoc_free(response);
}

static neoc_error_t parse_hash160(const char *hex, neoc_hash160_t **out) {
    if (!hex || !out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    neoc_hash160_t *hash = neoc_malloc(sizeof(neoc_hash160_t));
    if (!hash) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    neoc_error_t err = neoc_hash160_from_hex(hash, hex);
    if (err != NEOC_SUCCESS) {
        neoc_free(hash);
        return err;
    }
    *out = hash;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_hash256(const char *hex, neoc_hash256_t **out) {
    if (!hex || !out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    neoc_hash256_t *hash = neoc_malloc(sizeof(neoc_hash256_t));
    if (!hash) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    neoc_error_t err = neoc_hash256_from_hex(hash, hex);
    if (err != NEOC_SUCCESS) {
        neoc_free(hash);
        return err;
    }
    *out = hash;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_transfer(neoc_json_t *obj, neoc_token_transfer_t **out_transfer) {
    if (!obj || !out_transfer) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    uint64_t timestamp = 0;
    int64_t ts_val = 0;
    if (neoc_json_get_int(obj, "timestamp", &ts_val) == NEOC_SUCCESS) {
        timestamp = (uint64_t)ts_val;
    }

    const char *asset_hash_hex = neoc_json_get_string(obj, "assethash");
    if (!asset_hash_hex) {
        asset_hash_hex = neoc_json_get_string(obj, "asset_hash");
    }
    const char *address = neoc_json_get_string(obj, "transferaddress");
    if (!address) {
        address = neoc_json_get_string(obj, "transfer_address");
    }
    const char *amount = neoc_json_get_string(obj, "amount");
    int64_t block_index = 0;
    neoc_json_get_int(obj, "blockindex", &block_index);
    int64_t notify_index = 0;
    neoc_json_get_int(obj, "transfernotifyindex", &notify_index);
    const char *tx_hash_hex = neoc_json_get_string(obj, "txhash");
    if (!tx_hash_hex) {
        tx_hash_hex = neoc_json_get_string(obj, "tx_hash");
    }

    neoc_hash160_t *asset_hash = NULL;
    neoc_hash256_t *tx_hash = NULL;

    if (asset_hash_hex && parse_hash160(asset_hash_hex, &asset_hash) != NEOC_SUCCESS) {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    if (tx_hash_hex && parse_hash256(tx_hash_hex, &tx_hash) != NEOC_SUCCESS) {
        if (asset_hash) {
            neoc_free(asset_hash);
        }
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_token_transfer_t *transfer = NULL;
    neoc_error_t err = neoc_token_transfer_create_full(timestamp,
                                                       asset_hash,
                                                       address,
                                                       amount,
                                                       (uint32_t)block_index,
                                                       (uint32_t)notify_index,
                                                       tx_hash,
                                                       &transfer);

    if (asset_hash) {
        neoc_free(asset_hash);
    }
    if (tx_hash) {
        neoc_free(tx_hash);
    }
    if (err != NEOC_SUCCESS) {
        return err;
    }

    *out_transfer = transfer;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_transfer_array(neoc_json_t *array,
                                         neoc_token_transfer_t ***out_arr,
                                         size_t *out_count) {
    if (!out_arr || !out_count) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out_arr = NULL;
    *out_count = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }

    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_token_transfer_t **items = neoc_calloc(count, sizeof(neoc_token_transfer_t *));
    if (!items) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        neoc_token_transfer_t *transfer = NULL;
        if (!entry || parse_transfer(entry, &transfer) != NEOC_SUCCESS) {
            free_transfer_array(items, i);
            return NEOC_ERROR_INVALID_FORMAT;
        }
        items[i] = transfer;
    }

    *out_arr = items;
    *out_count = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_get_token_transfers_response_from_json(const char *json_str,
                                                         neoc_get_token_transfers_response_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_get_token_transfers_response_t *parsed = NULL;
    neoc_error_t err = neoc_get_token_transfers_response_create(&parsed);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return err;
    }

    const char *jsonrpc = neoc_json_get_string(json, "jsonrpc");
    if (jsonrpc) {
        neoc_free(parsed->jsonrpc);
        parsed->jsonrpc = dup_string(jsonrpc);
    }

    int64_t id_val = 0;
    if (neoc_json_get_int(json, "id", &id_val) == NEOC_SUCCESS) {
        parsed->id = (int)id_val;
    }

    neoc_json_t *error_obj = neoc_json_get_object(json, "error");
    if (error_obj) {
        int64_t code = 0;
        if (neoc_json_get_int(error_obj, "code", &code) == NEOC_SUCCESS) {
            parsed->error_code = (int)code;
        }
        const char *message = neoc_json_get_string(error_obj, "message");
        if (message) {
            parsed->error_message = dup_string(message);
        }
    }

    neoc_json_t *result = neoc_json_get_object(json, "result");
    if (!result) {
        result = json;
    }

    neoc_token_transfers_t *transfers = NULL;
    err = neoc_token_transfers_create(&transfers);
    if (err != NEOC_SUCCESS) {
        neoc_get_token_transfers_response_free(parsed);
        neoc_json_free(json);
        return err;
    }

    const char *address = neoc_json_get_string(result, "address");
    if (address) {
        neoc_token_transfers_set_address(transfers, address);
    }

    neoc_json_t *sent_arr = neoc_json_get_array(result, "sent");
    neoc_json_t *received_arr = neoc_json_get_array(result, "received");

    err = parse_transfer_array(sent_arr, &transfers->sent, &transfers->sent_count);
    if (err == NEOC_SUCCESS) {
        err = parse_transfer_array(received_arr, &transfers->received, &transfers->received_count);
    }

    if (err != NEOC_SUCCESS) {
        neoc_token_transfers_free(transfers);
        neoc_get_token_transfers_response_free(parsed);
        neoc_json_free(json);
        return err;
    }

    parsed->result = transfers;
    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

static neoc_error_t transfer_to_json_object(const neoc_token_transfer_t *transfer, neoc_json_t *obj) {
    if (!transfer || !obj) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    neoc_json_add_int(obj, "timestamp", (int64_t)transfer->timestamp);

    if (transfer->asset_hash) {
        char hash_hex[NEOC_HASH160_STRING_LENGTH] = {0};
        if (neoc_hash160_to_hex(transfer->asset_hash, hash_hex, sizeof(hash_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(obj, "assethash", hash_hex);
        }
    }
    if (transfer->transfer_address) {
        neoc_json_add_string(obj, "transferaddress", transfer->transfer_address);
    }
    if (transfer->amount) {
        neoc_json_add_string(obj, "amount", transfer->amount);
    }
    neoc_json_add_int(obj, "blockindex", transfer->block_index);
    neoc_json_add_int(obj, "transfernotifyindex", transfer->transfer_notify_index);

    if (transfer->tx_hash) {
        char tx_hex[NEOC_HASH256_STRING_LENGTH] = {0};
        if (neoc_hash256_to_hex(transfer->tx_hash, tx_hex, sizeof(tx_hex), false) == NEOC_SUCCESS) {
            neoc_json_add_string(obj, "txhash", tx_hex);
        }
    }
    return NEOC_SUCCESS;
}

static neoc_error_t transfers_array_to_json(neoc_json_t *parent,
                                            const char *key,
                                            neoc_token_transfer_t **array,
                                            size_t count) {
    if (!parent || !key) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    neoc_json_t *arr = neoc_json_create_array();
    if (!arr) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_create_object();
        if (!entry) {
            neoc_json_free(arr);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        if (transfer_to_json_object(array[i], entry) != NEOC_SUCCESS) {
            neoc_json_free(entry);
            neoc_json_free(arr);
            return NEOC_ERROR_INVALID_FORMAT;
        }
        neoc_json_array_add(arr, entry);
    }
    neoc_json_add_object(parent, key, arr);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_get_token_transfers_response_to_json(const neoc_get_token_transfers_response_t *response,
                                                       char **json_str) {
    if (!response || !json_str) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *json_str = NULL;

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (response->jsonrpc) {
        neoc_json_add_string(root, "jsonrpc", response->jsonrpc);
    }
    neoc_json_add_int(root, "id", response->id);

    if (response->error_message) {
        neoc_json_t *error_obj = neoc_json_create_object();
        if (!error_obj) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        neoc_json_add_int(error_obj, "code", response->error_code);
        neoc_json_add_string(error_obj, "message", response->error_message);
        neoc_json_add_object(root, "error", error_obj);
    } else if (response->result) {
        neoc_json_t *result = neoc_json_create_object();
        if (!result) {
            neoc_json_free(root);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        if (response->result->transfer_address) {
            neoc_json_add_string(result, "address", response->result->transfer_address);
        }
        transfers_array_to_json(result, "sent", response->result->sent, response->result->sent_count);
        transfers_array_to_json(result, "received", response->result->received, response->result->received_count);
        neoc_json_add_object(root, "result", result);
    }

    *json_str = neoc_json_to_string(root);
    neoc_json_free(root);
    return *json_str ? NEOC_SUCCESS : NEOC_ERROR_OUT_OF_MEMORY;
}
