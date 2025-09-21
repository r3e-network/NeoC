# Makefile for NeoC SDK Unit Tests

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I../../include -g -O0
LDFLAGS = -L../../lib -lneoc -lm

# Directories
SRC_DIRS = contract crypto protocol script serialization transaction types wallet witnessrule
BUILD_DIR = build

# Find all test source files
TEST_SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/test_*.c))
TEST_TARGETS := $(patsubst %.c,$(BUILD_DIR)/%,$(TEST_SOURCES))

# Colors for output
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[1;33m
NC := \033[0m

# Default target
all: $(BUILD_DIR) $(TEST_TARGETS)
	@echo "$(GREEN)All tests compiled successfully!$(NC)"

# Create build directory structure
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@$(foreach dir,$(SRC_DIRS),mkdir -p $(BUILD_DIR)/$(dir);)

# Pattern rule for compiling tests
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%: %.c
	@echo "Building test: $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

# Run all tests
test: all
	@echo "$(YELLOW)Running all unit tests...$(NC)"
	@echo "================================"
	@failed=0; \
	total=0; \
	for test in $(TEST_TARGETS); do \
		total=$$((total + 1)); \
		echo -n "Running $$(basename $$test)... "; \
		if $$test > /dev/null 2>&1; then \
			echo "$(GREEN)PASSED$(NC)"; \
		else \
			echo "$(RED)FAILED$(NC)"; \
			failed=$$((failed + 1)); \
		fi; \
	done; \
	echo "================================"; \
	echo "Total: $$total | Passed: $$((total - failed)) | Failed: $$failed"; \
	if [ $$failed -gt 0 ]; then \
		echo "$(RED)Some tests failed!$(NC)"; \
		exit 1; \
	else \
		echo "$(GREEN)All tests passed!$(NC)"; \
	fi

# Run tests by category
test-contract: $(filter $(BUILD_DIR)/contract/%,$(TEST_TARGETS))
	@echo "Running contract tests..."
	@for test in $^; do ./$$test || exit 1; done

test-crypto: $(filter $(BUILD_DIR)/crypto/%,$(TEST_TARGETS))
	@echo "Running crypto tests..."
	@for test in $^; do ./$$test || exit 1; done

test-protocol: $(filter $(BUILD_DIR)/protocol/%,$(TEST_TARGETS))
	@echo "Running protocol tests..."
	@for test in $^; do ./$$test || exit 1; done

test-script: $(filter $(BUILD_DIR)/script/%,$(TEST_TARGETS))
	@echo "Running script tests..."
	@for test in $^; do ./$$test || exit 1; done

test-serialization: $(filter $(BUILD_DIR)/serialization/%,$(TEST_TARGETS))
	@echo "Running serialization tests..."
	@for test in $^; do ./$$test || exit 1; done

test-transaction: $(filter $(BUILD_DIR)/transaction/%,$(TEST_TARGETS))
	@echo "Running transaction tests..."
	@for test in $^; do ./$$test || exit 1; done

test-types: $(filter $(BUILD_DIR)/types/%,$(TEST_TARGETS))
	@echo "Running types tests..."
	@for test in $^; do ./$$test || exit 1; done

test-wallet: $(filter $(BUILD_DIR)/wallet/%,$(TEST_TARGETS))
	@echo "Running wallet tests..."
	@for test in $^; do ./$$test || exit 1; done

test-witnessrule: $(filter $(BUILD_DIR)/witnessrule/%,$(TEST_TARGETS))
	@echo "Running witness rule tests..."
	@for test in $^; do ./$$test || exit 1; done

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "$(GREEN)Clean complete!$(NC)"

# Generate test report
report:
	@echo "Generating test report..."
	@echo "Test Coverage Report" > test_report.txt
	@echo "===================" >> test_report.txt
	@echo "" >> test_report.txt
	@echo "Total test files: $$(find . -name 'test_*.c' | wc -l)" >> test_report.txt
	@echo "Total test methods: $$(grep -r '^static void test_' . --include='*.c' | wc -l)" >> test_report.txt
	@echo "" >> test_report.txt
	@echo "By Category:" >> test_report.txt
	@for dir in $(SRC_DIRS); do \
		count=$$(find $$dir -name 'test_*.c' 2>/dev/null | wc -l); \
		methods=$$(grep '^static void test_' $$dir/*.c 2>/dev/null | wc -l); \
		echo "  $$dir: $$count files, $$methods methods" >> test_report.txt; \
	done
	@echo "" >> test_report.txt
	@echo "Report saved to test_report.txt"
	@cat test_report.txt

# Show help
help:
	@echo "NeoC SDK Unit Tests Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make all              - Compile all tests"
	@echo "  make test             - Run all tests"
	@echo "  make test-<category>  - Run tests for specific category"
	@echo "                          (contract, crypto, protocol, script,"
	@echo "                           serialization, transaction, types, wallet, witnessrule)"
	@echo "  make clean            - Remove build artifacts"
	@echo "  make report           - Generate test coverage report"
	@echo "  make help             - Show this help message"

.PHONY: all test clean report help test-contract test-crypto test-protocol test-script test-serialization test-transaction test-types test-wallet test-witnessrule