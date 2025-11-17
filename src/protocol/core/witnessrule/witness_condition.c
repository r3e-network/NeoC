/**
 * @file witness_condition.c
 * @brief WitnessCondition implementation mirroring NeoSwift behaviour.
 */

#include "neoc/witnessrule/witness_condition.h"

#include "neoc/neo_constants.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/types/neoc_hash160.h"
#include "neoc/utils/json.h"
#include "neoc/utils/neoc_hex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *NEOC_WITNESS_CONDITION_BOOLEAN_STR = "Boolean";
static const char *NEOC_WITNESS_CONDITION_NOT_STR = "Not";
static const char *NEOC_WITNESS_CONDITION_AND_STR = "And";
static const char *NEOC_WITNESS_CONDITION_OR_STR = "Or";
static const char *NEOC_WITNESS_CONDITION_SCRIPT_HASH_STR = "ScriptHash";
static const char *NEOC_WITNESS_CONDITION_GROUP_STR = "Group";
static const char *NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY_STR = "CalledByEntry";
static const char *NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT_STR = "CalledByContract";
static const char *NEOC_WITNESS_CONDITION_CALLED_BY_GROUP_STR = "CalledByGroup";

static uint8_t neoc_witness_condition_type_to_byte(neoc_witness_condition_type_t type);
static size_t neoc_witness_condition_internal_size(const neoc_witness_condition_t *condition);
static void neoc_witness_condition_release(neoc_witness_condition_t *condition);
static neoc_error_t neoc_witness_condition_clone_public_key(
    const neoc_ec_public_key_t *source,
    neoc_ec_public_key_t **dest);
static neoc_error_t neoc_witness_condition_clone_internal(
    const neoc_witness_condition_t *src,
    neoc_witness_condition_t **dest);
static size_t neoc_var_int_size(uint64_t value);
static neoc_error_t neoc_witness_condition_serialize_internal(
    const neoc_witness_condition_t *condition,
    neoc_binary_writer_t *writer,
    size_t depth);
static neoc_error_t neoc_witness_condition_deserialize_internal(
    neoc_binary_reader_t *reader,
    size_t depth,
    neoc_witness_condition_t **condition);

static neoc_witness_condition_t *neoc_witness_condition_alloc(neoc_witness_condition_type_t type) {
    neoc_witness_condition_t *condition = calloc(1, sizeof(neoc_witness_condition_t));
    if (!condition) {
        return NULL;
    }
    condition->type = type;
    return condition;
}

static neoc_error_t neoc_witness_condition_clone_public_key(
    const neoc_ec_public_key_t *source,
    neoc_ec_public_key_t **dest) {
    if (!source || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid public key arguments");
    }

    uint8_t *encoded = NULL;
    size_t encoded_len = 0;
    neoc_error_t err = neoc_ec_public_key_get_encoded(source, true, &encoded, &encoded_len);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_ec_public_key_from_bytes(encoded, encoded_len, dest);
    free(encoded);
    return err;
}

