/**
 * @file witness_rule.c
 * @brief WitnessRule implementation mirroring NeoSwift logic.
 */

#include "neoc/witnessrule/witness_rule.h"

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/utils/json.h"

#include <stdlib.h>
#include <string.h>

static neoc_witness_rule_t *neoc_witness_rule_alloc(void);
static neoc_error_t neoc_witness_rule_copy_condition(
    const neoc_witness_condition_t *source,
    neoc_witness_condition_t **dest);

static neoc_witness_rule_t *neoc_witness_rule_alloc(void) {
    return calloc(1, sizeof(neoc_witness_rule_t));
}

static neoc_error_t neoc_witness_rule_copy_condition(
    const neoc_witness_condition_t *source,
    neoc_witness_condition_t **dest) {
    if (!source || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid condition arguments");
    }
    return neoc_witness_condition_clone(source, dest);
}

neoc_error_t neoc_witness_rule_create(
    neoc_witness_action_t action,
    neoc_witness_condition_t *condition,
    neoc_witness_rule_t **rule) {
    if (!condition || !rule) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness rule arguments");
    }

    neoc_witness_rule_t *result = neoc_witness_rule_alloc();
    if (!result) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness rule");
    }

    neoc_error_t err = neoc_witness_rule_copy_condition(condition, &result->condition);
    if (err != NEOC_SUCCESS) {
        free(result);
        return err;
    }

    result->action = action;
    *rule = result;
    return NEOC_SUCCESS;
}

void neoc_witness_rule_free(
    neoc_witness_rule_t *rule) {
    if (!rule) return;
    if (rule->condition) {
        neoc_witness_condition_free(rule->condition);
    }
    free(rule);
}

neoc_error_t neoc_witness_rule_clone(
    const neoc_witness_rule_t *src,
    neoc_witness_rule_t **dest) {
    if (!src || !dest) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness rule arguments");
    }

    neoc_witness_rule_t *copy = neoc_witness_rule_alloc();
    if (!copy) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness rule clone");
    }

    copy->action = src->action;
    if (src->condition) {
        neoc_error_t err = neoc_witness_condition_clone(src->condition, &copy->condition);
        if (err != NEOC_SUCCESS) {
            neoc_witness_rule_free(copy);
            return err;
        }
    }

    *dest = copy;
    return NEOC_SUCCESS;
}

neoc_witness_action_t neoc_witness_rule_get_action(
    const neoc_witness_rule_t *rule) {
    return rule ? rule->action : NEOC_WITNESS_ACTION_DENY;
}

const neoc_witness_condition_t* neoc_witness_rule_get_condition(
    const neoc_witness_rule_t *rule) {
    return rule ? rule->condition : NULL;
}

size_t neoc_witness_rule_get_size(
    const neoc_witness_rule_t *rule) {
    if (!rule) return 0;
    return 1 + neoc_witness_condition_get_size(rule->condition);
}

neoc_error_t neoc_witness_rule_serialize(
    const neoc_witness_rule_t *rule,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *bytes_written) {
    if (!rule || !buffer) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    size_t required = neoc_witness_rule_get_size(rule);
    if (buffer_size < required) {
        if (bytes_written) *bytes_written = required;
        return neoc_error_set(NEOC_ERROR_BUFFER_TOO_SMALL, "Buffer too small for witness rule serialization");
    }

    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(required, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    err = neoc_binary_writer_write_byte(writer, neoc_witness_action_get_byte(rule->action));
    if (err == NEOC_SUCCESS) {
        err = neoc_witness_condition_serialize(rule->condition, writer);
    }

    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }

    uint8_t *serialized = NULL;
    size_t serialized_len = 0;
    err = neoc_binary_writer_to_array(writer, &serialized, &serialized_len);
    neoc_binary_writer_free(writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    memcpy(buffer, serialized, serialized_len);
    free(serialized);

    if (bytes_written) {
        *bytes_written = serialized_len;
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_witness_rule_deserialize(
    const uint8_t *buffer,
    size_t buffer_size,
    neoc_witness_rule_t **rule,
    size_t *bytes_read) {
    if (!buffer || !rule) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }

    neoc_binary_reader_t *reader = NULL;
    neoc_error_t err = neoc_binary_reader_create(buffer, buffer_size, &reader);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    uint8_t action_byte = 0;
    err = neoc_binary_reader_read_byte(reader, &action_byte);
    if (err != NEOC_SUCCESS) {
        neoc_binary_reader_free(reader);
        return err;
    }

    neoc_witness_action_t action;
    err = neoc_witness_action_from_byte(action_byte, &action);
    if (err != NEOC_SUCCESS) {
        neoc_binary_reader_free(reader);
        return err;
    }

    neoc_witness_condition_t *condition = NULL;
    err = neoc_witness_condition_deserialize(reader, &condition);
    if (err != NEOC_SUCCESS) {
        neoc_binary_reader_free(reader);
        return err;
    }

    neoc_witness_rule_t *result = neoc_witness_rule_alloc();
    if (!result) {
        neoc_binary_reader_free(reader);
        neoc_witness_condition_free(condition);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness rule");
    }

    result->action = action;
    result->condition = condition;

    if (bytes_read) {
        *bytes_read = neoc_binary_reader_get_position(reader);
    }
    neoc_binary_reader_free(reader);

    *rule = result;
    return NEOC_SUCCESS;
}

#ifdef HAVE_CJSON

neoc_error_t neoc_witness_rule_from_json(
    const char *json_str,
    neoc_witness_rule_t **rule) {
    if (!json_str || !rule) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness rule JSON input");
    }

    *rule = NULL;
    neoc_json_t *root = neoc_json_parse(json_str);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse witness rule JSON");
    }

    const char *action_str = neoc_json_get_string(root, "action");
    neoc_error_t err = NEOC_SUCCESS;
    neoc_witness_action_t action = NEOC_WITNESS_ACTION_DENY;
    neoc_witness_condition_t *condition = NULL;

    if (!action_str) {
        err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness rule missing action");
        goto cleanup;
    }

    err = neoc_witness_action_from_json_string(action_str, &action);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }

    neoc_json_t *condition_json = neoc_json_get_object(root, "condition");
    if (!condition_json) {
        err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Witness rule missing condition");
        goto cleanup;
    }

    char *condition_str = neoc_json_to_string(condition_json);
    if (!condition_str) {
        err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to serialize condition JSON");
        goto cleanup;
    }

    err = neoc_witness_condition_from_json(condition_str, &condition);
    neoc_free(condition_str);
    if (err != NEOC_SUCCESS) {
        goto cleanup;
    }

    neoc_witness_rule_t *result = neoc_witness_rule_alloc();
    if (!result) {
        err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness rule");
        goto cleanup;
    }

    result->action = action;
    result->condition = condition;
    *rule = result;
    condition = NULL; /* Ownership transferred */

