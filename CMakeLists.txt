cmake_minimum_required(VERSION 3.17)
project(MatrixFilter VERSION 1.0.0 LANGUAGES C CXX)

# Project description
message(STATUS "Building flark's MatrixFilter Plugin Suite")
message(STATUS "Formats: VST3, LV2")

# Build options
option(BUILD_VST3 "Build VST3 plugin" ON)
option(BUILD_LV2 "Build LV2 plugin" ON)

# Check for required tools
find_package(PkgConfig REQUIRED)

# Build VST3 if enabled
if(BUILD_VST3)
    message(STATUS "VST3 plugin enabled")
    add_subdirectory(vst3)
endif()

# Build LV2 if enabled
if(BUILD_LV2)
    message(STATUS "LV2 plugin enabled")
    add_subdirectory(lv2)
endif()

# Installation
install(FILES README.md FORMATS.md QUICKSTART.md DESTINATION .)
