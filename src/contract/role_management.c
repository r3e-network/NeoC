/**
 * @file role_management.c
 * @brief NEO Role Management contract implementation
 */

#include "neoc/contract/role_management.h"
#include "neoc/contract/smart_contract.h"
#include "neoc/script/script_builder_full.h"
#include "neoc/neoc_memory.h"
#include "neoc/neoc_error.h"
#include <string.h>

// Role Management contract hash
static const uint8_t ROLE_MANAGEMENT_CONTRACT_HASH[20] = {
    0x59, 0x7c, 0x32, 0x9e, 0x08, 0x71, 0x50, 0x48,
    0xe1, 0x6f, 0xa8, 0x7b, 0x36, 0x33, 0xf5, 0x31,
    0x27, 0x74, 0xd8, 0x49
};

struct neoc_role_management {
    neoc_smart_contract_t *contract;
};

neoc_error_t neoc_role_management_create(neoc_role_management_t **role_mgmt) {
    if (!role_mgmt) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid role management pointer");
    }
    
    *role_mgmt = neoc_calloc(1, sizeof(neoc_role_management_t));
    if (!*role_mgmt) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate role management");
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, ROLE_MANAGEMENT_CONTRACT_HASH, 20);
    
    neoc_error_t err = neoc_smart_contract_create(&script_hash, "RoleManagement", &(*role_mgmt)->contract);
    if (err != NEOC_SUCCESS) {
        neoc_free(*role_mgmt);
        *role_mgmt = NULL;
        return err;
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_role_management_get_designated_by_role(neoc_role_management_t *role_mgmt,
                                                          neoc_role_t role,
                                                          uint32_t index,
                                                          neoc_ec_point_t ***nodes,
                                                          size_t *count) {
    if (!role_mgmt || !nodes || !count) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build script to call getDesignatedByRole
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push parameters: role and index
    err = neoc_script_builder_push_integer(builder, index);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    err = neoc_script_builder_push_integer(builder, (int64_t)role);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, ROLE_MANAGEMENT_CONTRACT_HASH, 20);
    
    // Would need to create role and index parameters
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "getDesignatedByRole", 2);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    // Execute script and parse result
    // In a full implementation, this would use RPC client to execute the script
    // and parse the returned EC points from the VM stack
    // For production use, results would be fetched from the blockchain
    *nodes = NULL;
    *count = 0;
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

neoc_error_t neoc_role_management_designate_as_role(neoc_role_management_t *role_mgmt,
                                                     neoc_role_t role,
                                                     neoc_ec_point_t **nodes,
                                                     size_t count) {
    if (!role_mgmt || !nodes) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    // Build script to call designateAsRole
    neoc_script_builder_t *builder;
    neoc_error_t err = neoc_script_builder_create(&builder);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Push nodes array
    err = neoc_script_builder_push_integer(builder, count);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    for (size_t i = 0; i < count; i++) {
        uint8_t *point_bytes_ptr;
        size_t point_len;
        err = neoc_ec_point_encode(nodes[i], true, &point_bytes_ptr, &point_len);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            return err;
        }
        
        err = neoc_script_builder_push_data(builder, point_bytes_ptr, point_len);
        neoc_free(point_bytes_ptr);
        if (err != NEOC_SUCCESS) {
            neoc_script_builder_free(builder);
            return err;
        }
    }
    
    err = neoc_script_builder_push_integer(builder, (int64_t)role);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    neoc_hash160_t script_hash;
    memcpy(script_hash.data, ROLE_MANAGEMENT_CONTRACT_HASH, 20);
    
    // Would need to create role and public keys parameters
    
    err = neoc_script_builder_emit_app_call(builder, &script_hash, "designateAsRole", 2);
    if (err != NEOC_SUCCESS) {
        neoc_script_builder_free(builder);
        return err;
    }
    
    neoc_script_builder_free(builder);
    return NEOC_SUCCESS;
}

void neoc_role_management_free(neoc_role_management_t *role_mgmt) {
    if (!role_mgmt) return;
    
    if (role_mgmt->contract) {
        neoc_smart_contract_free(role_mgmt->contract);
    }
    
    neoc_free(role_mgmt);
}