cleanup:
    if (condition) {
        neoc_witness_condition_free(condition);
    }
    neoc_json_free(root);
    return err;
}

neoc_error_t neoc_witness_rule_to_json(
    const neoc_witness_rule_t *rule,
    char **json_str) {
    if (!rule || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness rule");
    }

    *json_str = NULL;
    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate witness rule JSON object");
    }

    neoc_error_t err = neoc_json_add_string(
        root, "action", neoc_witness_action_to_json_string(rule->action));
    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    char *condition_str = NULL;
    err = neoc_witness_condition_to_json(rule->condition, &condition_str);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(root);
        return err;
    }

    neoc_json_t *condition_json = neoc_json_parse(condition_str);
    neoc_free(condition_str);
    if (!condition_json) {
        neoc_json_free(root);
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse condition JSON");
    }

    err = neoc_json_add_object(root, "condition", condition_json);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(condition_json);
        neoc_json_free(root);
        return err;
    }

    char *serialized = neoc_json_to_string(root);
    neoc_json_free(root);
    if (!serialized) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to serialize witness rule JSON");
    }

    *json_str = serialized;
    return NEOC_SUCCESS;
}

#else /* HAVE_CJSON */

neoc_error_t neoc_witness_rule_from_json(
    const char *json_str,
    neoc_witness_rule_t **rule) {
    (void)json_str;
    (void)rule;
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "Witness rule JSON parsing not implemented");
}

neoc_error_t neoc_witness_rule_to_json(
    const neoc_witness_rule_t *rule,
    char **json_str) {
    (void)rule;
    if (json_str) {
        *json_str = NULL;
    }
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED, "Witness rule JSON serialization not implemented");
}

#endif /* HAVE_CJSON */

bool neoc_witness_rule_equals(
    const neoc_witness_rule_t *rule1,
    const neoc_witness_rule_t *rule2) {
    if (rule1 == rule2) return true;
    if (!rule1 || !rule2) return false;
    if (rule1->action != rule2->action) return false;

    size_t size1 = neoc_witness_rule_get_size(rule1);
    size_t size2 = neoc_witness_rule_get_size(rule2);
    if (size1 != size2) return false;

    uint8_t *buf1 = malloc(size1);
    uint8_t *buf2 = malloc(size2);
    if (!buf1 || !buf2) {
        free(buf1);
        free(buf2);
        return false;
    }

    size_t written1 = 0, written2 = 0;
    neoc_witness_rule_serialize(rule1, buf1, size1, &written1);
    neoc_witness_rule_serialize(rule2, buf2, size2, &written2);

    bool equal = (written1 == written2) && memcmp(buf1, buf2, written1) == 0;
    free(buf1);
    free(buf2);
    return equal;
}

neoc_error_t neoc_witness_rule_validate(
    const neoc_witness_rule_t *rule) {
    if (!rule || !rule->condition) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid witness rule");
    }
    return NEOC_SUCCESS;
}
