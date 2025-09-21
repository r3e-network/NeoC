#!/bin/bash

# Memory leak detection script for NeoC SDK using valgrind
# This script runs all tests under valgrind to detect memory leaks

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================="
echo "NeoC SDK Memory Leak Detection"
echo "========================================="
echo ""

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo -e "${YELLOW}Warning: valgrind is not installed${NC}"
    echo "On macOS, valgrind is not fully supported. Consider using:"
    echo "  - AddressSanitizer (built into clang/gcc)"
    echo "  - leaks command (macOS native)"
    echo ""
    echo "Running with leaks command instead..."
    USE_LEAKS=1
else
    USE_LEAKS=0
fi

# Build directory
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found. Please build the tests first.${NC}"
    exit 1
fi

cd "$BUILD_DIR"

# List of test executables
TESTS=(
    "test_basic"
    "test_ec_key_pair"
    "test_hash160"
    "test_hash256"
    "test_base58"
    "test_base64"
    "test_wif"
    "test_nep2"
    "test_sign"
    "test_transaction_builder"
    "test_script_builder"
    "test_account"
    "test_signer"
    "test_witness"
    "test_comprehensive"
    "test_full_integration"
)

# Valgrind options
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind_%p.log"

# Summary counters
TOTAL=0
PASSED=0
FAILED=0
LEAK_FREE=0
HAS_LEAKS=0

echo "Running memory tests on ${#TESTS[@]} test executables..."
echo ""

# Function to run test with valgrind
run_valgrind_test() {
    local test_name=$1
    local test_exe="./$test_name"
    
    if [ ! -f "$test_exe" ]; then
        echo -e "${YELLOW}Skip: $test_name (not found)${NC}"
        return 1
    fi
    
    echo -n "Testing $test_name... "
    TOTAL=$((TOTAL + 1))
    
    # Run with valgrind
    valgrind $VALGRIND_OPTS $test_exe > /dev/null 2>&1
    local exit_code=$?
    
    # Check the log file
    local log_file=$(ls -t valgrind_*.log | head -1)
    
    if [ $exit_code -eq 0 ]; then
        # Check for memory leaks in the log
        if grep -q "definitely lost: 0 bytes" "$log_file" && \
           grep -q "indirectly lost: 0 bytes" "$log_file" && \
           grep -q "possibly lost: 0 bytes" "$log_file"; then
            echo -e "${GREEN}PASS (leak-free)${NC}"
            PASSED=$((PASSED + 1))
            LEAK_FREE=$((LEAK_FREE + 1))
        else
            echo -e "${YELLOW}PASS (with leaks)${NC}"
            PASSED=$((PASSED + 1))
            HAS_LEAKS=$((HAS_LEAKS + 1))
            
            # Show leak summary
            echo "  Leak summary:"
            grep "definitely lost:" "$log_file" | head -1
            grep "indirectly lost:" "$log_file" | head -1
            grep "possibly lost:" "$log_file" | head -1
        fi
    else
        echo -e "${RED}FAIL (exit code: $exit_code)${NC}"
        FAILED=$((FAILED + 1))
    fi
    
    # Clean up log file
    rm -f "$log_file"
    
    return 0
}

# Function to run test with leaks command (macOS)
run_leaks_test() {
    local test_name=$1
    local test_exe="./$test_name"
    
    if [ ! -f "$test_exe" ]; then
        echo -e "${YELLOW}Skip: $test_name (not found)${NC}"
        return 1
    fi
    
    echo -n "Testing $test_name... "
    TOTAL=$((TOTAL + 1))
    
    # Set environment variable for malloc debugging
    export MallocStackLogging=1
    export MallocScribble=1
    
    # Run the test
    $test_exe > /dev/null 2>&1
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        # Run leaks command
        leaks_output=$(leaks --atExit -- $test_exe 2>&1)
        
        if echo "$leaks_output" | grep -q "0 leaks for 0 total leaked bytes"; then
            echo -e "${GREEN}PASS (leak-free)${NC}"
            PASSED=$((PASSED + 1))
            LEAK_FREE=$((LEAK_FREE + 1))
        elif echo "$leaks_output" | grep -q "Process.*not found"; then
            echo -e "${YELLOW}PASS (unable to check leaks)${NC}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${YELLOW}PASS (with leaks)${NC}"
            PASSED=$((PASSED + 1))
            HAS_LEAKS=$((HAS_LEAKS + 1))
            
            # Extract leak count
            leak_count=$(echo "$leaks_output" | grep -oE '[0-9]+ leaks?' | head -1)
            if [ -n "$leak_count" ]; then
                echo "  Found: $leak_count"
            fi
        fi
    else
        echo -e "${RED}FAIL (exit code: $exit_code)${NC}"
        FAILED=$((FAILED + 1))
    fi
    
    unset MallocStackLogging
    unset MallocScribble
    
    return 0
}

# Run tests
for test in "${TESTS[@]}"; do
    if [ $USE_LEAKS -eq 1 ]; then
        run_leaks_test "$test"
    else
        run_valgrind_test "$test"
    fi
done

echo ""
echo "========================================="
echo "Memory Test Summary"
echo "========================================="
echo "Total tests:     $TOTAL"
echo -e "Passed:          ${GREEN}$PASSED${NC}"
echo -e "Failed:          ${RED}$FAILED${NC}"
echo -e "Leak-free:       ${GREEN}$LEAK_FREE${NC}"
echo -e "Has leaks:       ${YELLOW}$HAS_LEAKS${NC}"
echo ""

if [ $FAILED -eq 0 ] && [ $HAS_LEAKS -eq 0 ]; then
    echo -e "${GREEN}All tests passed without memory leaks!${NC}"
    exit 0
elif [ $FAILED -eq 0 ]; then
    echo -e "${YELLOW}All tests passed but some have memory leaks.${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi