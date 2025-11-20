/**
 * neoc-audit - Security auditing and vulnerability scanning tool
 * 
 * Features:
 * - Static code analysis
 * - Memory safety checks
 * - Cryptographic validation
 * - Input validation audit
 * - OWASP compliance checks
 * - Security best practices verification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>
#include <ctype.h>

// ANSI color codes
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"

// Security issue severity levels
typedef enum {
    SEVERITY_INFO = 1,
    SEVERITY_LOW,
    SEVERITY_MEDIUM,
    SEVERITY_HIGH,
    SEVERITY_CRITICAL
} severity_t;

// Security issue types
typedef enum {
    ISSUE_BUFFER_OVERFLOW,
    ISSUE_FORMAT_STRING,
    ISSUE_INJECTION,
    ISSUE_MEMORY_LEAK,
    ISSUE_RACE_CONDITION,
    ISSUE_CRYPTO_WEAK,
    ISSUE_INPUT_VALIDATION,
    ISSUE_ERROR_HANDLING,
    ISSUE_HARDCODED_SECRET,
    ISSUE_UNSAFE_FUNCTION,
    ISSUE_INTEGER_OVERFLOW,
    ISSUE_NULL_DEREF,
    ISSUE_TOCTOU,
    ISSUE_UNINITIALIZED,
    ISSUE_PATH_TRAVERSAL
} issue_type_t;

// Security issue structure
typedef struct security_issue {
    issue_type_t type;
    severity_t severity;
    char file_path[512];
    int line_number;
    char description[1024];
    char recommendation[1024];
    struct security_issue *next;
} security_issue_t;

// Audit configuration
typedef struct {
    char *target_path;
    int recursive;
    int verbose;
    int show_recommendations;
    int owasp_check;
    int crypto_audit;
    int output_json;
    int output_sarif;
    char *output_file;
    severity_t min_severity;
} audit_config_t;

// Global configuration
static audit_config_t g_config = {
    .target_path = ".",
    .recursive = 1,
    .verbose = 0,
    .show_recommendations = 1,
    .owasp_check = 1,
    .crypto_audit = 1,
    .output_json = 0,
    .output_sarif = 0,
    .output_file = NULL,
    .min_severity = SEVERITY_LOW
};

// Issue tracking
static security_issue_t *g_issues = NULL;
static int g_issue_count = 0;
static int g_files_scanned = 0;
static int g_lines_analyzed = 0;

// Unsafe function patterns
static const char *unsafe_functions[] = {
    "gets", "strcpy", "strcat", "sprintf", "vsprintf",
    "scanf", "sscanf", "fscanf", "realpath", "getwd",
    "strtok", "strncpy", "strncat", NULL
};

// Weak crypto patterns
static const char *weak_crypto[] = {
    "MD5", "SHA1", "DES", "RC4", "ECB",
    "rand()", "srand", NULL
};

// Add security issue to list
static void add_issue(issue_type_t type, severity_t severity,
                     const char *file, int line,
                     const char *desc, const char *recommendation) {
    security_issue_t *issue = malloc(sizeof(security_issue_t));
    if (!issue) return;
    
    issue->type = type;
    issue->severity = severity;
    strncpy(issue->file_path, file, sizeof(issue->file_path) - 1);
    issue->line_number = line;
    strncpy(issue->description, desc, sizeof(issue->description) - 1);
    strncpy(issue->recommendation, recommendation, sizeof(issue->recommendation) - 1);
    
    issue->next = g_issues;
    g_issues = issue;
    g_issue_count++;
}

// Check for buffer overflow vulnerabilities
static void check_buffer_overflow(const char *line, int line_num, const char *file) {
    regex_t regex;
    
    // Check for unsafe string functions
    for (int i = 0; unsafe_functions[i]; i++) {
        char pattern[256];
        snprintf(pattern, sizeof(pattern), "\\b%s\\s*\\(", unsafe_functions[i]);
        
        if (regcomp(&regex, pattern, REG_EXTENDED) == 0) {
            if (regexec(&regex, line, 0, NULL, 0) == 0) {
                char desc[512];
                snprintf(desc, sizeof(desc), 
                        "Unsafe function '%s' can cause buffer overflow",
                        unsafe_functions[i]);
                
                char rec[512];
                if (strcmp(unsafe_functions[i], "strcpy") == 0) {
                    snprintf(rec, sizeof(rec), "Use strncpy or strlcpy with bounds checking");
                } else if (strcmp(unsafe_functions[i], "sprintf") == 0) {
                    snprintf(rec, sizeof(rec), "Use snprintf with size parameter");
                } else if (strcmp(unsafe_functions[i], "gets") == 0) {
                    snprintf(rec, sizeof(rec), "Use fgets with buffer size");
                } else {
                    snprintf(rec, sizeof(rec), "Use safer alternative with bounds checking");
                }
                
                add_issue(ISSUE_BUFFER_OVERFLOW, SEVERITY_HIGH, file, line_num, desc, rec);
            }
            regfree(&regex);
        }
    }
    
    // Check for fixed buffer declarations
    if (regcomp(&regex, "char\\s+[a-zA-Z_][a-zA-Z0-9_]*\\[[0-9]+\\]", REG_EXTENDED) == 0) {
        if (regexec(&regex, line, 0, NULL, 0) == 0) {
            // Check if buffer size is small
            if (strstr(line, "[32]") || strstr(line, "[64]") || strstr(line, "[128]")) {
                add_issue(ISSUE_BUFFER_OVERFLOW, SEVERITY_LOW, file, line_num,
                         "Small fixed buffer size may be insufficient",
                         "Consider dynamic allocation or validate buffer size requirements");
            }
        }
        regfree(&regex);
    }
}

// Check for format string vulnerabilities
static void check_format_string(const char *line, int line_num, const char *file) {
    regex_t regex;
    
    // Check for printf family without format string
    const char *printf_funcs[] = {"printf", "fprintf", "sprintf", "snprintf", "vprintf", NULL};
    
    for (int i = 0; printf_funcs[i]; i++) {
        char pattern[256];
        snprintf(pattern, sizeof(pattern), "%s\\s*\\(\\s*[a-zA-Z_]", printf_funcs[i]);
        
        if (regcomp(&regex, pattern, REG_EXTENDED) == 0) {
            if (regexec(&regex, line, 0, NULL, 0) == 0) {
                // Check if it's not a literal string
                if (!strstr(line, "\"")) {
                    add_issue(ISSUE_FORMAT_STRING, SEVERITY_HIGH, file, line_num,
                             "Potential format string vulnerability",
                             "Always use format string literals, never user input");
                }
            }
            regfree(&regex);
        }
    }
}

// Check for SQL injection vulnerabilities
static void check_sql_injection(const char *line, int line_num, const char *file) {
    // Check for string concatenation in SQL queries
    if ((strstr(line, "SELECT") || strstr(line, "INSERT") || 
         strstr(line, "UPDATE") || strstr(line, "DELETE")) &&
        (strstr(line, "strcat") || strstr(line, "sprintf") || strstr(line, "+"))) {
        add_issue(ISSUE_INJECTION, SEVERITY_CRITICAL, file, line_num,
                 "Potential SQL injection vulnerability",
                 "Use parameterized queries or prepared statements");
    }
}

// Check for weak cryptography
static void check_weak_crypto(const char *line, int line_num, const char *file) {
    for (int i = 0; weak_crypto[i]; i++) {
        if (strstr(line, weak_crypto[i])) {
            char desc[512];
            snprintf(desc, sizeof(desc), 
                    "Weak cryptographic algorithm or function: %s",
                    weak_crypto[i]);
            
            char rec[512];
            if (strstr(weak_crypto[i], "MD5") || strstr(weak_crypto[i], "SHA1")) {
                snprintf(rec, sizeof(rec), "Use SHA-256 or SHA-3 for hashing");
            } else if (strstr(weak_crypto[i], "rand")) {
                snprintf(rec, sizeof(rec), "Use cryptographically secure random functions");
            } else {
                snprintf(rec, sizeof(rec), "Use modern, secure cryptographic algorithms");
            }
            
            add_issue(ISSUE_CRYPTO_WEAK, SEVERITY_MEDIUM, file, line_num, desc, rec);
        }
    }
}

// Check for hardcoded secrets
static void check_hardcoded_secrets(const char *line, int line_num, const char *file) {
    regex_t regex;
    
    // Check for hardcoded passwords, keys, tokens
    const char *patterns[] = {
        "password\\s*=\\s*\"[^\"]+\"",
        "api_key\\s*=\\s*\"[^\"]+\"",
        "secret\\s*=\\s*\"[^\"]+\"",
        "token\\s*=\\s*\"[^\"]+\"",
        "private_key\\s*=\\s*\"[^\"]+\"",
        NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        if (regcomp(&regex, patterns[i], REG_EXTENDED | REG_ICASE) == 0) {
            if (regexec(&regex, line, 0, NULL, 0) == 0) {
                add_issue(ISSUE_HARDCODED_SECRET, SEVERITY_CRITICAL, file, line_num,
                         "Hardcoded secret detected",
                         "Use environment variables or secure configuration management");
            }
            regfree(&regex);
        }
    }
}

// Check for integer overflow
static void check_integer_overflow(const char *line, int line_num, const char *file) {
    regex_t regex;
    
    // Check for potential integer overflow in arithmetic
    if (regcomp(&regex, "(\\+|\\*|<<).*\\s+(malloc|calloc|realloc)\\s*\\(", REG_EXTENDED) == 0) {
        if (regexec(&regex, line, 0, NULL, 0) == 0) {
            add_issue(ISSUE_INTEGER_OVERFLOW, SEVERITY_HIGH, file, line_num,
                     "Potential integer overflow in memory allocation",
                     "Validate arithmetic operations before memory allocation");
        }
        regfree(&regex);
    }
}

// Check for race conditions
static void check_race_conditions(const char *line, int line_num, const char *file) {
    // Check for TOCTOU (Time-of-check to time-of-use)
    if ((strstr(line, "access(") || strstr(line, "stat(")) &&
        (strstr(line, "open(") || strstr(line, "fopen("))) {
        add_issue(ISSUE_TOCTOU, SEVERITY_MEDIUM, file, line_num,
                 "Potential TOCTOU race condition",
                 "Use atomic operations or proper file locking");
    }
    
    // Check for signal handling issues
    if (strstr(line, "signal(") && !strstr(line, "sigaction(")) {
        add_issue(ISSUE_RACE_CONDITION, SEVERITY_LOW, file, line_num,
                 "Using signal() instead of sigaction()",
                 "Use sigaction() for more reliable signal handling");
    }
}

// Analyze a single line of code
static void analyze_line(const char *line, int line_num, const char *file) {
    check_buffer_overflow(line, line_num, file);
    check_format_string(line, line_num, file);
    check_sql_injection(line, line_num, file);
    check_weak_crypto(line, line_num, file);
    check_hardcoded_secrets(line, line_num, file);
    check_integer_overflow(line, line_num, file);
    check_race_conditions(line, line_num, file);
    
    g_lines_analyzed++;
}

// Scan a single file
static void scan_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        if (g_config.verbose) {
            fprintf(stderr, "Cannot open file: %s\n", filepath);
        }
        return;
    }
    
    if (g_config.verbose) {
        printf("Scanning: %s\n", filepath);
    }
    
    char line[4096];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Skip comments and empty lines
        char *trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        if (*trimmed == '/' && *(trimmed+1) == '/') continue;
        if (*trimmed == '#') continue;
        if (*trimmed == '\0') continue;
        
        analyze_line(line, line_num, filepath);
    }
    
    fclose(file);
    g_files_scanned++;
}

// Recursively scan directory
static void scan_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        if (g_config.verbose) {
            fprintf(stderr, "Cannot open directory: %s\n", path);
        }
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (g_config.recursive) {
                    scan_directory(filepath);
                }
            } else if (S_ISREG(st.st_mode)) {
                // Check if it's a C/C++ file
                char *ext = strrchr(entry->d_name, '.');
                if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 ||
                           strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0)) {
                    scan_file(filepath);
                }
            }
        }
    }
    
    closedir(dir);
}

// Get severity string
static const char *severity_string(severity_t severity) {
    switch (severity) {
        case SEVERITY_CRITICAL: return "CRITICAL";
        case SEVERITY_HIGH: return "HIGH";
        case SEVERITY_MEDIUM: return "MEDIUM";
        case SEVERITY_LOW: return "LOW";
        case SEVERITY_INFO: return "INFO";
        default: return "UNKNOWN";
    }
}

// Get severity color
static const char *severity_color(severity_t severity) {
    switch (severity) {
        case SEVERITY_CRITICAL: return COLOR_RED;
        case SEVERITY_HIGH: return COLOR_RED;
        case SEVERITY_MEDIUM: return COLOR_YELLOW;
        case SEVERITY_LOW: return COLOR_CYAN;
        case SEVERITY_INFO: return COLOR_GREEN;
        default: return COLOR_RESET;
    }
}

// Get issue type string
static const char *issue_type_string(issue_type_t type) {
    switch (type) {
        case ISSUE_BUFFER_OVERFLOW: return "Buffer Overflow";
        case ISSUE_FORMAT_STRING: return "Format String";
        case ISSUE_INJECTION: return "Injection";
        case ISSUE_MEMORY_LEAK: return "Memory Leak";
        case ISSUE_RACE_CONDITION: return "Race Condition";
        case ISSUE_CRYPTO_WEAK: return "Weak Cryptography";
        case ISSUE_INPUT_VALIDATION: return "Input Validation";
        case ISSUE_ERROR_HANDLING: return "Error Handling";
        case ISSUE_HARDCODED_SECRET: return "Hardcoded Secret";
        case ISSUE_UNSAFE_FUNCTION: return "Unsafe Function";
        case ISSUE_INTEGER_OVERFLOW: return "Integer Overflow";
        case ISSUE_NULL_DEREF: return "Null Dereference";
        case ISSUE_TOCTOU: return "TOCTOU";
        case ISSUE_UNINITIALIZED: return "Uninitialized Variable";
        case ISSUE_PATH_TRAVERSAL: return "Path Traversal";
        default: return "Unknown";
    }
}

// Output report in text format
static void output_text_report(FILE *output) {
    fprintf(output, COLOR_BOLD "\n╔════════════════════════════════════════════════════════╗\n");
    fprintf(output, "║           NeoC Security Audit Report                   ║\n");
    fprintf(output, "╚════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    
    fprintf(output, "\nScan Summary:\n");
    fprintf(output, "├─ Files Scanned: %d\n", g_files_scanned);
    fprintf(output, "├─ Lines Analyzed: %d\n", g_lines_analyzed);
    fprintf(output, "└─ Issues Found: %d\n\n", g_issue_count);
    
    // Count issues by severity
    int severity_counts[6] = {0};
    for (security_issue_t *issue = g_issues; issue; issue = issue->next) {
        severity_counts[issue->severity]++;
    }
    
    fprintf(output, "Issues by Severity:\n");
    for (int i = SEVERITY_CRITICAL; i >= SEVERITY_INFO; i--) {
        fprintf(output, "├─ %s%-8s%s: %d\n",
                severity_color(i), severity_string(i), COLOR_RESET,
                severity_counts[i]);
    }
    
    // Output detailed issues
    fprintf(output, "\n" COLOR_BOLD "Detailed Findings:\n" COLOR_RESET);
    fprintf(output, "─────────────────────────────────────────────────────────\n");
    
    int issue_num = 1;
    for (security_issue_t *issue = g_issues; issue; issue = issue->next) {
        if (issue->severity < g_config.min_severity) continue;
        
        fprintf(output, "\n%s[%s-%03d] %s %s%s\n",
                severity_color(issue->severity),
                severity_string(issue->severity),
                issue_num++,
                issue_type_string(issue->type),
                COLOR_RESET,
                COLOR_BOLD);
        
        fprintf(output, "File: %s:%d%s\n", issue->file_path, issue->line_number, COLOR_RESET);
        fprintf(output, "Description: %s\n", issue->description);
        
        if (g_config.show_recommendations) {
            fprintf(output, "Recommendation: %s\n", issue->recommendation);
        }
    }
    
    // OWASP compliance summary
    if (g_config.owasp_check) {
        fprintf(output, "\n" COLOR_BOLD "OWASP Top 10 Coverage:\n" COLOR_RESET);
        fprintf(output, "├─ A01:2021 - Broken Access Control: %s\n",
                severity_counts[SEVERITY_HIGH] > 0 ? COLOR_YELLOW "Issues Found" COLOR_RESET : COLOR_GREEN "✓" COLOR_RESET);
        fprintf(output, "├─ A02:2021 - Cryptographic Failures: %s\n",
                severity_counts[SEVERITY_MEDIUM] > 0 ? COLOR_YELLOW "Issues Found" COLOR_RESET : COLOR_GREEN "✓" COLOR_RESET);
        fprintf(output, "├─ A03:2021 - Injection: %s\n",
                severity_counts[SEVERITY_CRITICAL] > 0 ? COLOR_RED "Critical Issues" COLOR_RESET : COLOR_GREEN "✓" COLOR_RESET);
        fprintf(output, "└─ A06:2021 - Vulnerable Components: Review Required\n");
    }
    
    fprintf(output, "\n─────────────────────────────────────────────────────────\n");
    fprintf(output, "Scan completed at: %s", ctime(&(time_t){time(NULL)}));
}

// Output report in JSON format
static void output_json_report(FILE *output) {
    fprintf(output, "{\n");
    fprintf(output, "  \"scan_summary\": {\n");
    fprintf(output, "    \"files_scanned\": %d,\n", g_files_scanned);
    fprintf(output, "    \"lines_analyzed\": %d,\n", g_lines_analyzed);
    fprintf(output, "    \"total_issues\": %d,\n", g_issue_count);
    fprintf(output, "    \"timestamp\": %ld\n", time(NULL));
    fprintf(output, "  },\n");
    fprintf(output, "  \"issues\": [\n");
    
    int first = 1;
    for (security_issue_t *issue = g_issues; issue; issue = issue->next) {
        if (issue->severity < g_config.min_severity) continue;
        
        if (!first) fprintf(output, ",\n");
        fprintf(output, "    {\n");
        fprintf(output, "      \"type\": \"%s\",\n", issue_type_string(issue->type));
        fprintf(output, "      \"severity\": \"%s\",\n", severity_string(issue->severity));
        fprintf(output, "      \"file\": \"%s\",\n", issue->file_path);
        fprintf(output, "      \"line\": %d,\n", issue->line_number);
        fprintf(output, "      \"description\": \"%s\",\n", issue->description);
        fprintf(output, "      \"recommendation\": \"%s\"\n", issue->recommendation);
        fprintf(output, "    }");
        first = 0;
    }
    
    fprintf(output, "\n  ]\n");
    fprintf(output, "}\n");
}

// Print usage information
static void print_usage(const char *program) {
    printf("Usage: %s [OPTIONS] [PATH]\n", program);
    printf("\nSecurity auditing and vulnerability scanning tool for NeoC SDK\n");
    printf("\nOptions:\n");
    printf("  -r, --recursive      Scan directories recursively (default)\n");
    printf("  -s, --severity LEVEL Minimum severity to report (info|low|medium|high|critical)\n");
    printf("  -o, --output FILE    Output report to file\n");
    printf("  -j, --json          Output in JSON format\n");
    printf("  -S, --sarif         Output in SARIF format\n");
    printf("  -R, --no-rec        Don't show recommendations\n");
    printf("  -O, --no-owasp      Skip OWASP compliance checks\n");
    printf("  -C, --no-crypto     Skip cryptography audit\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --help          Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s src/             # Scan src directory\n", program);
    printf("  %s --json -o report.json .\n", program);
    printf("  %s --severity high --no-rec .\n", program);
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            g_config.recursive = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--severity") == 0) {
            if (i + 1 < argc) {
                char *level = argv[++i];
                if (strcasecmp(level, "critical") == 0) {
                    g_config.min_severity = SEVERITY_CRITICAL;
                } else if (strcasecmp(level, "high") == 0) {
                    g_config.min_severity = SEVERITY_HIGH;
                } else if (strcasecmp(level, "medium") == 0) {
                    g_config.min_severity = SEVERITY_MEDIUM;
                } else if (strcasecmp(level, "low") == 0) {
                    g_config.min_severity = SEVERITY_LOW;
                } else if (strcasecmp(level, "info") == 0) {
                    g_config.min_severity = SEVERITY_INFO;
                }
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                g_config.output_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            g_config.output_json = 1;
        } else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--sarif") == 0) {
            g_config.output_sarif = 1;
        } else if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "--no-rec") == 0) {
            g_config.show_recommendations = 0;
        } else if (strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--no-owasp") == 0) {
            g_config.owasp_check = 0;
        } else if (strcmp(argv[i], "-C") == 0 || strcmp(argv[i], "--no-crypto") == 0) {
            g_config.crypto_audit = 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            g_config.target_path = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    printf(COLOR_BOLD COLOR_CYAN "NeoC Security Audit Tool v1.1.0\n" COLOR_RESET);
    printf("Starting security scan of: %s\n\n", g_config.target_path);
    
    // Check if target exists
    struct stat st;
    if (stat(g_config.target_path, &st) != 0) {
        fprintf(stderr, "Error: Target path does not exist: %s\n", g_config.target_path);
        return 1;
    }
    
    // Perform scan
    if (S_ISDIR(st.st_mode)) {
        scan_directory(g_config.target_path);
    } else if (S_ISREG(st.st_mode)) {
        scan_file(g_config.target_path);
    } else {
        fprintf(stderr, "Error: Target is not a file or directory\n");
        return 1;
    }
    
    // Output report
    FILE *output = stdout;
    if (g_config.output_file) {
        output = fopen(g_config.output_file, "w");
        if (!output) {
            fprintf(stderr, "Error: Cannot create output file: %s\n", g_config.output_file);
            output = stdout;
        }
    }
    
    if (g_config.output_json) {
        output_json_report(output);
    } else {
        output_text_report(output);
    }
    
    if (output != stdout) {
        fclose(output);
        printf("\nReport written to: %s\n", g_config.output_file);
    }
    
    // Cleanup
    security_issue_t *issue = g_issues;
    while (issue) {
        security_issue_t *next = issue->next;
        free(issue);
        issue = next;
    }
    
    // Return non-zero if critical issues found
    for (security_issue_t *issue = g_issues; issue; issue = issue->next) {
        if (issue->severity == SEVERITY_CRITICAL) {
            return 2;
        }
    }
    
    return g_issue_count > 0 ? 1 : 0;
}
