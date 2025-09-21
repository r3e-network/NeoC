/**
 * @file neoc-debug.c
 * @brief NeoC SDK Debug Utility
 * 
 * Advanced debugging and diagnostic tool for NeoC SDK
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <neoc/neoc.h>
#include <neoc/wallet/wallet.h>
#include <neoc/transaction/transaction.h>
#include <neoc/script/script.h>
#include <neoc/types/uint256.h>

#define DEBUG_VERSION "1.0.0"
#define MAX_BACKTRACE_SIZE 100
#define LOG_BUFFER_SIZE 4096

// Debug levels
typedef enum {
    DEBUG_LEVEL_ERROR = 0,
    DEBUG_LEVEL_WARNING,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_DEBUG,
    DEBUG_LEVEL_TRACE
} debug_level_t;

// Debug modes
typedef enum {
    DEBUG_MODE_NONE = 0,
    DEBUG_MODE_TRANSACTION = 1 << 0,
    DEBUG_MODE_WALLET = 1 << 1,
    DEBUG_MODE_CRYPTO = 1 << 2,
    DEBUG_MODE_SCRIPT = 1 << 3,
    DEBUG_MODE_MEMORY = 1 << 4,
    DEBUG_MODE_NETWORK = 1 << 5,
    DEBUG_MODE_ALL = 0xFFFF
} debug_mode_t;

// Global debug state
typedef struct {
    debug_level_t level;
    debug_mode_t mode;
    FILE *log_file;
    int enable_backtrace;
    int enable_memory_tracking;
    int enable_hex_dump;
    int enable_timestamps;
    char log_buffer[LOG_BUFFER_SIZE];
} debug_state_t;

static debug_state_t g_debug = {
    .level = DEBUG_LEVEL_INFO,
    .mode = DEBUG_MODE_ALL,
    .log_file = NULL,
    .enable_backtrace = 1,
    .enable_memory_tracking = 0,
    .enable_hex_dump = 0,
    .enable_timestamps = 1
};

// Memory tracking
typedef struct memory_allocation {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct memory_allocation *next;
} memory_allocation_t;

static memory_allocation_t *g_allocations = NULL;
static size_t g_total_allocated = 0;
static size_t g_peak_allocated = 0;

/**
 * Get current timestamp string
 */
static const char *get_timestamp() {
    static char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);
    return timestamp;
}

/**
 * Get debug level string
 */
static const char *level_to_string(debug_level_t level) {
    switch (level) {
        case DEBUG_LEVEL_ERROR: return "ERROR";
        case DEBUG_LEVEL_WARNING: return "WARN";
        case DEBUG_LEVEL_INFO: return "INFO";
        case DEBUG_LEVEL_DEBUG: return "DEBUG";
        case DEBUG_LEVEL_TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}

/**
 * Log debug message
 */
static void debug_log(debug_level_t level, debug_mode_t mode, const char *format, ...) {
    if (level > g_debug.level) return;
    if (!(mode & g_debug.mode)) return;
    
    FILE *output = g_debug.log_file ? g_debug.log_file : stderr;
    
    if (g_debug.enable_timestamps) {
        fprintf(output, "[%s] ", get_timestamp());
    }
    
    fprintf(output, "[%s] ", level_to_string(level));
    
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
    
    fprintf(output, "\n");
    fflush(output);
}

/**
 * Print backtrace
 */
static void print_backtrace() {
    if (!g_debug.enable_backtrace) return;
    
    void *array[MAX_BACKTRACE_SIZE];
    size_t size;
    char **strings;
    
    size = backtrace(array, MAX_BACKTRACE_SIZE);
    strings = backtrace_symbols(array, size);
    
    debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_ALL, "Backtrace (%zu frames):", size);
    
    for (size_t i = 0; i < size; i++) {
        debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_ALL, "  %s", strings[i]);
    }
    
    free(strings);
}

/**
 * Signal handler for crashes
 */
