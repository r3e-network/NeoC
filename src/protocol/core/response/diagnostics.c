#include "neoc/protocol/core/response/diagnostics.h"

static neoc_error_t neoc_diag_stub(const char *fn) {
    return neoc_error_set(NEOC_ERROR_NOT_IMPLEMENTED,
                          fn ? fn : "Diagnostics functionality not implemented");
}

neoc_error_t neoc_invoked_contract_create(
    const neoc_hash160_t *hash,
    const neoc_invoked_contract_t *invoked_contracts,
    size_t invoked_contracts_count,
    neoc_invoked_contract_t **contract) {
    (void)hash;
    (void)invoked_contracts;
    (void)invoked_contracts_count;
    if (contract) {
        *contract = NULL;
    }
    return neoc_diag_stub("neoc_invoked_contract_create");
}

void neoc_invoked_contract_free(
    neoc_invoked_contract_t *contract) {
    (void)contract;
}

neoc_error_t neoc_invoked_contract_copy(
    const neoc_invoked_contract_t *src,
    neoc_invoked_contract_t **dest) {
    (void)src;
    if (dest) {
        *dest = NULL;
    }
    return neoc_diag_stub("neoc_invoked_contract_copy");
}

neoc_error_t neoc_storage_change_create(
    const char *state,
    const char *key,
    const char *value,
    neoc_storage_change_t **change) {
    (void)state;
    (void)key;
    (void)value;
    if (change) {
        *change = NULL;
    }
    return neoc_diag_stub("neoc_storage_change_create");
}

void neoc_storage_change_free(
    neoc_storage_change_t *change) {
    (void)change;
}

neoc_error_t neoc_storage_change_copy(
    const neoc_storage_change_t *src,
    neoc_storage_change_t **dest) {
    (void)src;
    if (dest) {
        *dest = NULL;
    }
    return neoc_diag_stub("neoc_storage_change_copy");
}

neoc_error_t neoc_diagnostics_create(
    const neoc_invoked_contract_t *invoked_contracts,
    const neoc_storage_change_t *storage_changes,
    size_t storage_changes_count,
    neoc_diagnostics_t **diagnostics) {
    (void)invoked_contracts;
    (void)storage_changes;
    (void)storage_changes_count;
    if (diagnostics) {
        *diagnostics = NULL;
    }
    return neoc_diag_stub("neoc_diagnostics_create");
}

void neoc_diagnostics_free(
    neoc_diagnostics_t *diagnostics) {
    (void)diagnostics;
}

neoc_error_t neoc_diagnostics_from_json(
    const char *json_str,
    neoc_diagnostics_t **diagnostics) {
    (void)json_str;
    if (diagnostics) {
        *diagnostics = NULL;
    }
    return neoc_diag_stub("neoc_diagnostics_from_json");
}

neoc_error_t neoc_diagnostics_to_json(
    const neoc_diagnostics_t *diagnostics,
    char **json_str) {
    (void)diagnostics;
    if (json_str) {
        *json_str = NULL;
    }
    return neoc_diag_stub("neoc_diagnostics_to_json");
}

size_t neoc_invoked_contract_get_total_count(
    const neoc_invoked_contract_t *contract) {
    (void)contract;
    return 0;
}

bool neoc_invoked_contract_was_invoked(
    const neoc_invoked_contract_t *contract,
    const neoc_hash160_t *hash) {
    (void)contract;
    (void)hash;
    return false;
}
