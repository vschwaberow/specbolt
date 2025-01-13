cmake_minimum_required(VERSION 3.30)
project(specbolt)

set(CMAKE_CXX_STANDARD 26)
enable_testing()

add_subdirectory(test)

add_executable(specbolt main.cpp)

