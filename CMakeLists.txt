cmake_minimum_required(VERSION 3.9.1)

project(proto_utils)

# Source
include_directories(source)
add_subdirectory(source)

# Tests
add_subdirectory(tests)