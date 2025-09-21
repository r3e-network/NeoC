/**
 * @file neoc-profiler.c
 * @brief NeoC SDK Performance Profiler
 * 
 * Advanced profiling tool for measuring and analyzing NeoC SDK performance
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <signal.h>
#include <neoc/neoc.h>
#include <neoc/wallet/wallet.h>
#include <neoc/wallet/account.h>
#include <neoc/transaction/transaction_builder.h>
#include <neoc/crypto/keys.h>
#include <neoc/crypto/hash.h>
#include <neoc/types/uint256.h>

#define PROFILE_VERSION "1.0.0"
#define MAX_SAMPLES 1000000
#define DEFAULT_ITERATIONS 1000

// Profile operation types
typedef enum {
    PROFILE_ACCOUNT_CREATE,
    PROFILE_ACCOUNT_IMPORT,
    PROFILE_KEY_GENERATION,
    PROFILE_SIGNATURE_CREATE,
    PROFILE_SIGNATURE_VERIFY,
    PROFILE_HASH_SHA256,
    PROFILE_HASH_RIPEMD160,
    PROFILE_BASE58_ENCODE,
    PROFILE_BASE58_DECODE,
    PROFILE_TRANSACTION_BUILD,
    PROFILE_TRANSACTION_SIGN,
    PROFILE_TRANSACTION_SERIALIZE,
    PROFILE_WALLET_CREATE,
    PROFILE_WALLET_ADD_ACCOUNT,
    PROFILE_MULTISIG_CREATE,
    PROFILE_OPERATION_COUNT
} profile_operation_t;

// Profile statistics
typedef struct {
    const char *name;
    size_t count;
    double total_time;
    double min_time;
    double max_time;
    double avg_time;
    double std_dev;
    double *samples;
    size_t sample_count;
    size_t memory_used;
    size_t memory_peak;
} profile_stats_t;

// Global profiler state
typedef struct {
    profile_stats_t stats[PROFILE_OPERATION_COUNT];
    int iterations;
    int warmup_iterations;
    int verbose;
    int export_csv;
    int export_json;
    char *output_file;
    struct timeval start_time;
    struct rusage start_usage;
    pthread_mutex_t mutex;
} profiler_state_t;

static profiler_state_t g_profiler = {
    .iterations = DEFAULT_ITERATIONS,
    .warmup_iterations = 100,
    .verbose = 0,
    .export_csv = 0,
    .export_json = 0,
    .output_file = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

// Operation names
static const char *operation_names[PROFILE_OPERATION_COUNT] = {
    "Account Creation",
    "Account Import",
    "Key Generation",
    "Signature Creation",
    "Signature Verification",
    "SHA256 Hashing",
    "RIPEMD160 Hashing",
    "Base58 Encoding",
    "Base58 Decoding",
    "Transaction Building",
    "Transaction Signing",
    "Transaction Serialization",
    "Wallet Creation",
    "Wallet Add Account",
    "Multi-Sig Creation"
};

/**
 * Get current time in microseconds
 */
static double get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

/**
 * Get current memory usage in bytes
 */
static size_t get_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss * 1024; // Convert to bytes
}

/**
 * Initialize profiler statistics
 */
static void init_stats() {
    for (int i = 0; i < PROFILE_OPERATION_COUNT; i++) {
        g_profiler.stats[i].name = operation_names[i];
        g_profiler.stats[i].count = 0;
        g_profiler.stats[i].total_time = 0;
        g_profiler.stats[i].min_time = 1e9;
        g_profiler.stats[i].max_time = 0;
        g_profiler.stats[i].avg_time = 0;
        g_profiler.stats[i].std_dev = 0;
        g_profiler.stats[i].samples = calloc(g_profiler.iterations, sizeof(double));
        g_profiler.stats[i].sample_count = 0;
        g_profiler.stats[i].memory_used = 0;
        g_profiler.stats[i].memory_peak = 0;
    }
}

/**
 * Record profile sample
 */
