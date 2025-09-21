/**
 * @file oracle_response_code.c
 * @brief OracleResponseCode implementation
 * 
 * Based on Swift source: protocol/core/response/OracleResponseCode.swift
 */

#include "neoc/neoc.h"
#include <string.h>

/**
 * @brief Convert oracleresponsecode to string
 */
const char *neoc_oracleresponsecode_to_string(neoc_oracleresponsecode_t value) {
    switch (value) {
        case NEOC_ORACLERESPONSECODE_SUCCESS: return "SUCCESS";
        case NEOC_ORACLERESPONSECODE_PROTOCOL_NOT_SUPPORTED: return "PROTOCOL_NOT_SUPPORTED";
        case NEOC_ORACLERESPONSECODE_CONSENSUS_UNREACHABLE: return "CONSENSUS_UNREACHABLE";
        case NEOC_ORACLERESPONSECODE_NOT_FOUND: return "NOT_FOUND";
        case NEOC_ORACLERESPONSECODE_TIMEOUT: return "TIMEOUT";
        case NEOC_ORACLERESPONSECODE_FORBIDDEN: return "FORBIDDEN";
        case NEOC_ORACLERESPONSECODE_RESPONSE_TOO_LARGE: return "RESPONSE_TOO_LARGE";
        case NEOC_ORACLERESPONSECODE_INSUFFICIENT_FUNDS: return "INSUFFICIENT_FUNDS";
        case NEOC_ORACLERESPONSECODE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to oracleresponsecode
 */
neoc_oracleresponsecode_t neoc_oracleresponsecode_from_string(const char *str) {
    if (!str) return -1;
    
    if (strcmp(str, "SUCCESS") == 0) return NEOC_ORACLERESPONSECODE_SUCCESS;
    if (strcmp(str, "PROTOCOL_NOT_SUPPORTED") == 0) return NEOC_ORACLERESPONSECODE_PROTOCOL_NOT_SUPPORTED;
    if (strcmp(str, "CONSENSUS_UNREACHABLE") == 0) return NEOC_ORACLERESPONSECODE_CONSENSUS_UNREACHABLE;
    if (strcmp(str, "NOT_FOUND") == 0) return NEOC_ORACLERESPONSECODE_NOT_FOUND;
    if (strcmp(str, "TIMEOUT") == 0) return NEOC_ORACLERESPONSECODE_TIMEOUT;
    if (strcmp(str, "FORBIDDEN") == 0) return NEOC_ORACLERESPONSECODE_FORBIDDEN;
    if (strcmp(str, "RESPONSE_TOO_LARGE") == 0) return NEOC_ORACLERESPONSECODE_RESPONSE_TOO_LARGE;
    if (strcmp(str, "INSUFFICIENT_FUNDS") == 0) return NEOC_ORACLERESPONSECODE_INSUFFICIENT_FUNDS;
    if (strcmp(str, "ERROR") == 0) return NEOC_ORACLERESPONSECODE_ERROR;
    return -1;
}
