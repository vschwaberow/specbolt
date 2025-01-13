cmake_minimum_required(VERSION 3.30)
project(specbolt)
set(CMAKE_CXX_STANDARD 26)
enable_testing()

# add dependencies
include(cmake/CPM.cmake)
CPMAddPackage("gh:catchorg/Catch2@3.8.0")

add_subdirectory(test)

add_executable(specbolt main.cpp)
