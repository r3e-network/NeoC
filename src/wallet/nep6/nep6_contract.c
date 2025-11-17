/**
 * @file nep6_contract.c
 * @brief NEP-6 contract implementation
 */

#include "neoc/wallet/nep6/nep6_contract.h"

#include "neoc/utils/json.h"
#include "neoc/types/contract_parameter_type.h"
#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"

#include <string.h>

const char* neoc_nep6_contract_get_script(const neoc_nep6_contract_t *contract) {
    return contract ? contract->script : NULL;
}

size_t neoc_nep6_contract_get_parameter_count(const neoc_nep6_contract_t *contract) {
    return contract ? contract->parameter_count : 0;
}

neoc_nep6_parameter_t* neoc_nep6_contract_get_parameter(const neoc_nep6_contract_t *contract,
                                                        size_t index) {
    if (!contract || index >= contract->parameter_count) {
        return NULL;
    }
    return &contract->parameters[index];
}

bool neoc_nep6_contract_is_deployed(const neoc_nep6_contract_t *contract) {
    return contract ? contract->is_deployed : false;
}

const char* neoc_nep6_parameter_get_name(const neoc_nep6_parameter_t *parameter) {
    return parameter ? parameter->name : NULL;
}

neoc_contract_parameter_type_t neoc_nep6_parameter_get_type(const neoc_nep6_parameter_t *parameter) {
    return parameter ? parameter->type : NEOC_PARAM_TYPE_ANY;
}

static void neoc_nep6_contract_dispose(neoc_nep6_contract_t *contract) {
    if (!contract) {
        return;
    }

    if (contract->script) {
        neoc_free(contract->script);
    }

    if (contract->parameters) {
        for (size_t i = 0; i < contract->parameter_count; ++i) {
            if (contract->parameters[i].name) {
                neoc_free(contract->parameters[i].name);
                contract->parameters[i].name = NULL;
            }
        }
        neoc_free(contract->parameters);
    }

    neoc_free(contract);
}

neoc_error_t neoc_nep6_contract_create(const char *script,
                                       const neoc_nep6_parameter_t *parameters,
                                       size_t parameter_count,
                                       bool is_deployed,
                                       neoc_nep6_contract_t **contract) {
    if (!contract) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Contract pointer is NULL");
    }

    *contract = neoc_calloc(1, sizeof(neoc_nep6_contract_t));
    if (!*contract) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEP-6 contract");
    }

    if (script) {
        (*contract)->script = neoc_strdup(script);
        if (!(*contract)->script) {
            neoc_nep6_contract_free(*contract);
            *contract = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy contract script");
        }
    }

    if (parameter_count > 0 && parameters) {
        (*contract)->parameters = neoc_calloc(parameter_count, sizeof(neoc_nep6_parameter_t));
        if (!(*contract)->parameters) {
            neoc_nep6_contract_free(*contract);
            *contract = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate contract parameters");
        }

        for (size_t i = 0; i < parameter_count; ++i) {
            if (parameters[i].name) {
                (*contract)->parameters[i].name = neoc_strdup(parameters[i].name);
                if (!(*contract)->parameters[i].name) {
                    (*contract)->parameter_count = i;
                    neoc_nep6_contract_free(*contract);
                    *contract = NULL;
                    return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy parameter name");
                }
            }
            (*contract)->parameters[i].type = parameters[i].type;
        }
        (*contract)->parameter_count = parameter_count;
    }

    (*contract)->is_deployed = is_deployed;
    return NEOC_SUCCESS;
}

void neoc_nep6_contract_free(neoc_nep6_contract_t *contract) {
    neoc_nep6_contract_dispose(contract);
}

neoc_error_t neoc_nep6_parameter_create(const char *name,
                                        neoc_contract_parameter_type_t type,
                                        neoc_nep6_parameter_t **parameter) {
    if (!parameter) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Parameter pointer is NULL");
    }

    *parameter = neoc_calloc(1, sizeof(neoc_nep6_parameter_t));
    if (!*parameter) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEP-6 parameter");
    }

    if (name) {
        (*parameter)->name = neoc_strdup(name);
        if (!(*parameter)->name) {
            neoc_free(*parameter);
            *parameter = NULL;
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy parameter name");
        }
    }

    (*parameter)->type = type;
    return NEOC_SUCCESS;
}

void neoc_nep6_parameter_free(neoc_nep6_parameter_t *parameter) {
    if (!parameter) {
        return;
    }

    if (parameter->name) {
        neoc_free(parameter->name);
        parameter->name = NULL;
    }

    neoc_free(parameter);
}

