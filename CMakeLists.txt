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
    ${chronofs}/src/*.cpp
)
file(GLOB_RECURSE HEADERS
    ${chronofs}/src/*.hpp
)

# Create the executable
add_executable(chronofs ${SOURCES} ${HEADERS})

# Include directories for header files
target_include_directories(chronofs PRIVATE
    ${chronofs}/src
)

# Extra compiler warnings (optional but useful)
if (MSVC)
    target_compile_options(chronofs PRIVATE /W4 /permissive-)
else()
    target_compile_options(chronofs PRIVATE -Wall -Wextra -pedantic)
endif()

if (MINGW)
    target_link_libraries(${chronofs} PRIVATE stdc++fs)
endif()
