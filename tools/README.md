# NeoC SDK Tools

Comprehensive suite of development, debugging, and monitoring tools for the NeoC SDK.

## Overview

The NeoC SDK includes a collection of professional-grade tools designed to enhance development productivity, ensure code quality, and monitor blockchain operations.

## Available Tools

### 🔧 neoc-cli
**Command-line interface for SDK operations**

Provides direct access to all SDK functionality from the command line.

```bash
# Create a new wallet
neoc-cli wallet create MyWallet

# Generate a new address
neoc-cli wallet address MyWallet

# Build a transaction
neoc-cli transaction transfer --from <address> --to <address> --amount 10

# Interact with smart contracts
neoc-cli contract invoke <hash> <method> <params>
```

Features:
- Wallet management (create, import, export)
- Transaction building and signing
- Smart contract interaction
- Cryptographic operations
- Address generation and validation

### 📊 neoc-profiler
**Performance profiling and benchmarking**

Measures and analyzes the performance of all SDK operations.

```bash
# Run performance profiling
neoc-profiler -i 1000 -v

# Export results to CSV
neoc-profiler -c performance.csv

# Profile specific operations
neoc-profiler --ops crypto,transaction
```

Features:
- Cryptographic operation benchmarks
- Transaction building performance
- Memory usage tracking
- Statistical analysis (min, max, mean, percentiles)
- CSV export for analysis
- Comparative benchmarking

### 🐛 neoc-debug
**Advanced debugging and diagnostics**

Comprehensive debugging utilities for SDK development.

```bash
# Enable debug mode for an application
neoc-debug ./myapp

# Analyze memory usage
neoc-debug --memory ./myapp

# Generate stack traces
neoc-debug --backtrace ./myapp

# Hex dump analysis
neoc-debug --hexdump data.bin
```

Features:
- Stack trace generation
- Memory leak detection
- Hex dump utilities
- Signal handling
- Debug symbol analysis
- Core dump analysis

### 📡 neoc-monitor
**Real-time blockchain monitoring**

Monitor Neo blockchain network status and health.

```bash
# Dashboard mode with live updates
neoc-monitor --dashboard

# JSON output for integration
neoc-monitor --json --interval 10

# Monitor specific RPC endpoint
neoc-monitor --rpc http://seed1.neo.org:20332
```

Features:
- Real-time block height tracking
- Network peer monitoring
- Mempool analysis
- Transaction throughput metrics
- Alert system for anomalies
- Dashboard and JSON output modes

### 🔒 neoc-audit
**Security auditing and vulnerability scanning**

Comprehensive security analysis for C code.

```bash
# Audit entire project
neoc-audit src/

# Generate security report
neoc-audit --json -o security-report.json .

# Check only critical issues
neoc-audit --severity critical .
```

Features:
- Buffer overflow detection
- Format string vulnerability scanning
- SQL injection checks
- Weak cryptography identification
- Hardcoded secret detection
- OWASP compliance checking
- Multiple output formats (text, JSON, SARIF)

### 🔄 neoc-migrate
**Swift to C migration validator**

Validates the completeness and correctness of Swift to C migration.

```bash
# Validate migration
neoc-migrate ../NeoSwift ./src

# Generate migration mapping
neoc-migrate --mapping migration.md ../NeoSwift .

# Check API compatibility
neoc-migrate --check-compat ../NeoSwift .
```

Features:
- API coverage analysis
- Function signature validation
- Type mapping verification
- Migration completeness assessment
- Compatibility checking
- Progress visualization

## Installation

### Using Make

```bash
cd tools
make
sudo make install
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Individual Tool Compilation

```bash
# Compile specific tool
gcc -o neoc-cli neoc-cli.c -lneoc -lssl -lcrypto -lcurl -lpthread -lm

# Install to system
sudo cp neoc-cli /usr/local/bin/
```

## Building from Source

### Prerequisites

- GCC or Clang compiler
- OpenSSL development libraries
- libcurl development libraries
- pthread support
- NeoC SDK installed

### Ubuntu/Debian

```bash
sudo apt-get install build-essential libssl-dev libcurl4-openssl-dev
```

### macOS

```bash
brew install openssl curl
```

### RedHat/CentOS

```bash
sudo yum install gcc openssl-devel libcurl-devel
```

## Configuration

### Environment Variables

```bash
# Set default RPC endpoint
export NEOC_RPC_URL="http://localhost:20332"

# Set wallet directory
export NEOC_WALLET_DIR="$HOME/.neoc/wallets"

# Enable debug output
export NEOC_DEBUG=1
```

### Configuration Files

Tools can use configuration files in `~/.neoc/config`:

```ini
[network]
rpc_url = http://localhost:20332
timeout = 30

[profiler]
iterations = 1000
output_format = csv

[monitor]
refresh_interval = 5
alert_threshold = 10
```

## Usage Examples

### Complete Workflow Example

```bash
# 1. Create and setup wallet
neoc-cli wallet create TestWallet
neoc-cli wallet address TestWallet

# 2. Profile cryptographic operations
neoc-profiler --ops crypto -i 1000

# 3. Monitor network status
neoc-monitor --dashboard &

# 4. Audit code for security issues
neoc-audit src/ -o security.json

# 5. Validate migration completeness
neoc-migrate ../swift-sdk . --mapping

# 6. Debug application issues
neoc-debug --memory ./myapp
```

### Development Workflow

```bash
# During development
make debug              # Build with debug symbols
neoc-debug ./myapp     # Debug application
neoc-audit .           # Security check
neoc-profiler          # Performance analysis

# Before deployment
neoc-audit --severity high .
neoc-profiler -c baseline.csv
neoc-monitor --json > network-status.json
```

## Tool Options Reference

### Common Options

All tools support these common options:

- `-h, --help` - Display help message
- `-v, --verbose` - Enable verbose output
- `-V, --version` - Show version information

### Tool-Specific Options

See individual tool help (`tool --help`) for complete options.

## Integration

### CI/CD Integration

```yaml
# GitHub Actions example
- name: Security Audit
  run: neoc-audit src/ --severity high

- name: Performance Benchmark
  run: neoc-profiler -c benchmark.csv

- name: Migration Validation
  run: neoc-migrate swift/ c/ --check-compat
```

### Monitoring Integration

```bash
# Export metrics for Prometheus
neoc-monitor --json | jq '.metrics' > metrics.json

# Send alerts to webhook
neoc-monitor --webhook https://alerts.example.com
```

## Troubleshooting

### Common Issues

1. **Tool not found**: Ensure tools are in PATH
   ```bash
   export PATH=$PATH:/usr/local/bin
   ```

2. **Library errors**: Install required dependencies
   ```bash
   ldd neoc-cli  # Check missing libraries
   ```

3. **Permission denied**: Use appropriate permissions
   ```bash
   chmod +x neoc-*
   ```

## Contributing

Contributions are welcome! Please ensure:

1. Code follows C11 standards
2. All functions have proper error handling
3. Memory management is correct (no leaks)
4. Security best practices are followed
5. Documentation is updated

## License

Same as NeoC SDK - MIT License

## Support

- GitHub Issues: Report bugs and request features
- Documentation: See docs/ directory
- Examples: See examples/ directory

## Version History

- **v1.0.0** - Initial release with 6 core tools
- **v1.1.0** - Added neoc-bench and neoc-test (planned)
- **v1.2.0** - Enhanced monitoring capabilities (planned)