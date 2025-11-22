cmake_minimum_required(VERSION 3.17)
project(flark-matrixfilter-lv2 VERSION 1.0.0 LANGUAGES C CXX)

# Find LV2
find_package(PkgConfig REQUIRED)
pkg_check_modules(LV2 REQUIRED lv2)

add_library(flark-matrixfilter-lv2 MODULE)
set_target_properties(flark-matrixfilter-lv2 PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 17
    PREFIX ""
    SUFFIX ""
)

# Include directories
target_include_directories(flark-matrixfilter-lv2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${LV2_INCLUDE_DIRS}
)

# Source files
target_sources(flark-matrixfilter-lv2 PRIVATE
    ../include/dsp.h
    ../src/dsp.cpp
    ../src/gui.h
    ../src/gui.cpp
    matrixfilter-lv2.cpp
)

# Link libraries
target_link_libraries(flark-matrixfilter-lv2 PRIVATE
    ${LV2_LIBRARIES}
    m
    pthread
)

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenGL REQUIRED)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE OpenGL::GL)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE Threads::Threads)
    target_sources(flark-matrixfilter-lv2 PRIVATE matrixfilter-ui-lv2.cpp)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE flark-matrixfilter-lv2-ui)
    
    # Create separate UI library
    add_library(flark-matrixfilter-lv2-ui SHARED)
    set_target_properties(flark-matrixfilter-lv2-ui PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        PREFIX ""
        SUFFIX ""
    )
    
    target_include_directories(flark-matrixfilter-lv2-ui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${LV2_INCLUDE_DIRS}
    )
    
    target_sources(flark-matrixfilter-lv2-ui PRIVATE
        ../include/dsp.h
        ../src/dsp.cpp
        ../src/gui.h
        ../src/gui.cpp
        matrixfilter-ui-lv2.cpp
    )
    
    target_link_libraries(flark-matrixfilter-lv2-ui PRIVATE
        ${LV2_LIBRARIES}
        OpenGL::GL
        m
        pthread
    )
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(OpenGL REQUIRED)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE OpenGL::GL)
    target_sources(flark-matrixfilter-lv2 PRIVATE matrixfilter-ui-lv2.cpp)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE flark-matrixfilter-lv2-ui)
    
    # Create separate UI library
    add_library(flark-matrixfilter-lv2-ui SHARED)
    set_target_properties(flark-matrixfilter-lv2-ui PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        PREFIX ""
        SUFFIX ""
    )
    
    target_include_directories(flark-matrixfilter-lv2-ui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${LV2_INCLUDE_DIRS}
    )
    
    target_sources(flark-matrixfilter-lv2-ui PRIVATE
        ../include/dsp.h
        ../src/dsp.cpp
        ../src/gui.h
        ../src/gui.cpp
        matrixfilter-ui-lv2.cpp
    )
    
    target_link_libraries(flark-matrixfilter-lv2-ui PRIVATE
        ${LV2_LIBRARIES}
        OpenGL::GL
        m
    )
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE OpenGL::GL)
    target_sources(flark-matrixfilter-lv2 PRIVATE matrixfilter-ui-lv2.cpp)
    target_link_libraries(flark-matrixfilter-lv2 PRIVATE flark-matrixfilter-lv2-ui)
    
    # Create separate UI library
    add_library(flark-matrixfilter-lv2-ui SHARED)
    set_target_properties(flark-matrixfilter-lv2-ui PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        PREFIX ""
        SUFFIX ""
    )
    
    target_include_directories(flark-matrixfilter-lv2-ui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${LV2_INCLUDE_DIRS}
    )
    
    target_sources(flark-matrixfilter-lv2-ui PRIVATE
        ../include/dsp.h
        ../src/dsp.cpp
        ../src/gui.h
        ../src/gui.cpp
        matrixfilter-ui-lv2.cpp
    )
    
    target_link_libraries(flark-matrixfilter-lv2-ui PRIVATE
        ${LV2_LIBRARIES}
        OpenGL::GL
        m
    )
endif()

# LV2-specific settings
set_target_properties(flark-matrixfilter-lv2 PROPERTIES
    SUFFIX ".so"
)

set_target_properties(flark-matrixfilter-lv2-ui PROPERTIES
    SUFFIX ".so"
)

# Installation
install(TARGETS flark-matrixfilter-lv2 flark-matrixfilter-lv2-ui DESTINATION .)
install(FILES manifest.ttl flark-matrixfilter.ttl flark-matrixfilter-ui.ttl DESTINATION .)
install(FILES README.md DESTINATION .)

# Create LV2 bundle structure
set(LV2_BUNDLE_DIR "${CMAKE_CURRENT_BINARY_DIR}/flark-matrixfilter.lv2")
add_custom_command(TARGET flark-matrixfilter-lv2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LV2_BUNDLE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:flark-matrixfilter-lv2>
        "${LV2_BUNDLE_DIR}/flark-matrixfilter.so"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:flark-matrixfilter-lv2-ui>
        "${LV2_BUNDLE_DIR}/flark-matrixfilter-ui.so"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/manifest.ttl"
        "${LV2_BUNDLE_DIR}/manifest.ttl"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/flark-matrixfilter.ttl"
        "${LV2_BUNDLE_DIR}/flark-matrixfilter.ttl"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/flark-matrixfilter-ui.ttl"
        "${LV2_BUNDLE_DIR}/flark-matrixfilter-ui.ttl"
)

add_custom_target(lv2_bundle ALL
    DEPENDS flark-matrixfilter-lv2 flark-matrixfilter-lv2-ui
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)