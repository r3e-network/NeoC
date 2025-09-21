/**
 * @file response_types.c
 * @brief Implementation of Neo RPC response types
 */

#include "../../include/neoc/protocol/response_types.h"
#include "../../include/neoc/neoc_memory.h"
#include <string.h>
#include <stdlib.h>

// Helper function to free string array
static void free_string_array(char** array, size_t count) {
    if (!array) return;
    for (size_t i = 0; i < count; i++) {
        if (array[i]) {
            neoc_free(array[i]);
        }
    }
    neoc_free(array);
}

neoc_error_t neoc_get_version_response_create(neoc_get_version_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_version_response_t* resp = neoc_malloc(sizeof(neoc_get_version_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_version_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_version_response_free(neoc_get_version_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->user_agent) {
            neoc_free(response->result->user_agent);
        }
        if (response->result->protocol) {
            neoc_free(response->result->protocol);
        }
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_block_response_create(neoc_get_block_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_block_response_t* resp = neoc_malloc(sizeof(neoc_get_block_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_block_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_block_response_free(neoc_get_block_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->next_consensus) {
            neoc_free(response->result->next_consensus);
        }
        
        // Free witnesses array (would need witness implementation)
        if (response->result->witnesses) {
            neoc_free(response->result->witnesses);
        }
        
        // Free transactions array (would need transaction implementation)
        if (response->result->transactions) {
            neoc_free(response->result->transactions);
        }
        
        if (response->result->next_block_hash) {
            neoc_free(response->result->next_block_hash);
        }
        
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_nep17_balances_response_create(neoc_get_nep17_balances_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_nep17_balances_response_t* resp = neoc_malloc(sizeof(neoc_get_nep17_balances_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_nep17_balances_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_nep17_balances_response_free(neoc_get_nep17_balances_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->address) {
            neoc_free(response->result->address);
        }
        
        if (response->result->balances) {
            for (size_t i = 0; i < response->result->balance_count; i++) {
                neoc_nep17_balance_t* balance = response->result->balances[i];
                if (balance) {
                    if (balance->name) neoc_free(balance->name);
                    if (balance->symbol) neoc_free(balance->symbol);
                    if (balance->decimals) neoc_free(balance->decimals);
                    if (balance->amount) neoc_free(balance->amount);
                    neoc_free(balance);
                }
            }
            neoc_free(response->result->balances);
        }
        
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_nep17_transfers_response_create(neoc_get_nep17_transfers_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_nep17_transfers_response_t* resp = neoc_malloc(sizeof(neoc_get_nep17_transfers_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_nep17_transfers_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_nep17_transfers_response_free(neoc_get_nep17_transfers_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->address) {
            neoc_free(response->result->address);
        }
        
        // Free sent transfers
        if (response->result->sent) {
            for (size_t i = 0; i < response->result->sent_count; i++) {
                neoc_nep17_transfer_t* transfer = response->result->sent[i];
                if (transfer) {
                    if (transfer->transfer_address) neoc_free(transfer->transfer_address);
                    if (transfer->amount) neoc_free(transfer->amount);
                    neoc_free(transfer);
                }
            }
            neoc_free(response->result->sent);
        }
        
        // Free received transfers
        if (response->result->received) {
            for (size_t i = 0; i < response->result->received_count; i++) {
                neoc_nep17_transfer_t* transfer = response->result->received[i];
                if (transfer) {
                    if (transfer->transfer_address) neoc_free(transfer->transfer_address);
                    if (transfer->amount) neoc_free(transfer->amount);
                    neoc_free(transfer);
                }
            }
            neoc_free(response->result->received);
        }
        
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_peers_response_create(neoc_get_peers_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_peers_response_t* resp = neoc_malloc(sizeof(neoc_get_peers_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_peers_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_peers_response_free(neoc_get_peers_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        // Free unconnected peers
        if (response->result->unconnected) {
            for (size_t i = 0; i < response->result->unconnected_count; i++) {
                neoc_peer_t* peer = response->result->unconnected[i];
                if (peer) {
                    if (peer->address) neoc_free(peer->address);
                    neoc_free(peer);
                }
            }
            neoc_free(response->result->unconnected);
        }
        
        // Free bad peers
        if (response->result->bad) {
            for (size_t i = 0; i < response->result->bad_count; i++) {
                neoc_peer_t* peer = response->result->bad[i];
                if (peer) {
                    if (peer->address) neoc_free(peer->address);
                    neoc_free(peer);
                }
            }
            neoc_free(response->result->bad);
        }
        
        // Free connected peers
        if (response->result->connected) {
            for (size_t i = 0; i < response->result->connected_count; i++) {
                neoc_peer_t* peer = response->result->connected[i];
                if (peer) {
                    if (peer->address) neoc_free(peer->address);
                    neoc_free(peer);
                }
            }
            neoc_free(response->result->connected);
        }
        
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_mempool_response_create(neoc_get_mempool_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_mempool_response_t* resp = neoc_malloc(sizeof(neoc_get_mempool_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_mempool_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_mempool_response_free(neoc_get_mempool_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_contract_state_response_create(neoc_get_contract_state_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_contract_state_response_t* resp = neoc_malloc(sizeof(neoc_get_contract_state_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_contract_state_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_contract_state_response_free(neoc_get_contract_state_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->hash) neoc_free(response->result->hash);
        if (response->result->nef) neoc_free(response->result->nef);
        if (response->result->manifest) neoc_free(response->result->manifest);
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_app_log_response_create(neoc_get_app_log_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_app_log_response_t* resp = neoc_malloc(sizeof(neoc_get_app_log_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_app_log_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_app_log_response_free(neoc_get_app_log_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->trigger) neoc_free(response->result->trigger);
        if (response->result->vm_state) neoc_free(response->result->vm_state);
        if (response->result->exception) neoc_free(response->result->exception);
        
        free_string_array(response->result->stack, response->result->stack_count);
        free_string_array(response->result->notifications, response->result->notification_count);
        
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_unclaimed_gas_response_create(neoc_get_unclaimed_gas_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_unclaimed_gas_response_t* resp = neoc_malloc(sizeof(neoc_get_unclaimed_gas_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_unclaimed_gas_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_unclaimed_gas_response_free(neoc_get_unclaimed_gas_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        if (response->result->unclaimed) neoc_free(response->result->unclaimed);
        if (response->result->address) neoc_free(response->result->address);
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_next_block_validators_response_create(neoc_get_next_block_validators_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_next_block_validators_response_t* resp = neoc_malloc(sizeof(neoc_get_next_block_validators_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_next_block_validators_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_next_block_validators_response_free(neoc_get_next_block_validators_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        free_string_array(response->result->validators, response->result->validator_count);
        neoc_free(response->result);
    }
    
    neoc_free(response);
}

neoc_error_t neoc_get_committee_response_create(neoc_get_committee_response_t** response) {
    if (!response) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_get_committee_response_t* resp = neoc_malloc(sizeof(neoc_get_committee_response_t));
    if (!resp) {
        return NEOC_ERROR_MEMORY;
    }
    
    memset(resp, 0, sizeof(neoc_get_committee_response_t));
    *response = resp;
    return NEOC_SUCCESS;
}

void neoc_get_committee_response_free(neoc_get_committee_response_t* response) {
    if (!response) return;
    
    // Free base fields
    if (response->base.jsonrpc) {
        neoc_free(response->base.jsonrpc);
    }
    if (response->base.error) {
        neoc_free(response->base.error);
    }
    
    // Free result
    if (response->result) {
        free_string_array(response->result->members, response->result->member_count);
        neoc_free(response->result);
    }
    
    neoc_free(response);
}