static void record_sample(profile_operation_t op, double time_us, size_t memory) {
    pthread_mutex_lock(&g_profiler.mutex);
    
    profile_stats_t *stat = &g_profiler.stats[op];
    
    stat->count++;
    stat->total_time += time_us;
    
    if (time_us < stat->min_time) stat->min_time = time_us;
    if (time_us > stat->max_time) stat->max_time = time_us;
    
    if (stat->sample_count < g_profiler.iterations) {
        stat->samples[stat->sample_count++] = time_us;
    }
    
    stat->memory_used += memory;
    if (memory > stat->memory_peak) stat->memory_peak = memory;
    
    pthread_mutex_unlock(&g_profiler.mutex);
}

/**
 * Calculate statistics
 */
static void calculate_stats() {
    for (int i = 0; i < PROFILE_OPERATION_COUNT; i++) {
        profile_stats_t *stat = &g_profiler.stats[i];
        
        if (stat->count == 0) continue;
        
        // Calculate average
        stat->avg_time = stat->total_time / stat->count;
        
        // Calculate standard deviation
        double sum_sq = 0;
        for (size_t j = 0; j < stat->sample_count; j++) {
            double diff = stat->samples[j] - stat->avg_time;
            sum_sq += diff * diff;
        }
        stat->std_dev = sqrt(sum_sq / stat->sample_count);
    }
}

/**
 * Profile account operations
 */
static void profile_account_operations() {
    printf("Profiling account operations...\n");
    
    // Warmup
    for (int i = 0; i < g_profiler.warmup_iterations; i++) {
        neoc_account_t *account = NULL;
        neoc_account_create(&account);
        neoc_account_free(account);
    }
    
    // Profile account creation
    for (int i = 0; i < g_profiler.iterations; i++) {
        size_t mem_before = get_memory_usage();
        double start = get_time_us();
        
        neoc_account_t *account = NULL;
        neoc_error_t err = neoc_account_create(&account);
        
        double end = get_time_us();
        size_t mem_after = get_memory_usage();
        
        if (err == NEOC_SUCCESS) {
            record_sample(PROFILE_ACCOUNT_CREATE, end - start, mem_after - mem_before);
            
            // Profile account export
            char wif[256];
            start = get_time_us();
            neoc_account_export_wif(account, wif, sizeof(wif));
            end = get_time_us();
            record_sample(PROFILE_ACCOUNT_IMPORT, end - start, 0);
            
            neoc_account_free(account);
        }
        
        if (g_profiler.verbose && i % 100 == 0) {
            printf("  Account operations: %d/%d\r", i + 1, g_profiler.iterations);
            fflush(stdout);
        }
    }
    
    printf("  Account operations: Complete      \n");
}

/**
 * Profile cryptographic operations
 */
static void profile_crypto_operations() {
    printf("Profiling cryptographic operations...\n");
    
    // Prepare test data
    uint8_t data[1024];
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = rand() & 0xFF;
    }
    
    neoc_key_pair_t *key_pair = NULL;
    neoc_key_pair_create(&key_pair);
    
    // Profile key generation
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_key_pair_t *kp = NULL;
        neoc_key_pair_create(&kp);
        
        double end = get_time_us();
        record_sample(PROFILE_KEY_GENERATION, end - start, 0);
        
        neoc_key_pair_free(kp);
    }
    
    // Profile signature creation
    uint8_t signature[64];
    size_t sig_len;
    
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_ecdsa_sign(key_pair, data, 32, signature, &sig_len);
        
        double end = get_time_us();
        record_sample(PROFILE_SIGNATURE_CREATE, end - start, 0);
    }
    
    // Profile signature verification
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        bool valid;
        neoc_ecdsa_verify(
            neoc_key_pair_get_public(key_pair),
            data, 32, signature, sig_len, &valid
        );
        
        double end = get_time_us();
        record_sample(PROFILE_SIGNATURE_VERIFY, end - start, 0);
    }
    
    // Profile SHA256
    uint8_t hash[32];
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_sha256(data, sizeof(data), hash);
        
        double end = get_time_us();
        record_sample(PROFILE_HASH_SHA256, end - start, 0);
    }
    
    // Profile RIPEMD160
    uint8_t ripemd[20];
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_ripemd160(data, sizeof(data), ripemd);
        
        double end = get_time_us();
        record_sample(PROFILE_HASH_RIPEMD160, end - start, 0);
    }
    
    // Profile Base58 encoding
    char base58[256];
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_base58_encode(data, 32, base58, sizeof(base58));
        
        double end = get_time_us();
        record_sample(PROFILE_BASE58_ENCODE, end - start, 0);
    }
    
    // Profile Base58 decoding
    uint8_t decoded[256];
    size_t decoded_len;
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_base58_decode(base58, decoded, sizeof(decoded), &decoded_len);
        
        double end = get_time_us();
        record_sample(PROFILE_BASE58_DECODE, end - start, 0);
    }
    
    neoc_key_pair_free(key_pair);
    
    printf("  Cryptographic operations: Complete\n");
}

