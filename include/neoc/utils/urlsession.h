/**
 * @file urlsession.h
 * @brief URL session utilities (compatibility wrapper)
 * 
 * This file is a compatibility wrapper for url_session.h
 * Based on Swift source: utils/URLSession.swift
 */

#ifndef NEOC_UTILS_URLSESSION_H
#define NEOC_UTILS_URLSESSION_H

// Include the main URL session implementation
#include "neoc/utils/url_session.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This header provides compatibility for different include styles.
 * All functionality is provided by url_session.h
 * 
 * The following types and functions are available:
 * - neoc_url_session_t
 * - neoc_http_request_t
 * - neoc_http_response_t
 * - neoc_url_session_create()
 * - neoc_url_session_perform_request()
 * - neoc_url_session_get()
 * - neoc_url_session_post_json()
 * 
 * See url_session.h for complete documentation.
 */

#ifdef __cplusplus
}
#endif

#endif /* NEOC_UTILS_URLSESSION_H */
