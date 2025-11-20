/**
 * @file neoc.c
 * @brief Main implementation file for NeoC SDK
 */

#include "neoc/neoc.h"
#include "neoc/crypto/neoc_hash.h"
#include <string.h>

/* Version information */
#define NEOC_VERSION_MAJOR 1
#define NEOC_VERSION_MINOR 1
#define NEOC_VERSION_PATCH 0

/* Build information */
static const char* version_string = "1.1.0";
static const char* build_info = "NeoC SDK v1.1.0 - Built with OpenSSL";

/* Global state tracking */
static int neoc_is_initialized = 0;

neoc_error_t neoc_init(void) {
    if (neoc_is_initialized) {
        return NEOC_SUCCESS;
    }
    
    /* Initialize crypto subsystem */
    neoc_error_t result = neoc_crypto_init();
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    neoc_is_initialized = 1;
    return NEOC_SUCCESS;
}

void neoc_cleanup(void) {
    if (!neoc_is_initialized) {
        return;
    }
    
    /* Cleanup crypto subsystem */
    neoc_crypto_cleanup();
    
    neoc_is_initialized = 0;
}

const char* neoc_get_version(void) {
    return version_string;
}

const char* neoc_get_build_info(void) {
    return build_info;
}
