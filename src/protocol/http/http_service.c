/**
 * @file http_service.c
 * @brief HTTP service wrapper built on top of the generic service.
 */

#include "neoc/protocol/http/http_service.h"
#include "neoc/protocol/service.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"

#include <string.h>

static neoc_error_t create_base_service(const char *url,
                                        bool include_raw_responses,
                                        long timeout_seconds,
                                        neoc_service_t **out_base) {
    neoc_service_config_t config = {
        .endpoint_url = (char *)url,
        .include_raw_responses = include_raw_responses,
        .timeout_seconds = timeout_seconds,
        .auto_retry = false,
        .max_retries = 0
    };
    return neoc_service_create(NEOC_SERVICE_TYPE_HTTP, &config, out_base);
}

static void free_headers(neoc_http_header_t *headers, size_t count) {
    if (!headers) return;
    for (size_t i = 0; i < count; ++i) {
        neoc_free(headers[i].name);
        neoc_free(headers[i].value);
    }
    neoc_free(headers);
}

static neoc_error_t append_header(neoc_http_service_t *service,
                                  const char *name,
                                  const char *value) {
    size_t new_count = service->header_count + 1;
    neoc_http_header_t *headers = neoc_realloc(service->headers,
                                               new_count * sizeof(neoc_http_header_t));
    if (!headers) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    service->headers = headers;
    service->headers[service->header_count].name = neoc_strdup(name);
    service->headers[service->header_count].value = neoc_strdup(value);
    if (!service->headers[service->header_count].name ||
        !service->headers[service->header_count].value) {
        neoc_free(service->headers[service->header_count].name);
        neoc_free(service->headers[service->header_count].value);
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    service->header_count = new_count;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_http_service_create(neoc_http_service_t **service) {
    return neoc_http_service_create_with_config(NEOC_HTTP_SERVICE_DEFAULT_URL,
                                                false,
                                                NEOC_HTTP_SERVICE_DEFAULT_TIMEOUT,
                                                service);
}

neoc_error_t neoc_http_service_create_with_url(const char *url,
                                               neoc_http_service_t **service) {
    return neoc_http_service_create_with_config(url,
                                                false,
                                                NEOC_HTTP_SERVICE_DEFAULT_TIMEOUT,
                                                service);
}

neoc_error_t neoc_http_service_create_with_config(const char *url,
                                                  bool include_raw_responses,
                                                  long timeout_seconds,
                                                  neoc_http_service_t **service) {
    if (!service || !url) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }

    neoc_http_service_t *http_service = neoc_calloc(1, sizeof(neoc_http_service_t));
    if (!http_service) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }

    neoc_service_t *base = NULL;
    neoc_error_t err = create_base_service(url, include_raw_responses,
                                           timeout_seconds > 0 ? timeout_seconds : NEOC_HTTP_SERVICE_DEFAULT_TIMEOUT,
                                           &base);
    if (err != NEOC_SUCCESS) {
        neoc_free(http_service);
        return err;
    }

    http_service->base = base;
    http_service->headers = NULL;
    http_service->header_count = 0;
    http_service->owns_base = true;

    *service = http_service;
    return NEOC_SUCCESS;
}

void neoc_http_service_free(neoc_http_service_t *service) {
    if (!service) return;
    if (service->base && service->owns_base) {
        neoc_service_free(service->base);
    }
    free_headers(service->headers, service->header_count);
    neoc_free(service);
}

neoc_error_t neoc_http_service_add_header(neoc_http_service_t *service,
                                          const char *name,
                                          const char *value) {
    if (!service || !name || !value) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    return append_header(service, name, value);
}

neoc_error_t neoc_http_service_add_headers(neoc_http_service_t *service,
                                           const neoc_http_header_t *headers,
                                           size_t header_count) {
    if (!service || (!headers && header_count > 0)) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    for (size_t i = 0; i < header_count; ++i) {
        neoc_error_t err = append_header(service,
                                         headers[i].name ? headers[i].name : "",
                                         headers[i].value ? headers[i].value : "");
        if (err != NEOC_SUCCESS) {
            return err;
        }
    }
    return NEOC_SUCCESS;
}

neoc_error_t neoc_http_service_remove_header(neoc_http_service_t *service,
                                             const char *name) {
    if (!service || !name) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    for (size_t i = 0; i < service->header_count; ++i) {
        if (strcmp(service->headers[i].name, name) == 0) {
            neoc_free(service->headers[i].name);
            neoc_free(service->headers[i].value);
            if (i + 1 < service->header_count) {
                memmove(&service->headers[i], &service->headers[i + 1],
                        (service->header_count - i - 1) * sizeof(neoc_http_header_t));
            }
            service->header_count--;
            return NEOC_SUCCESS;
        }
    }
    return NEOC_ERROR_NOT_FOUND;
}

neoc_error_t neoc_http_service_clear_headers(neoc_http_service_t *service) {
    if (!service) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    free_headers(service->headers, service->header_count);
    service->headers = NULL;
    service->header_count = 0;
    return NEOC_SUCCESS;
}

neoc_error_t neoc_http_service_perform_io(neoc_http_service_t *service,
                                          const neoc_byte_array_t *payload,
                                          neoc_byte_array_t **result) {
    if (!service || !payload || !service->base) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    return neoc_service_perform_io(service->base, payload, result);
}

const char* neoc_http_service_get_url(const neoc_http_service_t *service) {
    if (!service || !service->base) {
        return NULL;
    }
    return neoc_service_get_url(service->base);
}

neoc_error_t neoc_http_service_set_url(neoc_http_service_t *service,
                                       const char *url) {
    if (!service || !service->base) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    return neoc_service_set_url(service->base, url);
}

bool neoc_http_service_includes_raw_responses(const neoc_http_service_t *service) {
    if (!service || !service->base) {
        return false;
    }
    return neoc_service_includes_raw_responses(service->base);
}

neoc_error_t neoc_http_service_set_include_raw_responses(neoc_http_service_t *service,
                                                          bool include_raw) {
    if (!service || !service->base) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    return neoc_service_set_include_raw_responses(service->base, include_raw);
}

neoc_service_t* neoc_http_service_get_base(neoc_http_service_t *http_service) {
    return http_service ? http_service->base : NULL;
}

neoc_http_service_t* neoc_http_service_from_base(neoc_service_t *service) {
    if (!service || service->type != NEOC_SERVICE_TYPE_HTTP) {
        return NULL;
    }
    neoc_http_service_t *http_service = neoc_calloc(1, sizeof(neoc_http_service_t));
    if (!http_service) {
        return NULL;
    }
    http_service->base = service;
    http_service->headers = NULL;
    http_service->header_count = 0;
    http_service->owns_base = false;
    return http_service;
}

size_t neoc_http_service_get_header_count(const neoc_http_service_t *service) {
    return service ? service->header_count : 0;
}

neoc_error_t neoc_http_service_get_header(const neoc_http_service_t *service,
                                          size_t index,
                                          const neoc_http_header_t **header) {
    if (!service || !header || index >= service->header_count) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    *header = &service->headers[index];
    return NEOC_SUCCESS;
}

neoc_error_t neoc_http_service_find_header(const neoc_http_service_t *service,
                                           const char *name,
                                           const char **value) {
    if (!service || !name || !value) {
        return NEOC_ERROR_INVALID_ARGUMENT;
    }
    for (size_t i = 0; i < service->header_count; ++i) {
        if (strcmp(service->headers[i].name, name) == 0) {
            *value = service->headers[i].value;
            return NEOC_SUCCESS;
        }
    }
    return NEOC_ERROR_NOT_FOUND;
}
