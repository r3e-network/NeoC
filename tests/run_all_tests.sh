#!/bin/bash

##############################################################################
# NeoC SDK Comprehensive Test Runner
# 
# This script runs all test suites for the NeoC SDK including:
# - Unit tests
# - Integration tests  
# - Performance benchmarks
# - Error handling tests
# - End-to-end scenarios
##############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="${BUILD_DIR:-build}"
TEST_RESULTS_DIR="${TEST_RESULTS_DIR:-test_results}"
COVERAGE_DIR="${COVERAGE_DIR:-coverage}"
VERBOSE="${VERBOSE:-0}"
PARALLEL="${PARALLEL:-1}"
TIMEOUT="${TIMEOUT:-300}"

# Test categories
RUN_UNIT="${RUN_UNIT:-1}"
RUN_INTEGRATION="${RUN_INTEGRATION:-1}"
RUN_BENCHMARKS="${RUN_BENCHMARKS:-1}"
RUN_E2E="${RUN_E2E:-1}"
RUN_ERROR="${RUN_ERROR:-1}"

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

##############################################################################
# Helper Functions
##############################################################################

print_header() {
    echo -e "${BLUE}============================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}============================================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

check_dependencies() {
    print_header "Checking Dependencies"
    
    # Check for required tools
    local deps_ok=1
    
    if ! command -v cmake &> /dev/null; then
        print_error "cmake not found"
        deps_ok=0
    else
        print_success "cmake found: $(cmake --version | head -n1)"
    fi
    
    if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
        print_error "No C compiler found"
        deps_ok=0
    else
        print_success "C compiler found"
    fi
    
    if ! command -v gcov &> /dev/null && ! command -v llvm-cov &> /dev/null; then
        print_warning "Coverage tools not found - coverage disabled"
    fi
    
    if [ $deps_ok -eq 0 ]; then
        echo "Please install missing dependencies"
        exit 1
    fi
}

setup_directories() {
    print_header "Setting Up Test Environment"
    
    # Create necessary directories
    mkdir -p "$TEST_RESULTS_DIR"
    mkdir -p "$COVERAGE_DIR"
    mkdir -p "$BUILD_DIR"
    
    print_success "Created test directories"
}

build_project() {
    print_header "Building NeoC SDK"
    
    cd "$BUILD_DIR"
    
    # Configure with testing enabled
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DBUILD_BENCHMARKS=ON \
        -DENABLE_COVERAGE=ON \
        -DCMAKE_C_FLAGS="-g -O0 --coverage" \
        -DCMAKE_EXE_LINKER_FLAGS="--coverage"
    
    # Build
    if [ "$PARALLEL" -eq 1 ]; then
        make -j$(nproc)
    else
        make
    fi
    
    cd ..
    print_success "Build completed"
}

run_unit_tests() {
    if [ "$RUN_UNIT" -ne 1 ]; then
        print_warning "Unit tests skipped"
        return
    fi
    
    print_header "Running Unit Tests"
    
    local test_output="$TEST_RESULTS_DIR/unit_tests.xml"
    local failed=0
    
    # Run all unit tests with CTest
    cd "$BUILD_DIR"
    if ctest --output-on-failure --timeout "$TIMEOUT" -T Test; then
        print_success "Unit tests passed"
        ((PASSED_TESTS++))
    else
        print_error "Some unit tests failed"
        ((FAILED_TESTS++))
        failed=1
    fi
    cd ..
    
    # Generate coverage report for unit tests
    if command -v gcov &> /dev/null; then
        gcov -o "$BUILD_DIR" src/**/*.c > "$COVERAGE_DIR/unit_coverage.txt" 2>&1
        print_success "Unit test coverage generated"
    fi
    
    ((TOTAL_TESTS++))
    return $failed
}

run_integration_tests() {
    if [ "$RUN_INTEGRATION" -ne 1 ]; then
        print_warning "Integration tests skipped"
        return
    fi
    
    print_header "Running Integration Tests"
    
    local test_suites=(
        "test_wallet_integration"
        "test_smart_contract_integration"
        "test_transaction_integration"
        "test_error_handling"
    )
    
    local suite_failed=0
    
    for suite in "${test_suites[@]}"; do
        echo -e "\n${BLUE}Running: $suite${NC}"
        
        if [ -x "$BUILD_DIR/tests/integration/$suite" ]; then
            if timeout "$TIMEOUT" "$BUILD_DIR/tests/integration/$suite" > "$TEST_RESULTS_DIR/$suite.log" 2>&1; then
                print_success "$suite completed"
                ((PASSED_TESTS++))
            else
                print_error "$suite failed"
                ((FAILED_TESTS++))
                suite_failed=1
            fi
        else
            print_warning "$suite not found or not executable"
            ((SKIPPED_TESTS++))
        fi
        ((TOTAL_TESTS++))
    done
    
    return $suite_failed
}

run_benchmarks() {
    if [ "$RUN_BENCHMARKS" -ne 1 ]; then
        print_warning "Benchmarks skipped"
        return
    fi
    
    print_header "Running Performance Benchmarks"
    
    local benchmarks=(
        "benchmark_crypto"
        "benchmark_serialization"
    )
    
    local bench_failed=0
    
    for benchmark in "${benchmarks[@]}"; do
        echo -e "\n${BLUE}Running: $benchmark${NC}"
        
        if [ -x "$BUILD_DIR/tests/benchmarks/$benchmark" ]; then
            if "$BUILD_DIR/tests/benchmarks/$benchmark" > "$TEST_RESULTS_DIR/$benchmark.txt"; then
                print_success "$benchmark completed"
                
                # Extract and display key metrics
                if [ "$VERBOSE" -eq 1 ]; then
                    grep -E "ops/sec|μs/op" "$TEST_RESULTS_DIR/$benchmark.txt" | head -5
                fi
                ((PASSED_TESTS++))
            else
                print_error "$benchmark failed"
                ((FAILED_TESTS++))
                bench_failed=1
            fi
        else
            print_warning "$benchmark not found"
            ((SKIPPED_TESTS++))
        fi
        ((TOTAL_TESTS++))
    done
    
    return $bench_failed
}

run_e2e_tests() {
    if [ "$RUN_E2E" -ne 1 ]; then
        print_warning "End-to-end tests skipped"
        return
    fi
    
    print_header "Running End-to-End Tests"
    
    if [ -x "$BUILD_DIR/tests/integration/test_e2e_scenarios" ]; then
        if timeout "$TIMEOUT" "$BUILD_DIR/tests/integration/test_e2e_scenarios" > "$TEST_RESULTS_DIR/e2e_scenarios.log" 2>&1; then
            print_success "E2E scenarios passed"
            ((PASSED_TESTS++))
        else
            print_error "E2E scenarios failed"
            ((FAILED_TESTS++))
            return 1
        fi
    else
        print_warning "E2E tests not found"
        ((SKIPPED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    
    return 0
}

run_error_handling_tests() {
    if [ "$RUN_ERROR" -ne 1 ]; then
        print_warning "Error handling tests skipped"
        return
    fi
    
    print_header "Running Error Handling Tests"
    
    if [ -x "$BUILD_DIR/tests/integration/test_error_handling" ]; then
        if timeout "$TIMEOUT" "$BUILD_DIR/tests/integration/test_error_handling" > "$TEST_RESULTS_DIR/error_handling.log" 2>&1; then
            print_success "Error handling tests passed"
            ((PASSED_TESTS++))
        else
            print_error "Error handling tests failed"
            ((FAILED_TESTS++))
            return 1
        fi
    else
        print_warning "Error handling tests not found"
        ((SKIPPED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    
    return 0
}

check_memory_leaks() {
    print_header "Checking for Memory Leaks"
    
    if ! command -v valgrind &> /dev/null; then
        print_warning "Valgrind not found - memory leak check skipped"
        return
    fi
    
    # Run a sample test with valgrind
    if [ -x "$BUILD_DIR/tests/unit/test_crypto" ]; then
        valgrind --leak-check=full \
                 --show-leak-kinds=all \
                 --track-origins=yes \
                 --verbose \
                 --log-file="$TEST_RESULTS_DIR/valgrind.log" \
                 "$BUILD_DIR/tests/unit/test_crypto" > /dev/null 2>&1
        
        if grep -q "definitely lost: 0 bytes" "$TEST_RESULTS_DIR/valgrind.log"; then
            print_success "No memory leaks detected"
        else
            print_warning "Potential memory leaks found (see valgrind.log)"
        fi
    fi
}

generate_coverage_report() {
    print_header "Generating Coverage Report"
    
    if command -v lcov &> /dev/null; then
        # Generate coverage info
        lcov --capture --directory "$BUILD_DIR" --output-file "$COVERAGE_DIR/coverage.info"
        
        # Generate HTML report
        genhtml "$COVERAGE_DIR/coverage.info" --output-directory "$COVERAGE_DIR/html"
        
        # Display summary
        lcov --summary "$COVERAGE_DIR/coverage.info"
        
        print_success "Coverage report generated in $COVERAGE_DIR/html"
    elif command -v gcov &> /dev/null; then
        # Basic coverage with gcov
        find "$BUILD_DIR" -name "*.gcda" -exec gcov {} \; > "$COVERAGE_DIR/gcov_summary.txt" 2>&1
        
        # Extract coverage percentage
        if [ -f "$COVERAGE_DIR/gcov_summary.txt" ]; then
            grep -E "Lines executed:" "$COVERAGE_DIR/gcov_summary.txt" | tail -1
        fi
        
        print_success "Basic coverage report generated"
    else
        print_warning "Coverage tools not available"
    fi
}

print_summary() {
    print_header "Test Summary"
    
    echo "Total Tests Run: $TOTAL_TESTS"
    echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"
    
    if [ $FAILED_TESTS -gt 0 ]; then
        echo -e "${RED}Failed: $FAILED_TESTS${NC}"
    else
        echo "Failed: $FAILED_TESTS"
    fi
    
    if [ $SKIPPED_TESTS -gt 0 ]; then
        echo -e "${YELLOW}Skipped: $SKIPPED_TESTS${NC}"
    else
        echo "Skipped: $SKIPPED_TESTS"
    fi
    
    # Calculate pass rate
    if [ $TOTAL_TESTS -gt 0 ]; then
        local pass_rate=$((PASSED_TESTS * 100 / TOTAL_TESTS))
        echo -e "\nPass Rate: ${pass_rate}%"
        
        if [ $pass_rate -ge 90 ]; then
            echo -e "${GREEN}✓ Excellent test coverage!${NC}"
        elif [ $pass_rate -ge 75 ]; then
            echo -e "${YELLOW}⚠ Good test coverage, but improvements needed${NC}"
        else
            echo -e "${RED}✗ Test coverage needs significant improvement${NC}"
        fi
    fi
    
    # Generate timestamp
    echo -e "\nTest Run Completed: $(date)"
    
    # Save summary to file
    {
        echo "NeoC SDK Test Results"
        echo "====================="
        echo "Date: $(date)"
        echo "Total: $TOTAL_TESTS"
        echo "Passed: $PASSED_TESTS"
        echo "Failed: $FAILED_TESTS"
        echo "Skipped: $SKIPPED_TESTS"
        echo "Pass Rate: ${pass_rate}%"
    } > "$TEST_RESULTS_DIR/summary.txt"
}

cleanup() {
    print_header "Cleaning Up"
    
    # Clean temporary files
    find . -name "*.gcda" -delete 2>/dev/null || true
    find . -name "*.gcno" -delete 2>/dev/null || true
    find . -name "*.gcov" -delete 2>/dev/null || true
    
    print_success "Cleanup completed"
}

##############################################################################
# Main Execution
##############################################################################

main() {
    echo "======================================================================"
    echo "                    NeoC SDK Test Suite Runner                       "
    echo "======================================================================"
    echo
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --verbose)
                VERBOSE=1
                shift
                ;;
            --no-parallel)
                PARALLEL=0
                shift
                ;;
            --unit-only)
                RUN_INTEGRATION=0
                RUN_BENCHMARKS=0
                RUN_E2E=0
                RUN_ERROR=0
                shift
                ;;
            --integration-only)
                RUN_UNIT=0
                RUN_BENCHMARKS=0
                RUN_E2E=0
                RUN_ERROR=0
                shift
                ;;
            --benchmarks-only)
                RUN_UNIT=0
                RUN_INTEGRATION=0
                RUN_E2E=0
                RUN_ERROR=0
                shift
                ;;
            --help)
                echo "Usage: $0 [options]"
                echo "Options:"
                echo "  --verbose          Show detailed output"
                echo "  --no-parallel      Disable parallel builds"
                echo "  --unit-only        Run only unit tests"
                echo "  --integration-only Run only integration tests"
                echo "  --benchmarks-only  Run only benchmarks"
                echo "  --help            Show this help message"
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # Run test pipeline
    check_dependencies
    setup_directories
    build_project
    
    # Run test suites
    run_unit_tests
    run_integration_tests
    run_benchmarks
    run_e2e_tests
    run_error_handling_tests
    
    # Additional checks
    check_memory_leaks
    generate_coverage_report
    
    # Print results
    print_summary
    
    # Cleanup if not verbose
    if [ "$VERBOSE" -eq 0 ]; then
        cleanup
    fi
    
    # Exit with appropriate code
    if [ $FAILED_TESTS -gt 0 ]; then
        exit 1
    else
        exit 0
    fi
}

# Run main function
main "$@"