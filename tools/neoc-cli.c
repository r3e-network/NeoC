/**
 * @file neoc-cli.c
 * @brief NeoC SDK Command Line Interface Tool
 * 
 * A comprehensive CLI tool for interacting with Neo blockchain using NeoC SDK
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <neoc/neoc.h>
#include <neoc/wallet/wallet.h>
#include <neoc/wallet/account.h>
#include <neoc/wallet/nep6_wallet.h>
#include <neoc/transaction/transaction_builder.h>
#include <neoc/contract/neo_token.h>
#include <neoc/contract/gas_token.h>
#include <neoc/rpc/rpc_client.h>
#include <neoc/crypto/keys.h>
#include <neoc/types/uint256.h>

#define VERSION "1.0.0"
#define DEFAULT_RPC_URL "http://localhost:10332"

// Command structure
typedef struct {
    const char *name;
    const char *description;
    int (*handler)(int argc, char *argv[]);
} command_t;

// Global options
static struct {
    char *rpc_url;
    char *wallet_path;
    char *password;
    int verbose;
    int testnet;
} g_options = {
    .rpc_url = NULL,
    .wallet_path = NULL,
    .password = NULL,
    .verbose = 0,
    .testnet = 0
};

/**
 * Print usage information
 */
void print_usage(const char *program_name) {
    printf("NeoC SDK CLI Tool v%s\n", VERSION);
    printf("Usage: %s [global-options] <command> [command-options]\n\n", program_name);
    
    printf("Global Options:\n");
    printf("  -r, --rpc <url>      Neo RPC node URL (default: %s)\n", DEFAULT_RPC_URL);
    printf("  -w, --wallet <path>  Wallet file path\n");
    printf("  -p, --password <pw>  Wallet password\n");
    printf("  -t, --testnet        Use testnet\n");
    printf("  -v, --verbose        Verbose output\n");
    printf("  -h, --help           Show this help message\n");
    printf("  --version            Show version information\n\n");
    
    printf("Commands:\n");
    printf("  wallet               Wallet management commands\n");
    printf("  account              Account management commands\n");
    printf("  transfer             Transfer tokens\n");
    printf("  balance              Check account balance\n");
    printf("  transaction          Transaction operations\n");
    printf("  contract             Smart contract interactions\n");
    printf("  blockchain           Blockchain queries\n");
    printf("  convert              Conversion utilities\n\n");
    
    printf("Examples:\n");
    printf("  %s wallet create --name MyWallet --path wallet.json\n", program_name);
    printf("  %s account create --wallet wallet.json\n", program_name);
    printf("  %s balance --address NXV7ZhHiyM1aHXwpVsRZC6BwNFP2jghXAq\n", program_name);
    printf("  %s transfer --from <addr> --to <addr> --amount 10 --asset NEO\n", program_name);
}

/**
 * Wallet command handler
 */
int cmd_wallet(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: neoc-cli wallet <action> [options]\n");
        printf("Actions:\n");
        printf("  create     Create a new wallet\n");
        printf("  open       Open existing wallet\n");
        printf("  list       List accounts in wallet\n");
        printf("  export     Export wallet to NEP-6\n");
        printf("  import     Import NEP-6 wallet\n");
        return 1;
    }
    
    const char *action = argv[1];
    neoc_error_t err;
    
    if (strcmp(action, "create") == 0) {
        // Parse wallet create options
        char *wallet_name = "MyWallet";
        char *output_path = "wallet.json";
        char *password = NULL;
        
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
                wallet_name = argv[++i];
            } else if (strcmp(argv[i], "--path") == 0 && i + 1 < argc) {
                output_path = argv[++i];
            } else if (strcmp(argv[i], "--password") == 0 && i + 1 < argc) {
                password = argv[++i];
            }
        }
        
        // Create wallet
        neoc_wallet_t *wallet = NULL;
        err = neoc_wallet_create(wallet_name, &wallet);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to create wallet: %s\n", neoc_error_string(err));
            return 1;
        }
        
        printf("✓ Wallet '%s' created\n", wallet_name);
        
        // Create a default account
        neoc_account_t *account = NULL;
        err = neoc_account_create(&account);
        if (err == NEOC_SUCCESS) {
            err = neoc_wallet_add_account(wallet, account);
            if (err == NEOC_SUCCESS) {
                printf("✓ Default account created: %s\n", neoc_account_get_address(account));
            }
        }
        
        // Export to NEP-6 if password provided
        if (password != NULL) {
            err = neoc_nep6_wallet_export(wallet, output_path, password);
            if (err == NEOC_SUCCESS) {
                printf("✓ Wallet exported to: %s\n", output_path);
            } else {
                fprintf(stderr, "Failed to export wallet: %s\n", neoc_error_string(err));
            }
        }
        
        neoc_wallet_free(wallet);
        
    } else if (strcmp(action, "open") == 0 || strcmp(action, "list") == 0) {
        // Open wallet and list accounts
        if (g_options.wallet_path == NULL) {
            fprintf(stderr, "Error: Wallet path required (use -w option)\n");
            return 1;
        }
        
        if (g_options.password == NULL) {
            fprintf(stderr, "Error: Password required (use -p option)\n");
            return 1;
        }
        
        neoc_wallet_t *wallet = NULL;
        err = neoc_nep6_wallet_import(g_options.wallet_path, g_options.password, &wallet);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to open wallet: %s\n", neoc_error_string(err));
            return 1;
        }
        
        printf("Wallet: %s\n", neoc_wallet_get_name(wallet));
        printf("Accounts:\n");
        
        size_t count = neoc_wallet_get_account_count(wallet);
        for (size_t i = 0; i < count; i++) {
            neoc_account_t *account = neoc_wallet_get_account(wallet, i);
            if (account != NULL) {
                const char *address = neoc_account_get_address(account);
                bool is_default = neoc_wallet_is_default_account(wallet, account);
                printf("  %zu. %s%s\n", i + 1, address, is_default ? " (default)" : "");
            }
        }
        
        neoc_wallet_free(wallet);
        
    } else {
        fprintf(stderr, "Unknown wallet action: %s\n", action);
        return 1;
    }
    
    return 0;
}

