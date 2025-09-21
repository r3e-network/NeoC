# CMakeLists.txt for NeoC SDK Tools

cmake_minimum_required(VERSION 3.10)
project(NeoC_Tools)

# Find NeoC SDK
find_package(NeoC REQUIRED)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Common compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Tool executables
set(TOOLS
    neoc-cli
    neoc-profiler
    neoc-debug
    neoc-monitor
    neoc-audit
    neoc-migrate
)

# Build each tool
foreach(tool ${TOOLS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${tool}.c")
        add_executable(${tool} ${tool}.c)
        
        # Link with NeoC SDK
        target_link_libraries(${tool} 
            NeoC::NeoC
            ${CMAKE_THREAD_LIBS_INIT}
            m
        )
        
        # Include directories
        target_include_directories(${tool} PRIVATE ${NEOC_INCLUDE_DIRS})
        
        # Install tool
        install(TARGETS ${tool}
                RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    endif()
endforeach()

# Create man pages if pandoc is available
find_program(PANDOC pandoc)
if(PANDOC)
    foreach(tool ${TOOLS})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${tool}.md")
            add_custom_command(
                OUTPUT ${tool}.1
                COMMAND ${PANDOC} -s -t man ${CMAKE_CURRENT_SOURCE_DIR}/${tool}.md -o ${tool}.1
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${tool}.md
            )
            add_custom_target(${tool}_man ALL DEPENDS ${tool}.1)
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${tool}.1
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)
        endif()
    endforeach()
endif()

# Create shell completion scripts
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/neoc-completion.bash
               ${CMAKE_CURRENT_BINARY_DIR}/neoc-completion.bash
               @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/neoc-completion.bash
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/bash-completion/completions
        RENAME neoc)