/**
 * Profile transaction operations
 */
static void profile_transaction_operations() {
    printf("Profiling transaction operations...\n");
    
    neoc_account_t *account = NULL;
    neoc_account_create(&account);
    
    for (int i = 0; i < g_profiler.iterations; i++) {
        // Profile transaction building
        double start = get_time_us();
        
        neoc_transaction_builder_t *builder = NULL;
        neoc_transaction_builder_create(&builder);
        
        // Add simple script
        uint8_t script[] = {0x51, 0x52, 0x93}; // PUSH1 PUSH2 ADD
        neoc_transaction_builder_set_script(builder, script, sizeof(script));
        neoc_transaction_builder_set_valid_until_block(builder, 1000000);
        neoc_transaction_builder_add_signer(builder, account);
        
        neoc_transaction_t *tx = NULL;
        neoc_transaction_builder_build(builder, &tx);
        
        double end = get_time_us();
        record_sample(PROFILE_TRANSACTION_BUILD, end - start, 0);
        
        // Profile transaction signing
        start = get_time_us();
        
        neoc_transaction_t *signed_tx = NULL;
        neoc_transaction_builder_sign(builder, account, &signed_tx);
        
        end = get_time_us();
        record_sample(PROFILE_TRANSACTION_SIGN, end - start, 0);
        
        // Profile transaction serialization
        if (signed_tx != NULL) {
            neoc_binary_writer_t *writer = NULL;
            neoc_binary_writer_create(&writer);
            
            start = get_time_us();
            
            neoc_transaction_serialize(signed_tx, writer);
            
            end = get_time_us();
            record_sample(PROFILE_TRANSACTION_SERIALIZE, end - start, 0);
            
            neoc_binary_writer_free(writer);
            neoc_transaction_free(signed_tx);
        }
        
        if (tx) neoc_transaction_free(tx);
        neoc_transaction_builder_free(builder);
        
        if (g_profiler.verbose && i % 100 == 0) {
            printf("  Transaction operations: %d/%d\r", i + 1, g_profiler.iterations);
            fflush(stdout);
        }
    }
    
    neoc_account_free(account);
    
    printf("  Transaction operations: Complete      \n");
}

/**
 * Profile wallet operations
 */
static void profile_wallet_operations() {
    printf("Profiling wallet operations...\n");
    
    for (int i = 0; i < g_profiler.iterations; i++) {
        // Profile wallet creation
        double start = get_time_us();
        
        neoc_wallet_t *wallet = NULL;
        neoc_wallet_create("TestWallet", &wallet);
        
        double end = get_time_us();
        record_sample(PROFILE_WALLET_CREATE, end - start, 0);
        
        // Profile adding accounts
        neoc_account_t *account = NULL;
        neoc_account_create(&account);
        
        start = get_time_us();
        
        neoc_wallet_add_account(wallet, account);
        
        end = get_time_us();
        record_sample(PROFILE_WALLET_ADD_ACCOUNT, end - start, 0);
        
        neoc_wallet_free(wallet);
        
        if (g_profiler.verbose && i % 100 == 0) {
            printf("  Wallet operations: %d/%d\r", i + 1, g_profiler.iterations);
            fflush(stdout);
        }
    }
    
    // Profile multi-sig creation
    neoc_account_t *accounts[3];
    neoc_ec_public_key_t *pub_keys[3];
    
    for (int i = 0; i < 3; i++) {
        neoc_account_create(&accounts[i]);
        pub_keys[i] = neoc_account_get_public_key(accounts[i]);
    }
    
    for (int i = 0; i < g_profiler.iterations; i++) {
        double start = get_time_us();
        
        neoc_account_t *multisig = NULL;
        neoc_account_create_multisig(2, pub_keys, 3, &multisig);
        
        double end = get_time_us();
        record_sample(PROFILE_MULTISIG_CREATE, end - start, 0);
        
        neoc_account_free(multisig);
    }
    
    for (int i = 0; i < 3; i++) {
        neoc_account_free(accounts[i]);
    }
    
    printf("  Wallet operations: Complete      \n");
}

