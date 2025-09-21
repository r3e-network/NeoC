#!/bin/bash

# valgrind_test.sh - Valgrind integration script for NeoC SDK memory leak detection
# Usage: ./valgrind_test.sh [test_executable] [options]

set -e

# Configuration
VALGRIND_OPTS_MEMCHECK="--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --error-exitcode=1"
VALGRIND_OPTS_HELGRIND="--tool=helgrind --verbose --error-exitcode=1"
VALGRIND_OPTS_DRD="--tool=drd --verbose --error-exitcode=1"
SUPPRESSION_FILE="valgrind.supp"
LOG_DIR="valgrind_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

usage() {
    cat << EOF
Usage: $0 [OPTIONS] [TEST_EXECUTABLE]

NeoC SDK Valgrind Memory Analysis Tool

OPTIONS:
    -h, --help          Show this help message
    -m, --memcheck      Run memory leak detection (default)
    -t, --threads       Run thread safety analysis  
    -a, --all           Run all analysis types
    -v, --verbose       Verbose output
    -s, --suppressions  Use suppression file
    -o, --output DIR    Output directory for logs (default: valgrind_logs)
    
EXAMPLES:
    $0 ./test_basic                    # Basic memory leak check
    $0 -t ./test_performance          # Thread safety analysis
    $0 -a ./test_comprehensive        # All analysis types
    $0 -v -s ./test_memory            # Verbose with suppressions

EOF
}

check_valgrind() {
    if ! command -v valgrind &> /dev/null; then
        print_error "Valgrind is not installed. Please install valgrind to run memory analysis."
        print_info "On macOS: brew install valgrind"
        print_info "On Ubuntu/Debian: sudo apt-get install valgrind"
        exit 1
    fi
    
    VALGRIND_VERSION=$(valgrind --version)
    print_info "Found valgrind: $VALGRIND_VERSION"
}

create_suppression_file() {
    if [ ! -f "$SUPPRESSION_FILE" ]; then
        print_info "Creating default suppression file: $SUPPRESSION_FILE"
        cat > "$SUPPRESSION_FILE" << 'EOF'
# NeoC SDK Valgrind Suppressions
# 
# Suppress known false positives and system-level allocations

{
   ignore_dyld_leaks
   Memcheck:Leak
   ...
   obj:*/dyld
}

{
   ignore_libsystem_leaks
   Memcheck:Leak
   ...
   obj:*/libsystem_*.dylib
}

{
   ignore_libc_leaks
   Memcheck:Leak
   ...
   obj:*/libc.so.*
}

{
   ignore_pthread_leaks
   Memcheck:Leak
   ...
   fun:pthread_*
}

{
   ignore_openssl_leaks
   Memcheck:Leak
   ...
   obj:*/libcrypto.*
}

{
   ignore_test_framework_leaks
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   fun:test_*
}

EOF
    else
        print_info "Using existing suppression file: $SUPPRESSION_FILE"
    fi
}

run_memcheck() {
    local executable="$1"
    local test_name=$(basename "$executable")
    local log_file="$LOG_DIR/memcheck_${test_name}_${TIMESTAMP}.log"
    local opts="$VALGRIND_OPTS_MEMCHECK"
    
    if [ "$USE_SUPPRESSIONS" = "true" ]; then
        opts="$opts --suppressions=$SUPPRESSION_FILE"
    fi
    
    if [ "$VERBOSE" = "true" ]; then
        opts="$opts --num-callers=20"
    fi
    
    print_info "Running memory leak check on $executable"
    print_info "Log file: $log_file"
    
    if valgrind $opts --log-file="$log_file" "$executable" 2>&1; then
        print_success "Memory check passed: $executable"
        
        # Check for leaks in log file
        if grep -q "definitely lost: 0 bytes" "$log_file" && 
           grep -q "possibly lost: 0 bytes" "$log_file"; then
            print_success "No memory leaks detected"
        else
            print_warning "Potential memory issues detected - check log file"
            if [ "$VERBOSE" = "true" ]; then
                echo "=== Memory Check Summary ==="
                grep -A 10 "HEAP SUMMARY" "$log_file" || true
                grep -A 10 "LEAK SUMMARY" "$log_file" || true
            fi
        fi
        
        return 0
    else
        print_error "Memory check failed: $executable"
        if [ "$VERBOSE" = "true" ]; then
            echo "=== Error Details ==="
            tail -50 "$log_file"
        fi
        return 1
    fi
}

