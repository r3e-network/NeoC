/**
 * @file neo_get_unspents.c
 * @brief getunspents RPC response implementation
 */

#include "neoc/protocol/core/response/neo_get_unspents.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/utils/json.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str) {
    return str ? neoc_strdup(str) : NULL;
}

static bool parse_double_string(const char *input, double *out) {
    if (!input || !out) {
        return false;
    }
    errno = 0;
    char *endptr = NULL;
    double val = strtod(input, &endptr);
    if (errno != 0 || endptr == input) {
        return false;
    }
    *out = val;
    return true;
}

neoc_error_t neoc_unspent_transaction_create(const char *tx_id,
                                             int index,
                                             double value,
                                             neoc_unspent_transaction_t **unspent_tx) {
    if (!tx_id || !unspent_tx) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *unspent_tx = NULL;
    neoc_unspent_transaction_t *created = neoc_calloc(1, sizeof(neoc_unspent_transaction_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->tx_id = dup_string(tx_id);
    if (!created->tx_id) {
        neoc_unspent_transaction_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->index = index;
    created->value = value;
    *unspent_tx = created;
    return NEOC_SUCCESS;
}

void neoc_unspent_transaction_free(neoc_unspent_transaction_t *unspent_tx) {
    if (!unspent_tx) {
        return;
    }
    neoc_free(unspent_tx->tx_id);
    neoc_free(unspent_tx);
}

static neoc_error_t clone_unspent_array(const neoc_unspent_transaction_t *src,
                                        size_t count,
                                        neoc_unspent_transaction_t **dst_out) {
    if (!dst_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *dst_out = NULL;

    if (!src || count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_unspent_transaction_t *dst = neoc_calloc(count, sizeof(neoc_unspent_transaction_t));
    if (!dst) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        dst[i].index = src[i].index;
        dst[i].value = src[i].value;
        if (src[i].tx_id) {
            dst[i].tx_id = dup_string(src[i].tx_id);
            if (!dst[i].tx_id) {
                for (size_t j = 0; j <= i; j++) {
                    neoc_unspent_transaction_free(&dst[j]);
                }
                neoc_free(dst);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
        }
    }

    *dst_out = dst;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_unspents_balance_create(const neoc_unspent_transaction_t *unspent_transactions,
                                          size_t unspent_transactions_count,
                                          const char *asset_hash,
                                          const char *asset_name,
                                          const char *asset_symbol,
                                          double amount,
                                          neoc_unspents_balance_t **balance) {
    if (!balance || !asset_hash || !asset_symbol) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *balance = NULL;
    neoc_unspents_balance_t *created = neoc_calloc(1, sizeof(neoc_unspents_balance_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->asset_hash = dup_string(asset_hash);
    created->asset_name = dup_string(asset_name);
    created->asset_symbol = dup_string(asset_symbol);
    created->amount = amount;

    if (unspent_transactions_count > 0 && unspent_transactions) {
        neoc_error_t err = clone_unspent_array(unspent_transactions,
                                               unspent_transactions_count,
                                               &created->unspent_transactions);
        if (err != NEOC_SUCCESS) {
            neoc_unspents_balance_free(created);
            return err;
        }
        created->unspent_transactions_count = unspent_transactions_count;
    }

    *balance = created;
    return NEOC_SUCCESS;
}

void neoc_unspents_balance_free(neoc_unspents_balance_t *balance) {
    if (!balance) {
        return;
    }

    if (balance->unspent_transactions) {
        for (size_t i = 0; i < balance->unspent_transactions_count; i++) {
            neoc_unspent_transaction_free(&balance->unspent_transactions[i]);
        }
        neoc_free(balance->unspent_transactions);
    }
    neoc_free(balance->asset_hash);
    neoc_free(balance->asset_name);
    neoc_free(balance->asset_symbol);
    neoc_free(balance);
}

static neoc_error_t clone_balance_array(const neoc_unspents_balance_t *src,
                                        size_t count,
                                        neoc_unspents_balance_t **dst_out) {
    if (!dst_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *dst_out = NULL;

    if (!src || count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_unspents_balance_t *dst = neoc_calloc(count, sizeof(neoc_unspents_balance_t));
    if (!dst) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        dst[i].asset_hash = dup_string(src[i].asset_hash);
        dst[i].asset_name = dup_string(src[i].asset_name);
        dst[i].asset_symbol = dup_string(src[i].asset_symbol);
        dst[i].amount = src[i].amount;

        if ((src[i].asset_hash && !dst[i].asset_hash) ||
            (src[i].asset_name && !dst[i].asset_name) ||
            (src[i].asset_symbol && !dst[i].asset_symbol)) {
            for (size_t j = 0; j <= i; j++) {
                neoc_unspents_balance_free(&dst[j]);
            }
            neoc_free(dst);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }

        if (src[i].unspent_transactions && src[i].unspent_transactions_count > 0) {
            neoc_error_t err = clone_unspent_array(src[i].unspent_transactions,
                                                   src[i].unspent_transactions_count,
                                                   &dst[i].unspent_transactions);
            if (err != NEOC_SUCCESS) {
                for (size_t j = 0; j <= i; j++) {
                    neoc_unspents_balance_free(&dst[j]);
                }
                neoc_free(dst);
                return err;
            }
            dst[i].unspent_transactions_count = src[i].unspent_transactions_count;
        }
    }

    *dst_out = dst;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_unspents_create(const neoc_unspents_balance_t *balances,
                                  size_t balances_count,
                                  const char *address,
                                  neoc_unspents_t **unspents) {
    if (!unspents || !address) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *unspents = NULL;
    neoc_unspents_t *created = neoc_calloc(1, sizeof(neoc_unspents_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->address = dup_string(address);
    if (!created->address) {
        neoc_unspents_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (balances && balances_count > 0) {
        neoc_error_t err = clone_balance_array(balances, balances_count, &created->balances);
        if (err != NEOC_SUCCESS) {
            neoc_unspents_free(created);
            return err;
        }
        created->balances_count = balances_count;
    }

    *unspents = created;
    return NEOC_SUCCESS;
}

void neoc_unspents_free(neoc_unspents_t *unspents) {
    if (!unspents) {
        return;
    }
    if (unspents->balances) {
        for (size_t i = 0; i < unspents->balances_count; i++) {
            neoc_unspents_balance_free(&unspents->balances[i]);
        }
        neoc_free(unspents->balances);
    }
    neoc_free(unspents->address);
    neoc_free(unspents);
}

neoc_error_t neoc_neo_get_unspents_response_create(const char *jsonrpc,
                                                   int id,
                                                   neoc_unspents_t *result,
                                                   const char *error,
                                                   int error_code,
                                                   neoc_neo_get_unspents_response_t **response) {
    if (!response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_neo_get_unspents_response_t *created = neoc_calloc(1, sizeof(neoc_neo_get_unspents_response_t));
    if (!created) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->jsonrpc = dup_string(jsonrpc ? jsonrpc : "2.0");
    if (!created->jsonrpc) {
        neoc_free(created);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    created->id = id;
    created->result = result;
    created->error_code = error_code;

    if (error) {
        created->error = dup_string(error);
        if (!created->error) {
            neoc_neo_get_unspents_response_free(created);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *response = created;
    return NEOC_SUCCESS;
}

void neoc_neo_get_unspents_response_free(neoc_neo_get_unspents_response_t *response) {
    if (!response) {
        return;
    }
    neoc_free(response->jsonrpc);
    neoc_free(response->error);
    if (response->result) {
        neoc_unspents_free(response->result);
    }
    neoc_free(response);
}

static neoc_error_t parse_unspent_transactions(neoc_json_t *array,
                                               neoc_unspent_transaction_t **out,
                                               size_t *count_out) {
    if (!out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }

    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_unspent_transaction_t *items = neoc_calloc(count, sizeof(neoc_unspent_transaction_t));
    if (!items) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        if (!entry) {
            neoc_free(items);
            return NEOC_ERROR_INVALID_FORMAT;
        }

        const char *txid = neoc_json_get_string(entry, "txid");
        if (!txid) {
            txid = neoc_json_get_string(entry, "tx_id");
        }
        int64_t index = 0;
        if (neoc_json_get_int(entry, "n", &index) != NEOC_SUCCESS) {
            neoc_json_get_int(entry, "index", &index);
        }
        double value = 0.0;
        const char *value_str = neoc_json_get_string(entry, "value");
        if (value_str) {
            parse_double_string(value_str, &value);
        }

        if (!txid) {
            neoc_free(items);
            return NEOC_ERROR_INVALID_FORMAT;
        }
        items[i].tx_id = dup_string(txid);
        if (!items[i].tx_id) {
            for (size_t j = 0; j <= i; j++) {
                neoc_unspent_transaction_free(&items[j]);
            }
            neoc_free(items);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        items[i].index = (int)index;
        items[i].value = value;
    }

    *out = items;
    *count_out = count;
    return NEOC_SUCCESS;
}

static neoc_error_t parse_balances(neoc_json_t *array,
                                   neoc_unspents_balance_t **balances_out,
                                   size_t *count_out) {
    if (!balances_out || !count_out) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *balances_out = NULL;
    *count_out = 0;

    if (!array) {
        return NEOC_SUCCESS;
    }

    size_t count = neoc_json_array_size(array);
    if (count == 0) {
        return NEOC_SUCCESS;
    }

    neoc_unspents_balance_t *balances = neoc_calloc(count, sizeof(neoc_unspents_balance_t));
    if (!balances) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < count; i++) {
        neoc_json_t *entry = neoc_json_array_get(array, i);
        if (!entry) {
            neoc_free(balances);
            return NEOC_ERROR_INVALID_FORMAT;
        }

        const char *asset_hash = neoc_json_get_string(entry, "asset_hash");
        if (!asset_hash) {
            asset_hash = neoc_json_get_string(entry, "asset");
        }
        const char *asset_name = neoc_json_get_string(entry, "assetname");
        if (!asset_name) {
            asset_name = neoc_json_get_string(entry, "asset_name");
        }
        const char *asset_symbol = neoc_json_get_string(entry, "symbol");
        if (!asset_symbol) {
            asset_symbol = neoc_json_get_string(entry, "asset_symbol");
        }

        double amount = 0.0;
        const char *amount_str = neoc_json_get_string(entry, "amount");
        if (amount_str) {
            parse_double_string(amount_str, &amount);
        }

        neoc_json_t *unspents_array = neoc_json_get_array(entry, "unspent");
        neoc_unspent_transaction_t *unspents = NULL;
        size_t unspent_count = 0;
        neoc_error_t err = parse_unspent_transactions(unspents_array, &unspents, &unspent_count);
        if (err != NEOC_SUCCESS) {
            for (size_t j = 0; j < i; j++) {
                neoc_unspents_balance_free(&balances[j]);
            }
            neoc_free(balances);
            return err;
        }

        balances[i].asset_hash = dup_string(asset_hash ? asset_hash : "");
        balances[i].asset_name = dup_string(asset_name);
        balances[i].asset_symbol = dup_string(asset_symbol ? asset_symbol : "");
        balances[i].amount = amount;
        balances[i].unspent_transactions = unspents;
        balances[i].unspent_transactions_count = unspent_count;

        if (!balances[i].asset_hash || !balances[i].asset_symbol) {
            for (size_t j = 0; j <= i; j++) {
                neoc_unspents_balance_free(&balances[j]);
            }
            neoc_free(balances);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    *balances_out = balances;
    *count_out = count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_unspents_from_json(const char *json_str,
                                     neoc_unspents_t **unspents) {
    if (!json_str || !unspents) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *unspents = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_json_t *root = neoc_json_get_object(json, "result");
    if (!root) {
        root = json;
    }

    const char *address = neoc_json_get_string(root, "address");
    neoc_json_t *balances_array = neoc_json_get_array(root, "balance");

    neoc_unspents_balance_t *balances = NULL;
    size_t balances_count = 0;
    neoc_error_t err = parse_balances(balances_array, &balances, &balances_count);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(json);
        return err;
    }

    err = neoc_unspents_create(balances, balances_count, address ? address : "", unspents);
    if (balances) {
        for (size_t i = 0; i < balances_count; i++) {
            neoc_unspents_balance_free(&balances[i]);
        }
        neoc_free(balances);
    }
    neoc_json_free(json);
    return err;
}

neoc_error_t neoc_neo_get_unspents_response_from_json(const char *json_str,
                                                      neoc_neo_get_unspents_response_t **response) {
    if (!json_str || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *response = NULL;
    neoc_json_t *json = neoc_json_parse(json_str);
    if (!json) {
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_neo_get_unspents_response_t *parsed = neoc_calloc(1, sizeof(neoc_neo_get_unspents_response_t));
    if (!parsed) {
        neoc_json_free(json);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    parsed->jsonrpc = dup_string(neoc_json_get_string(json, "jsonrpc"));
    if (!parsed->jsonrpc) {
        parsed->jsonrpc = dup_string("2.0");
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
            parsed->error = dup_string(message);
        }
    }

    neoc_json_t *result_obj = neoc_json_get_object(json, "result");
    if (!result_obj) {
        result_obj = json;
    }

    char *result_json_str = neoc_json_to_string(result_obj);
    if (result_json_str) {
        neoc_unspents_t *unspents = NULL;
        neoc_unspents_from_json(result_json_str, &unspents);
        parsed->result = unspents;
        neoc_free(result_json_str);
    }

    *response = parsed;
    neoc_json_free(json);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_unspents_get_asset_balance(const neoc_unspents_t *unspents,
                                             const char *asset_hash,
                                             double *total_balance) {
    if (!unspents || !asset_hash || !total_balance) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    for (size_t i = 0; i < unspents->balances_count; i++) {
        const neoc_unspents_balance_t *balance = &unspents->balances[i];
        if (balance->asset_hash && strcmp(balance->asset_hash, asset_hash) == 0) {
            *total_balance = balance->amount;
            return NEOC_SUCCESS;
        }
    }

    return NEOC_ERROR_NOT_FOUND;
}
