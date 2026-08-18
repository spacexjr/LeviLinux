cmake_minimum_required(VERSION 3.17)

project(LeviLinux VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build configuration
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

# Target architectures
set(ARCHITECTURES x86_64 aarch64)

# Platform detection and configuration
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    message(STATUS "Building for Linux")
    
    # X11/Wayland detection
    find_package(X11 QUIET)
    if(X11_FOUND)
        message(STATUS "X11 support available")
    endif()
    
    # Wayland detection
    find_package(Wayland QUIET)
    if(Wayland_FOUND)
        message(STATUS "Wayland support available")
    endif()
    
    # Audio system detection
    find_package(ALSA QUIET)
    if(ALSA_FOUND)
        message(STATUS "ALSA support available")
    endif()
    
    find_package(PulseAudio QUIET)
    if(PulseAudio_FOUND)
        message(STATUS "PulseAudio support available")
    endif()
    
    find_package(PipeWire QUIET)
    if(PipeWire_FOUND)
        message(STATUS "PipeWire support available")
    endif()
endif()

# Library loading configuration
find_package(dl QUIET)
if(dl_FOUND)
    message(STATUS "dl library support available")
endif()

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Include directories
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/loader)
add_subdirectory(src/ui)

# Create a test target
add_executable(test_core_engine
    src/test_main.cpp
)

target_link_libraries(test_core_engine
    core
    loader
    ${X11_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${PULSEAudio_LIBRARIES}
    ${PipeWire_LIBRARIES}
    ${dl_LIBRARIES}
    Qt6::Core
)

target_include_directories(test_core_engine PRIVATE include)

# Test configuration
enable_testing()
add_test(NAME TestCoreEngine COMMAND test_core_engine)