neoc_error_t neoc_witness_condition_create_boolean(
    bool value,
    neoc_witness_condition_t **condition) {
    if (!condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "condition pointer is NULL");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_BOOLEAN);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }
    result->data.boolean.value = value;
    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_create_not(
    neoc_witness_condition_t *expression,
    neoc_witness_condition_t **condition) {
    if (!expression || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_NOT);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    neoc_error_t err = neoc_witness_condition_clone_internal(expression, &result->data.not_expr.expression);
    if (err != NEOC_SUCCESS) {
        neoc_witness_condition_release(result);
        return err;
    }

    *condition = result;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_witness_condition_create_logical(
    neoc_witness_condition_type_t type,
    neoc_witness_condition_t **expressions,
    size_t count,
    neoc_witness_condition_t **condition) {
    if (!expressions || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    if (count == 0 || count > NEOC_WITNESS_CONDITION_MAX_SUBITEMS) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid expression count");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(type);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    result->data.logical.expressions = calloc(count, sizeof(neoc_witness_condition_t *));
    if (!result->data.logical.expressions) {
        neoc_witness_condition_release(result);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition list");
    }
    result->data.logical.count = count;

    for (size_t i = 0; i < count; ++i) {
        neoc_error_t err = neoc_witness_condition_clone_internal(expressions[i], &result->data.logical.expressions[i]);
        if (err != NEOC_SUCCESS) {
            neoc_witness_condition_release(result);
            return err;
        }
    }

    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_create_and(
    neoc_witness_condition_t **expressions,
    size_t count,
    neoc_witness_condition_t **condition) {
    return neoc_witness_condition_create_logical(
        NEOC_WITNESS_CONDITION_AND, expressions, count, condition);
}

neoc_error_t neoc_witness_condition_create_or(
    neoc_witness_condition_t **expressions,
    size_t count,
    neoc_witness_condition_t **condition) {
    return neoc_witness_condition_create_logical(
        NEOC_WITNESS_CONDITION_OR, expressions, count, condition);
}

neoc_error_t neoc_witness_condition_create_script_hash(
    const neoc_hash160_t *hash,
    neoc_witness_condition_t **condition) {
    if (!hash || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_SCRIPT_HASH);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    memcpy(&result->data.hash_condition.hash, hash, sizeof(neoc_hash160_t));
    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_create_group(
    const neoc_ec_public_key_t *public_key,
    neoc_witness_condition_t **condition) {
    if (!public_key || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_GROUP);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    neoc_error_t err = neoc_witness_condition_clone_public_key(public_key,
                                                               &result->data.group_condition.public_key);
    if (err != NEOC_SUCCESS) {
        neoc_witness_condition_release(result);
        return err;
    }

    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_create_called_by_entry(
    neoc_witness_condition_t **condition) {
    if (!condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "condition pointer is NULL");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }
    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_create_called_by_contract(
    const neoc_hash160_t *contract_hash,
    neoc_witness_condition_t **condition) {
    if (!contract_hash || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_witness_condition_t *result =
        neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    memcpy(&result->data.hash_condition.hash, contract_hash, sizeof(neoc_hash160_t));
    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_create_called_by_group(
    const neoc_ec_public_key_t *group_key,
    neoc_witness_condition_t **condition) {
    if (!group_key || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_witness_condition_t *result =
        neoc_witness_condition_alloc(NEOC_WITNESS_CONDITION_CALLED_BY_GROUP);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    neoc_error_t err = neoc_witness_condition_clone_public_key(group_key,
                                                               &result->data.group_condition.public_key);
    if (err != NEOC_SUCCESS) {
        neoc_witness_condition_release(result);
        return err;
    }

    *condition = result;
    return NEOC_SUCCESS;
}

void neoc_witness_condition_free(neoc_witness_condition_t *condition) {
    if (!condition) return;
    neoc_witness_condition_release(condition);
}

static void neoc_witness_condition_release(neoc_witness_condition_t *condition) {
    if (!condition) return;

    switch (condition->type) {
        case NEOC_WITNESS_CONDITION_NOT:
            neoc_witness_condition_free(condition->data.not_expr.expression);
            break;
        case NEOC_WITNESS_CONDITION_AND:
        case NEOC_WITNESS_CONDITION_OR:
            if (condition->data.logical.expressions) {
                for (size_t i = 0; i < condition->data.logical.count; ++i) {
                    neoc_witness_condition_free(condition->data.logical.expressions[i]);
                }
                free(condition->data.logical.expressions);
            }
            break;
        case NEOC_WITNESS_CONDITION_GROUP:
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP:
            if (condition->data.group_condition.public_key) {
                neoc_ec_public_key_free(condition->data.group_condition.public_key);
            }
            break;
        default:
            break;
    }

    free(condition);
}

neoc_witness_condition_type_t neoc_witness_condition_get_type(
    const neoc_witness_condition_t *condition) {
    return condition ? condition->type : NEOC_WITNESS_CONDITION_BOOLEAN;
}

neoc_error_t neoc_witness_condition_get_boolean(
    const neoc_witness_condition_t *condition,
    bool *value) {
    if (!condition || !value) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    if (condition->type != NEOC_WITNESS_CONDITION_BOOLEAN) {
        return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "Condition is not boolean");
    }
    *value = condition->data.boolean.value;
    return NEOC_SUCCESS;
}

uint8_t neoc_witness_condition_type_get_byte(
    neoc_witness_condition_type_t type) {
    return neoc_witness_condition_type_to_byte(type);
}

static uint8_t neoc_witness_condition_type_to_byte(neoc_witness_condition_type_t type) {
    switch (type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN: return 0x00;
        case NEOC_WITNESS_CONDITION_NOT: return 0x01;
        case NEOC_WITNESS_CONDITION_AND: return 0x02;
        case NEOC_WITNESS_CONDITION_OR: return 0x03;
        case NEOC_WITNESS_CONDITION_SCRIPT_HASH: return 0x18;
        case NEOC_WITNESS_CONDITION_GROUP: return 0x19;
        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY: return 0x20;
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT: return 0x28;
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP: return 0x29;
        default: return 0x00;
    }
}

const char* neoc_witness_condition_type_to_json_string(
    neoc_witness_condition_type_t type) {
    switch (type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN: return NEOC_WITNESS_CONDITION_BOOLEAN_STR;
        case NEOC_WITNESS_CONDITION_NOT: return NEOC_WITNESS_CONDITION_NOT_STR;
        case NEOC_WITNESS_CONDITION_AND: return NEOC_WITNESS_CONDITION_AND_STR;
        case NEOC_WITNESS_CONDITION_OR: return NEOC_WITNESS_CONDITION_OR_STR;
        case NEOC_WITNESS_CONDITION_SCRIPT_HASH: return NEOC_WITNESS_CONDITION_SCRIPT_HASH_STR;
        case NEOC_WITNESS_CONDITION_GROUP: return NEOC_WITNESS_CONDITION_GROUP_STR;
        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY: return NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY_STR;
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT: return NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT_STR;
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP: return NEOC_WITNESS_CONDITION_CALLED_BY_GROUP_STR;
        default: return "Unknown";
    }
}

#ifdef HAVE_CJSON

static neoc_error_t neoc_witness_condition_from_json_object(
    neoc_json_t *object,
    size_t depth,
    neoc_witness_condition_t **condition);
static neoc_error_t neoc_witness_condition_array_from_json(
    neoc_json_t *array,
    size_t depth,
    neoc_witness_condition_t ***items,
    size_t *count);
static neoc_error_t neoc_witness_condition_to_json_object(
    const neoc_witness_condition_t *condition,
    neoc_json_t **object,
    size_t depth);

neoc_error_t neoc_witness_condition_from_json(
    const char *json_str,
    neoc_witness_condition_t **condition) {
    if (!json_str || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid JSON input");
    }

    *condition = NULL;
    neoc_json_t *root = neoc_json_parse(json_str);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse witness condition JSON");
    }

    neoc_error_t err = neoc_witness_condition_from_json_object(root, 0, condition);
    neoc_json_free(root);
    return err;
}

neoc_error_t neoc_witness_condition_to_json(
    const neoc_witness_condition_t *condition,
    char **json_str) {
    if (!condition || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness condition");
    }

    *json_str = NULL;
    neoc_json_t *root = NULL;
    neoc_error_t err = neoc_witness_condition_to_json_object(condition, &root, 0);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    char *serialized = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!serialized) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to serialize witness condition JSON");
    }

    *json_str = serialized;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_witness_condition_from_json_object(
    neoc_json_t *object,
    size_t depth,
    neoc_witness_condition_t **condition) {
    if (!object || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid JSON condition object");
    }
    if (depth > NEOC_WITNESS_CONDITION_MAX_NESTING_DEPTH) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness condition nesting depth exceeded");
    }

    const char *type_str = neoc_json_get_string(object, "type");
    if (!type_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness condition missing type");
    }

    neoc_error_t err = NEOC_SUCCESS;
    if (strcmp(type_str, NEOC_WITNESS_CONDITION_BOOLEAN_STR) == 0) {
        bool value = false;
        err = neoc_json_get_bool(object, "expression", &value);
        if (err == NEOC_SUCCESS) {
            err = neoc_witness_condition_create_boolean(value, condition);
        }
    } else if (strcmp(type_str, NEOC_WITNESS_CONDITION_NOT_STR) == 0) {
        neoc_json_t *expr_json = neoc_json_get_object(object, "expression");
        if (!expr_json) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "NOT condition missing expression");
        } else {
            neoc_witness_condition_t *inner = NULL;
            err = neoc_witness_condition_from_json_object(expr_json, depth + 1, &inner);
            if (err == NEOC_SUCCESS) {
                err = neoc_witness_condition_create_not(inner, condition);
            }
            if (inner) {
                neoc_witness_condition_free(inner);
            }
        }
    } else if (strcmp(type_str, NEOC_WITNESS_CONDITION_AND_STR) == 0 ||
               strcmp(type_str, NEOC_WITNESS_CONDITION_OR_STR) == 0) {
        neoc_json_t *array = neoc_json_get_array(object, "expressions");
        if (!array) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Logical condition missing expressions");
        } else {
            neoc_witness_condition_t **items = NULL;
            size_t count = 0;
            err = neoc_witness_condition_array_from_json(array, depth + 1, &items, &count);
            if (err == NEOC_SUCCESS) {
                if (strcmp(type_str, NEOC_WITNESS_CONDITION_AND_STR) == 0) {
                    err = neoc_witness_condition_create_and(items, count, condition);
                } else {
                    err = neoc_witness_condition_create_or(items, count, condition);
                }
            }
            if (items) {
                for (size_t i = 0; i < count; ++i) {
                    neoc_witness_condition_free(items[i]);
                }
                neoc_free(items);
            }
        }
    } else if (strcmp(type_str, NEOC_WITNESS_CONDITION_SCRIPT_HASH_STR) == 0 ||
               strcmp(type_str, NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT_STR) == 0) {
        const char *hash_str = neoc_json_get_string(object, "hash");
        if (!hash_str) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Hash condition missing hash");
        } else {
            neoc_hash160_t hash;
            err = neoc_hash160_from_string(hash_str, &hash);
            if (err == NEOC_SUCCESS) {
                if (strcmp(type_str, NEOC_WITNESS_CONDITION_SCRIPT_HASH_STR) == 0) {
                    err = neoc_witness_condition_create_script_hash(&hash, condition);
                } else {
                    err = neoc_witness_condition_create_called_by_contract(&hash, condition);
                }
            }
        }
    } else if (strcmp(type_str, NEOC_WITNESS_CONDITION_GROUP_STR) == 0 ||
               strcmp(type_str, NEOC_WITNESS_CONDITION_CALLED_BY_GROUP_STR) == 0) {
        const char *group_str = neoc_json_get_string(object, "group");
        if (!group_str) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Group condition missing group");
        } else {
            size_t key_len = 0;
            uint8_t *key_bytes = neoc_hex_decode_alloc(group_str, &key_len);
            if (!key_bytes || (key_len != 33 && key_len != 65)) {
                if (key_bytes) {
                    neoc_free(key_bytes);
                }
                err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid group key encoding");
            } else {
                neoc_ec_public_key_t *public_key = NULL;
                err = neoc_ec_public_key_from_bytes(key_bytes, key_len, &public_key);
                neoc_free(key_bytes);
                if (err == NEOC_SUCCESS) {
                    if (strcmp(type_str, NEOC_WITNESS_CONDITION_GROUP_STR) == 0) {
                        err = neoc_witness_condition_create_group(public_key, condition);
                    } else {
                        err = neoc_witness_condition_create_called_by_group(public_key, condition);
                    }
                    neoc_ec_public_key_free(public_key);
                }
            }
        }
    } else if (strcmp(type_str, NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY_STR) == 0) {
        err = neoc_witness_condition_create_called_by_entry(condition);
    } else {
        err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unknown witness condition type");
    }

    return err;
}

