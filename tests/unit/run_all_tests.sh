#!/bin/bash

# NeoC SDK Unit Test Runner Script
# This script runs all unit tests and generates a report

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "================================================"
echo "         NeoC SDK Unit Test Suite"
echo "================================================"
echo ""

# Track statistics
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
FAILED_LIST=""

# Function to run a test
run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file" .c)
    
    echo -n "Running $test_name... "
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Compile the test if not already compiled
    if [ ! -f "$test_name" ] || [ "$test_file" -nt "$test_name" ]; then
        gcc -o "$test_name" "$test_file" -I../../include -L../../lib -lneoc -lm 2>/dev/null || {
            echo -e "${RED}COMPILATION FAILED${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
            FAILED_LIST="$FAILED_LIST\n  - $test_name (compilation)"
            return 1
        }
    fi
    
    # Run the test
    if ./"$test_name" > /dev/null 2>&1; then
        echo -e "${GREEN}PASSED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}FAILED${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_LIST="$FAILED_LIST\n  - $test_name (runtime)"
    fi
}

# Find and run all test files by category
echo "Running Contract Tests..."
echo "------------------------"
for test in contract/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Crypto Tests..."
echo "----------------------"
for test in crypto/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Protocol Tests..."
echo "------------------------"
for test in protocol/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Script Tests..."
echo "----------------------"
for test in script/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Serialization Tests..."
echo "-----------------------------"
for test in serialization/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Transaction Tests..."
echo "---------------------------"
for test in transaction/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Types Tests..."
echo "---------------------"
for test in types/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Wallet Tests..."
echo "----------------------"
for test in wallet/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

echo "Running Witness Rule Tests..."
echo "----------------------------"
for test in witnessrule/test_*.c; do
    [ -f "$test" ] && run_test "$test"
done
echo ""

# Print summary
echo "================================================"
echo "                Test Summary"
echo "================================================"
echo -e "Total Tests:  $TOTAL_TESTS"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "\n${YELLOW}Failed Tests:${NC}"
    echo -e "$FAILED_LIST"
    exit 1
else
    echo -e "\n${GREEN}All tests passed successfully!${NC}"
fi

echo "================================================"

# Generate detailed report
REPORT_FILE="test_report_$(date +%Y%m%d_%H%M%S).txt"
echo "Generating detailed report: $REPORT_FILE"

{
    echo "NeoC SDK Test Report"
    echo "Generated: $(date)"
    echo ""
    echo "Test Statistics:"
    echo "  Total Tests: $TOTAL_TESTS"
    echo "  Passed: $PASSED_TESTS"
    echo "  Failed: $FAILED_TESTS"
    echo ""
    echo "Test Categories:"
    echo "  Contract: $(find contract -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Crypto: $(find crypto -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Protocol: $(find protocol -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Script: $(find script -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Serialization: $(find serialization -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Transaction: $(find transaction -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Types: $(find types -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Wallet: $(find wallet -name "test_*.c" 2>/dev/null | wc -l) files"
    echo "  Witness Rule: $(find witnessrule -name "test_*.c" 2>/dev/null | wc -l) files"
} > "$REPORT_FILE"

echo "Report saved to: $REPORT_FILE"