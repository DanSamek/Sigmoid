cmake_minimum_required(VERSION 3.30)
project(Sigmoid)

set(CMAKE_CXX_STANDARD 20)

add_executable(Sigmoid src/main.cpp)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(Sigmoid PRIVATE -O3 -funroll-loops)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(Sigmoid PRIVATE -Wall -pedantic -fsanitize=address)
    target_link_options(Sigmoid PRIVATE -fsanitize=address)
endif()