static neoc_error_t neoc_witness_condition_array_from_json(
    neoc_json_t *array,
    size_t depth,
    neoc_witness_condition_t ***items,
    size_t *count) {
    if (!array || !items || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness condition array");
    }

    size_t array_count = neoc_json_array_size(array);
    if (array_count == 0 || array_count > NEOC_WITNESS_CONDITION_MAX_SUBITEMS) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid witness condition array size");
    }

    neoc_witness_condition_t **result =
        neoc_calloc(array_count, sizeof(neoc_witness_condition_t *));
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition array");
    }

    neoc_error_t err = NEOC_SUCCESS;
    size_t i = 0;
    for (; i < array_count; ++i) {
        neoc_json_t *item = neoc_json_array_get(array, i);
        if (!item) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness condition array item missing");
            break;
        }

        err = neoc_witness_condition_from_json_object(item, depth, &result[i]);
        if (err != NEOC_SUCCESS) {
            break;
        }
    }

    if (err != NEOC_SUCCESS) {
        for (size_t j = 0; j < i; ++j) {
            neoc_witness_condition_free(result[j]);
        }
        neoc_free(result);
        return err;
    }

    *items = result;
    *count = array_count;
    return NEOC_SUCCESS;
}

static neoc_error_t neoc_witness_condition_to_json_object(
    const neoc_witness_condition_t *condition,
    neoc_json_t **object,
    size_t depth) {
    if (!condition || !object) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness condition");
    }
    if (depth > NEOC_WITNESS_CONDITION_MAX_NESTING_DEPTH) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness condition nesting depth exceeded");
    }

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate JSON object");
    }

    neoc_error_t err = neoc_json_add_string(
        root, "type", neoc_witness_condition_type_to_json_string(condition->type));
    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    switch (condition->type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN:
            err = neoc_json_add_bool(root, "expression", condition->data.boolean.value);
            break;

        case NEOC_WITNESS_CONDITION_NOT: {
            neoc_json_t *child = NULL;
            err = neoc_witness_condition_to_json_object(
                condition->data.not_expr.expression, &child, depth + 1);
            if (err == NEOC_SUCCESS) {
                err = neoc_json_add_object(root, "expression", child);
                if (err != NEOC_SUCCESS) {
                    neoc_json_free(child);
                }
            }
            break;
        }

        case NEOC_WITNESS_CONDITION_AND:
        case NEOC_WITNESS_CONDITION_OR: {
            neoc_json_t *array = neoc_json_create_array();
            if (!array) {
                err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate JSON array");
                break;
            }

            for (size_t i = 0; i < condition->data.logical.count; ++i) {
                neoc_json_t *child = NULL;
                err = neoc_witness_condition_to_json_object(
                    condition->data.logical.expressions[i], &child, depth + 1);
                if (err != NEOC_SUCCESS) {
                    neoc_json_free(child);
                    break;
                }
                err = neoc_json_array_add(array, child);
                if (err != NEOC_SUCCESS) {
                    neoc_json_free(child);
                    break;
                }
            }

            if (err == NEOC_SUCCESS) {
                err = neoc_json_add_object(root, "expressions", array);
                if (err != NEOC_SUCCESS) {
                    neoc_json_free(array);
                }
            } else {
                neoc_json_free(array);
            }
            break;
        }

        case NEOC_WITNESS_CONDITION_SCRIPT_HASH:
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT: {
            char hash_hex[NEOC_HASH160_STRING_LENGTH];
            err = neoc_hash160_to_hex(
                &condition->data.hash_condition.hash, hash_hex, sizeof(hash_hex), false);
            if (err != NEOC_SUCCESS) {
                break;
            }
            char prefixed[NEOC_HASH160_STRING_LENGTH + 3];
            snprintf(prefixed, sizeof(prefixed), "0x%s", hash_hex);
            err = neoc_json_add_string(root, "hash", prefixed);
            break;
        }

        case NEOC_WITNESS_CONDITION_GROUP:
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP: {
            if (!condition->data.group_condition.public_key) {
                err = neoc_error_set(NEOC_ERROR_INVALID_STATE, "Group condition missing key");
                break;
            }

            uint8_t *encoded = NULL;
            size_t encoded_len = 0;
            err = neoc_ec_public_key_get_encoded(
                condition->data.group_condition.public_key, true, &encoded, &encoded_len);
            if (err != NEOC_SUCCESS) {
                break;
            }
            char *hex = neoc_hex_encode_alloc(encoded, encoded_len, false, false);
            free(encoded);
            if (!hex) {
                err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to encode group key");
                break;
            }
            err = neoc_json_add_string(root, "group", hex);
            neoc_free(hex);
            break;
        }

        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY:
            err = NEOC_SUCCESS;
            break;

        default:
            err = neoc_error_set(NEOC_ERROR_INVALID_TYPE, "Unknown witness condition type");
            break;
    }

    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    *object = root;
    return NEOC_SUCCESS;
}