/**
 * Account command handler
 */
int cmd_account(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: neoc-cli account <action> [options]\n");
        printf("Actions:\n");
        printf("  create     Create a new account\n");
        printf("  import     Import account from WIF/NEP-2\n");
        printf("  export     Export account to WIF/NEP-2\n");
        printf("  multisig   Create multi-signature account\n");
        return 1;
    }
    
    const char *action = argv[1];
    neoc_error_t err;
    
    if (strcmp(action, "create") == 0) {
        // Create new account
        neoc_account_t *account = NULL;
        err = neoc_account_create(&account);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to create account: %s\n", neoc_error_string(err));
            return 1;
        }
        
        printf("✓ Account created\n");
        printf("  Address: %s\n", neoc_account_get_address(account));
        
        // Export WIF if requested
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--export-wif") == 0) {
                char wif[256];
                err = neoc_account_export_wif(account, wif, sizeof(wif));
                if (err == NEOC_SUCCESS) {
                    printf("  Private Key (WIF): %s\n", wif);
                    printf("  ⚠️  Keep this private key secure!\n");
                }
                break;
            }
        }
        
        neoc_account_free(account);
        
    } else if (strcmp(action, "import") == 0) {
        // Import account from WIF
        char *wif = NULL;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--wif") == 0 && i + 1 < argc) {
                wif = argv[++i];
                break;
            }
        }
        
        if (wif == NULL) {
            fprintf(stderr, "Error: WIF required (--wif <key>)\n");
            return 1;
        }
        
        neoc_account_t *account = NULL;
        err = neoc_account_from_wif(wif, &account);
        if (err != NEOC_SUCCESS) {
            fprintf(stderr, "Failed to import account: %s\n", neoc_error_string(err));
            return 1;
        }
        
        printf("✓ Account imported\n");
        printf("  Address: %s\n", neoc_account_get_address(account));
        
        neoc_account_free(account);
        
    } else {
        fprintf(stderr, "Unknown account action: %s\n", action);
        return 1;
    }
    
    return 0;
}

/**
 * Balance command handler
 */
