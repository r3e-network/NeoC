/**
 * @file role_management.h
 * @brief NEO Role Management contract implementation
 */

#ifndef NEOC_ROLE_MANAGEMENT_H
#define NEOC_ROLE_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/crypto/ecpoint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct neoc_role_management neoc_role_management_t;

/**
 * @brief Role enumeration based on Neo protocol
 * 
 * Roles that can be designated in the Neo network.
 */
typedef enum {
    NEOC_ROLE_STATE_VALIDATOR = 0x04,    ///< State validator role
    NEOC_ROLE_ORACLE = 0x08,             ///< Oracle role
    NEOC_ROLE_NEOFS_ALPHABET_NODE = 0x10 ///< NeoFS alphabet node role
} neoc_role_t;

/**
 * @brief Create a new role management contract interface
 * 
 * @param role_mgmt Pointer to store the created role management instance
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_role_management_create(neoc_role_management_t **role_mgmt);

/**
 * @brief Get designated nodes by role
 * 
 * @param role_mgmt Role management contract instance
 * @param role The role to query
 * @param index Block index for the query
 * @param nodes Output array of EC points (caller must free)
 * @param count Output number of nodes
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_role_management_get_designated_by_role(neoc_role_management_t *role_mgmt,
                                                          neoc_role_t role,
                                                          uint32_t index,
                                                          neoc_ec_point_t ***nodes,
                                                          size_t *count);

/**
 * @brief Designate nodes as a role
 * 
 * @param role_mgmt Role management contract instance
 * @param role The role to designate
 * @param nodes Array of EC points to designate
 * @param count Number of nodes
 * @return NEOC_SUCCESS on success, error code on failure
 */
neoc_error_t neoc_role_management_designate_as_role(neoc_role_management_t *role_mgmt,
                                                     neoc_role_t role,
                                                     neoc_ec_point_t **nodes,
                                                     size_t count);

/**
 * @brief Free role management contract instance
 * 
 * @param role_mgmt Role management instance to free
 */
void neoc_role_management_free(neoc_role_management_t *role_mgmt);

#ifdef __cplusplus
}
#endif

#endif // NEOC_ROLE_MANAGEMENT_H
