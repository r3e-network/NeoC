/**
 * @file neo.c
 * @brief Core Neo protocol interface implementation
 * 
 * This implementation provides the core Neo blockchain protocol functionality
 * for RPC communication, smart contract interaction, and blockchain data retrieval.
 * Based on the Swift Neo protocol implementation.
 */

#include "neoc/neoc.h"
#include "neoc/protocol/core/neo.h"
#include "neoc/protocol/core/request.h"
#include "neoc/protocol/core/response/neo_validate_address.h"
#include "neoc/protocol/core/response/neo_network_fee.h"
#include "neoc/protocol/rpc_client.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/utils/neoc_base64.h"
#include "neoc/neo_constants.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/types/neoc_hash256.h"
#include "neoc/transaction/witness_scope.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CJSON
#include <cjson/cJSON.h>
#endif

static neoc_error_t neoc_ensure_rpc_client(const neoc_neo_client_t *client) {
    if (!client) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Neo client reference is NULL");
    }
    if (!client->rpc_client) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Neo client RPC layer not initialized");
    }
    return NEOC_SUCCESS;
}

#ifdef HAVE_CJSON

static neoc_error_t neoc_contract_param_to_cjson(const neoc_contract_parameter_t *param, cJSON **out);
static neoc_error_t neoc_contract_params_to_cjson_array(const neoc_contract_parameter_t *params,
                                                       size_t count,
                                                       cJSON **out);
static neoc_error_t neoc_signer_to_cjson(const neoc_signer_t *signer, cJSON **out);
static neoc_error_t neoc_signers_to_cjson_array(const neoc_signer_t *signers,
                                                size_t count,
                                                cJSON **out);