#else /* HAVE_CJSON */

neoc_error_t neoc_witness_condition_from_json(
    const char *json_str,
    neoc_witness_condition_t **condition) {
    if (condition) {
        *condition = NULL;
    }
    (void)json_str;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON parsing not implemented (cJSON required)");
}

neoc_error_t neoc_witness_condition_to_json(
    const neoc_witness_condition_t *condition,
    char **json_str) {
    (void)condition;
    if (json_str) {
        *json_str = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "JSON serialization not implemented (cJSON required)");
}

#endif /* HAVE_CJSON */

size_t neoc_witness_condition_get_size(
    const neoc_witness_condition_t *condition) {
    if (!condition) return 0;
    return neoc_witness_condition_internal_size(condition);
}

static size_t neoc_witness_condition_internal_size(const neoc_witness_condition_t *condition) {
    if (!condition) return 0;

    size_t size = 1; /* type byte */
    switch (condition->type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN:
            size += 1;
            break;
        case NEOC_WITNESS_CONDITION_NOT:
            size += neoc_witness_condition_internal_size(condition->data.not_expr.expression);
            break;
        case NEOC_WITNESS_CONDITION_AND:
        case NEOC_WITNESS_CONDITION_OR: {
            size += neoc_var_int_size(condition->data.logical.count);
            for (size_t i = 0; i < condition->data.logical.count; ++i) {
                size += neoc_witness_condition_internal_size(condition->data.logical.expressions[i]);
            }
            break;
        }
        case NEOC_WITNESS_CONDITION_SCRIPT_HASH:
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT:
            size += NEOC_HASH160_SIZE;
            break;
        case NEOC_WITNESS_CONDITION_GROUP:
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP:
            size += NEOC_PUBLIC_KEY_SIZE_COMPRESSED;
            break;
        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY:
            /* type byte only */
            break;
        default:
            break;
    }
    return size;
}

