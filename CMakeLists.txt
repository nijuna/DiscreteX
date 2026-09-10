cmake_minimum_required(VERSION 3.20)
project(DiscreteX VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only interface library
add_library(discretex INTERFACE)
target_include_directories(discretex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Test executable
enable_testing()
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests PRIVATE discretex)
target_compile_options(run_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME core_tests COMMAND run_tests)