run_helgrind() {
    local executable="$1"
    local test_name=$(basename "$executable")
    local log_file="$LOG_DIR/helgrind_${test_name}_${TIMESTAMP}.log"
    
    print_info "Running thread safety analysis with Helgrind on $executable"
    print_info "Log file: $log_file"
    
    if valgrind $VALGRIND_OPTS_HELGRIND --log-file="$log_file" "$executable" 2>&1; then
        print_success "Thread safety check passed: $executable"
        return 0
    else
        print_error "Thread safety issues detected: $executable"
        if [ "$VERBOSE" = "true" ]; then
            echo "=== Thread Safety Issues ==="
            tail -50 "$log_file"
        fi
        return 1
    fi
}

run_drd() {
    local executable="$1"
    local test_name=$(basename "$executable")
    local log_file="$LOG_DIR/drd_${test_name}_${TIMESTAMP}.log"
    
    print_info "Running data race detection with DRD on $executable"
    print_info "Log file: $log_file"
    
    if valgrind $VALGRIND_OPTS_DRD --log-file="$log_file" "$executable" 2>&1; then
        print_success "Data race check passed: $executable"
        return 0
    else
        print_error "Data race issues detected: $executable"
        if [ "$VERBOSE" = "true" ]; then
            echo "=== Data Race Issues ==="
            tail -50 "$log_file"
        fi
        return 1
    fi
}

analyze_logs() {
    local log_dir="$1"
    
    print_info "Analyzing valgrind logs in $log_dir"
    
    echo "=== Memory Analysis Summary ==="
    
    # Count different types of issues
    local memcheck_logs=$(find "$log_dir" -name "memcheck_*_${TIMESTAMP}.log" 2>/dev/null | wc -l)
    local helgrind_logs=$(find "$log_dir" -name "helgrind_*_${TIMESTAMP}.log" 2>/dev/null | wc -l)
    local drd_logs=$(find "$log_dir" -name "drd_*_${TIMESTAMP}.log" 2>/dev/null | wc -l)
    
    echo "Generated logs:"
    echo "  - Memcheck logs: $memcheck_logs"
    echo "  - Helgrind logs: $helgrind_logs"
    echo "  - DRD logs: $drd_logs"
    
    # Analyze memory leaks
    if [ "$memcheck_logs" -gt 0 ]; then
        echo ""
        echo "=== Memory Leak Analysis ==="
        for log in "$log_dir"/memcheck_*_${TIMESTAMP}.log; do
            if [ -f "$log" ]; then
                local test_name=$(basename "$log" | sed 's/memcheck_\(.*\)_[0-9]*_[0-9]*.log/\1/')
                local definitely_lost=$(grep "definitely lost:" "$log" | tail -1 | awk '{print $4}')
                local possibly_lost=$(grep "possibly lost:" "$log" | tail -1 | awk '{print $4}')
                
                if [ "$definitely_lost" = "0" ] && [ "$possibly_lost" = "0" ]; then
                    print_success "$test_name: No leaks detected"
                else
                    print_warning "$test_name: Definitely lost: $definitely_lost, Possibly lost: $possibly_lost"
                fi
            fi
        done
    fi
    
    # Analyze thread issues
    if [ "$helgrind_logs" -gt 0 ] || [ "$drd_logs" -gt 0 ]; then
        echo ""
        echo "=== Thread Safety Analysis ==="
        
        for log in "$log_dir"/helgrind_*_${TIMESTAMP}.log "$log_dir"/drd_*_${TIMESTAMP}.log; do
            if [ -f "$log" ]; then
                local test_name=$(basename "$log" | sed 's/\(helgrind\|drd\)_\(.*\)_[0-9]*_[0-9]*.log/\2/')
                local tool=$(basename "$log" | sed 's/\(helgrind\|drd\)_.*/\1/')
                local errors=$(grep -c "^==[0-9]*== " "$log" | head -1)
                
                if [ "$errors" -eq 0 ]; then
                    print_success "$test_name ($tool): No thread issues detected"
                else
                    print_warning "$test_name ($tool): $errors potential thread issues"
                fi
            fi
        done
    fi
}