static void signal_handler(int sig) {
    debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_ALL, "Caught signal %d (%s)", sig, strsignal(sig));
    print_backtrace();
    
    // Dump memory allocations if tracking
    if (g_debug.enable_memory_tracking) {
        debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_MEMORY, 
                 "Memory allocated: %zu bytes (peak: %zu bytes)", 
                 g_total_allocated, g_peak_allocated);
        
        if (g_allocations) {
            debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_MEMORY, "Outstanding allocations:");
            memory_allocation_t *alloc = g_allocations;
            while (alloc) {
                debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_MEMORY, 
                         "  %p: %zu bytes (%s:%d)", 
                         alloc->ptr, alloc->size, alloc->file, alloc->line);
                alloc = alloc->next;
            }
        }
    }
    
    exit(sig);
}

/**
 * Hex dump utility
 */
static void hex_dump(const void *data, size_t size, const char *label) {
    if (!g_debug.enable_hex_dump) return;
    
    const uint8_t *bytes = (const uint8_t *)data;
    char ascii[17];
    size_t i, j;
    
    debug_log(DEBUG_LEVEL_DEBUG, DEBUG_MODE_ALL, "Hex dump: %s (%zu bytes)", label, size);
    
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            fprintf(stderr, "%08zx  ", i);
        }
        
        fprintf(stderr, "%02x ", bytes[i]);
        
        if (bytes[i] >= ' ' && bytes[i] <= '~') {
            ascii[i % 16] = bytes[i];
        } else {
            ascii[i % 16] = '.';
        }
        
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            fprintf(stderr, " ");
            if ((i + 1) % 16 == 0) {
                fprintf(stderr, "|%s|\n", ascii);
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                for (j = (i + 1) % 16; j < 16; ++j) {
                    fprintf(stderr, "   ");
                    if (j % 8 == 0) fprintf(stderr, " ");
                }
                fprintf(stderr, "|%-16s|\n", ascii);
            }
        }
    }
}

/**
 * Debug transaction
 */
static void debug_transaction(neoc_transaction_t *tx) {
    if (!tx) {
        debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_TRANSACTION, "Transaction is NULL");
        return;
    }
    
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "=== Transaction Debug Info ===");
    
    // Get transaction hash
    neoc_uint256_t *hash = neoc_transaction_get_hash(tx);
    char hash_str[65];
    neoc_uint256_to_string(hash, hash_str, sizeof(hash_str));
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Hash: %s", hash_str);
    
    // Get transaction details
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Version: %u", neoc_transaction_get_version(tx));
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Nonce: %u", neoc_transaction_get_nonce(tx));
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "System Fee: %lld", neoc_transaction_get_system_fee(tx));
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Network Fee: %lld", neoc_transaction_get_network_fee(tx));
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Valid Until: %u", neoc_transaction_get_valid_until_block(tx));
    
    // Get script
    size_t script_size;
    const uint8_t *script = neoc_transaction_get_script(tx, &script_size);
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Script Size: %zu bytes", script_size);
    
    if (g_debug.enable_hex_dump && script) {
        hex_dump(script, script_size > 256 ? 256 : script_size, "Transaction Script");
    }
    
    // Get witnesses
    size_t witness_count = neoc_transaction_get_witness_count(tx);
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Witnesses: %zu", witness_count);
    
    // Serialize and show size
    neoc_binary_writer_t *writer = NULL;
    if (neoc_binary_writer_create(&writer) == NEOC_SUCCESS) {
        if (neoc_transaction_serialize(tx, writer) == NEOC_SUCCESS) {
            size_t serialized_size = neoc_binary_writer_get_size(writer);
            debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_TRANSACTION, "Serialized Size: %zu bytes", serialized_size);
            
            if (g_debug.enable_hex_dump) {
                const uint8_t *data = neoc_binary_writer_get_data(writer);
                hex_dump(data, serialized_size > 256 ? 256 : serialized_size, "Serialized Transaction");
            }
        }
        neoc_binary_writer_free(writer);
    }
}

/**
 * Debug wallet
 */