/**
 * Export results to CSV
 */
static void export_csv(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to create CSV file: %s\n", filename);
        return;
    }
    
    // Header
    fprintf(f, "Operation,Count,Total(ms),Min(μs),Max(μs),Avg(μs),StdDev(μs),Throughput(ops/s),Memory(KB)\n");
    
    // Data
    for (int i = 0; i < PROFILE_OPERATION_COUNT; i++) {
        profile_stats_t *stat = &g_profiler.stats[i];
        if (stat->count == 0) continue;
        
        double throughput = stat->count / (stat->total_time / 1000000.0);
        
        fprintf(f, "%s,%zu,%.2f,%.2f,%.2f,%.2f,%.2f,%.0f,%zu\n",
            stat->name,
            stat->count,
            stat->total_time / 1000.0, // Convert to ms
            stat->min_time,
            stat->max_time,
            stat->avg_time,
            stat->std_dev,
            throughput,
            stat->memory_peak / 1024
        );
    }
    
    fclose(f);
    printf("Results exported to: %s\n", filename);
}

/**
 * Export results to JSON
 */
static void export_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to create JSON file: %s\n", filename);
        return;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  \"version\": \"%s\",\n", PROFILE_VERSION);
    fprintf(f, "  \"timestamp\": %ld,\n", time(NULL));
    fprintf(f, "  \"iterations\": %d,\n", g_profiler.iterations);
    fprintf(f, "  \"results\": [\n");
    
    int first = 1;
    for (int i = 0; i < PROFILE_OPERATION_COUNT; i++) {
        profile_stats_t *stat = &g_profiler.stats[i];
        if (stat->count == 0) continue;
        
        if (!first) fprintf(f, ",\n");
        first = 0;
        
        double throughput = stat->count / (stat->total_time / 1000000.0);
        
        fprintf(f, "    {\n");
        fprintf(f, "      \"operation\": \"%s\",\n", stat->name);
        fprintf(f, "      \"count\": %zu,\n", stat->count);
        fprintf(f, "      \"total_ms\": %.2f,\n", stat->total_time / 1000.0);
        fprintf(f, "      \"min_us\": %.2f,\n", stat->min_time);
        fprintf(f, "      \"max_us\": %.2f,\n", stat->max_time);
        fprintf(f, "      \"avg_us\": %.2f,\n", stat->avg_time);
        fprintf(f, "      \"std_dev_us\": %.2f,\n", stat->std_dev);
        fprintf(f, "      \"throughput_ops_per_sec\": %.0f,\n", throughput);
        fprintf(f, "      \"memory_peak_kb\": %zu\n", stat->memory_peak / 1024);
        fprintf(f, "    }");
    }
    
    fprintf(f, "\n  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
    printf("Results exported to: %s\n", filename);
}

/**
 * Print results summary
 */