int cmd_balance(int argc, char *argv[]) {
    char *address = NULL;
    
    // Parse options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--address") == 0 && i + 1 < argc) {
            address = argv[++i];
        }
    }
    
    if (address == NULL) {
        fprintf(stderr, "Error: Address required (--address <addr>)\n");
        return 1;
    }
    
    // Connect to RPC node
    const char *rpc_url = g_options.rpc_url ? g_options.rpc_url : DEFAULT_RPC_URL;
    neoc_rpc_client_t *client = NULL;
    neoc_error_t err = neoc_rpc_client_create(rpc_url, &client);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to connect to RPC node: %s\n", neoc_error_string(err));
        return 1;
    }
    
    printf("Checking balance for: %s\n", address);
    printf("RPC Node: %s\n\n", rpc_url);
    
    // Convert address to hash160
    neoc_hash160_t *account_hash = NULL;
    err = neoc_address_to_script_hash(address, &account_hash);
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Invalid address format\n");
        neoc_rpc_client_free(client);
        return 1;
    }
    
    // Check NEO balance
    neoc_smart_contract_t *neo_token = NULL;
    err = neoc_neo_token_create(&neo_token);
    if (err == NEOC_SUCCESS) {
        int64_t neo_balance = 0;
        err = neoc_rpc_invoke_function(
            client,
            neoc_smart_contract_get_script_hash(neo_token),
            "balanceOf",
            account_hash,
            &neo_balance
        );
        
        if (err == NEOC_SUCCESS) {
            printf("NEO Balance: %lld\n", (long long)neo_balance);
        } else {
            printf("NEO Balance: Unable to fetch\n");
        }
        
        neoc_smart_contract_free(neo_token);
    }
    
    // Check GAS balance
    neoc_smart_contract_t *gas_token = NULL;
    err = neoc_gas_token_create(&gas_token);
    if (err == NEOC_SUCCESS) {
        int64_t gas_balance = 0;
        err = neoc_rpc_invoke_function(
            client,
            neoc_smart_contract_get_script_hash(gas_token),
            "balanceOf",
            account_hash,
            &gas_balance
        );
        
        if (err == NEOC_SUCCESS) {
            printf("GAS Balance: %.8f\n", gas_balance / 100000000.0);
        } else {
            printf("GAS Balance: Unable to fetch\n");
        }
        
        neoc_smart_contract_free(gas_token);
    }
    
    // Clean up
    neoc_hash160_free(account_hash);
    neoc_rpc_client_free(client);
    
    return 0;
}

/**
 * Convert command handler
 */
int cmd_convert(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: neoc-cli convert <type> <value>\n");
        printf("Types:\n");
        printf("  address-to-scripthash    Convert address to script hash\n");
        printf("  scripthash-to-address    Convert script hash to address\n");
        printf("  hex-to-base64           Convert hex to base64\n");
        printf("  base64-to-hex           Convert base64 to hex\n");
        printf("  wif-to-address          Convert WIF to address\n");
        return 1;
    }
    
    const char *type = argv[1];
    if (argc < 3) {
        fprintf(stderr, "Error: Value required\n");
        return 1;
    }
    
    const char *value = argv[2];
    
    if (strcmp(type, "address-to-scripthash") == 0) {
        neoc_hash160_t *hash = NULL;
        neoc_error_t err = neoc_address_to_script_hash(value, &hash);
        if (err == NEOC_SUCCESS) {
            char hex[41];
            neoc_hash160_to_string(hash, hex, sizeof(hex));
            printf("Script Hash: %s\n", hex);
            neoc_hash160_free(hash);
        } else {
            fprintf(stderr, "Invalid address\n");
            return 1;
        }
        
    } else if (strcmp(type, "wif-to-address") == 0) {
        neoc_account_t *account = NULL;
        neoc_error_t err = neoc_account_from_wif(value, &account);
        if (err == NEOC_SUCCESS) {
            printf("Address: %s\n", neoc_account_get_address(account));
            neoc_account_free(account);
        } else {
            fprintf(stderr, "Invalid WIF\n");
            return 1;
        }
        
    } else {
        fprintf(stderr, "Unknown conversion type: %s\n", type);
        return 1;
    }
    
    return 0;
}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    // Parse global options
    static struct option long_options[] = {
        {"rpc", required_argument, 0, 'r'},
        {"wallet", required_argument, 0, 'w'},
        {"password", required_argument, 0, 'p'},
        {"testnet", no_argument, 0, 't'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 0},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "r:w:p:tvh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'r':
                g_options.rpc_url = optarg;
                break;
            case 'w':
                g_options.wallet_path = optarg;
                break;
            case 'p':
                g_options.password = optarg;
                break;
            case 't':
                g_options.testnet = 1;
                break;
            case 'v':
                g_options.verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 0:
                if (strcmp(long_options[option_index].name, "version") == 0) {
                    printf("NeoC CLI version %s\n", VERSION);
                    return 0;
                }
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Check for command
    if (optind >= argc) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[optind];
    
    // Initialize NeoC SDK
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC SDK: %s\n", neoc_error_string(err));
        return 1;
    }
    
    // Route to command handler
    int result = 1;
    if (strcmp(command, "wallet") == 0) {
        result = cmd_wallet(argc - optind, argv + optind);
    } else if (strcmp(command, "account") == 0) {
        result = cmd_account(argc - optind, argv + optind);
    } else if (strcmp(command, "balance") == 0) {
        result = cmd_balance(argc - optind, argv + optind);
    } else if (strcmp(command, "convert") == 0) {
        result = cmd_convert(argc - optind, argv + optind);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage(argv[0]);
    }
    
    // Clean up
    neoc_cleanup();
    
    return result;
}