static void debug_wallet(neoc_wallet_t *wallet) {
    if (!wallet) {
        debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_WALLET, "Wallet is NULL");
        return;
    }
    
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_WALLET, "=== Wallet Debug Info ===");
    
    const char *name = neoc_wallet_get_name(wallet);
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_WALLET, "Name: %s", name ? name : "(null)");
    
    size_t account_count = neoc_wallet_get_account_count(wallet);
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_WALLET, "Accounts: %zu", account_count);
    
    for (size_t i = 0; i < account_count; i++) {
        neoc_account_t *account = neoc_wallet_get_account(wallet, i);
        if (account) {
            const char *address = neoc_account_get_address(account);
            bool is_default = neoc_wallet_is_default_account(wallet, account);
            bool is_multisig = neoc_account_is_multisig(account);
            
            debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_WALLET, 
                     "  Account %zu: %s%s%s", 
                     i, address,
                     is_default ? " [DEFAULT]" : "",
                     is_multisig ? " [MULTISIG]" : "");
        }
    }
}

/**
 * Debug script
 */
static void debug_script(const uint8_t *script, size_t size) {
    if (!script) {
        debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_SCRIPT, "Script is NULL");
        return;
    }
    
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_SCRIPT, "=== Script Debug Info ===");
    debug_log(DEBUG_LEVEL_INFO, DEBUG_MODE_SCRIPT, "Size: %zu bytes", size);
    
    // Parse and display opcodes
    size_t offset = 0;
    while (offset < size) {
        uint8_t opcode = script[offset];
        const char *opname = neoc_opcode_to_string(opcode);
        
        debug_log(DEBUG_LEVEL_DEBUG, DEBUG_MODE_SCRIPT, 
                 "%04zu: 0x%02x %s", offset, opcode, opname);
        
        // Handle push operations
        if (opcode >= 0x01 && opcode <= 0x4B) {
            // PUSHBYTES1-75
            size_t push_size = opcode;
            offset++;
            if (offset + push_size <= size) {
                if (g_debug.enable_hex_dump) {
                    hex_dump(script + offset, push_size, "Push Data");
                }
                offset += push_size;
            } else {
                debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_SCRIPT, 
                         "Invalid push size at offset %zu", offset - 1);
                break;
            }
        } else if (opcode == 0x4C) {
            // PUSHDATA1
            offset++;
            if (offset < size) {
                size_t push_size = script[offset];
                offset++;
                if (offset + push_size <= size) {
                    if (g_debug.enable_hex_dump) {
                        hex_dump(script + offset, push_size, "Push Data");
                    }
                    offset += push_size;
                } else {
                    debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_SCRIPT, 
                             "Invalid PUSHDATA1 at offset %zu", offset - 2);
                    break;
                }
            }
        } else if (opcode == 0x4D) {
            // PUSHDATA2
            offset++;
            if (offset + 2 <= size) {
                size_t push_size = script[offset] | (script[offset + 1] << 8);
                offset += 2;
                if (offset + push_size <= size) {
                    if (g_debug.enable_hex_dump) {
                        hex_dump(script + offset, push_size > 256 ? 256 : push_size, "Push Data");
                    }
                    offset += push_size;
                } else {
                    debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_SCRIPT, 
                             "Invalid PUSHDATA2 at offset %zu", offset - 3);
                    break;
                }
            }
        } else if (opcode == 0x4E) {
            // PUSHDATA4
            offset++;
            if (offset + 4 <= size) {
                size_t push_size = script[offset] | 
                                  (script[offset + 1] << 8) |
                                  (script[offset + 2] << 16) |
                                  (script[offset + 3] << 24);
                offset += 4;
                if (offset + push_size <= size) {
                    if (g_debug.enable_hex_dump) {
                        hex_dump(script + offset, push_size > 256 ? 256 : push_size, "Push Data");
                    }
                    offset += push_size;
                } else {
                    debug_log(DEBUG_LEVEL_ERROR, DEBUG_MODE_SCRIPT, 
                             "Invalid PUSHDATA4 at offset %zu", offset - 5);
                    break;
                }
            }
        } else {
            offset++;
        }
    }
}

/**
 * Memory allocation wrapper
 */
void *debug_malloc(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    
    if (g_debug.enable_memory_tracking && ptr) {
        memory_allocation_t *alloc = malloc(sizeof(memory_allocation_t));
        alloc->ptr = ptr;
        alloc->size = size;
        alloc->file = file;
        alloc->line = line;
        alloc->next = g_allocations;
        g_allocations = alloc;
        
        g_total_allocated += size;
        if (g_total_allocated > g_peak_allocated) {
            g_peak_allocated = g_total_allocated;
        }
        
        debug_log(DEBUG_LEVEL_TRACE, DEBUG_MODE_MEMORY, 
                 "Allocated %zu bytes at %p (%s:%d)", 
                 size, ptr, file, line);
    }
    
    return ptr;
}

