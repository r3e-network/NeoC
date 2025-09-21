# CMakeLists.txt for NeoC SDK Unit Tests

cmake_minimum_required(VERSION 3.10)
project(NeoC_Unit_Tests)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(../../include)

# Find all test source files
file(GLOB_RECURSE TEST_SOURCES "*.c")

# Create executables for each test file
foreach(TEST_SOURCE ${TEST_SOURCES})
    # Get the test name from the file name
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    # Create executable
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # Link with NeoC library
    target_link_libraries(${TEST_NAME} neoc)
    
    # Add as a test
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# Enable testing
enable_testing()

# Custom target to run all tests
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all unit tests"
)