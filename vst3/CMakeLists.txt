cmake_minimum_required(VERSION 3.17)
project(flark-matrixfilter-vst3 VERSION 1.0.0 LANGUAGES C CXX)

# Find VST3 SDK
find_package(PkgConfig REQUIRED)
pkg_check_modules(VST3 REQUIRED vst3sdk)

add_library(flark-matrixfilter-vst3 MODULE)
set_target_properties(flark-matrixfilter-vst3 PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 17
    BUNDLE TRUE
    BUNDLE_EXTENSION vst3
    BUNDLE_NAME "flark-MatrixFilter"
)

# Include directories
target_include_directories(flark-matrixfilter-vst3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${VST3_INCLUDE_DIRS}
)

# Source files
target_sources(flark-matrixfilter-vst3 PRIVATE
    ../include/dsp.h
    ../src/dsp.cpp
    ../src/gui.h
    ../src/gui.cpp
    plugin.cpp
    processor.cpp
    controller.cpp
    editcontroller.cpp
    vst3plugin.cpp
    vst3processor.cpp
    vst3controller.cpp
)

# Link libraries
target_link_libraries(flark-matrixfilter-vst3 PRIVATE
    ${VST3_LIBRARIES}
    m
    pthread
)

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenGL REQUIRED)
    target_link_libraries(flark-matrixfilter-vst3 PRIVATE OpenGL::GL)
    target_link_libraries(flark-matrixfilter-vst3 PRIVATE Threads::Threads)
    set_target_properties(flark-matrixfilter-vst3 PROPERTIES PREFIX "")
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(OpenGL REQUIRED)
    target_link_libraries(flark-matrixfilter-vst3 PRIVATE OpenGL::GL)
    set_target_properties(flark-matrixfilter-vst3 PROPERTIES PREFIX "")
    set_target_properties(flark-matrixfilter-vst3 PROPERTIES BUNDLE_EXTENSION "vst3")
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_link_libraries(flark-matrixfilter-vst3 PRIVATE OpenGL::GL)
    set_target_properties(flark-matrixfilter-vst3 PROPERTIES PREFIX "")
    set_target_properties(flark-matrixfilter-vst3 PROPERTIES BUNDLE_EXTENSION "vst3")
endif()

# VST3 specific settings
set_target_properties(flark-matrixfilter-vst3 PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "flark-MatrixFilter"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.flark.matrixfilter.vst3"
)

# Installation
install(TARGETS flark-matrixfilter-vst3 DESTINATION .)
install(FILES README.md DESTINATION .)