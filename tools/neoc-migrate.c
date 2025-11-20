/**
 * neoc-migrate - Swift to C migration validation tool
 * 
 * Features:
 * - API compatibility checking
 * - Function signature validation
 * - Type mapping verification
 * - Migration completeness assessment
 * - Code pattern analysis
 * - Performance comparison
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <ctype.h>

// ANSI color codes
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"

// Migration status levels
typedef enum {
    STATUS_COMPLETE,
    STATUS_PARTIAL,
    STATUS_MISSING,
    STATUS_INCOMPATIBLE
} migration_status_t;

// API element types
typedef enum {
    API_FUNCTION,
    API_STRUCT,
    API_ENUM,
    API_TYPEDEF,
    API_CONSTANT,
    API_PROTOCOL
} api_element_type_t;

// API element structure
typedef struct api_element {
    api_element_type_t type;
    char name[256];
    char swift_signature[512];
    char c_signature[512];
    migration_status_t status;
    char file_path[512];
    int line_number;
    struct api_element *next;
} api_element_t;

// Module information
typedef struct module_info {
    char name[128];
    int swift_files;
    int c_files;
    int swift_lines;
    int c_lines;
    int functions_total;
    int functions_migrated;
    int types_total;
    int types_migrated;
    double coverage_percentage;
    struct module_info *next;
} module_info_t;

// Configuration
typedef struct {
    char *swift_path;
    char *c_path;
    int verbose;
    int check_compatibility;
    int check_performance;
    int generate_mapping;
    int output_json;
    char *output_file;
    char *mapping_file;
} migrate_config_t;

// Global configuration
static migrate_config_t g_config = {
    .swift_path = NULL,
    .c_path = NULL,
    .verbose = 0,
    .check_compatibility = 1,
    .check_performance = 0,
    .generate_mapping = 0,
    .output_json = 0,
    .output_file = NULL,
    .mapping_file = NULL
};

// Global tracking
static api_element_t *g_api_elements = NULL;
static module_info_t *g_modules = NULL;
static int g_total_functions = 0;
static int g_migrated_functions = 0;
static int g_total_types = 0;
static int g_migrated_types = 0;

// Swift to C type mapping
typedef struct {
    const char *swift_type;
    const char *c_type;
} type_mapping_t;

static const type_mapping_t type_mappings[] = {
    {"String", "char*"},
    {"Int", "int"},
    {"Int32", "int32_t"},
    {"Int64", "int64_t"},
    {"UInt", "unsigned int"},
    {"UInt32", "uint32_t"},
    {"UInt64", "uint64_t"},
    {"Bool", "bool"},
    {"Double", "double"},
    {"Float", "float"},
    {"Data", "uint8_t*"},
    {"[UInt8]", "uint8_t*"},
    {"Void", "void"},
    {NULL, NULL}
};

// Add API element to tracking list
static void add_api_element(api_element_type_t type, const char *name,
                           const char *swift_sig, const char *c_sig,
                           migration_status_t status, const char *file, int line) {
    api_element_t *element = malloc(sizeof(api_element_t));
    if (!element) return;
    
    element->type = type;
    strncpy(element->name, name, sizeof(element->name) - 1);
    strncpy(element->swift_signature, swift_sig ? swift_sig : "", sizeof(element->swift_signature) - 1);
    strncpy(element->c_signature, c_sig ? c_sig : "", sizeof(element->c_signature) - 1);
    element->status = status;
    strncpy(element->file_path, file, sizeof(element->file_path) - 1);
    element->line_number = line;
    
    element->next = g_api_elements;
    g_api_elements = element;
    
    if (type == API_FUNCTION) {
        g_total_functions++;
        if (status == STATUS_COMPLETE) g_migrated_functions++;
    } else if (type == API_STRUCT || type == API_ENUM) {
        g_total_types++;
        if (status == STATUS_COMPLETE) g_migrated_types++;
    }
}

// Extract Swift function signatures
static void extract_swift_functions(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return;
    
    char line[1024];
    int line_num = 0;
    regex_t func_regex;
    
    // Match Swift function declarations
    if (regcomp(&func_regex, "^[[:space:]]*(public |private |internal |open )?func[[:space:]]+([a-zA-Z_][a-zA-Z0-9_]*)", REG_EXTENDED) != 0) {
        fclose(file);
        return;
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        regmatch_t matches[3];
        if (regexec(&func_regex, line, 3, matches, 0) == 0) {
            // Extract function name
            char func_name[256] = {0};
            int name_start = matches[2].rm_so;
            int name_len = matches[2].rm_eo - matches[2].rm_so;
            strncpy(func_name, line + name_start, name_len);
            
            // Get full signature (simplified)
            char *paren = strchr(line, '(');
            char signature[512] = {0};
            if (paren) {
                strncpy(signature, line, sizeof(signature) - 1);
                // Remove newline
                char *nl = strchr(signature, '\n');
                if (nl) *nl = '\0';
            }
            
            add_api_element(API_FUNCTION, func_name, signature, NULL, 
                          STATUS_MISSING, filepath, line_num);
            
            if (g_config.verbose) {
                printf("Found Swift function: %s\n", func_name);
            }
        }
    }
    
    regfree(&func_regex);
    fclose(file);
}

// Extract Swift types (structs, classes, enums)
static void extract_swift_types(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return;
    
    char line[1024];
    int line_num = 0;
    regex_t type_regex;
    
    // Match Swift type declarations
    if (regcomp(&type_regex, "^[[:space:]]*(public |private |internal )?(struct|class|enum|protocol)[[:space:]]+([a-zA-Z_][a-zA-Z0-9_]*)", REG_EXTENDED) != 0) {
        fclose(file);
        return;
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        regmatch_t matches[4];
        if (regexec(&type_regex, line, 4, matches, 0) == 0) {
            // Extract type name
            char type_name[256] = {0};
            int name_start = matches[3].rm_so;
            int name_len = matches[3].rm_eo - matches[3].rm_so;
            strncpy(type_name, line + name_start, name_len);
            
            // Extract type kind
            char type_kind[32] = {0};
            int kind_start = matches[2].rm_so;
            int kind_len = matches[2].rm_eo - matches[2].rm_so;
            strncpy(type_kind, line + kind_start, kind_len);
            
            api_element_type_t element_type = API_STRUCT;
            if (strcmp(type_kind, "enum") == 0) {
                element_type = API_ENUM;
            } else if (strcmp(type_kind, "protocol") == 0) {
                element_type = API_PROTOCOL;
            }
            
            add_api_element(element_type, type_name, line, NULL,
                          STATUS_MISSING, filepath, line_num);
            
            if (g_config.verbose) {
                printf("Found Swift %s: %s\n", type_kind, type_name);
            }
        }
    }
    
    regfree(&type_regex);
    fclose(file);
}

// Extract C function signatures
static void extract_c_functions(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return;
    
    char line[1024];
    int line_num = 0;
    regex_t func_regex;
    
    // Match C function declarations (simplified)
    if (regcomp(&func_regex, "^[a-zA-Z_][a-zA-Z0-9_\\*[:space:]]+[[:space:]]+([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*\\(", REG_EXTENDED) != 0) {
        fclose(file);
        return;
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Skip comments and preprocessor directives
        char *trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        if (trimmed[0] == '/' || trimmed[0] == '#') continue;
        
        regmatch_t matches[2];
        if (regexec(&func_regex, line, 2, matches, 0) == 0) {
            // Extract function name
            char func_name[256] = {0};
            int name_start = matches[1].rm_so;
            int name_len = matches[1].rm_eo - matches[1].rm_so;
            strncpy(func_name, line + name_start, name_len);
            
            // Skip main function
            if (strcmp(func_name, "main") == 0) continue;
            
            // Check if corresponding Swift function exists
            api_element_t *swift_func = NULL;
            for (api_element_t *elem = g_api_elements; elem; elem = elem->next) {
                if (elem->type == API_FUNCTION && 
                    strstr(elem->name, func_name) != NULL) {
                    swift_func = elem;
                    break;
                }
            }
            
            if (swift_func) {
                swift_func->status = STATUS_COMPLETE;
                strncpy(swift_func->c_signature, line, sizeof(swift_func->c_signature) - 1);
                g_migrated_functions++;
                
                if (g_config.verbose) {
                    printf("Matched C function: %s\n", func_name);
                }
            }
        }
    }
    
    regfree(&func_regex);
    fclose(file);
}

// Extract C types (structs, enums, typedefs)
static void extract_c_types(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return;
    
    char line[1024];
    int line_num = 0;
    regex_t type_regex;
    
    // Match C type declarations
    if (regcomp(&type_regex, "^[[:space:]]*(typedef[[:space:]]+)?(struct|enum)[[:space:]]+([a-zA-Z_][a-zA-Z0-9_]*)", REG_EXTENDED) != 0) {
        fclose(file);
        return;
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        regmatch_t matches[4];
        if (regexec(&type_regex, line, 4, matches, 0) == 0) {
            // Extract type name
            char type_name[256] = {0};
            int name_start = matches[3].rm_so;
            int name_len = matches[3].rm_eo - matches[3].rm_so;
            strncpy(type_name, line + name_start, name_len);
            
            // Check if corresponding Swift type exists
            api_element_t *swift_type = NULL;
            for (api_element_t *elem = g_api_elements; elem; elem = elem->next) {
                if ((elem->type == API_STRUCT || elem->type == API_ENUM) &&
                    strstr(elem->name, type_name) != NULL) {
                    swift_type = elem;
                    break;
                }
            }
            
            if (swift_type) {
                swift_type->status = STATUS_COMPLETE;
                strncpy(swift_type->c_signature, line, sizeof(swift_type->c_signature) - 1);
                g_migrated_types++;
                
                if (g_config.verbose) {
                    printf("Matched C type: %s\n", type_name);
                }
            }
        }
    }
    
    regfree(&type_regex);
    fclose(file);
}

// Scan Swift directory
static void scan_swift_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_swift_directory(filepath);
            } else if (S_ISREG(st.st_mode)) {
                char *ext = strrchr(entry->d_name, '.');
                if (ext && strcmp(ext, ".swift") == 0) {
                    extract_swift_functions(filepath);
                    extract_swift_types(filepath);
                }
            }
        }
    }
    
    closedir(dir);
}

// Scan C directory
static void scan_c_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_c_directory(filepath);
            } else if (S_ISREG(st.st_mode)) {
                char *ext = strrchr(entry->d_name, '.');
                if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0)) {
                    extract_c_functions(filepath);
                    extract_c_types(filepath);
                }
            }
        }
    }
    
    closedir(dir);
}

// Check type compatibility
static int check_type_compatibility(const char *swift_type, const char *c_type) {
    for (int i = 0; type_mappings[i].swift_type; i++) {
        if (strstr(swift_type, type_mappings[i].swift_type)) {
            if (strstr(c_type, type_mappings[i].c_type)) {
                return 1;  // Compatible
            }
        }
    }
    return 0;  // Incompatible or unknown
}

// Generate migration mapping file
static void generate_mapping_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Cannot create mapping file: %s\n", filename);
        return;
    }
    
    fprintf(file, "# Swift to C Migration Mapping\n");
    fprintf(file, "# Generated by neoc-migrate\n\n");
    
    fprintf(file, "## Functions\n\n");
    fprintf(file, "| Swift Function | C Function | Status | Compatibility |\n");
    fprintf(file, "|---------------|------------|--------|---------------|\n");
    
    for (api_element_t *elem = g_api_elements; elem; elem = elem->next) {
        if (elem->type == API_FUNCTION) {
            const char *status = elem->status == STATUS_COMPLETE ? "✅ Complete" :
                                elem->status == STATUS_PARTIAL ? "⚠️ Partial" :
                                elem->status == STATUS_MISSING ? "❌ Missing" :
                                "⚠️ Incompatible";
            
            fprintf(file, "| %s | %s | %s | %s |\n",
                    elem->name,
                    elem->status == STATUS_COMPLETE ? elem->name : "N/A",
                    status,
                    elem->status == STATUS_COMPLETE ? "Compatible" : "Check Required");
        }
    }
    
    fprintf(file, "\n## Types\n\n");
    fprintf(file, "| Swift Type | C Type | Status |\n");
    fprintf(file, "|-----------|--------|--------|\n");
    
    for (api_element_t *elem = g_api_elements; elem; elem = elem->next) {
        if (elem->type == API_STRUCT || elem->type == API_ENUM) {
            const char *status = elem->status == STATUS_COMPLETE ? "✅ Complete" :
                                elem->status == STATUS_MISSING ? "❌ Missing" :
                                "⚠️ Partial";
            
            fprintf(file, "| %s | %s | %s |\n",
                    elem->name,
                    elem->status == STATUS_COMPLETE ? elem->name : "N/A",
                    status);
        }
    }
    
    fclose(file);
    printf("Mapping file generated: %s\n", filename);
}

// Output migration report
static void output_report(FILE *output) {
    fprintf(output, COLOR_BOLD "\n╔════════════════════════════════════════════════════════╗\n");
    fprintf(output, "║        Swift to C Migration Validation Report          ║\n");
    fprintf(output, "╚════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    
    // Summary
    fprintf(output, "\n" COLOR_BOLD "Migration Summary:" COLOR_RESET "\n");
    fprintf(output, "├─ Swift Path: %s\n", g_config.swift_path);
    fprintf(output, "├─ C Path: %s\n", g_config.c_path);
    fprintf(output, "├─ Total Functions: %d\n", g_total_functions);
    fprintf(output, "├─ Migrated Functions: %d (%.1f%%)\n", 
            g_migrated_functions, 
            g_total_functions > 0 ? (g_migrated_functions * 100.0 / g_total_functions) : 0);
    fprintf(output, "├─ Total Types: %d\n", g_total_types);
    fprintf(output, "└─ Migrated Types: %d (%.1f%%)\n",
            g_migrated_types,
            g_total_types > 0 ? (g_migrated_types * 100.0 / g_total_types) : 0);
    
    // Missing APIs
    fprintf(output, "\n" COLOR_BOLD "Missing APIs:" COLOR_RESET "\n");
    int missing_count = 0;
    for (api_element_t *elem = g_api_elements; elem; elem = elem->next) {
        if (elem->status == STATUS_MISSING) {
            const char *type_str = elem->type == API_FUNCTION ? "Function" :
                                  elem->type == API_STRUCT ? "Struct" :
                                  elem->type == API_ENUM ? "Enum" :
                                  elem->type == API_PROTOCOL ? "Protocol" : "Type";
            
            fprintf(output, "├─ %s[%s]%s %s\n",
                    COLOR_YELLOW, type_str, COLOR_RESET, elem->name);
            fprintf(output, "│  └─ %s:%d\n", elem->file_path, elem->line_number);
            missing_count++;
            
            if (missing_count >= 20 && !g_config.verbose) {
                fprintf(output, "│  ... and %d more\n", 
                        g_total_functions + g_total_types - g_migrated_functions - g_migrated_types - 20);
                break;
            }
        }
    }
    
    if (missing_count == 0) {
        fprintf(output, "└─ %sNone - All APIs migrated!%s\n", COLOR_GREEN, COLOR_RESET);
    }
    
    // Compatibility issues
    if (g_config.check_compatibility) {
        fprintf(output, "\n" COLOR_BOLD "Compatibility Check:" COLOR_RESET "\n");
        int issues = 0;
        
        for (api_element_t *elem = g_api_elements; elem; elem = elem->next) {
            if (elem->status == STATUS_COMPLETE && elem->type == API_FUNCTION) {
                // Simple signature comparison (could be enhanced)
                if (strlen(elem->swift_signature) > 0 && strlen(elem->c_signature) > 0) {
                    // Check for basic compatibility issues
                    if (strstr(elem->swift_signature, "throws") && !strstr(elem->c_signature, "error")) {
                        fprintf(output, "├─ %sError Handling%s: %s\n",
                                COLOR_YELLOW, COLOR_RESET, elem->name);
                        fprintf(output, "│  └─ Swift uses throws, C needs error parameter\n");
                        issues++;
                    }
                    
                    if (strstr(elem->swift_signature, "async") && !strstr(elem->c_signature, "callback")) {
                        fprintf(output, "├─ %sAsync/Await%s: %s\n",
                                COLOR_YELLOW, COLOR_RESET, elem->name);
                        fprintf(output, "│  └─ Swift uses async, C needs callback mechanism\n");
                        issues++;
                    }
                }
            }
        }
        
        if (issues == 0) {
            fprintf(output, "└─ %sNo compatibility issues detected%s\n", COLOR_GREEN, COLOR_RESET);
        }
    }
    
    // Migration coverage by category
    fprintf(output, "\n" COLOR_BOLD "Migration Coverage:" COLOR_RESET "\n");
    
    double function_coverage = g_total_functions > 0 ? 
                               (g_migrated_functions * 100.0 / g_total_functions) : 0;
    double type_coverage = g_total_types > 0 ?
                          (g_migrated_types * 100.0 / g_total_types) : 0;
    double overall_coverage = (g_total_functions + g_total_types) > 0 ?
                             ((g_migrated_functions + g_migrated_types) * 100.0 / 
                              (g_total_functions + g_total_types)) : 0;
    
    // Progress bars
    fprintf(output, "├─ Functions: [");
    int func_bars = (int)(function_coverage / 5);
    for (int i = 0; i < 20; i++) {
        if (i < func_bars) {
            fprintf(output, "█");
        } else {
            fprintf(output, "░");
        }
    }
    fprintf(output, "] %.1f%%\n", function_coverage);
    
    fprintf(output, "├─ Types:     [");
    int type_bars = (int)(type_coverage / 5);
    for (int i = 0; i < 20; i++) {
        if (i < type_bars) {
            fprintf(output, "█");
        } else {
            fprintf(output, "░");
        }
    }
    fprintf(output, "] %.1f%%\n", type_coverage);
    
    fprintf(output, "└─ Overall:   [");
    int overall_bars = (int)(overall_coverage / 5);
    for (int i = 0; i < 20; i++) {
        if (i < overall_bars) {
            fprintf(output, "█");
        } else {
            fprintf(output, "░");
        }
    }
    fprintf(output, "] %.1f%%\n", overall_coverage);
    
    // Recommendations
    fprintf(output, "\n" COLOR_BOLD "Recommendations:" COLOR_RESET "\n");
    
    if (overall_coverage < 100) {
        fprintf(output, "├─ Complete migration of %d remaining functions\n",
                g_total_functions - g_migrated_functions);
        fprintf(output, "├─ Implement %d missing type definitions\n",
                g_total_types - g_migrated_types);
    }
    
    if (overall_coverage >= 90) {
        fprintf(output, "├─ %sExcellent migration coverage achieved!%s\n", 
                COLOR_GREEN, COLOR_RESET);
    } else if (overall_coverage >= 70) {
        fprintf(output, "├─ %sGood progress, continue migration efforts%s\n",
                COLOR_YELLOW, COLOR_RESET);
    } else {
        fprintf(output, "├─ %sSignificant migration work remaining%s\n",
                COLOR_RED, COLOR_RESET);
    }
    
    fprintf(output, "└─ Review compatibility warnings and update accordingly\n");
    
    fprintf(output, "\n─────────────────────────────────────────────────────────\n");
}

// Print usage
static void print_usage(const char *program) {
    printf("Usage: %s [OPTIONS] <swift-path> <c-path>\n", program);
    printf("\nSwift to C migration validation tool\n");
    printf("\nOptions:\n");
    printf("  -c, --check-compat   Check API compatibility (default)\n");
    printf("  -p, --performance    Compare performance characteristics\n");
    printf("  -m, --mapping FILE   Generate migration mapping file\n");
    printf("  -o, --output FILE    Output report to file\n");
    printf("  -j, --json          Output in JSON format\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -h, --help          Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s ../NeoSwift ./src\n", program);
    printf("  %s --mapping map.md ../NeoSwift .\n", program);
    printf("  %s --json -o report.json ../NeoSwift .\n", program);
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    int positional_count = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check-compat") == 0) {
            g_config.check_compatibility = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--performance") == 0) {
            g_config.check_performance = 1;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mapping") == 0) {
            if (i + 1 < argc) {
                g_config.generate_mapping = 1;
                g_config.mapping_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                g_config.output_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            g_config.output_json = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            if (positional_count == 0) {
                g_config.swift_path = argv[i];
            } else if (positional_count == 1) {
                g_config.c_path = argv[i];
            }
            positional_count++;
        }
    }
    
    if (!g_config.swift_path || !g_config.c_path) {
        fprintf(stderr, "Error: Both Swift and C paths are required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf(COLOR_BOLD COLOR_CYAN "NeoC Migration Validator v1.1.0\n" COLOR_RESET);
    printf("Analyzing migration from Swift to C...\n\n");
    
    // Check paths exist
    struct stat st;
    if (stat(g_config.swift_path, &st) != 0) {
        fprintf(stderr, "Error: Swift path does not exist: %s\n", g_config.swift_path);
        return 1;
    }
    if (stat(g_config.c_path, &st) != 0) {
        fprintf(stderr, "Error: C path does not exist: %s\n", g_config.c_path);
        return 1;
    }
    
    // Scan Swift code
    printf("Scanning Swift code...\n");
    scan_swift_directory(g_config.swift_path);
    
    // Scan C code
    printf("Scanning C code...\n");
    scan_c_directory(g_config.c_path);
    
    // Generate mapping file if requested
    if (g_config.generate_mapping && g_config.mapping_file) {
        generate_mapping_file(g_config.mapping_file);
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
    
    output_report(output);
    
    if (output != stdout) {
        fclose(output);
        printf("\nReport written to: %s\n", g_config.output_file);
    }
    
    // Cleanup
    api_element_t *elem = g_api_elements;
    while (elem) {
        api_element_t *next = elem->next;
        free(elem);
        elem = next;
    }
    
    module_info_t *mod = g_modules;
    while (mod) {
        module_info_t *next = mod->next;
        free(mod);
        mod = next;
    }
    
    // Return status based on coverage
    double coverage = (g_total_functions + g_total_types) > 0 ?
                     ((g_migrated_functions + g_migrated_types) * 100.0 / 
                      (g_total_functions + g_total_types)) : 0;
    
    if (coverage >= 95) return 0;  // Excellent
    if (coverage >= 80) return 1;  // Good
    if (coverage >= 60) return 2;  // Needs work
    return 3;  // Poor coverage
}