/**
 * Memory free wrapper
 */
void debug_free(void *ptr, const char *file, int line) {
    if (g_debug.enable_memory_tracking && ptr) {
        memory_allocation_t *prev = NULL;
        memory_allocation_t *alloc = g_allocations;
        
        while (alloc) {
            if (alloc->ptr == ptr) {
                if (prev) {
                    prev->next = alloc->next;
                } else {
                    g_allocations = alloc->next;
                }
                
                g_total_allocated -= alloc->size;
                
                debug_log(DEBUG_LEVEL_TRACE, DEBUG_MODE_MEMORY, 
                         "Freed %zu bytes at %p (%s:%d)", 
                         alloc->size, ptr, file, line);
                
                free(alloc);
                break;
            }
            prev = alloc;
            alloc = alloc->next;
        }
        
        if (!alloc) {
            debug_log(DEBUG_LEVEL_WARNING, DEBUG_MODE_MEMORY, 
                     "Freeing untracked pointer %p (%s:%d)", 
                     ptr, file, line);
        }
    }
    
    free(ptr);
}

/**
 * Run diagnostic tests
 */
static int run_diagnostics() {
    printf("Running NeoC SDK diagnostics...\n\n");
    
    int errors = 0;
    
    // Test initialization
    printf("Testing SDK initialization... ");
    neoc_error_t err = neoc_init();
    if (err == NEOC_SUCCESS || err == NEOC_ERROR_ALREADY_INITIALIZED) {
        printf("✓\n");
    } else {
        printf("✗ (%s)\n", neoc_error_string(err));
        errors++;
    }
    
    // Test account creation
    printf("Testing account creation... ");
    neoc_account_t *account = NULL;
    err = neoc_account_create(&account);
    if (err == NEOC_SUCCESS && account != NULL) {
        printf("✓\n");
        neoc_account_free(account);
    } else {
        printf("✗\n");
        errors++;
    }
    
    // Test wallet creation
    printf("Testing wallet creation... ");
    neoc_wallet_t *wallet = NULL;
    err = neoc_wallet_create("TestWallet", &wallet);
    if (err == NEOC_SUCCESS && wallet != NULL) {
        printf("✓\n");
        neoc_wallet_free(wallet);
    } else {
        printf("✗\n");
        errors++;
    }
    
    // Test transaction building
    printf("Testing transaction building... ");
    neoc_transaction_builder_t *builder = NULL;
    err = neoc_transaction_builder_create(&builder);
    if (err == NEOC_SUCCESS && builder != NULL) {
        printf("✓\n");
        neoc_transaction_builder_free(builder);
    } else {
        printf("✗\n");
        errors++;
    }
    
    // Test cryptographic operations
    printf("Testing cryptographic operations... ");
    uint8_t data[32] = {0};
    uint8_t hash[32];
    err = neoc_sha256(data, sizeof(data), hash);
    if (err == NEOC_SUCCESS) {
        printf("✓\n");
    } else {
        printf("✗\n");
        errors++;
    }
    
    printf("\nDiagnostics complete. ");
    if (errors == 0) {
        printf("All tests passed.\n");
    } else {
        printf("%d tests failed.\n", errors);
    }
    
    return errors;
}

/**
 * Print usage
 */
