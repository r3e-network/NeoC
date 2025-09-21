#!/bin/bash

# run_tests.sh - Test runner script for NeoC SDK
# Runs all tests with various configurations and generates reports

set -e

# Configuration
BUILD_DIR="../build"
TESTS_DIR="."
REPORTS_DIR="test_reports"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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
Usage: $0 [OPTIONS]

NeoC SDK Test Runner

OPTIONS:
    -h, --help          Show this help message
    -b, --build         Build tests before running
    -v, --valgrind      Run valgrind memory checks
    -p, --performance   Run performance benchmarks
    -c, --coverage      Generate code coverage report
    -r, --reports       Generate test reports
    -a, --all           Run all tests and checks
    --clean             Clean build directory first

EXAMPLES:
    $0                  # Run basic tests
    $0 -a               # Run all tests with reports
    $0 -b -v -p         # Build, run tests, valgrind, and performance

EOF
}

# Parse arguments
BUILD=false
VALGRIND=false
PERFORMANCE=false
COVERAGE=false
REPORTS=false
ALL=false
CLEAN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -b|--build)
            BUILD=true
            shift
            ;;
        -v|--valgrind)
            VALGRIND=true
            shift
            ;;
        -p|--performance)
            PERFORMANCE=true
            shift
            ;;
        -c|--coverage)
            COVERAGE=true
            shift
            ;;
        -r|--reports)
            REPORTS=true
            shift
            ;;
        -a|--all)
            ALL=true
            BUILD=true
            VALGRIND=true
            PERFORMANCE=true
            COVERAGE=true
            REPORTS=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Create reports directory
mkdir -p "$REPORTS_DIR"

print_info "NeoC SDK Test Runner"
print_info "===================="
print_info "Timestamp: $TIMESTAMP"

# Clean build directory if requested
if [ "$CLEAN" = "true" ]; then
    print_info "Cleaning build directory"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    mkdir -p "$BUILD_DIR"
fi

# Build tests if requested
if [ "$BUILD" = "true" ]; then
    print_info "Building NeoC SDK and tests"
    
    cd "$BUILD_DIR"
    
    # Configure with coverage if requested
    if [ "$COVERAGE" = "true" ]; then
        cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
    else
        cmake -DCMAKE_BUILD_TYPE=Debug ..
    fi
    
    # Build main library
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    # Build tests
    cd tests
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    cd ../..
    
    print_success "Build completed"
fi

# Verify test executables exist
TEST_EXECUTABLES=(
    "$BUILD_DIR/tests/test_basic"
    "$BUILD_DIR/tests/test_memory" 
    "$BUILD_DIR/tests/test_comprehensive"
    "$BUILD_DIR/tests/test_performance"
)

for exe in "${TEST_EXECUTABLES[@]}"; do
    if [ ! -f "$exe" ]; then
        print_warning "Test executable not found: $exe"
        print_info "Run with --build to build tests first"
        # Remove from array if not found
        TEST_EXECUTABLES=(${TEST_EXECUTABLES[@]/$exe})
    fi
done

