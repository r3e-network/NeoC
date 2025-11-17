/**
 * @file service.c
 * @brief Base service implementation for Neo JSON-RPC
 * 
 * Based on Swift source: protocol/Service.swift
 * Provides the base service functionality for all Neo blockchain services
 */

#include "neoc/neoc_error.h"
#include "neoc/neoc_memory.h"
#include "neoc/protocol/service.h"
#include "neoc/protocol/core/request.h"
#include "neoc/protocol/core/response.h"
#include "neoc/utils/array.h"
#include "neoc/utils/url_session.h"
#include "neoc/utils/decode.h"
#include <cjson/cJSON.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static neoc_error_t neoc_service_config_copy(neoc_service_config_t *dest,
                                             const neoc_service_config_t *src) {
    if (!dest || !src) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    memset(dest, 0, sizeof(*dest));

    if (src->endpoint_url) {
        size_t len = strlen(src->endpoint_url) + 1;
        dest->endpoint_url = neoc_malloc(len);
        if (!dest->endpoint_url) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(dest->endpoint_url, src->endpoint_url, len);
    }

    dest->include_raw_responses = src->include_raw_responses;
    dest->timeout_seconds = src->timeout_seconds;
    dest->auto_retry = src->auto_retry;
    dest->max_retries = src->max_retries;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_service_config_create_default(const char *url,
                                                neoc_service_config_t **config) {
    if (!config) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    *config = NULL;

    neoc_service_config_t *cfg = neoc_calloc(1, sizeof(neoc_service_config_t));
    if (!cfg) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    cfg->endpoint_url = NULL;
    if (url) {
        size_t len = strlen(url) + 1;
        cfg->endpoint_url = neoc_malloc(len);
        if (!cfg->endpoint_url) {
            neoc_free(cfg);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(cfg->endpoint_url, url, len);
    }

    cfg->include_raw_responses = false;
    cfg->timeout_seconds = 30;
    cfg->auto_retry = true;
    cfg->max_retries = 3;

    *config = cfg;
    return NEOC_SUCCESS;
}

void neoc_service_config_free(neoc_service_config_t *config) {
    if (!config) return;
    if (config->endpoint_url) {
        neoc_free(config->endpoint_url);
        config->endpoint_url = NULL;
    }
    neoc_free(config);
}

neoc_error_t neoc_service_init(neoc_service_t *service,
                               neoc_service_type_t type,
                               const neoc_service_config_t *config,
                               neoc_service_vtable_t *vtable,
                               void *impl_data) {
    if (!service || !config) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    memset(service, 0, sizeof(*service));
    service->type = type;
    service->vtable = vtable;
    service->impl_data = impl_data;

    neoc_error_t err = neoc_service_config_copy(&service->config, config);
    if (err != NEOC_SUCCESS) {
        memset(&service->config, 0, sizeof(service->config));
        return err;
    }
    return NEOC_SUCCESS;
}

/**
 * @brief Create a new service instance
 */
neoc_error_t neoc_service_create(neoc_service_type_t type,
                                 const neoc_service_config_t *config,
                                 neoc_service_t **service) {
    if (!service || !config) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *service = NULL;
    
    neoc_service_t *new_service = neoc_malloc(sizeof(neoc_service_t));
    if (!new_service) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    // Initialize the service structure
    new_service->type = type;
    new_service->vtable = NULL;
    new_service->impl_data = NULL;

    neoc_error_t err = neoc_service_config_copy(&new_service->config, config);
    if (err != NEOC_SUCCESS) {
        neoc_free(new_service);
        return err;
    }
    
    if (new_service->config.timeout_seconds <= 0) {
        new_service->config.timeout_seconds = 60;
    }

    neoc_url_session_config_t session_config;
    neoc_url_session_get_default_config(&session_config);
    session_config.timeout_seconds = new_service->config.timeout_seconds;
    neoc_url_session_t *session = NULL;
    neoc_error_t session_err = neoc_url_session_create_with_config(&session_config, &session);
    if (session_err != NEOC_SUCCESS) {
        if (new_service->config.endpoint_url) {
            neoc_free(new_service->config.endpoint_url);
        }
        neoc_free(new_service);
        return session_err;
    }
    new_service->impl_data = session;
    
    *service = new_service;
    return NEOC_SUCCESS;
}

/**
 * @brief Free a service
 */
void neoc_service_free(neoc_service_t *service) {
    if (!service) {
        return;
    }
    
    if (service->vtable && service->vtable->free_impl) {
        service->vtable->free_impl(service);
    }

    if (service->impl_data) {
        neoc_url_session_free((neoc_url_session_t *)service->impl_data);
        service->impl_data = NULL;
    }
    
    // Free configuration
    if (service->config.endpoint_url) {
        neoc_free(service->config.endpoint_url);
    }
    
    neoc_free(service);
}

/**
 * @brief Send a JSON-RPC request through the service
 */
neoc_error_t neoc_service_send_request(neoc_service_t *service,
                                       const neoc_request_t *request,
                                       neoc_response_t **response) {
    if (!service || !request || !response) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *response = NULL;
    
    cJSON *request_json = cJSON_CreateObject();
    if (!request_json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    cJSON_AddStringToObject(request_json, "jsonrpc", request->jsonrpc ? request->jsonrpc : "2.0");
    cJSON_AddStringToObject(request_json, "method", request->method ? request->method : "");

    cJSON *params_node = NULL;
    if (request->params && strlen(request->params) > 0) {
        params_node = cJSON_Parse(request->params);
    }
    if (!params_node) {
        params_node = cJSON_CreateArray();
    }
    cJSON_AddItemToObject(request_json, "params", params_node);
    cJSON_AddNumberToObject(request_json, "id", request->id);

    char *payload_json = cJSON_PrintUnformatted(request_json);
    cJSON_Delete(request_json);
    if (!payload_json) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_byte_array_t payload = { .data = (uint8_t*)payload_json, .length = strlen(payload_json), .capacity = 0 };

    neoc_byte_array_t *result = NULL;
    neoc_error_t err = neoc_service_perform_io(service, &payload, &result);
    neoc_free(payload_json);
    if (err != NEOC_SUCCESS) {
        return err;
    }

    if (!result || !result->data) {
        neoc_byte_array_free(result);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    char *response_str = neoc_malloc(result->length + 1);
    if (!response_str) {
        neoc_byte_array_free(result);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    memcpy(response_str, result->data, result->length);
    response_str[result->length] = '\0';

    cJSON *response_json = cJSON_Parse(response_str);
    if (!response_json || !cJSON_IsObject(response_json)) {
        neoc_byte_array_free(result);
        neoc_free(response_str);
        if (response_json) cJSON_Delete(response_json);
        return NEOC_ERROR_INVALID_FORMAT;
    }

    neoc_response_t *resp = neoc_response_create(request->id);
    if (!resp) {
        cJSON_Delete(response_json);
        neoc_byte_array_free(result);
        neoc_free(response_str);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    const cJSON *jsonrpc = cJSON_GetObjectItemCaseSensitive(response_json, "jsonrpc");
    if (cJSON_IsString(jsonrpc) && jsonrpc->valuestring) {
        neoc_free(resp->jsonrpc);
        resp->jsonrpc = neoc_strdup(jsonrpc->valuestring);
    }

    const cJSON *id = cJSON_GetObjectItemCaseSensitive(response_json, "id");
    if (id) {
        if (cJSON_IsNumber(id)) {
            resp->id = (int)id->valuedouble;
        } else if (cJSON_IsString(id) && id->valuestring) {
            int parsed_id = 0;
            if (neoc_decode_int_from_string(id->valuestring, &parsed_id) == NEOC_SUCCESS) {
                resp->id = parsed_id;
            }
        }
    }

    if (service->config.include_raw_responses) {
        neoc_response_set_raw(resp, response_str);
    }

    const cJSON *error_obj = cJSON_GetObjectItemCaseSensitive(response_json, "error");
    if (error_obj && cJSON_IsObject(error_obj)) {
        const cJSON *code = cJSON_GetObjectItemCaseSensitive(error_obj, "code");
        const cJSON *message = cJSON_GetObjectItemCaseSensitive(error_obj, "message");
        const cJSON *data = cJSON_GetObjectItemCaseSensitive(error_obj, "data");

        char *data_str = NULL;
        if (data) {
            if (cJSON_IsString(data) && data->valuestring) {
                data_str = data->valuestring;
            } else {
                data_str = cJSON_PrintUnformatted((cJSON *)data);
            }
        }

        neoc_response_set_error(resp,
                                 code && cJSON_IsNumber(code) ? (int)code->valuedouble : -1,
                                 message && cJSON_IsString(message) ? message->valuestring : NULL,
                                 data_str);

        if (data && data_str && data_str != data->valuestring) {
            cJSON_free(data_str);
        }
    } else {
        const cJSON *result_obj = cJSON_GetObjectItemCaseSensitive(response_json, "result");
        if (result_obj) {
            char *result_str = cJSON_PrintUnformatted((cJSON *)result_obj);
            if (!result_str) {
                neoc_response_free(resp);
                cJSON_Delete(response_json);
                neoc_byte_array_free(result);
                neoc_free(response_str);
                return NEOC_ERROR_OUT_OF_MEMORY;
            }
            resp->result = result_str;
            resp->has_error = false;
        }
    }

    *response = resp;

    cJSON_Delete(response_json);
    neoc_byte_array_free(result);
    neoc_free(response_str);
    return NEOC_SUCCESS;
}

/**
 * @brief Perform low-level IO operation
 */
neoc_error_t neoc_service_perform_io(neoc_service_t *service,
                                      const neoc_byte_array_t *payload,
                                      neoc_byte_array_t **result) {
    if (!service || !payload || !result) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    *result = NULL;

    if (service->vtable && service->vtable->perform_io) {
        return service->vtable->perform_io(service, payload, result);
    }

    neoc_url_session_t *session = (neoc_url_session_t *)service->impl_data;
    if (!session) {
        return NEOC_ERROR_INVALID_STATE;
    }

    const char *url = service->config.endpoint_url ? service->config.endpoint_url : "http://localhost:10333/";
    char *payload_str = NULL;
    if (payload->data && payload->length > 0) {
        payload_str = neoc_malloc(payload->length + 1);
        if (!payload_str) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(payload_str, payload->data, payload->length);
        payload_str[payload->length] = '\0';
    } else {
        payload_str = neoc_strdup("{}");
        if (!payload_str) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
    }

    neoc_http_response_t *http_response = NULL;
    neoc_error_t err = neoc_url_session_post_json(session, url, payload_str, &http_response);
    neoc_free(payload_str);
    if (err != NEOC_SUCCESS) {
        if (http_response) {
            neoc_http_response_free(http_response);
        }
        return err;
    }

    if (!http_response) {
        return NEOC_ERROR_NETWORK;
    }

    if (http_response->status_code >= 400) {
        neoc_http_response_free(http_response);
        return NEOC_ERROR_RPC;
    }

    neoc_byte_array_t *out = neoc_calloc(1, sizeof(neoc_byte_array_t));
    if (!out) {
        neoc_http_response_free(http_response);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    if (http_response->body && http_response->body->data && http_response->body->length > 0) {
        out->data = neoc_malloc(http_response->body->length);
        if (!out->data) {
            neoc_http_response_free(http_response);
            neoc_free(out);
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(out->data, http_response->body->data, http_response->body->length);
        out->length = http_response->body->length;
        out->capacity = http_response->body->length;
    }

    neoc_http_response_free(http_response);
    *result = out;
    return NEOC_SUCCESS;
}

/**
 * @brief Check if service includes raw responses
 */
bool neoc_service_includes_raw_responses(const neoc_service_t *service) {
    if (!service) {
        return false;
    }
    
    return service->config.include_raw_responses;
}

/**
 * @brief Set whether to include raw responses
 */
neoc_error_t neoc_service_set_include_raw_responses(neoc_service_t *service,
                                                    bool include_raw) {
    if (!service) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    service->config.include_raw_responses = include_raw;
    return NEOC_SUCCESS;
}

/**
 * @brief Get service endpoint URL
 */
const char* neoc_service_get_url(const neoc_service_t *service) {
    if (!service) {
        return NULL;
    }
    
    return service->config.endpoint_url;
}

/**
 * @brief Set service endpoint URL
 */
neoc_error_t neoc_service_set_url(neoc_service_t *service, const char *url) {
    if (!service) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    // Free existing URL
    if (service->config.endpoint_url) {
        neoc_free(service->config.endpoint_url);
        service->config.endpoint_url = NULL;
    }
    
    // Set new URL if provided
    if (url) {
        size_t url_len = strlen(url) + 1;
        service->config.endpoint_url = neoc_malloc(url_len);
        if (!service->config.endpoint_url) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        strcpy(service->config.endpoint_url, url);
    }
    
    return NEOC_SUCCESS;
}

/**
 * @brief Get service type as string
 */
const char* neoc_service_type_to_string(neoc_service_type_t type) {
    switch (type) {
        case NEOC_SERVICE_TYPE_HTTP:
            return "HTTP";
        case NEOC_SERVICE_TYPE_WEBSOCKET:
            return "WebSocket";
        case NEOC_SERVICE_TYPE_IPC:
            return "IPC";
        default:
            return "Unknown";
    }
}

/**
 * @brief Create service from URL (factory method)
 */
neoc_error_t neoc_service_create_from_url(const char *url,
                                          neoc_service_t **service) {
    if (!url || !service) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    *service = NULL;
    
    // Determine service type from URL scheme
    neoc_service_type_t type;
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        type = NEOC_SERVICE_TYPE_HTTP;
    } else if (strncmp(url, "ws://", 5) == 0 || strncmp(url, "wss://", 6) == 0) {
        type = NEOC_SERVICE_TYPE_WEBSOCKET;
    } else {
        return NEOC_ERROR_INVALID_FORMAT;
    }
    
    // Create default configuration
    neoc_service_config_t config = {
        .endpoint_url = (char*)url,
        .include_raw_responses = false,
        .timeout_seconds = 30,
        .auto_retry = true,
        .max_retries = 3
    };
    
    return neoc_service_create(type, &config, service);
}

/**
 * @brief Get service configuration
 */
neoc_error_t neoc_service_get_config(neoc_service_t *service,
                                     neoc_service_config_t *config) {
    if (!service || !config) {
        return NEOC_ERROR_INVALID_PARAM;
    }

    if (service->vtable && service->vtable->get_config) {
        return service->vtable->get_config(service, config);
    }

    config->endpoint_url = NULL;
    config->include_raw_responses = service->config.include_raw_responses;
    config->timeout_seconds = service->config.timeout_seconds;
    config->auto_retry = service->config.auto_retry;
    config->max_retries = service->config.max_retries;

    if (service->config.endpoint_url) {
        size_t url_len = strlen(service->config.endpoint_url) + 1;
        config->endpoint_url = neoc_malloc(url_len);
        if (!config->endpoint_url) {
            return NEOC_ERROR_OUT_OF_MEMORY;
        }
        memcpy(config->endpoint_url, service->config.endpoint_url, url_len);
    }

    return NEOC_SUCCESS;
}

/**
 * @brief Set service configuration
 */
neoc_error_t neoc_service_set_config(neoc_service_t *service,
                                     const neoc_service_config_t *config) {
    if (!service || !config) {
        return NEOC_ERROR_INVALID_PARAM;
    }
    
    if (service->vtable && service->vtable->set_config) {
        return service->vtable->set_config(service, config);
    }
    
    // Default implementation - update configuration
    neoc_error_t error = neoc_service_set_url(service, config->endpoint_url);
    if (error != NEOC_SUCCESS) {
        return error;
    }
    
    service->config.include_raw_responses = config->include_raw_responses;
    service->config.timeout_seconds = config->timeout_seconds;
    service->config.auto_retry = config->auto_retry;
    service->config.max_retries = config->max_retries;
    
    return NEOC_SUCCESS;
}