static void print_usage(const char *program) {
    printf("NeoC SDK Debug Utility v%s\n", DEBUG_VERSION);
    printf("Usage: %s [options] [command]\n", program);
    printf("\n");
    printf("Options:\n");
    printf("  -l, --level LEVEL    Set debug level (error|warning|info|debug|trace)\n");
    printf("  -m, --mode MODE      Set debug mode (transaction|wallet|crypto|script|memory|all)\n");
    printf("  -o, --output FILE    Output to file\n");
    printf("  -b, --backtrace      Enable backtrace on error\n");
    printf("  -M, --memory         Enable memory tracking\n");
    printf("  -x, --hexdump        Enable hex dumps\n");
    printf("  -t, --timestamps     Enable timestamps\n");
    printf("  -h, --help           Show this help message\n");
    printf("\n");
    printf("Commands:\n");
    printf("  diagnose             Run diagnostic tests\n");
    printf("  test-transaction     Test transaction debugging\n");
    printf("  test-wallet          Test wallet debugging\n");
    printf("  test-script          Test script debugging\n");
}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    const char *command = NULL;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--level") == 0) {
            if (i + 1 < argc) {
                const char *level = argv[++i];
                if (strcmp(level, "error") == 0) g_debug.level = DEBUG_LEVEL_ERROR;
                else if (strcmp(level, "warning") == 0) g_debug.level = DEBUG_LEVEL_WARNING;
                else if (strcmp(level, "info") == 0) g_debug.level = DEBUG_LEVEL_INFO;
                else if (strcmp(level, "debug") == 0) g_debug.level = DEBUG_LEVEL_DEBUG;
                else if (strcmp(level, "trace") == 0) g_debug.level = DEBUG_LEVEL_TRACE;
            }
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mode") == 0) {
            if (i + 1 < argc) {
                const char *mode = argv[++i];
                if (strcmp(mode, "transaction") == 0) g_debug.mode = DEBUG_MODE_TRANSACTION;
                else if (strcmp(mode, "wallet") == 0) g_debug.mode = DEBUG_MODE_WALLET;
                else if (strcmp(mode, "crypto") == 0) g_debug.mode = DEBUG_MODE_CRYPTO;
                else if (strcmp(mode, "script") == 0) g_debug.mode = DEBUG_MODE_SCRIPT;
                else if (strcmp(mode, "memory") == 0) g_debug.mode = DEBUG_MODE_MEMORY;
                else if (strcmp(mode, "all") == 0) g_debug.mode = DEBUG_MODE_ALL;
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                g_debug.log_file = fopen(argv[++i], "w");
                if (!g_debug.log_file) {
                    fprintf(stderr, "Failed to open log file\n");
                }
            }
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--backtrace") == 0) {
            g_debug.enable_backtrace = 1;
        } else if (strcmp(argv[i], "-M") == 0 || strcmp(argv[i], "--memory") == 0) {
            g_debug.enable_memory_tracking = 1;
        } else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--hexdump") == 0) {
            g_debug.enable_hex_dump = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timestamps") == 0) {
            g_debug.enable_timestamps = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            command = argv[i];
            break;
        }
    }
    
    // Set up signal handlers
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    
    // Initialize SDK
    neoc_init();
    
    // Execute command
    if (command == NULL || strcmp(command, "diagnose") == 0) {
        return run_diagnostics();
    } else if (strcmp(command, "test-transaction") == 0) {
        // Test transaction debugging
        neoc_transaction_builder_t *builder = NULL;
        neoc_transaction_builder_create(&builder);
        
        uint8_t script[] = {0x51, 0x52, 0x93}; // PUSH1 PUSH2 ADD
        neoc_transaction_builder_set_script(builder, script, sizeof(script));
        
        neoc_transaction_t *tx = NULL;
        neoc_transaction_builder_build(builder, &tx);
        
        debug_transaction(tx);
        
        neoc_transaction_free(tx);
        neoc_transaction_builder_free(builder);
    } else if (strcmp(command, "test-wallet") == 0) {
        // Test wallet debugging
        neoc_wallet_t *wallet = NULL;
        neoc_wallet_create("DebugWallet", &wallet);
        
        for (int i = 0; i < 3; i++) {
            neoc_account_t *account = NULL;
            neoc_account_create(&account);
            neoc_wallet_add_account(wallet, account);
        }
        
        debug_wallet(wallet);
        
        neoc_wallet_free(wallet);
    } else if (strcmp(command, "test-script") == 0) {
        // Test script debugging
        uint8_t script[] = {
            0x0c, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f,  // PUSHDATA1 "Hello"
            0x51,  // PUSH1
            0x52,  // PUSH2
            0x93,  // ADD
            0x61,  // NOP
            0x68   // ENDTRY
        };
        
        debug_script(script, sizeof(script));
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    // Cleanup
    if (g_debug.log_file) {
        fclose(g_debug.log_file);
    }
    
    neoc_cleanup();
    
    return 0;
}