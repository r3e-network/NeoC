cmake_minimum_required(VERSION 3.16)
project(NeoC VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags (disable deprecated warnings for OpenSSL compatibility)
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations")

# Find required packages
find_package(OpenSSL REQUIRED)

# Try to find cJSON using pkg-config first, then manual search
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(CJSON libcjson)
endif()

# If pkg-config didn't find cJSON, try manual search
if(NOT CJSON_FOUND)
    find_path(CJSON_INCLUDE_DIR 
        NAMES cjson/cJSON.h cJSON.h
        PATHS /usr/include /usr/local/include /opt/homebrew/include
        PATH_SUFFIXES cjson
    )
    find_library(CJSON_LIBRARY 
        NAMES cjson libcjson
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
    )
    
    if(CJSON_INCLUDE_DIR AND CJSON_LIBRARY)
        set(CJSON_FOUND TRUE)
        set(CJSON_INCLUDE_DIRS ${CJSON_INCLUDE_DIR})
        set(CJSON_LIBRARIES ${CJSON_LIBRARY})
        message(STATUS "Found cJSON: ${CJSON_LIBRARY}")
    endif()
endif()

# cJSON is required for full protocol support
if(NOT CJSON_FOUND)
    message(WARNING "cJSON not found - RPC features will be disabled. Install with: brew install cjson")
    set(CJSON_INCLUDE_DIRS "")
    set(CJSON_LIBRARIES "")
else()
    message(STATUS "cJSON found: ${CJSON_INCLUDE_DIRS}")
endif()

# Find libcurl
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    message(WARNING "libcurl not found - HTTP features will be disabled")
    set(CURL_INCLUDE_DIRS "")
    set(CURL_LIBRARIES "")
endif()

# Include directories
include_directories(include)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CJSON_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

# Source files
file(GLOB_RECURSE SOURCES "src/*.c")
list(APPEND SOURCES src/script/script_builder_full.c)

# Create static library
add_library(neoc STATIC ${SOURCES})

# Link libraries
target_link_libraries(neoc 
    ${OPENSSL_LIBRARIES}
    ${CJSON_LIBRARIES}
    ${CURL_LIBRARIES}
)

# Compiler definitions
target_compile_definitions(neoc PRIVATE 
    ${CJSON_CFLAGS_OTHER}
)

# Define macros based on found libraries
if(CJSON_FOUND)
    target_compile_definitions(neoc PRIVATE HAVE_CJSON)
endif()

if(CURL_FOUND)
    target_compile_definitions(neoc PRIVATE HAVE_CURL)
endif()

# Installation
install(TARGETS neoc 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/neoc 
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Export package
export(TARGETS neoc FILE NeoCConfig.cmake)

# Create pkg-config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/neoc.pc.in 
               ${CMAKE_CURRENT_BINARY_DIR}/neoc.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/neoc.pc 
        DESTINATION lib/pkgconfig)