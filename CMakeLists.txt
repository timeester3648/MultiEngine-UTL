cmake_minimum_required(VERSION 3.9.1)

project(proto_utils)

# Source
include_directories(source include)
add_subdirectory(source)
add_subdirectory(benchmarks)

# Tests
add_subdirectory(tests)