static neoc_error_t neoc_contract_param_to_cjson(const neoc_contract_parameter_t *param, cJSON **out) {
    if (!param || !out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid contract parameter");
    }

    cJSON *obj = cJSON_CreateObject();
    if (!obj) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate JSON object");
    }

    const char *type_str = neoc_contract_param_type_to_string(param->type);
    if (!type_str) {
        type_str = "Unknown";
    }

    if (!cJSON_AddStringToObject(obj, "type", type_str)) {
        cJSON_Delete(obj);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to set parameter type");
    }

    if (param->name && param->name[0] != '\0') {
        if (!cJSON_AddStringToObject(obj, "name", param->name)) {
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to set parameter name");
        }
    }

    neoc_error_t err = NEOC_SUCCESS;

    switch (param->type) {
        case NEOC_CONTRACT_PARAM_BOOLEAN:
            if (!cJSON_AddBoolToObject(obj, "value", param->value.boolean_value)) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode boolean parameter");
            }
            break;

        case NEOC_CONTRACT_PARAM_INTEGER: {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%lld", (long long)param->value.integer_value);
            if (!cJSON_AddStringToObject(obj, "value", buffer)) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode integer parameter");
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_STRING: {
            const char *value = param->value.string_value ? param->value.string_value : "";
            if (!cJSON_AddStringToObject(obj, "value", value)) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode string parameter");
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_BYTE_ARRAY: {
            size_t len = param->value.byte_array.len;
            const uint8_t *data = param->value.byte_array.data;
            if (len == 0 || !data) {
                if (!cJSON_AddStringToObject(obj, "value", "")) {
                    err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode empty bytearray parameter");
                }
            } else {
                size_t encoded_len = neoc_base64_encode_buffer_size(len);
                char *encoded = (char*)neoc_malloc(encoded_len);
                if (!encoded) {
                    err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate Base64 buffer");
                } else {
                    neoc_error_t enc_err = neoc_base64_encode(data, len, encoded, encoded_len);
                    if (enc_err != NEOC_SUCCESS) {
                        err = enc_err;
                        neoc_free(encoded);
                    } else {
                        if (!cJSON_AddStringToObject(obj, "value", encoded)) {
                            err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode bytearray parameter");
                        }
                        neoc_free(encoded);
                    }
                }
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_HASH160: {
            char hash_hex[NEOC_HASH160_STRING_LENGTH];
            err = neoc_hash160_to_hex(&param->value.hash160, hash_hex, sizeof(hash_hex), false);
            if (err == NEOC_SUCCESS) {
                char prefixed[NEOC_HASH160_STRING_LENGTH];
                snprintf(prefixed, sizeof(prefixed), "0x%s", hash_hex);
                if (!cJSON_AddStringToObject(obj, "value", prefixed)) {
                    err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode hash160 parameter");
                }
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_HASH256: {
            char hash_hex[67];
            err = neoc_hash256_to_hex(&param->value.hash256, hash_hex, sizeof(hash_hex), false);
            if (err == NEOC_SUCCESS) {
                char prefixed[69];
                snprintf(prefixed, sizeof(prefixed), "0x%s", hash_hex);
                if (!cJSON_AddStringToObject(obj, "value", prefixed)) {
                    err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode hash256 parameter");
                }
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_PUBLIC_KEY: {
            size_t encoded_len = neoc_base64_encode_buffer_size(33);
            char *encoded = (char*)neoc_malloc(encoded_len);
            if (!encoded) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode public key parameter");
            } else {
                neoc_error_t enc_err = neoc_base64_encode(param->value.public_key.data, 33, encoded, encoded_len);
                if (enc_err != NEOC_SUCCESS) {
                    err = enc_err;
                    neoc_free(encoded);
                } else {
                    if (!cJSON_AddStringToObject(obj, "value", encoded)) {
                        err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode public key parameter");
                    }
                    neoc_free(encoded);
                }
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_SIGNATURE: {
            size_t encoded_len = neoc_base64_encode_buffer_size(64);
            char *encoded = (char*)neoc_malloc(encoded_len);
            if (!encoded) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode signature parameter");
            } else {
                neoc_error_t enc_err = neoc_base64_encode(param->value.signature.data, 64, encoded, encoded_len);
                if (enc_err != NEOC_SUCCESS) {
                    err = enc_err;
                    neoc_free(encoded);
                } else {
                    if (!cJSON_AddStringToObject(obj, "value", encoded)) {
                        err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode signature parameter");
                    }
                    neoc_free(encoded);
                }
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_ARRAY: {
            cJSON *array = cJSON_CreateArray();
            if (!array) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate parameter array");
                break;
            }
            for (size_t i = 0; i < param->value.array.count && err == NEOC_SUCCESS; i++) {
                cJSON *item = NULL;
                err = neoc_contract_param_to_cjson(param->value.array.items[i], &item);
                if (err == NEOC_SUCCESS) {
                    cJSON_AddItemToArray(array, item);
                } else if (item) {
                    cJSON_Delete(item);
                }
            }
            if (err == NEOC_SUCCESS) {
                cJSON_AddItemToObject(obj, "value", array);
            } else {
                cJSON_Delete(array);
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_MAP: {
            cJSON *entries = cJSON_CreateArray();
            if (!entries) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate parameter map");
                break;
            }
            for (size_t i = 0; i < param->value.map.count && err == NEOC_SUCCESS; i++) {
                cJSON *entry = cJSON_CreateObject();
                if (!entry) {
                    err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate map entry");
                    break;
                }
                cJSON *key_json = NULL;
                cJSON *value_json = NULL;
                err = neoc_contract_param_to_cjson(param->value.map.keys[i], &key_json);
                if (err == NEOC_SUCCESS) {
                    err = neoc_contract_param_to_cjson(param->value.map.values[i], &value_json);
                }
                if (err == NEOC_SUCCESS) {
                    cJSON_AddItemToObject(entry, "key", key_json);
                    cJSON_AddItemToObject(entry, "value", value_json);
                    cJSON_AddItemToArray(entries, entry);
                } else {
                    if (key_json) cJSON_Delete(key_json);
                    if (value_json) cJSON_Delete(value_json);
                    cJSON_Delete(entry);
                }
            }
            if (err == NEOC_SUCCESS) {
                cJSON_AddItemToObject(obj, "value", entries);
            } else {
                cJSON_Delete(entries);
            }
            break;
        }

        case NEOC_CONTRACT_PARAM_VOID:
        case NEOC_CONTRACT_PARAM_ANY:
        case NEOC_CONTRACT_PARAM_INTEROP_INTERFACE:
        default:
            if (!cJSON_AddNullToObject(obj, "value")) {
                err = neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode null parameter");
            }
            break;
    }

    if (err != NEOC_SUCCESS) {
        cJSON_Delete(obj);
        return err;
    }

    *out = obj;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_contract_params_to_cjson_array(const neoc_contract_parameter_t *params,
                                                       size_t count,
                                                       cJSON **out) {
    if (!out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid output pointer for parameters");
    }

    cJSON *array = cJSON_CreateArray();
    if (!array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate parameter array JSON");
    }

    neoc_error_t err = NEOC_SUCCESS;
    if (count > 0 && !params) {
        cJSON_Delete(array);
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Parameter array is NULL");
    }

    for (size_t i = 0; i < count && err == NEOC_SUCCESS; i++) {
        cJSON *item = NULL;
        err = neoc_contract_param_to_cjson(&params[i], &item);
        if (err == NEOC_SUCCESS) {
            cJSON_AddItemToArray(array, item);
        } else if (item) {
            cJSON_Delete(item);
        }
    }

    if (err != NEOC_SUCCESS) {
        cJSON_Delete(array);
        return err;
    }

    *out = array;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_signer_to_cjson(const neoc_signer_t *signer, cJSON **out) {
    if (!signer || !out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid signer reference");
    }

    if (signer->rules && signer->rules_count > 0) {
        return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "Witness rules not yet supported in JSON conversion");
    }

    cJSON *obj = cJSON_CreateObject();
    if (!obj) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate signer JSON");
    }

    char hash_hex[NEOC_HASH160_STRING_LENGTH];
    neoc_error_t err = neoc_hash160_to_hex(&signer->account, hash_hex, sizeof(hash_hex), false);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(obj);
        return err;
    }

    char account_prefixed[NEOC_HASH160_STRING_LENGTH];
    snprintf(account_prefixed, sizeof(account_prefixed), "0x%s", hash_hex);
    if (!cJSON_AddStringToObject(obj, "account", account_prefixed)) {
        cJSON_Delete(obj);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode signer account");
    }

    neoc_witness_scope_t extracted[8];
    size_t scope_count = 0;
    err = neoc_witness_scope_extract(signer->scopes, extracted, sizeof(extracted)/sizeof(extracted[0]), &scope_count);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(obj);
        return err;
    }

    if (scope_count == 0) {
        if (!cJSON_AddStringToObject(obj, "scopes", neoc_witness_scope_to_json(NEOC_WITNESS_SCOPE_NONE))) {
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode signer scopes");
        }
    } else if (scope_count == 1) {
        const char *scope_str = neoc_witness_scope_to_json(extracted[0]);
        if (!cJSON_AddStringToObject(obj, "scopes", scope_str ? scope_str : "Unknown")) {
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode signer scope");
        }
    } else {
        size_t total_len = 1; // null terminator
        for (size_t i = 0; i < scope_count; i++) {
            const char *scope_str = neoc_witness_scope_to_json(extracted[i]);
            total_len += strlen(scope_str ? scope_str : "Unknown") + 1; // include delimiter
        }
        char *combined = (char*)neoc_malloc(total_len);
        if (!combined) {
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to combine signer scopes");
        }
        combined[0] = '\0';
        for (size_t i = 0; i < scope_count; i++) {
            if (i > 0) {
                strcat(combined, "|");
            }
            const char *scope_str = neoc_witness_scope_to_json(extracted[i]);
            strcat(combined, scope_str ? scope_str : "Unknown");
        }
        if (!cJSON_AddStringToObject(obj, "scopes", combined)) {
            neoc_free(combined);
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode combined signer scopes");
        }
        neoc_free(combined);
    }

    if (signer->allowed_contracts && signer->allowed_contracts_count > 0) {
        cJSON *contracts = cJSON_CreateArray();
        if (!contracts) {
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate allowed contracts array");
        }
        for (size_t i = 0; i < signer->allowed_contracts_count; i++) {
            char contract_hex[NEOC_HASH160_STRING_LENGTH];
            err = neoc_hash160_to_hex(&signer->allowed_contracts[i], contract_hex, sizeof(contract_hex), false);
            if (err != NEOC_SUCCESS) {
                cJSON_Delete(contracts);
                cJSON_Delete(obj);
                return err;
            }
            char prefixed[NEOC_HASH160_STRING_LENGTH];
            snprintf(prefixed, sizeof(prefixed), "0x%s", contract_hex);
            cJSON *contract_node = cJSON_CreateString(prefixed);
            if (!contract_node) {
                cJSON_Delete(contracts);
                cJSON_Delete(obj);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode allowed contract");
            }
            cJSON_AddItemToArray(contracts, contract_node);
        }
        cJSON_AddItemToObject(obj, "allowedcontracts", contracts);
    }

    if (signer->allowed_groups && signer->allowed_groups_count > 0 && signer->allowed_groups_sizes) {
        cJSON *groups = cJSON_CreateArray();
        if (!groups) {
            cJSON_Delete(obj);
            return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate allowed groups array");
        }
        for (size_t i = 0; i < signer->allowed_groups_count; i++) {
            size_t size = signer->allowed_groups_sizes[i];
            const uint8_t *data = signer->allowed_groups[i];
            if (!data || size == 0) {
                continue;
            }
            size_t encoded_len = neoc_base64_encode_buffer_size(size);
            char *encoded = (char*)neoc_malloc(encoded_len);
            if (!encoded) {
                cJSON_Delete(groups);
                cJSON_Delete(obj);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode allowed group");
            }
            neoc_error_t enc_err = neoc_base64_encode(data, size, encoded, encoded_len);
            if (enc_err != NEOC_SUCCESS) {
                neoc_free(encoded);
                cJSON_Delete(groups);
                cJSON_Delete(obj);
                return enc_err;
            }
            cJSON *encoded_item = cJSON_CreateString(encoded);
            neoc_free(encoded);
            if (!encoded_item) {
                cJSON_Delete(groups);
                cJSON_Delete(obj);
                return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode allowed group string");
            }
            cJSON_AddItemToArray(groups, encoded_item);
        }
        cJSON_AddItemToObject(obj, "allowedgroups", groups);
    }

    *out = obj;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_signers_to_cjson_array(const neoc_signer_t *signers,
                                                size_t count,
                                                cJSON **out) {
    if (!out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid output pointer for signers");
    }

    cJSON *array = cJSON_CreateArray();
    if (!array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate signers array JSON");
    }

    neoc_error_t err = NEOC_SUCCESS;
    if (count > 0 && !signers) {
        cJSON_Delete(array);
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Signer array is NULL");
    }

    for (size_t i = 0; i < count && err == NEOC_SUCCESS; i++) {
        cJSON *item = NULL;
        err = neoc_signer_to_cjson(&signers[i], &item);
        if (err == NEOC_SUCCESS) {
            cJSON_AddItemToArray(array, item);
        } else if (item) {
            cJSON_Delete(item);
        }
    }

    if (err != NEOC_SUCCESS) {
        cJSON_Delete(array);
        return err;
    }

    *out = array;
    return NEOC_SUCCESS;
}

#endif // HAVE_CJSON

neoc_invocation_result_t *neoc_invocation_result_from_json(const char *json_str);

/**
 * @brief Create a new Neo protocol client
 * @param rpc_url The RPC endpoint URL
 * @param network_magic The network magic number
 * @param client Pointer to store the created client
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_client_create(const char *rpc_url, uint32_t network_magic, neoc_neo_client_t **client) {
    if (!rpc_url || !client) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_client_create");
    }
    
    neoc_neo_client_t *result = neoc_malloc(sizeof(neoc_neo_client_t));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate Neo client");
    }
    
    size_t url_len = strlen(rpc_url);
    result->rpc_url = neoc_malloc(url_len + 1);
    if (!result->rpc_url) {
        neoc_free(result);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate RPC URL");
    }
    
    strcpy(result->rpc_url, rpc_url);
    result->network_magic = network_magic;
    result->network_magic_set = (network_magic != 0);
    result->rpc_client = NULL;
    result->context = NULL;

    neoc_error_t err = neoc_rpc_client_create(rpc_url, &result->rpc_client);
    if (err != NEOC_SUCCESS) {
        neoc_free(result->rpc_url);
        neoc_free(result);
        return err;
    }

    *client = result;
    return NEOC_SUCCESS;
}

/**
 * @brief Free Neo protocol client
 * @param client The client to free
 */
void neoc_neo_client_free(neoc_neo_client_t *client) {
    if (!client) return;
    
    if (client->rpc_url) {
        neoc_free(client->rpc_url);
    }
    
    if (client->rpc_client) {
        neoc_rpc_client_free(client->rpc_client);
    }
    
    neoc_free(client);
}

// MARK: Blockchain Methods

/**
 * @brief Get the hash of the latest block
 * @param client The Neo client
 * @param block_hash Pointer to store the block hash
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_best_block_hash(neoc_neo_client_t *client, neoc_hash256_t *block_hash) {
    if (!client || !block_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_best_block_hash");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_best_block_hash(client->rpc_client, block_hash);
}

/**
 * @brief Get block hash by index
 * @param client The Neo client
 * @param block_index The block index
 * @param block_hash Pointer to store the block hash
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_hash(neoc_neo_client_t *client, uint32_t block_index, neoc_hash256_t *block_hash) {
    if (!client || !block_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_block_hash");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_block_hash(client->rpc_client, block_index, block_hash);
}

/**
 * @brief Get block by hash
 * @param client The Neo client
 * @param block_hash The block hash
 * @param full_transactions Whether to return full transaction objects
 * @param block Pointer to store the block
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_by_hash(neoc_neo_client_t *client, const neoc_hash256_t *block_hash, bool full_transactions, neoc_block_t **block) {
    if (!client || !block_hash || !block) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_block_by_hash");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_block(client->rpc_client, block_hash, full_transactions, block);
}

/**
 * @brief Get block by index
 * @param client The Neo client
 * @param block_index The block index
 * @param full_transactions Whether to return full transaction objects
 * @param block Pointer to store the block
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_by_index(neoc_neo_client_t *client, uint32_t block_index, bool full_transactions, neoc_block_t **block) {
    if (!client || !block) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_block_by_index");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_hash256_t block_hash;
    err = neoc_rpc_get_block_hash(client->rpc_client, block_index, &block_hash);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_block(client->rpc_client, &block_hash, full_transactions, block);
}

/**
 * @brief Get block count
 * @param client The Neo client
 * @param count Pointer to store the block count
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_block_count(neoc_neo_client_t *client, uint32_t *count) {
    if (!client || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_block_count");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_block_count(client->rpc_client, count);
}

/**
 * @brief Get the network magic number
 * @param client The Neo client
 * @param magic_out Pointer to store the network magic
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_network_magic_number(neoc_neo_client_t *client, int *magic_out) {
    if (!client || !magic_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_network_magic_number");
    }

    if (client->network_magic_set) {
        *magic_out = (int)client->network_magic;
        return NEOC_SUCCESS;
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    char *version_json = NULL;
    err = neoc_rpc_get_version(client->rpc_client, &version_json);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    int magic = 0;

#ifdef HAVE_CJSON
    cJSON *root = cJSON_Parse(version_json);
    if (!root) {
        free(version_json);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse getversion response");
    }

    cJSON *protocol = cJSON_GetObjectItem(root, "protocol");
    cJSON *network_field = NULL;
    if (protocol && cJSON_IsObject(protocol)) {
        network_field = cJSON_GetObjectItem(protocol, "network");
    }
    if (!network_field) {
        network_field = cJSON_GetObjectItem(root, "network");
    }

    if (network_field) {
        if (cJSON_IsNumber(network_field)) {
            magic = (int)network_field->valuedouble;
        } else if (cJSON_IsString(network_field) && network_field->valuestring) {
            magic = (int)strtol(network_field->valuestring, NULL, 10);
        }
    }

    cJSON_Delete(root);
#else
    err = neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support required to parse getversion response");
#endif

    free(version_json);

#ifndef HAVE_CJSON
    if (err != NEOC_SUCCESS) {
        return err;
    }
#endif

    if (magic <= 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Network magic number missing in version response");
    }

    client->network_magic = (uint32_t)magic;
    client->network_magic_set = true;
    *magic_out = magic;
    return NEOC_SUCCESS;
}

/**
 * @brief Get the network magic number as bytes
 * @param client The Neo client
 * @param bytes_out Output array of 4 bytes (big-endian)
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_network_magic_number_bytes(neoc_neo_client_t *client, uint8_t bytes_out[4]) {
    if (!client || !bytes_out) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_network_magic_number_bytes");
    }

    int magic_int = 0;
    neoc_error_t err = neoc_neo_get_network_magic_number(client, &magic_int);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint32_t magic_uint = (uint32_t)magic_int;
    bytes_out[0] = (uint8_t)((magic_uint >> 24) & 0xFF);
    bytes_out[1] = (uint8_t)((magic_uint >> 16) & 0xFF);
    bytes_out[2] = (uint8_t)((magic_uint >> 8) & 0xFF);
    bytes_out[3] = (uint8_t)(magic_uint & 0xFF);

    return NEOC_SUCCESS;
}

/**
 * @brief Get contract state
 * @param client The Neo client
 * @param contract_hash The contract hash
 * @param state Pointer to store the contract state
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_contract_state(neoc_neo_client_t *client, const neoc_hash160_t *contract_hash, neoc_contract_state_t **state) {
    if (!client || !contract_hash || !state) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_contract_state");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_contract_state(client->rpc_client, contract_hash, state);
}

/**
 * @brief Get transaction by hash
 * @param client The Neo client
 * @param tx_hash The transaction hash
 * @param transaction Pointer to store the transaction
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_transaction(neoc_neo_client_t *client, const neoc_hash256_t *tx_hash, neoc_transaction_t **transaction) {
    if (!client || !tx_hash || !transaction) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_transaction");
    }

    (void)client;
    (void)tx_hash;
    (void)transaction;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "Transaction retrieval bridge not implemented yet");
}

// MARK: Node Methods

/**
 * @brief Get connection count
 * @param client The Neo client
 * @param count Pointer to store the connection count
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_get_connection_count(neoc_neo_client_t *client, uint32_t *count) {
    if (!client || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_get_connection_count");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    return neoc_rpc_get_connection_count(client->rpc_client, count);
}

/**
 * @brief Send raw transaction
 * @param client The Neo client
 * @param raw_transaction_hex The raw transaction in hex format
 * @param tx_hash Pointer to store the transaction hash
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_send_raw_transaction(neoc_neo_client_t *client, const char *raw_transaction_hex, neoc_hash256_t *tx_hash) {
    if (!client || !raw_transaction_hex || !tx_hash) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_send_raw_transaction");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    size_t tx_size = 0;
    uint8_t *tx_bytes = neoc_hex_decode_alloc(raw_transaction_hex, &tx_size);
    if (!tx_bytes || tx_size == 0) {
        if (tx_bytes) {
            neoc_free(tx_bytes);
        }
        return neoc_error_set(NEOC_ERROR_INVALID_HEX, "Failed to decode raw transaction hex");
    }

    err = neoc_rpc_send_raw_transaction(client->rpc_client, tx_bytes, tx_size, tx_hash);
    neoc_free(tx_bytes);
    return err;
}

// MARK: Smart Contract Methods

/**
 * @brief Invoke contract function
 * @param client The Neo client
 * @param contract_hash The contract hash
 * @param function_name The function name to invoke
 * @param params Array of contract parameters (can be NULL)
 * @param param_count Number of parameters
 * @param signers Array of signers (can be NULL)
 * @param signer_count Number of signers
 * @param result Pointer to store the invocation result
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_invoke_function(neoc_neo_client_t *client, 
                                     const neoc_hash160_t *contract_hash,
                                     const char *function_name,
                                     const neoc_contract_parameter_t *params,
                                     size_t param_count,
                                     const neoc_signer_t *signers,
                                     size_t signer_count,
                                     neoc_invocation_result_t **result) {
    if (!client || !contract_hash || !function_name || !result) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_invoke_function");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

#ifndef HAVE_CJSON
    (void)params;
    (void)param_count;
    (void)signers;
    (void)signer_count;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support required for invokefunction");
#else
    cJSON *request_array = cJSON_CreateArray();
    if (!request_array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate invokefunction parameters");
    }

    char contract_hex[NEOC_HASH160_STRING_LENGTH];
    err = neoc_hash160_to_hex(contract_hash, contract_hex, sizeof(contract_hex), false);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(request_array);
        return err;
    }

    char contract_prefixed[NEOC_HASH160_STRING_LENGTH];
    snprintf(contract_prefixed, sizeof(contract_prefixed), "0x%s", contract_hex);

    cJSON *contract_node = cJSON_CreateString(contract_prefixed);
    if (!contract_node) {
        cJSON_Delete(request_array);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode contract hash");
    }
    cJSON_AddItemToArray(request_array, contract_node);

    cJSON *method_node = cJSON_CreateString(function_name);
    if (!method_node) {
        cJSON_Delete(request_array);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode function name");
    }
    cJSON_AddItemToArray(request_array, method_node);

    cJSON *params_json = NULL;
    err = neoc_contract_params_to_cjson_array(params, param_count, &params_json);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(request_array);
        return err;
    }
    cJSON_AddItemToArray(request_array, params_json);

    cJSON *signers_json = NULL;
    err = neoc_signers_to_cjson_array(signers, signer_count, &signers_json);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(request_array);
        return err;
    }
    cJSON_AddItemToArray(request_array, signers_json);

    char *rpc_params = cJSON_PrintUnformatted(request_array);
    cJSON_Delete(request_array);
    if (!rpc_params) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to serialize invokefunction parameters");
    }

    char *raw_response = NULL;
    err = neoc_rpc_call_raw(client->rpc_client, RPC_INVOKE_FUNCTION, rpc_params, &raw_response);
    free(rpc_params);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_invocation_result_t *parsed = neoc_invocation_result_from_json(raw_response);
    free(raw_response);
    if (!parsed) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse invokefunction result");
    }

    *result = parsed;
    return NEOC_SUCCESS;
#endif
}

/**
 * @brief Invoke script
 * @param client The Neo client
 * @param script_hex The script in hex format
 * @param signers Array of signers (can be NULL)
 * @param signer_count Number of signers
 * @param result Pointer to store the invocation result
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_invoke_script(neoc_neo_client_t *client,
                                   const char *script_hex,
                                   const neoc_signer_t *signers,
                                   size_t signer_count,
                                   neoc_invocation_result_t **result) {
    if (!client || !script_hex || !result) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_invoke_script");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

#ifndef HAVE_CJSON
    (void)signers;
    (void)signer_count;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support required for invokescript");
#else
    cJSON *request_array = cJSON_CreateArray();
    if (!request_array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate invokescript parameters");
    }

    const char *script_value = script_hex;
    if (script_hex[0] == '0' && (script_hex[1] == 'x' || script_hex[1] == 'X')) {
        script_value = script_hex + 2;
    }

    cJSON *script_node = cJSON_CreateString(script_value);
    if (!script_node) {
        cJSON_Delete(request_array);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode script");
    }
    cJSON_AddItemToArray(request_array, script_node);

    cJSON *signers_json = NULL;
    err = neoc_signers_to_cjson_array(signers, signer_count, &signers_json);
    if (err != NEOC_SUCCESS) {
        cJSON_Delete(request_array);
        return err;
    }
    cJSON_AddItemToArray(request_array, signers_json);

    char *rpc_params = cJSON_PrintUnformatted(request_array);
    cJSON_Delete(request_array);
    if (!rpc_params) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to serialize invokescript parameters");
    }

    char *raw_response = NULL;
    err = neoc_rpc_call_raw(client->rpc_client, RPC_INVOKE_SCRIPT, rpc_params, &raw_response);
    free(rpc_params);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_invocation_result_t *parsed = neoc_invocation_result_from_json(raw_response);
    free(raw_response);
    if (!parsed) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse invokescript result");
    }

    *result = parsed;
    return NEOC_SUCCESS;
#endif
}

// MARK: Utilities Methods

/**
 * @brief Validate Neo address
 * @param client The Neo client
 * @param address The address to validate
 * @param is_valid Pointer to store the validation result
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_validate_address(neoc_neo_client_t *client, const char *address, bool *is_valid) {
    if (!client || !address || !is_valid) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_validate_address");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

#ifndef HAVE_CJSON
    (void)address;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support required for validateaddress");
#else
    cJSON *params_array = cJSON_CreateArray();
    if (!params_array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate validateaddress parameters");
    }
    cJSON *address_node = cJSON_CreateString(address);
    if (!address_node) {
        cJSON_Delete(params_array);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode address");
    }
    cJSON_AddItemToArray(params_array, address_node);

    char *rpc_params = cJSON_PrintUnformatted(params_array);
    cJSON_Delete(params_array);
    if (!rpc_params) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to serialize validateaddress parameters");
    }

    char *raw_response = NULL;
    err = neoc_rpc_call_raw(client->rpc_client, "validateaddress", rpc_params, &raw_response);
    free(rpc_params);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_neo_validate_address_response_t *response = NULL;
    err = neoc_neo_validate_address_response_from_json(raw_response, &response);
    free(raw_response);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    *is_valid = neoc_neo_validate_address_response_is_valid_address(response);
    neoc_neo_validate_address_response_free(response);
    return NEOC_SUCCESS;
#endif
}

/**
 * @brief Calculate network fee for transaction
 * @param client The Neo client
 * @param transaction_hex The transaction in hex format
 * @param network_fee Pointer to store the calculated network fee
 * @return Error code indicating success or failure
 */
neoc_error_t neoc_neo_calculate_network_fee(neoc_neo_client_t *client, const char *transaction_hex, uint64_t *network_fee) {
    if (!client || !transaction_hex || !network_fee) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments to neoc_neo_calculate_network_fee");
    }

    neoc_error_t err = neoc_ensure_rpc_client(client);
    if (err != NEOC_SUCCESS) {
        return err;
    }

#ifndef HAVE_CJSON
    (void)transaction_hex;
    (void)network_fee;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "cJSON support required for calculatenetworkfee");
#else
    cJSON *params_array = cJSON_CreateArray();
    if (!params_array) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to allocate calculatenetworkfee parameters");
    }
    cJSON *tx_node = cJSON_CreateString(transaction_hex);
    if (!tx_node) {
        cJSON_Delete(params_array);
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to encode transaction hex");
    }
    cJSON_AddItemToArray(params_array, tx_node);

    char *rpc_params = cJSON_PrintUnformatted(params_array);
    cJSON_Delete(params_array);
    if (!rpc_params) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_MEMORY, "Failed to serialize calculatenetworkfee parameters");
    }

    char *raw_response = NULL;
    err = neoc_rpc_call_raw(client->rpc_client, "calculatenetworkfee", rpc_params, &raw_response);
    free(rpc_params);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    neoc_neo_calculate_network_fee_t *parsed = NULL;
    err = neoc_neo_calculate_network_fee_from_json(raw_response, &parsed);
    free(raw_response);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    int64_t fee_value = 0;
    err = neoc_neo_calculate_network_fee_get_fee_value(parsed, &fee_value);
    neoc_neo_calculate_network_fee_free(parsed);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (fee_value < 0) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Network fee value is negative");
    }

    *network_fee = (uint64_t)fee_value;
    return NEOC_SUCCESS;
#endif
}