static size_t neoc_var_int_size(uint64_t value) {
    if (value < 0xFD) return 1;
    if (value <= 0xFFFF) return 3;
    if (value <= 0xFFFFFFFF) return 5;
    return 9;
}

neoc_error_t neoc_witness_condition_serialize(
    const neoc_witness_condition_t *condition,
    neoc_binary_writer_t *writer) {
    if (!condition || !writer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    return neoc_witness_condition_serialize_internal(condition, writer, 0);
}

static neoc_error_t neoc_witness_condition_serialize_internal(
    const neoc_witness_condition_t *condition,
    neoc_binary_writer_t *writer,
    size_t depth) {
    if (depth > NEOC_WITNESS_CONDITION_MAX_NESTING_DEPTH) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness condition nesting depth exceeded");
    }

    neoc_error_t err = neoc_binary_writer_write_byte(writer,
                                                     neoc_witness_condition_type_to_byte(condition->type));
    if (err != NEOC_SUCCESS) return err;

    switch (condition->type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN:
            return neoc_binary_writer_write_bool(writer, condition->data.boolean.value);

        case NEOC_WITNESS_CONDITION_NOT:
            return neoc_witness_condition_serialize_internal(
                condition->data.not_expr.expression, writer, depth + 1);

        case NEOC_WITNESS_CONDITION_AND:
        case NEOC_WITNESS_CONDITION_OR: {
            err = neoc_binary_writer_write_var_int(writer, condition->data.logical.count);
            if (err != NEOC_SUCCESS) return err;
            for (size_t i = 0; i < condition->data.logical.count; ++i) {
                err = neoc_witness_condition_serialize_internal(
                    condition->data.logical.expressions[i], writer, depth + 1);
                if (err != NEOC_SUCCESS) return err;
            }
            return NEOC_SUCCESS;
        }

        case NEOC_WITNESS_CONDITION_SCRIPT_HASH:
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT:
            return neoc_binary_writer_write_bytes(
                writer,
                condition->data.hash_condition.hash.data,
                NEOC_HASH160_SIZE);

        case NEOC_WITNESS_CONDITION_GROUP:
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP: {
            uint8_t *encoded = NULL;
            size_t encoded_len = 0;
            err = neoc_ec_public_key_get_encoded(
                condition->data.group_condition.public_key,
                true,
                &encoded,
                &encoded_len);
            if (err != NEOC_SUCCESS) {
                return err;
            }
            if (encoded_len != NEOC_PUBLIC_KEY_SIZE_COMPRESSED) {
                free(encoded);
                return neoc_error_set(NEOC_ERROR_INVALID_LENGTH, "Invalid compressed public key length");
            }
            err = neoc_binary_writer_write_bytes(writer, encoded, encoded_len);
            free(encoded);
            return err;
        }

        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY:
            return NEOC_SUCCESS;

        default:
            return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "Unknown witness condition type");
    }
}