if [ ${#TEST_EXECUTABLES[@]} -eq 0 ]; then
    print_error "No test executables found"
    exit 1
fi

# Run basic tests
print_info "Running basic unit tests"

BASIC_TESTS=(
    "$BUILD_DIR/tests/test_basic"
    "$BUILD_DIR/tests/test_comprehensive"
)

TESTS_PASSED=0
TESTS_FAILED=0

for test in "${BASIC_TESTS[@]}"; do
    if [ -f "$test" ]; then
        test_name=$(basename "$test")
        print_info "Running $test_name"
        
        if $test > "$REPORTS_DIR/${test_name}_${TIMESTAMP}.log" 2>&1; then
            print_success "$test_name passed"
            TESTS_PASSED=$((TESTS_PASSED + 1))
        else
            print_error "$test_name failed"
            TESTS_FAILED=$((TESTS_FAILED + 1))
            
            # Show last few lines of output on failure
            echo "=== Last 10 lines of output ==="
            tail -10 "$REPORTS_DIR/${test_name}_${TIMESTAMP}.log"
        fi
    fi
done

# Run memory tests
if [ -f "$BUILD_DIR/tests/test_memory" ]; then
    print_info "Running memory tests"
    
    if $BUILD_DIR/tests/test_memory > "$REPORTS_DIR/test_memory_${TIMESTAMP}.log" 2>&1; then
        print_success "Memory tests passed"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        print_error "Memory tests failed"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        
        echo "=== Memory test output ==="
        tail -10 "$REPORTS_DIR/test_memory_${TIMESTAMP}.log"
    fi
fi

# Run valgrind checks
if [ "$VALGRIND" = "true" ]; then
    print_info "Running valgrind memory checks"
    
    VALGRIND_TARGETS=(
        "$BUILD_DIR/tests/test_basic"
        "$BUILD_DIR/tests/test_memory"
        "$BUILD_DIR/tests/test_comprehensive"
    )
    
    for target in "${VALGRIND_TARGETS[@]}"; do
        if [ -f "$target" ]; then
            test_name=$(basename "$target")
            print_info "Running valgrind on $test_name"
            
            if ./valgrind_test.sh -m -o "$REPORTS_DIR/valgrind" "$target"; then
                print_success "Valgrind check passed for $test_name"
            else
                print_warning "Valgrind issues detected in $test_name"
            fi
        fi
    done
fi

# Run performance benchmarks
if [ "$PERFORMANCE" = "true" ] && [ -f "$BUILD_DIR/tests/test_performance" ]; then
    print_info "Running performance benchmarks"
    
    if $BUILD_DIR/tests/test_performance > "$REPORTS_DIR/performance_${TIMESTAMP}.log" 2>&1; then
        print_success "Performance benchmarks completed"
        
        # Extract and display key performance metrics
        echo "=== Performance Summary ==="
        grep "ops/sec\|MB/sec" "$REPORTS_DIR/performance_${TIMESTAMP}.log" | head -10
    else
        print_error "Performance benchmarks failed"
        tail -20 "$REPORTS_DIR/performance_${TIMESTAMP}.log"
    fi
fi

# Generate code coverage report
if [ "$COVERAGE" = "true" ]; then
    print_info "Generating code coverage report"
    
    if command -v gcov &> /dev/null && command -v lcov &> /dev/null; then
        cd "$BUILD_DIR"
        
        # Generate coverage data
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --remove coverage.info '*/tests/*' --output-file coverage.info
        
        # Generate HTML report
        genhtml coverage.info --output-directory "$REPORTS_DIR/coverage"
        
        cd ..
        print_success "Coverage report generated: $REPORTS_DIR/coverage/index.html"
        
        # Display coverage summary
        echo "=== Coverage Summary ==="
        lcov --summary "$BUILD_DIR/coverage.info"
    else
        print_warning "gcov/lcov not available - skipping coverage report"
    fi
fi

# Generate test report
if [ "$REPORTS" = "true" ]; then
    print_info "Generating test report"
    
    REPORT_FILE="$REPORTS_DIR/test_report_${TIMESTAMP}.html"
    
    cat > "$REPORT_FILE" << EOF
<!DOCTYPE html>
<html>
<head>
    <title>NeoC SDK Test Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }
        .summary { margin: 20px 0; }
        .passed { color: green; font-weight: bold; }
        .failed { color: red; font-weight: bold; }
        .warning { color: orange; font-weight: bold; }
        .section { margin: 20px 0; padding: 10px; border: 1px solid #ddd; }
        pre { background-color: #f5f5f5; padding: 10px; overflow-x: auto; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <div class="header">
        <h1>NeoC SDK Test Report</h1>
        <p>Generated: $(date)</p>
        <p>Timestamp: ${TIMESTAMP}</p>
    </div>
    
    <div class="summary">
        <h2>Test Summary</h2>
        <p>Tests Passed: <span class="passed">$TESTS_PASSED</span></p>
        <p>Tests Failed: <span class="failed">$TESTS_FAILED</span></p>
        <p>Total Tests: $(($TESTS_PASSED + $TESTS_FAILED))</p>
    </div>
    
    <div class="section">
        <h2>Test Results</h2>
        <table>
            <tr><th>Test</th><th>Status</th><th>Log File</th></tr>
EOF

    # Add test results to report
    for log_file in "$REPORTS_DIR"/*_${TIMESTAMP}.log; do
        if [ -f "$log_file" ]; then
            test_name=$(basename "$log_file" | sed "s/_${TIMESTAMP}.log//")
            
            # Determine status from log file
            if grep -q "All tests passed\|PASS\|OK" "$log_file" 2>/dev/null; then
                status="<span class=\"passed\">PASSED</span>"
            elif grep -q "FAIL\|ERROR\|fail\|error" "$log_file" 2>/dev/null; then
                status="<span class=\"failed\">FAILED</span>"
            else
                status="<span class=\"warning\">UNKNOWN</span>"
            fi
            
            echo "<tr><td>$test_name</td><td>$status</td><td>$(basename "$log_file")</td></tr>" >> "$REPORT_FILE"
        fi
    done
    
    cat >> "$REPORT_FILE" << EOF
        </table>
    </div>
    
    <div class="section">
        <h2>System Information</h2>
        <pre>
OS: $(uname -a)
Compiler: $(gcc --version | head -1 2>/dev/null || echo "N/A")
Build Type: Debug
        </pre>
    </div>
    
    <div class="section">
        <h2>Available Log Files</h2>
        <ul>
EOF

    # List all log files
    for log_file in "$REPORTS_DIR"/*_${TIMESTAMP}.log; do
        if [ -f "$log_file" ]; then
            echo "<li><a href=\"$(basename "$log_file")\">$(basename "$log_file")</a></li>" >> "$REPORT_FILE"
        fi
    done
    
    cat >> "$REPORT_FILE" << EOF
        </ul>
    </div>
    
</body>
</html>
EOF

    print_success "Test report generated: $REPORT_FILE"
fi

# Final summary
echo ""
echo "============================================"
echo "            TEST EXECUTION SUMMARY"
echo "============================================"
echo "Tests Passed: $TESTS_PASSED"
echo "Tests Failed: $TESTS_FAILED"
echo "Reports Directory: $REPORTS_DIR"

if [ $TESTS_FAILED -eq 0 ]; then
    print_success "All tests completed successfully!"
    exit 0
else
    print_error "Some tests failed - please check the logs"
    exit 1
fi