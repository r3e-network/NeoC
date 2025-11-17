# CMakeLists.txt for NeoC SDK Unit Tests

cmake_minimum_required(VERSION 3.10)
project(NeoC_Unit_Tests)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Local Unity build for unit tests
add_library(unit_unity STATIC ../unity.c ../unity.h)
target_include_directories(unit_unity PUBLIC .. ../../include)

include_directories(..)
include_directories(../../include)

file(GLOB_RECURSE TEST_SOURCES "*.c")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    set(TARGET_NAME unit_${TEST_NAME})

    add_executable(${TARGET_NAME} ${TEST_SOURCE})
    target_link_libraries(${TARGET_NAME} unit_unity neoc OpenSSL::SSL OpenSSL::Crypto)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endforeach()

enable_testing()