neoc_error_t neoc_witness_condition_deserialize(
    neoc_binary_reader_t *reader,
    neoc_witness_condition_t **condition) {
    if (!reader || !condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    return neoc_witness_condition_deserialize_internal(reader, 0, condition);
}

static neoc_error_t neoc_witness_condition_deserialize_internal(
    neoc_binary_reader_t *reader,
    size_t depth,
    neoc_witness_condition_t **condition) {
    if (depth > NEOC_WITNESS_CONDITION_MAX_NESTING_DEPTH) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness condition nesting depth exceeded");
    }

    uint8_t type_byte = 0;
    neoc_error_t err = neoc_binary_reader_read_byte(reader, &type_byte);
    if (err != NEOC_SUCCESS) return err;

    neoc_witness_condition_type_t type;
    switch (type_byte) {
        case 0x00: type = NEOC_WITNESS_CONDITION_BOOLEAN; break;
        case 0x01: type = NEOC_WITNESS_CONDITION_NOT; break;
        case 0x02: type = NEOC_WITNESS_CONDITION_AND; break;
        case 0x03: type = NEOC_WITNESS_CONDITION_OR; break;
        case 0x18: type = NEOC_WITNESS_CONDITION_SCRIPT_HASH; break;
        case 0x19: type = NEOC_WITNESS_CONDITION_GROUP; break;
        case 0x20: type = NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY; break;
        case 0x28: type = NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT; break;
        case 0x29: type = NEOC_WITNESS_CONDITION_CALLED_BY_GROUP; break;
        default:
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Unknown witness condition type byte");
    }

    neoc_witness_condition_t *result = neoc_witness_condition_alloc(type);
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition");
    }

    switch (type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN: {
            bool value = false;
            err = neoc_binary_reader_read_bool(reader, &value);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(result);
                return err;
            }
            result->data.boolean.value = value;
            break;
        }
        case NEOC_WITNESS_CONDITION_NOT: {
            err = neoc_witness_condition_deserialize_internal(reader, depth + 1,
                                                               &result->data.not_expr.expression);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(result);
                return err;
            }
            break;
        }
        case NEOC_WITNESS_CONDITION_AND:
        case NEOC_WITNESS_CONDITION_OR: {
            uint64_t count = 0;
            err = neoc_binary_reader_read_var_int(reader, &count);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(result);
                return err;
            }
            if (count == 0 || count > NEOC_WITNESS_CONDITION_MAX_SUBITEMS) {
                neoc_witness_condition_release(result);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid witness condition list size");
            }

            result->data.logical.count = (size_t)count;
            result->data.logical.expressions = calloc(result->data.logical.count,
                                                      sizeof(neoc_witness_condition_t *));
            if (!result->data.logical.expressions) {
                neoc_witness_condition_release(result);
                return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition list");
            }

            for (size_t i = 0; i < result->data.logical.count; ++i) {
                err = neoc_witness_condition_deserialize_internal(
                    reader, depth + 1, &result->data.logical.expressions[i]);
                if (err != NEOC_SUCCESS) {
                    neoc_witness_condition_release(result);
                    return err;
                }
            }
            break;
        }
        case NEOC_WITNESS_CONDITION_SCRIPT_HASH:
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT: {
            err = neoc_hash160_deserialize(&result->data.hash_condition.hash, reader);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(result);
                return err;
            }
            break;
        }
        case NEOC_WITNESS_CONDITION_GROUP:
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP: {
            uint8_t buffer[NEOC_PUBLIC_KEY_SIZE_COMPRESSED] = {0};
            err = neoc_binary_reader_read_bytes(reader, buffer, sizeof(buffer));
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(result);
                return err;
            }
            err = neoc_ec_public_key_from_bytes(buffer, sizeof(buffer),
                                                &result->data.group_condition.public_key);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(result);
                return err;
            }
            break;
        }
        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY:
            /* No payload */
            break;
        default:
            neoc_witness_condition_release(result);
            return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "Unsupported witness condition type");
    }

    *condition = result;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_condition_clone(
    const neoc_witness_condition_t *src,
    neoc_witness_condition_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    return neoc_witness_condition_clone_internal(src, dest);
}

