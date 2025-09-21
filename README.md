# NeoC SDK

A comprehensive C library for Neo blockchain development providing core functionality for building Neo applications, smart contracts, wallets, and blockchain integrations.

## Features

### Core Types
- **Hash160**: 20-byte hashes for script hashes and addresses
- **Hash256**: 32-byte hashes for transactions and blocks
- **Bytes**: Dynamic byte array management with memory safety

### Cryptographic Operations
- SHA-256 and double SHA-256 hashing
- RIPEMD-160 hashing
- Hash160 computation (SHA-256 + RIPEMD-160)
- HMAC-SHA256
- OpenSSL-based implementation for security and performance

### Encoding Utilities
- **Hexadecimal**: Encode/decode binary data to/from hex strings
- **Base58**: Bitcoin-style Base58 encoding/decoding
- **Base58Check**: Base58 with checksum validation
- **Base64**: Standard and URL-safe Base64 encoding/decoding

### Memory Management
- Safe memory allocation with leak detection (debug mode)
- Secure memory clearing for sensitive data
- Custom allocator support
- Thread-safe operations

### Error Handling
- Comprehensive error codes and messages
- Thread-local error information
- Stack trace support for debugging
- Detailed error context

## Quick Start

### Installation on macOS

```bash
# Install dependencies
brew install cmake openssl cjson curl

# Clone the repository
git clone https://github.com/yourusername/NeoSwift.git
cd NeoSwift/NeoC

# Build the SDK
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8

# Run tests
./tests/test_basic
./tests/test_comprehensive

# Install (optional)
sudo make install
```

### Installation on Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y cmake build-essential libssl-dev libcjson-dev libcurl4-openssl-dev

# Clone and build
git clone https://github.com/yourusername/NeoSwift.git
cd NeoSwift/NeoC
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8

# Run tests
./tests/test_basic
./tests/test_comprehensive

# Install (optional)
sudo make install
```

## Building

### Prerequisites
- CMake 3.16 or later
- C99-compatible compiler (GCC 7+, Clang 3.4+, MSVC 2015+)
- OpenSSL 1.1.1+ or 3.x development libraries
- cJSON library (optional, for RPC client)
- libcurl library (optional, for HTTP support)

### Build Instructions

```bash
# Clone and navigate to NeoC directory
cd NeoSwift/NeoC

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the library
make

# Run tests
make test

# Install (optional)
sudo make install
```

### Build Options

```bash
# Debug build with memory leak detection
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Disable examples
cmake -DBUILD_EXAMPLES=OFF ..

# Disable tests
cmake -DBUILD_TESTS=OFF ..
```

## Quick Start

### Basic Usage

```c
#include "neoc/neoc.h"

int main(void) {
    // Initialize NeoC SDK
    if (neoc_init() != NEOC_SUCCESS) {
        return 1;
    }
    
    // Create Hash160 from hex string
    neoc_hash160_t hash;
    if (neoc_hash160_from_hex(&hash, "17694821c6e3ea8b7a7d770952e7de86c73d94c3") == NEOC_SUCCESS) {
        // Convert to Neo address
        char address[64];
        if (neoc_hash160_to_address(&hash, address, sizeof(address)) == NEOC_SUCCESS) {
            printf("Address: %s\n", address);
        }
    }
    
    // Cleanup
    neoc_cleanup();
    return 0;
}
```

### Hash Operations

```c
// Create Hash256 from data
const char* data = "Hello, Neo!";
neoc_hash256_t hash;
neoc_hash256_from_data_hash(&hash, (const uint8_t*)data, strlen(data));

// Convert to hex string
char hex_string[65];
neoc_hash256_to_hex(&hash, hex_string, sizeof(hex_string), false);
printf("SHA-256: %s\n", hex_string);
```

### Encoding Examples

```c
// Hex encoding
const uint8_t data[] = {0xde, 0xad, 0xbe, 0xef};
char* hex = neoc_hex_encode_alloc(data, 4, false, false);
printf("Hex: %s\n", hex);  // Output: deadbeef
neoc_free(hex);

// Base58 encoding
char* base58 = neoc_base58_encode_alloc(data, 4);
printf("Base58: %s\n", base58);
neoc_free(base58);