generate_report() {
    local report_file="$LOG_DIR/valgrind_report_${TIMESTAMP}.html"
    
    print_info "Generating HTML report: $report_file"
    
    cat > "$report_file" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>NeoC SDK Valgrind Analysis Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }
        .success { color: green; }
        .warning { color: orange; }
        .error { color: red; }
        .log-section { margin: 20px 0; padding: 10px; border: 1px solid #ccc; }
        pre { background-color: #f5f5f5; padding: 10px; overflow-x: auto; }
    </style>
</head>
<body>
    <div class="header">
        <h1>NeoC SDK Valgrind Analysis Report</h1>
        <p>Generated: $(date)</p>
        <p>Timestamp: ${TIMESTAMP}</p>
    </div>
    
    <h2>Test Results Summary</h2>
EOF

    # Add summary information
    for log in "$LOG_DIR"/*_${TIMESTAMP}.log; do
        if [ -f "$log" ]; then
            local filename=$(basename "$log")
            echo "<div class=\"log-section\">" >> "$report_file"
            echo "<h3>$filename</h3>" >> "$report_file"
            echo "<pre>" >> "$report_file"
            head -20 "$log" >> "$report_file"
            echo "</pre>" >> "$report_file"
            echo "</div>" >> "$report_file"
        fi
    done
    
    cat >> "$report_file" << EOF
    
    <h2>Full Logs</h2>
    <p>Full log files are available in the $LOG_DIR directory.</p>
    
</body>
</html>
EOF

    print_success "HTML report generated: $report_file"
}

# Parse command line arguments
MEMCHECK=true
THREADS=false
ALL=false
VERBOSE=false
USE_SUPPRESSIONS=false
OUTPUT_DIR="valgrind_logs"

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -m|--memcheck)
            MEMCHECK=true
            THREADS=false
            ALL=false
            shift
            ;;
        -t|--threads)
            MEMCHECK=false
            THREADS=true
            ALL=false
            shift
            ;;
        -a|--all)
            ALL=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -s|--suppressions)
            USE_SUPPRESSIONS=true
            shift
            ;;
        -o|--output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -*)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
        *)
            EXECUTABLE="$1"
            shift
            ;;
    esac
done

# Validate arguments
if [ -z "$EXECUTABLE" ]; then
    print_error "No test executable specified"
    usage
    exit 1
fi

if [ ! -f "$EXECUTABLE" ]; then
    print_error "Test executable not found: $EXECUTABLE"
    exit 1
fi

if [ ! -x "$EXECUTABLE" ]; then
    print_error "Test executable is not executable: $EXECUTABLE"
    exit 1
fi

# Main execution
print_info "NeoC SDK Valgrind Memory Analysis"
print_info "================================="

# Setup
check_valgrind
LOG_DIR="$OUTPUT_DIR"
mkdir -p "$LOG_DIR"

if [ "$USE_SUPPRESSIONS" = "true" ]; then
    create_suppression_file
fi

# Run tests
EXIT_CODE=0

if [ "$ALL" = "true" ]; then
    print_info "Running all analysis types"
    
    if ! run_memcheck "$EXECUTABLE"; then
        EXIT_CODE=1
    fi
    
    if ! run_helgrind "$EXECUTABLE"; then
        EXIT_CODE=1
    fi
    
    if ! run_drd "$EXECUTABLE"; then
        EXIT_CODE=1
    fi
    
elif [ "$THREADS" = "true" ]; then
    print_info "Running thread safety analysis"
    
    if ! run_helgrind "$EXECUTABLE"; then
        EXIT_CODE=1
    fi
    
    if ! run_drd "$EXECUTABLE"; then
        EXIT_CODE=1
    fi
    
else
    print_info "Running memory leak check"
    
    if ! run_memcheck "$EXECUTABLE"; then
        EXIT_CODE=1
    fi
fi

# Generate reports
analyze_logs "$LOG_DIR"
generate_report

if [ $EXIT_CODE -eq 0 ]; then
    print_success "All valgrind checks completed successfully"
else
    print_error "Some valgrind checks failed - please review the logs"
fi

exit $EXIT_CODE