static neoc_error_t neoc_witness_condition_clone_internal(
    const neoc_witness_condition_t *src,
    neoc_witness_condition_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_witness_condition_t *copy = neoc_witness_condition_alloc(src->type);
    if (!copy) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness condition clone");
    }

    switch (src->type) {
        case NEOC_WITNESS_CONDITION_BOOLEAN:
            copy->data.boolean.value = src->data.boolean.value;
            break;

        case NEOC_WITNESS_CONDITION_NOT: {
            neoc_error_t err = neoc_witness_condition_clone_internal(
                src->data.not_expr.expression,
                &copy->data.not_expr.expression);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(copy);
                return err;
            }
            break;
        }

        case NEOC_WITNESS_CONDITION_AND:
        case NEOC_WITNESS_CONDITION_OR: {
            size_t count = src->data.logical.count;
            copy->data.logical.count = count;
            copy->data.logical.expressions = calloc(count, sizeof(neoc_witness_condition_t *));
            if (!copy->data.logical.expressions) {
                neoc_witness_condition_release(copy);
                return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate clone list");
            }
            for (size_t i = 0; i < count; ++i) {
                neoc_error_t err = neoc_witness_condition_clone_internal(
                    src->data.logical.expressions[i],
                    &copy->data.logical.expressions[i]);
                if (err != NEOC_SUCCESS) {
                    neoc_witness_condition_release(copy);
                    return err;
                }
            }
            break;
        }

        case NEOC_WITNESS_CONDITION_SCRIPT_HASH:
        case NEOC_WITNESS_CONDITION_CALLED_BY_CONTRACT:
            memcpy(&copy->data.hash_condition.hash,
                   &src->data.hash_condition.hash,
                   sizeof(neoc_hash160_t));
            break;

        case NEOC_WITNESS_CONDITION_GROUP:
        case NEOC_WITNESS_CONDITION_CALLED_BY_GROUP: {
            neoc_error_t err = neoc_witness_condition_clone_public_key(
                src->data.group_condition.public_key,
                &copy->data.group_condition.public_key);
            if (err != NEOC_SUCCESS) {
                neoc_witness_condition_release(copy);
                return err;
            }
            break;
        }

        case NEOC_WITNESS_CONDITION_CALLED_BY_ENTRY:
            /* Nothing more to copy */
            break;

        default:
            neoc_witness_condition_release(copy);
            return neoc_error_set(NEOC_ERROR_INVALID_TYPE, "Unsupported witness condition type");
    }

    *dest = copy;
    return NEOC_SUCCESS;
}