// Base64 encoding
char* base64 = neoc_base64_encode_alloc(data, 4);
printf("Base64: %s\n", base64);
neoc_free(base64);
```

## Examples

The `examples/` directory contains comprehensive examples:

- `basic_example.c`: Core SDK functionality
- `encoding_example.c`: Encoding utilities
- `address_example.c`: Address operations

Build and run examples:

```bash
cd build
./examples/basic_example
./examples/encoding_example
```

## Testing

Run the test suite:

```bash
cd build
make test

# Or run tests directly
./tests/test_basic
```

Debug builds include memory leak detection:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./tests/test_memory
```

## API Reference

### Core Functions

```c
// SDK initialization
neoc_error_t neoc_init(void);
void neoc_cleanup(void);
const char* neoc_get_version(void);

// Error handling
const char* neoc_error_string(neoc_error_t error_code);
bool neoc_is_success(neoc_error_t error_code);
```

### Hash160 Operations

```c
// Creation
neoc_error_t neoc_hash160_from_hex(neoc_hash160_t* hash, const char* hex_string);
neoc_error_t neoc_hash160_from_address(neoc_hash160_t* hash, const char* address);
neoc_error_t neoc_hash160_from_script(neoc_hash160_t* hash, const uint8_t* script, size_t length);

// Conversion
neoc_error_t neoc_hash160_to_hex(const neoc_hash160_t* hash, char* buffer, size_t buffer_size, bool uppercase);
neoc_error_t neoc_hash160_to_address(const neoc_hash160_t* hash, char* buffer, size_t buffer_size);

// Comparison
bool neoc_hash160_equal(const neoc_hash160_t* a, const neoc_hash160_t* b);
int neoc_hash160_compare(const neoc_hash160_t* a, const neoc_hash160_t* b);
```

### Hash256 Operations

```c
// Creation
neoc_error_t neoc_hash256_from_hex(neoc_hash256_t* hash, const char* hex_string);
neoc_error_t neoc_hash256_from_data_hash(neoc_hash256_t* hash, const uint8_t* data, size_t length);
neoc_error_t neoc_hash256_from_data_double_hash(neoc_hash256_t* hash, const uint8_t* data, size_t length);

// Conversion
neoc_error_t neoc_hash256_to_hex(const neoc_hash256_t* hash, char* buffer, size_t buffer_size, bool uppercase);
```

### Encoding Functions

```c
// Hexadecimal
neoc_error_t neoc_hex_encode(const uint8_t* data, size_t length, char* buffer, size_t buffer_size, bool uppercase, bool prefix);
neoc_error_t neoc_hex_decode(const char* hex_string, uint8_t* buffer, size_t buffer_size, size_t* decoded_length);

// Base58
char* neoc_base58_encode_alloc(const uint8_t* data, size_t length);
uint8_t* neoc_base58_decode_alloc(const char* base58_string, size_t* decoded_length);

// Base58Check
char* neoc_base58_check_encode_alloc(const uint8_t* data, size_t length);
uint8_t* neoc_base58_check_decode_alloc(const char* base58_string, size_t* decoded_length);

// Base64
char* neoc_base64_encode_alloc(const uint8_t* data, size_t length);
uint8_t* neoc_base64_decode_alloc(const char* base64_string, size_t* decoded_length);
```

## Memory Management

NeoC provides safe memory management:

```c
// Allocation
void* ptr = neoc_malloc(size);
void* array = neoc_calloc(count, size);

// Secure deallocation (clears memory)
neoc_secure_free(ptr, size);

// Regular deallocation
neoc_free(ptr);
```

## Error Handling

All functions return `neoc_error_t` codes:

```c
neoc_error_t result = neoc_hash160_from_hex(&hash, hex_string);
if (result != NEOC_SUCCESS) {
    printf("Error: %s\n", neoc_error_string(result));
    return result;
}
```

## Thread Safety

NeoC is designed to be thread-safe:
- Memory management functions are thread-safe
- Error handling uses thread-local storage
- Crypto functions can be used from multiple threads

## Contributing

1. Follow C11 standards
2. Include comprehensive tests
3. Document all public APIs
4. Use consistent naming conventions
5. Ensure memory safety and leak-free code

## License

[Add appropriate license information]

## Support

For issues and questions:
- Check the examples in `examples/`
- Review test cases in `tests/`
- Consult the API documentation in header files