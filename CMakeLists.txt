cmake_minimum_required(VERSION 3.10)

# Project details
project(chronofs VERSION 1.0 LANGUAGES CXX)

# Use modern C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Gather all source files
file(GLOB_RECURSE SOURCES
    src/cli/*.cpp
    src/fs/*.cpp
    src/util/*.cpp
    src/vcs/*.cpp
)

file(GLOB_RECURSE HEADERS
    src/cli/*.hpp
    src/fs/*.hpp
    src/util/*.hpp
    src/vcs/*.hpp
)

# Create the executable
add_executable(chronofs ${SOURCES} ${HEADERS})

# Include directories for header files
target_include_directories(chronofs PRIVATE
    src
    src/cli
    src/fs
    src/util
    src/vcs
)

# Extra compiler warnings (optional but useful)
if (MSVC)
    target_compile_options(chronofs PRIVATE /W4 /permissive-)
else()
    target_compile_options(chronofs PRIVATE -Wall -Wextra -pedantic)
endif()
