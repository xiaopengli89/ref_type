cmake_minimum_required(VERSION 3.14)

project(ref_type)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    debug
    examples/debug.cc
)

target_include_directories(
    debug
    PRIVATE
    "include"
)
