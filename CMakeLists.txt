cmake_minimum_required(VERSION 3.30)
project(Sigmoid)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-O3)
    add_definitions(-funroll-loops)
endif()

add_executable(Sigmoid src/main.cpp)
