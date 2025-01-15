
# Minimum CMake version required
cmake_minimum_required(VERSION 3.30)

# Project definition
project(specbolt VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Add dependencies before we fiddle with compilation options
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

# Add packages
CPMAddPackage("gh:catchorg/Catch2@3.8.0")
CPMAddPackage("gh:microsoft/GSL@4.1.0")

# Compiler options
add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wconversion)

# Find packages
find_package(X11 REQUIRED)

# Define executable
add_executable(specbolt
    src/main.cpp
    src/ShmId.cpp
    src/Logger.cpp
    src/X11MainWindow.cpp
    src/cpu/Z80.cpp
)

# Include directories
target_include_directories(specbolt PRIVATE src)

# Link libraries
target_link_libraries(specbolt GSL X11::X11 X11::Xext)
