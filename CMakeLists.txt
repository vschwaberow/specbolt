cmake_minimum_required(VERSION 3.30)
project(specbolt)
set(CMAKE_CXX_STANDARD 26)
enable_testing()

# add dependencies before we fiddle with compilation options
set(CPM_DOWNLOAD_VERSION 0.40.5)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include(${CPM_DOWNLOAD_LOCATION})


CPMAddPackage("gh:catchorg/Catch2@3.8.0")
CPMAddPackage("gh:microsoft/GSL@4.1.0")

add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wconversion)

add_subdirectory(test)

find_package(X11 REQUIRED)
add_executable(specbolt src/main.cpp src/ShmId.cpp src/ShmId.hpp
        src/X11MainWindow.cpp
        src/X11MainWindow.hpp
        src/cpu/Z80.cpp)
target_link_libraries(specbolt GSL X11::X11 X11::Xext)
