cmake_minimum_required(VERSION 3.30)
project(specbolt)
set(CMAKE_CXX_STANDARD 26)
enable_testing()

# add dependencies before we fiddle with compilation options
include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake
  GIT_TAG v0.40.5
)
FetchContent_MakeAvailable(CPM)

CPMAddPackage("gh:catchorg/Catch2@3.8.0")
CPMAddPackage("gh:microsoft/GSL@4.1.0")

add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wconversion)

add_subdirectory(test)

find_package(X11 REQUIRED)
add_executable(specbolt src/main.cpp src/ShmId.cpp src/ShmId.hpp
        src/X11MainWindow.cpp
        src/X11MainWindow.hpp)
target_link_libraries(specbolt GSL X11::X11 X11::Xext)