static void print_results() {
    printf("\n");
    printf("================================================================================\n");
    printf("                           NeoC SDK Performance Profile                         \n");
    printf("================================================================================\n");
    printf("\n");
    
    printf("Configuration:\n");
    printf("  Iterations: %d\n", g_profiler.iterations);
    printf("  Warmup: %d\n", g_profiler.warmup_iterations);
    printf("\n");
    
    printf("Results:\n");
    printf("%-30s %8s %10s %10s %10s %10s %12s\n",
           "Operation", "Count", "Avg(μs)", "Min(μs)", "Max(μs)", "StdDev", "Throughput");
    printf("%-30s %8s %10s %10s %10s %10s %12s\n",
           "---------", "-----", "-------", "-------", "-------", "------", "----------");
    
    for (int i = 0; i < PROFILE_OPERATION_COUNT; i++) {
        profile_stats_t *stat = &g_profiler.stats[i];
        if (stat->count == 0) continue;
        
        double throughput = stat->count / (stat->total_time / 1000000.0);
        
        printf("%-30s %8zu %10.2f %10.2f %10.2f %10.2f %10.0f/s\n",
            stat->name,
            stat->count,
            stat->avg_time,
            stat->min_time,
            stat->max_time,
            stat->std_dev,
            throughput
        );
    }
    
    printf("\n");
    
    // Calculate total time
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    double total_time = (end_time.tv_sec - g_profiler.start_time.tv_sec) +
                       (end_time.tv_usec - g_profiler.start_time.tv_usec) / 1000000.0;
    
    printf("Total profiling time: %.2f seconds\n", total_time);
    
    // Memory stats
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Peak memory usage: %ld KB\n", usage.ru_maxrss);
}

/**
 * Print usage
 */
static void print_usage(const char *program) {
    printf("NeoC SDK Performance Profiler v%s\n", PROFILE_VERSION);
    printf("Usage: %s [options]\n", program);
    printf("\n");
    printf("Options:\n");
    printf("  -i, --iterations N   Number of iterations (default: %d)\n", DEFAULT_ITERATIONS);
    printf("  -w, --warmup N       Warmup iterations (default: 100)\n");
    printf("  -v, --verbose        Verbose output\n");
    printf("  -c, --csv FILE       Export results to CSV\n");
    printf("  -j, --json FILE      Export results to JSON\n");
    printf("  -o, --output FILE    Output file for results\n");
    printf("  -h, --help           Show this help message\n");
    printf("\n");
    printf("Profile operations:\n");
    printf("  - Account operations (creation, import)\n");
    printf("  - Cryptographic operations (hashing, signing)\n");
    printf("  - Transaction operations (building, signing)\n");
    printf("  - Wallet operations (creation, management)\n");
}

/**
 * Signal handler for clean shutdown
 */
static void signal_handler(int sig) {
    printf("\n\nInterrupted. Generating partial results...\n");
    calculate_stats();
    print_results();
    exit(0);
}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--iterations") == 0) {
            if (i + 1 < argc) {
                g_profiler.iterations = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--warmup") == 0) {
            if (i + 1 < argc) {
                g_profiler.warmup_iterations = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_profiler.verbose = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--csv") == 0) {
            if (i + 1 < argc) {
                g_profiler.export_csv = 1;
                g_profiler.output_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            if (i + 1 < argc) {
                g_profiler.export_json = 1;
                g_profiler.output_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    
    // Initialize
    printf("NeoC SDK Performance Profiler v%s\n", PROFILE_VERSION);
    printf("Initializing...\n");
    
    neoc_error_t err = neoc_init();
    if (err != NEOC_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoC SDK: %s\n", neoc_error_string(err));
        return 1;
    }
    
    init_stats();
    gettimeofday(&g_profiler.start_time, NULL);
    getrusage(RUSAGE_SELF, &g_profiler.start_usage);
    
    printf("Running performance profile with %d iterations...\n\n", g_profiler.iterations);
    
    // Run profiling
    profile_account_operations();
    profile_crypto_operations();
    profile_transaction_operations();
    profile_wallet_operations();
    
    // Calculate statistics
    calculate_stats();
    
    // Print results
    print_results();
    
    // Export if requested
    if (g_profiler.export_csv && g_profiler.output_file) {
        export_csv(g_profiler.output_file);
    }
    if (g_profiler.export_json && g_profiler.output_file) {
        export_json(g_profiler.output_file);
    }
    
    // Cleanup
    for (int i = 0; i < PROFILE_OPERATION_COUNT; i++) {
        free(g_profiler.stats[i].samples);
    }
    
    neoc_cleanup();
    
    return 0;
}