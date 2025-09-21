#!/bin/bash

##############################################################################
# NeoC SDK Installation Script
# 
# This script builds and installs the NeoC SDK on your system
##############################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Default values
PREFIX="/usr/local"
BUILD_TYPE="Release"
BUILD_TESTS="ON"
BUILD_EXAMPLES="ON"
BUILD_BENCHMARKS="OFF"
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
INSTALL_DOCS="ON"
SUDO_CMD="sudo"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix)
            PREFIX="$2"
            shift 2
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES="OFF"
            shift
            ;;
        --benchmarks)
            BUILD_BENCHMARKS="ON"
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --no-sudo)
            SUDO_CMD=""
            shift
            ;;
        --help)
            echo "NeoC SDK Installation Script"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --prefix PATH      Installation prefix (default: /usr/local)"
            echo "  --debug            Build in debug mode"
            echo "  --no-tests         Don't build tests"
            echo "  --no-examples      Don't build examples"
            echo "  --benchmarks       Build benchmarks"
            echo "  --jobs N           Number of parallel build jobs"
            echo "  --no-sudo          Don't use sudo for installation"
            echo "  --help             Show this help message"
            echo ""
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

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

print_info() {
    echo -e "${CYAN}ℹ $1${NC}"
}

check_dependency() {
    local cmd=$1
    local package=$2
    local required=${3:-true}
    
    if command -v "$cmd" &> /dev/null; then
        print_success "$cmd found"
        return 0
    else
        if [ "$required" = true ]; then
            print_error "$cmd not found. Please install $package"
            return 1
        else
            print_warning "$cmd not found. $package is optional"
            return 0
        fi
    fi
}

detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
        if [ -f /etc/debian_version ]; then
            DISTRO="debian"
        elif [ -f /etc/redhat-release ]; then
            DISTRO="redhat"
        elif [ -f /etc/arch-release ]; then
            DISTRO="arch"
        else
            DISTRO="unknown"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        DISTRO="darwin"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
        OS="windows"
        DISTRO="msys"
    else
        OS="unknown"
        DISTRO="unknown"
    fi
}

install_dependencies() {
    print_header "Installing Dependencies"
    
    detect_os
    
    case "$OS" in
        linux)
            case "$DISTRO" in
                debian)
                    print_info "Detected Debian/Ubuntu system"
                    print_info "Installing dependencies..."
                    $SUDO_CMD apt-get update
                    $SUDO_CMD apt-get install -y \
                        build-essential \
                        cmake \
                        pkg-config \
                        libssl-dev \
                        libcurl4-openssl-dev \
                        libsodium-dev \
                        valgrind \
                        lcov
                    ;;
                redhat)
                    print_info "Detected RedHat/CentOS/Fedora system"
                    $SUDO_CMD yum install -y \
                        gcc \
                        gcc-c++ \
                        make \
                        cmake \
                        pkg-config \
                        openssl-devel \
                        libcurl-devel \
                        libsodium-devel \
                        valgrind
                    ;;
                arch)
                    print_info "Detected Arch Linux system"
                    $SUDO_CMD pacman -Syu --noconfirm \
                        base-devel \
                        cmake \
                        pkg-config \
                        openssl \
                        curl \
                        libsodium \
                        valgrind
                    ;;
                *)
                    print_warning "Unknown Linux distribution"
                    print_info "Please install: cmake, openssl, curl, pkg-config"
                    ;;
            esac
            ;;
        macos)
            print_info "Detected macOS system"
            if command -v brew &> /dev/null; then
                print_info "Installing dependencies via Homebrew..."
                brew install cmake pkg-config openssl curl libsodium
            else
                print_error "Homebrew not found. Please install Homebrew first"
                exit 1
            fi
            ;;
        windows)
            print_info "Detected Windows system (MSYS/Cygwin)"
            print_warning "Please ensure you have installed:"
            print_info "  - MinGW-w64 or MSVC"
            print_info "  - CMake"
            print_info "  - OpenSSL"
            print_info "  - libcurl"
            ;;
        *)
            print_warning "Unknown operating system"
            ;;
    esac
}