static neoc_error_t append_parameter_json(const neoc_nep6_parameter_t *parameter,
                                          neoc_json_t *array) {
    neoc_json_t *param_obj = neoc_json_create_object();
    if (!param_obj) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create parameter JSON object");
    }

    if (parameter->name) {
        neoc_error_t err = neoc_json_add_string(param_obj, "name", parameter->name);
        if (err != NEOC_SUCCESS) {
            neoc_json_free(param_obj);
            return err;
        }
    }

    const char *type_str = neoc_contract_parameter_type_to_string(parameter->type);
    if (type_str) {
        neoc_error_t err = neoc_json_add_string(param_obj, "type", type_str);
        if (err != NEOC_SUCCESS) {
            neoc_json_free(param_obj);
            return err;
        }
    } else {
        neoc_error_t err = neoc_json_add_number(param_obj, "type", (double)parameter->type);
        if (err != NEOC_SUCCESS) {
            neoc_json_free(param_obj);
            return err;
        }
    }

    neoc_error_t err = neoc_json_array_add(array, param_obj);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(param_obj);
        return err;
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep6_contract_to_json(const neoc_nep6_contract_t *contract,
                                        char **json_str) {
    if (!contract || !json_str) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Contract to JSON: invalid arguments");
    }

    neoc_json_t *root = neoc_json_create_object();
    if (!root) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create JSON object");
    }

    neoc_error_t err = NEOC_SUCCESS;

    if (contract->script) {
        err = neoc_json_add_string(root, "script", contract->script);
        if (err != NEOC_SUCCESS) goto cleanup;
    }

    err = neoc_json_add_bool(root, "isDeployed", contract->is_deployed);
    if (err != NEOC_SUCCESS) goto cleanup;

    neoc_json_t *params_array = neoc_json_create_array();
    if (!params_array) {
        err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to create parameters array");
        goto cleanup;
    }

    for (size_t i = 0; i < contract->parameter_count; ++i) {
        err = append_parameter_json(&contract->parameters[i], params_array);
        if (err != NEOC_SUCCESS) {
            neoc_json_free(params_array);
            goto cleanup;
        }
    }

    err = neoc_json_add_object(root, "parameters", params_array);
    if (err != NEOC_SUCCESS) {
        neoc_json_free(params_array);
        goto cleanup;
    }

    *json_str = neoc_json_to_string(root);
    if (!*json_str) {
        err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to serialize contract JSON");
    }

cleanup:
    neoc_json_free(root);
    return err;
}

static neoc_error_t parse_parameter_from_json(neoc_json_t *param_json,
                                              neoc_nep6_parameter_t *parameter) {
    memset(parameter, 0, sizeof(*parameter));

    const char *name = neoc_json_get_string(param_json, "name");
    if (name) {
        parameter->name = neoc_strdup(name);
        if (!parameter->name) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy parameter name");
        }
    }

    const char *type_str = neoc_json_get_string(param_json, "type");
    if (type_str) {
        neoc_error_t err = neoc_contract_parameter_type_from_string(type_str, &parameter->type);
        if (err != NEOC_SUCCESS) {
            if (parameter->name) neoc_free(parameter->name);
            return err;
        }
    } else {
        int64_t type_value = 0;
        neoc_error_t err = neoc_json_get_int(param_json, "type", &type_value);
        if (err == NEOC_SUCCESS) {
            parameter->type = (neoc_contract_parameter_type_t)type_value;
        } else {
            if (parameter->name) neoc_free(parameter->name);
            return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Parameter type missing");
        }
    }

    return NEOC_SUCCESS;
}

neoc_error_t neoc_nep6_contract_from_json(const char *json_str,
                                          neoc_nep6_contract_t **contract) {
    if (!json_str || !contract) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Contract from JSON: invalid arguments");
    }

    neoc_json_t *root = neoc_json_parse(json_str);
    if (!root) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Failed to parse contract JSON");
    }

    neoc_nep6_contract_t *result = neoc_calloc(1, sizeof(neoc_nep6_contract_t));
    if (!result) {
        neoc_json_free(root);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEP-6 contract");
    }

    const char *script = neoc_json_get_string(root, "script");
    if (script) {
        result->script = neoc_strdup(script);
        if (!result->script) {
            neoc_json_free(root);
            neoc_nep6_contract_free(result);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to copy contract script");
        }
    }

    bool is_deployed = false;
    neoc_json_get_bool(root, "isDeployed", &is_deployed);
    result->is_deployed = is_deployed;

    neoc_json_t *params_array = neoc_json_get_array(root, "parameters");
    if (params_array && neoc_json_array_size(params_array) > 0) {
        size_t count = neoc_json_array_size(params_array);
        result->parameters = neoc_calloc(count, sizeof(neoc_nep6_parameter_t));
        if (!result->parameters) {
            neoc_json_free(root);
            neoc_nep6_contract_free(result);
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate parameters array");
        }

        for (size_t i = 0; i < count; ++i) {
            neoc_json_t *param_json = neoc_json_array_get(params_array, i);
            if (!param_json) {
                neoc_json_free(root);
                result->parameter_count = i;
                neoc_nep6_contract_free(result);
                return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid parameter entry");
            }

            neoc_error_t err = parse_parameter_from_json(param_json, &result->parameters[i]);
            if (err != NEOC_SUCCESS) {
                neoc_json_free(root);
                result->parameter_count = i;
                neoc_nep6_contract_free(result);
                return err;
            }
        }
        result->parameter_count = count;
    }

    neoc_json_free(root);
    *contract = result;
    return NEOC_SUCCESS;
}
