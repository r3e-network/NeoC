/**
 * @file account_signer.h
 * @brief Account signer for NEO transactions
 */

#ifndef NEOC_ACCOUNT_SIGNER_H
#define NEOC_ACCOUNT_SIGNER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "neoc/neoc_error.h"
#include "neoc/transaction/signer.h"
#include "neoc/transaction/witness_scope.h"
#include "neoc/wallet/account.h"
#include "neoc/types/neoc_hash160.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Account signer structure
 */
typedef struct neoc_account_signer {
    neoc_signer_t base;           // Base signer
    neoc_account_t *account;      // The account of this signer
    bool owns_account;            // Whether this signer owns the account
} neoc_account_signer_t;

/**
 * Create an account signer with no witness scope
 * The signature is only used for transactions and is disabled in contracts
 * @param account The signer account
 * @param signer Output account signer
 * @return Error code
 */
neoc_error_t neoc_account_signer_none(neoc_account_t *account,
                                       neoc_account_signer_t **signer);

/**
 * Create an account signer with no witness scope using hash
 * @param account_hash The script hash of the signer account
 * @param signer Output account signer
 * @return Error code
 */
neoc_error_t neoc_account_signer_none_hash(neoc_hash160_t *account_hash,
                                            neoc_account_signer_t **signer);

/**
 * Create an account signer with called-by-entry scope
 * Only allows the entry point contract to use this signer's witness
 * @param account The signer account
 * @param signer Output account signer
 * @return Error code
 */
neoc_error_t neoc_account_signer_called_by_entry(neoc_account_t *account,
                                                  neoc_account_signer_t **signer);

/**
 * Create an account signer with called-by-entry scope using hash
 * @param account_hash The script hash of the signer account
 * @param signer Output account signer
 * @return Error code
 */
neoc_error_t neoc_account_signer_called_by_entry_hash(neoc_hash160_t *account_hash,
                                                       neoc_account_signer_t **signer);

/**
 * Create an account signer with global witness scope
 * @param account The signer account
 * @param signer Output account signer
 * @return Error code
 */
neoc_error_t neoc_account_signer_global(neoc_account_t *account,
                                         neoc_account_signer_t **signer);

/**
 * Create an account signer with global witness scope using hash
 * @param account_hash The script hash of the signer account
 * @param signer Output account signer
 * @return Error code
 */
neoc_error_t neoc_account_signer_global_hash(neoc_hash160_t *account_hash,
                                              neoc_account_signer_t **signer);

/**
 * Get the account from an account signer
 * @param signer The account signer
 * @return The account
 */
neoc_account_t* neoc_account_signer_get_account(neoc_account_signer_t *signer);

/**
 * Free an account signer
 * @param signer The signer to free
 */
void neoc_account_signer_free(neoc_account_signer_t *signer);

#ifdef __cplusplus
}
#endif

#endif // NEOC_ACCOUNT_SIGNER_H