##############################################################################
# Main Installation Process
##############################################################################

main() {
    print_header "NeoC SDK Installation"
    
    # Check current directory
    if [ ! -f "CMakeLists.txt" ]; then
        print_error "CMakeLists.txt not found. Please run from NeoC root directory"
        exit 1
    fi
    
    # Display configuration
    print_header "Configuration"
    print_info "Installation prefix: $PREFIX"
    print_info "Build type: $BUILD_TYPE"
    print_info "Build tests: $BUILD_TESTS"
    print_info "Build examples: $BUILD_EXAMPLES"
    print_info "Build benchmarks: $BUILD_BENCHMARKS"
    print_info "Parallel jobs: $PARALLEL_JOBS"
    
    # Check dependencies
    print_header "Checking Dependencies"
    
    check_dependency cmake CMake true || exit 1
    check_dependency gcc "GCC or Clang" true || check_dependency clang "GCC or Clang" true || exit 1
    check_dependency pkg-config pkg-config false
    check_dependency git Git false
    check_dependency valgrind Valgrind false
    check_dependency doxygen Doxygen false
    
    # Ask to install dependencies
    read -p "Do you want to install missing dependencies? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_dependencies
    fi
    
    # Create build directory
    print_header "Building NeoC SDK"
    
    BUILD_DIR="build"
    if [ -d "$BUILD_DIR" ]; then
        print_warning "Build directory exists. Cleaning..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_info "Configuring with CMake..."
    cmake .. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DBUILD_TESTS="$BUILD_TESTS" \
        -DBUILD_EXAMPLES="$BUILD_EXAMPLES" \
        -DBUILD_BENCHMARKS="$BUILD_BENCHMARKS" \
        -DBUILD_SHARED_LIBS=ON \
        -DENABLE_COVERAGE=$([ "$BUILD_TYPE" = "Debug" ] && echo "ON" || echo "OFF")
    
    if [ $? -ne 0 ]; then
        print_error "CMake configuration failed"
        exit 1
    fi
    print_success "Configuration complete"
    
    # Build
    print_info "Building NeoC SDK..."
    make -j"$PARALLEL_JOBS"
    
    if [ $? -ne 0 ]; then
        print_error "Build failed"
        exit 1
    fi
    print_success "Build complete"
    
    # Run tests if enabled
    if [ "$BUILD_TESTS" = "ON" ]; then
        print_header "Running Tests"
        ctest --output-on-failure || print_warning "Some tests failed"
    fi
    
    # Install
    print_header "Installing NeoC SDK"
    
    print_info "Installing to $PREFIX..."
    $SUDO_CMD make install
    
    if [ $? -ne 0 ]; then
        print_error "Installation failed"
        exit 1
    fi
    print_success "Installation complete"
    
    # Install pkg-config file
    if [ -f "../pkg-config/neoc.pc.in" ]; then
        print_info "Installing pkg-config file..."
        PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig"
        $SUDO_CMD mkdir -p "$PKG_CONFIG_PATH"
        $SUDO_CMD cp neoc.pc "$PKG_CONFIG_PATH/"
        print_success "pkg-config file installed"
    fi
    
    # Update library cache on Linux
    if [ "$OS" = "linux" ]; then
        print_info "Updating library cache..."
        $SUDO_CMD ldconfig
        print_success "Library cache updated"
    fi
    
    # Install documentation
    if [ "$INSTALL_DOCS" = "ON" ] && command -v doxygen &> /dev/null; then
        print_header "Generating Documentation"
        if [ -f "../Doxyfile" ]; then
            cd ..
            doxygen Doxyfile
            if [ -d "docs/html" ]; then
                DOC_DIR="${PREFIX}/share/doc/neoc"
                $SUDO_CMD mkdir -p "$DOC_DIR"
                $SUDO_CMD cp -r docs/html "$DOC_DIR/"
                print_success "Documentation installed to $DOC_DIR"
            fi
            cd "$BUILD_DIR"
        fi
    fi
    
    # Create uninstall script
    print_info "Creating uninstall script..."
    cat > uninstall.sh << 'EOF'
#!/bin/bash
# NeoC SDK Uninstall Script

PREFIX="$1"
if [ -z "$PREFIX" ]; then
    PREFIX="/usr/local"
fi

echo "Uninstalling NeoC SDK from $PREFIX..."

# Remove headers
rm -rf "$PREFIX/include/neoc"

# Remove libraries
rm -f "$PREFIX/lib/libneoc.*"
rm -f "$PREFIX/lib/pkgconfig/neoc.pc"

# Remove CMake files
rm -rf "$PREFIX/lib/cmake/NeoC"

# Remove documentation
rm -rf "$PREFIX/share/doc/neoc"

# Remove examples
rm -rf "$PREFIX/share/neoc"

# Update library cache on Linux
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    ldconfig
fi

echo "NeoC SDK has been uninstalled"
EOF
    chmod +x uninstall.sh
    $SUDO_CMD cp uninstall.sh "$PREFIX/bin/neoc-uninstall"
    print_success "Uninstall script created: $PREFIX/bin/neoc-uninstall"
    
    # Print summary
    print_header "Installation Summary"
    
    print_success "NeoC SDK has been successfully installed!"
    echo ""
    print_info "Installation details:"
    echo "  • Prefix: $PREFIX"
    echo "  • Headers: $PREFIX/include/neoc"
    echo "  • Libraries: $PREFIX/lib"
    echo "  • pkg-config: $PREFIX/lib/pkgconfig/neoc.pc"
    
    if [ "$BUILD_EXAMPLES" = "ON" ]; then
        echo "  • Examples: $PREFIX/share/neoc/examples"
    fi
    
    if [ -d "$PREFIX/share/doc/neoc" ]; then
        echo "  • Documentation: $PREFIX/share/doc/neoc"
    fi
    
    echo ""
    print_info "To use NeoC in your project:"
    echo ""
    echo "  With CMake:"
    echo "    find_package(NeoC REQUIRED)"
    echo "    target_link_libraries(your_target NeoC::NeoC)"
    echo ""
    echo "  With pkg-config:"
    echo "    gcc your_file.c \$(pkg-config --cflags --libs neoc)"
    echo ""
    echo "  Direct compilation:"
    echo "    gcc your_file.c -I$PREFIX/include/neoc -L$PREFIX/lib -lneoc"
    echo ""
    
    # Test installation
    print_header "Testing Installation"
    
    # Create a simple test program
    cat > test_install.c << 'EOF'
#include <neoc/neoc.h>
#include <stdio.h>

int main() {
    neoc_error_t err = neoc_init();
    if (err == NEOC_SUCCESS) {
        printf("NeoC SDK successfully initialized!\n");
        printf("Version: %s\n", neoc_get_version());
        neoc_cleanup();
        return 0;
    }
    return 1;
}
EOF
    
    # Try to compile the test program
    if gcc test_install.c -o test_install -I"$PREFIX/include" -L"$PREFIX/lib" -lneoc 2>/dev/null; then
        if ./test_install; then
            print_success "Installation verified successfully!"
        else
            print_warning "Installation complete but runtime test failed"
        fi
        rm -f test_install
    else
        print_warning "Installation complete but compilation test failed"
        print_info "You may need to add $PREFIX/lib to your library path"
    fi
    
    rm -f test_install.c
    
    # Final message
    echo ""
    print_success "Installation complete!"
    print_info "To uninstall, run: $PREFIX/bin/neoc-uninstall"
}

# Run main installation
main