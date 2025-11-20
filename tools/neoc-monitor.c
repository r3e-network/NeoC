/**
 * neoc-monitor - Real-time blockchain and network monitoring tool
 * 
 * Features:
 * - Block height monitoring
 * - Transaction tracking
 * - Network health checks
 * - Node synchronization status
 * - Performance metrics
 * - Alert system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <errno.h>

// ANSI color codes
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"

// Monitoring configuration
typedef struct {
    char *rpc_url;
    int refresh_interval;  // seconds
    int alert_threshold;   // blocks behind
    int verbose;
    int dashboard_mode;
    int json_output;
    char *log_file;
    char *alert_webhook;
} monitor_config_t;

// Network statistics
typedef struct {
    uint64_t block_height;
    uint64_t last_block_time;
    double tps;  // transactions per second
    int peer_count;
    char consensus_state[32];
    double sync_percentage;
    uint64_t memory_pool_count;
    double average_block_time;
    uint64_t total_transactions;
} network_stats_t;

// Node information
typedef struct {
    char address[256];
    int port;
    char version[64];
    int is_synced;
    uint64_t block_height;
    double response_time;  // milliseconds
    int is_active;
    time_t last_check;
} node_info_t;

// Alert system
typedef enum {
    ALERT_BLOCK_LAG,
    ALERT_HIGH_TPS,
    ALERT_LOW_PEERS,
    ALERT_SYNC_ISSUE,
    ALERT_NODE_DOWN,
    ALERT_SLOW_BLOCKS
} alert_type_t;

typedef struct {
    alert_type_t type;
    char message[512];
    time_t timestamp;
    int severity;  // 1=info, 2=warning, 3=critical
} alert_t;

// Global state
static monitor_config_t g_config = {
    .rpc_url = "http://localhost:20332",
    .refresh_interval = 5,
    .alert_threshold = 10,
    .verbose = 0,
    .dashboard_mode = 0,
    .json_output = 0,
    .log_file = NULL,
    .alert_webhook = NULL
};

static network_stats_t g_stats = {0};
static node_info_t g_nodes[100];
static int g_node_count = 0;
static volatile int g_running = 1;
static pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Alert queue
#define MAX_ALERTS 100
static alert_t g_alerts[MAX_ALERTS];
static int g_alert_count = 0;
static pthread_mutex_t g_alert_mutex = PTHREAD_MUTEX_INITIALIZER;

// HTTP response buffer
typedef struct {
    char *data;
    size_t size;
} response_buffer_t;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    response_buffer_t *buf = (response_buffer_t *)userp;
    
    char *ptr = realloc(buf->data, buf->size + total_size + 1);
    if (!ptr) {
        return 0;
    }
    
    buf->data = ptr;
    memcpy(&(buf->data[buf->size]), contents, total_size);
    buf->size += total_size;
    buf->data[buf->size] = 0;
    
    return total_size;
}

static int rpc_call(const char *method, const char *params, char **result) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return -1;
    }
    
    char json_request[1024];
    snprintf(json_request, sizeof(json_request),
             "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":%s,\"id\":1}",
             method, params ? params : "[]");
    
    response_buffer_t response = {0};
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, g_config.rpc_url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_request);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        free(response.data);
        return -1;
    }
    
    *result = response.data;
    return 0;
}

// Send webhook notification
static void send_webhook_notification(const alert_t *alert) {
    if (!g_config.alert_webhook || !alert) {
        return;
    }
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL for webhook notification\n");
        return;
    }
    
    // Prepare JSON payload
    char json_payload[2048];
    char escaped_message[1024];
    
    // Escape quotes in message for JSON
    const char *src = alert->message;
    char *dst = escaped_message;
    size_t remaining = sizeof(escaped_message) - 1;
    
    while (*src && remaining > 1) {
        if (*src == '"' || *src == '\\') {
            if (remaining > 2) {
                *dst++ = '\\';
                remaining--;
            }
        }
        *dst++ = *src++;
        remaining--;
    }
    *dst = '\0';
    
    // Get severity string
    const char *severity_str = "INFO";
    switch (alert->severity) {
        case 1: severity_str = "INFO"; break;
        case 2: severity_str = "WARNING"; break;
        case 3: severity_str = "CRITICAL"; break;
        default: severity_str = "UNKNOWN"; break;
    }
    
    // Get alert type string
    const char *type_str = "UNKNOWN";
    switch (alert->type) {
        case ALERT_BLOCK_LAG: type_str = "BLOCK_LAG"; break;
        case ALERT_HIGH_TPS: type_str = "HIGH_TPS"; break;
        case ALERT_LOW_PEERS: type_str = "LOW_PEERS"; break;
        case ALERT_SYNC_ISSUE: type_str = "SYNC_ISSUE"; break;
        case ALERT_NODE_DOWN: type_str = "NODE_DOWN"; break;
        case ALERT_SLOW_BLOCKS: type_str = "SLOW_BLOCKS"; break;
    }
    
    // Format timestamp
    char timestamp_str[64];
    struct tm *tm_info = localtime(&alert->timestamp);
    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Create JSON payload (compatible with common webhook formats like Slack, Discord, Teams)
    snprintf(json_payload, sizeof(json_payload),
        "{"
        "\"username\":\"NeoC Monitor\","
        "OptionName\":\"🚨 NeoC Network Alert\","
        "\"embeds\":[{"
            "\"title\":\"Alert: %s\","
            "\"description\":\"%s\","
            "\"color\":%d,"
            "\"fields\":["
                "{\"name\":\"Severity\",\"value\":\"%s\",\"inline\":true},"
                "{\"name\":\"Type\",\"value\":\"%s\",\"inline\":true},"
                "{\"name\":\"Time\",\"value\":\"%s\",\"inline\":false},"
                "{\"name\":\"Network\",\"value\":\"%s\",\"inline\":true}"
            "],"
            "\"footer\":{\"text\":\"NeoC Monitor v1.1.0\"}"
        "}],"
        "\"attachments\":[{"
            "\"color\":\"%s\","
            "\"title\":\"Neo Network Alert\","
            "\"text\":\"%s\","
            "\"fields\":["
                "{\"title\":\"Severity\",\"value\":\"%s\",\"short\":true},"
                "{\"title\":\"Type\",\"value\":\"%s\",\"short\":true}"
            "],"
            "\"footer\":\"NeoC Monitor\","
            "\"ts\":%ld"
        "}]"
        "}",
        type_str,
        escaped_message,
        alert->severity >= 3 ? 0xFF0000 : (alert->severity == 2 ? 0xFFFF00 : 0x00FF00),
        severity_str,
        type_str,
        timestamp_str,
        g_config.rpc_url,
        alert->severity >= 3 ? "danger" : (alert->severity == 2 ? "warning" : "good"),
        escaped_message,
        severity_str,
        type_str,
        alert->timestamp
    );
    
    // Set up CURL for POST request
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: NeoC-Monitor/1.0");
    
    // Response buffer (we don't need to process it, just capture it)
    response_buffer_t response = {0};
    
    curl_easy_setopt(curl, CURLOPT_URL, g_config.alert_webhook);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Send the webhook
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to send webhook notification: %s\n", curl_easy_strerror(res));
        
        // Log webhook failure
        if (g_config.log_file) {
            FILE *log = fopen(g_config.log_file, "a");
            if (log) {
                fprintf(log, "[%ld] WEBHOOK FAILED: %s (Alert: %s)\n", 
                        time(NULL), curl_easy_strerror(res), alert->message);
                fclose(log);
            }
        }
    } else {
        // Check HTTP response code
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        if (response_code >= 200 && response_code < 300) {
            if (g_config.verbose) {
                printf("Webhook notification sent successfully (HTTP %ld)\n", response_code);
            }
        } else {
            fprintf(stderr, "Webhook returned error: HTTP %ld\n", response_code);
            
            // Log HTTP error
            if (g_config.log_file) {
                FILE *log = fopen(g_config.log_file, "a");
                if (log) {
                    fprintf(log, "[%ld] WEBHOOK HTTP ERROR %ld: %s\n", 
                            time(NULL), response_code, alert->message);
                    fclose(log);
                }
            }
        }
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (response.data) {
        free(response.data);
    }
}

// Send test webhook notification
static void send_test_webhook() {
    if (!g_config.alert_webhook) {
        fprintf(stderr, "No webhook URL configured\n");
        return;
    }
    
    alert_t test_alert;
    test_alert.type = ALERT_BLOCK_LAG;
    test_alert.severity = 1;
    test_alert.timestamp = time(NULL);
    snprintf(test_alert.message, sizeof(test_alert.message), 
             "Test webhook notification from NeoC Monitor");
    
    printf("Sending test webhook to: %s\n", g_config.alert_webhook);
    send_webhook_notification(&test_alert);
}

static void add_alert(alert_type_t type, int severity, const char *format, ...) {
    pthread_mutex_lock(&g_alert_mutex);
    
    if (g_alert_count >= MAX_ALERTS) {
        // Remove oldest alert
        memmove(&g_alerts[0], &g_alerts[1], sizeof(alert_t) * (MAX_ALERTS - 1));
        g_alert_count = MAX_ALERTS - 1;
    }
    
    alert_t *alert = &g_alerts[g_alert_count++];
    alert->type = type;
    alert->severity = severity;
    alert->timestamp = time(NULL);
    
    va_list args;
    va_start(args, format);
    vsnprintf(alert->message, sizeof(alert->message), format, args);
    va_end(args);
    
    pthread_mutex_unlock(&g_alert_mutex);
    
    // Log alert
    if (g_config.log_file) {
        FILE *log = fopen(g_config.log_file, "a");
        if (log) {
            fprintf(log, "[%ld] ALERT (severity=%d): %s\n", 
                    alert->timestamp, severity, alert->message);
            fclose(log);
        }
    }
    
    // Send webhook notification for critical alerts
    if (g_config.alert_webhook && severity >= 3) {
        send_webhook_notification(alert);
    }
}

static void update_network_stats() {
    char *response = NULL;
    
    // Get block count
    if (rpc_call("getblockcount", NULL, &response) == 0) {
        // Parse block height from response
        char *height_str = strstr(response, "\"result\":");
        if (height_str) {
            g_stats.block_height = strtoull(height_str + 9, NULL, 10);
        }
        free(response);
    }
    
    // Get connection count
    if (rpc_call("getconnectioncount", NULL, &response) == 0) {
        char *count_str = strstr(response, "\"result\":");
        if (count_str) {
            g_stats.peer_count = atoi(count_str + 9);
        }
        free(response);
    }
    
    // Get raw mempool
    if (rpc_call("getrawmempool", NULL, &response) == 0) {
        // Count transactions in mempool
        int count = 0;
        char *p = response;
        while ((p = strchr(p, '"')) != NULL) {
            count++;
            p++;
        }
        g_stats.memory_pool_count = count / 2;  // Rough estimate
        free(response);
    }
    
    // Check for alerts
    if (g_stats.peer_count < 3) {
        add_alert(ALERT_LOW_PEERS, 2, "Low peer count: %d", g_stats.peer_count);
    }
    
    if (g_stats.memory_pool_count > 1000) {
        add_alert(ALERT_HIGH_TPS, 1, "High mempool count: %lu", g_stats.memory_pool_count);
    }
}

static void clear_screen() {
    printf("\033[2J\033[H");
}

static void draw_dashboard() {
    clear_screen();
    
    // Header
    printf(COLOR_BOLD COLOR_CYAN);
    printf("╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    NeoC Network Monitor Dashboard                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
    
    // Network Stats
    printf("\n" COLOR_BOLD "Network Statistics:" COLOR_RESET "\n");
    printf("├─ Block Height:     " COLOR_GREEN "%lu" COLOR_RESET "\n", g_stats.block_height);
    printf("├─ Peer Count:       %s%d%s\n", 
           g_stats.peer_count < 3 ? COLOR_RED : COLOR_GREEN,
           g_stats.peer_count, COLOR_RESET);
    printf("├─ Mempool Size:     %s%lu%s transactions\n",
           g_stats.memory_pool_count > 1000 ? COLOR_YELLOW : COLOR_GREEN,
           g_stats.memory_pool_count, COLOR_RESET);
    printf("├─ Avg Block Time:   %.2f seconds\n", g_stats.average_block_time);
    printf("└─ Sync Status:      %.1f%%\n", g_stats.sync_percentage);
    
    // Node Status
    if (g_node_count > 0) {
        printf("\n" COLOR_BOLD "Node Status:" COLOR_RESET "\n");
        for (int i = 0; i < g_node_count && i < 5; i++) {
            node_info_t *node = &g_nodes[i];
            printf("├─ %s:%d - Height: %lu, Response: %.1fms %s\n",
                   node->address, node->port, node->block_height,
                   node->response_time,
                   node->is_active ? COLOR_GREEN "●" COLOR_RESET : COLOR_RED "●" COLOR_RESET);
        }
    }
    
    // Recent Alerts
    pthread_mutex_lock(&g_alert_mutex);
    if (g_alert_count > 0) {
        printf("\n" COLOR_BOLD "Recent Alerts:" COLOR_RESET "\n");
        int start = g_alert_count > 5 ? g_alert_count - 5 : 0;
        for (int i = start; i < g_alert_count; i++) {
            alert_t *alert = &g_alerts[i];
            const char *color = alert->severity >= 3 ? COLOR_RED :
                               alert->severity == 2 ? COLOR_YELLOW : COLOR_CYAN;
            printf("├─ %s[%s] %s%s\n", color,
                   alert->severity >= 3 ? "CRITICAL" :
                   alert->severity == 2 ? "WARNING" : "INFO",
                   alert->message, COLOR_RESET);
        }
    }
    pthread_mutex_unlock(&g_alert_mutex);
    
    // Footer
    time_t now = time(NULL);
    printf("\n" COLOR_CYAN "Last Update: %s" COLOR_RESET, ctime(&now));
    printf("Press Ctrl+C to exit\n");
}

static void output_json() {
    pthread_mutex_lock(&g_stats_mutex);
    printf("{\n");
    printf("  \"block_height\": %lu,\n", g_stats.block_height);
    printf("  \"peer_count\": %d,\n", g_stats.peer_count);
    printf("  \"mempool_count\": %lu,\n", g_stats.memory_pool_count);
    printf("  \"average_block_time\": %.2f,\n", g_stats.average_block_time);
    printf("  \"sync_percentage\": %.1f,\n", g_stats.sync_percentage);
    printf("  \"timestamp\": %ld\n", time(NULL));
    printf("}\n");
    pthread_mutex_unlock(&g_stats_mutex);
}

static void *monitor_thread(void *arg) {
    while (g_running) {
        pthread_mutex_lock(&g_stats_mutex);
        update_network_stats();
        pthread_mutex_unlock(&g_stats_mutex);
        
        if (g_config.dashboard_mode) {
            draw_dashboard();
        } else if (g_config.json_output) {
            output_json();
        } else if (g_config.verbose) {
            printf("Block: %lu | Peers: %d | Mempool: %lu\n",
                   g_stats.block_height, g_stats.peer_count, g_stats.memory_pool_count);
        }
        
        sleep(g_config.refresh_interval);
    }
    return NULL;
}

static void signal_handler(int sig) {
    g_running = 0;
}

static void print_usage(const char *program) {
    printf("Usage: %s [OPTIONS]\n", program);
    printf("\nReal-time Neo blockchain and network monitoring tool\n");
    printf("\nOptions:\n");
    printf("  -r, --rpc URL        RPC endpoint URL (default: http://localhost:20332)\n");
    printf("  -i, --interval SEC   Refresh interval in seconds (default: 5)\n");
    printf("  -t, --threshold N    Alert threshold for blocks behind (default: 10)\n");
    printf("  -d, --dashboard      Dashboard mode with live updates\n");
    printf("  -j, --json          Output in JSON format\n");
    printf("  -l, --log FILE      Log alerts to file\n");
    printf("  -w, --webhook URL   Alert webhook URL (Discord, Slack, Teams compatible)\n");
    printf("  --test-webhook       Send test webhook notification and exit\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --help          Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s --dashboard\n", program);
    printf("  %s --json --interval 10\n", program);
    printf("  %s --rpc http://seed1.neo.org:20332 --log alerts.log\n", program);
    printf("  %s --webhook https://discord.com/api/webhooks/... --test-webhook\n", program);
    printf("\nWebhook formats supported:\n");
    printf("  - Discord: https://discord.com/api/webhooks/...\n");
    printf("  - Slack: https://hooks.slack.com/services/...\n");
    printf("  - MS Teams: https://outlook.office.com/webhook/...\n");
}

int main(int argc, char *argv[]) {
    int test_webhook_mode = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rpc") == 0) {
            if (i + 1 < argc) {
                g_config.rpc_url = argv[++i];
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interval") == 0) {
            if (i + 1 < argc) {
                g_config.refresh_interval = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threshold") == 0) {
            if (i + 1 < argc) {
                g_config.alert_threshold = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dashboard") == 0) {
            g_config.dashboard_mode = 1;
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            g_config.json_output = 1;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0) {
            if (i + 1 < argc) {
                g_config.log_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--webhook") == 0) {
            if (i + 1 < argc) {
                g_config.alert_webhook = argv[++i];
            }
        } else if (strcmp(argv[i], "--test-webhook") == 0) {
            test_webhook_mode = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Initialize curl
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Test webhook mode
    if (test_webhook_mode) {
        send_test_webhook();
        curl_global_cleanup();
        return 0;
    }
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create monitoring thread
    pthread_t monitor_tid;
    if (pthread_create(&monitor_tid, NULL, monitor_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create monitor thread\n");
        return 1;
    }
    
    // Wait for thread to finish
    pthread_join(monitor_tid, NULL);
    
    // Cleanup
    curl_global_cleanup();
    
    if (g_config.verbose) {
        printf("\nMonitoring stopped\n");
    }
    
    return 0;
}
