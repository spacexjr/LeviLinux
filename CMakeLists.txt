cmake_minimum_required(VERSION 3.22)
project(LeviLinux VERSION 1.0.0 LANGUAGES CXX C)

# ─────────────────────────────────────────────────────────────────────────────
# PROJECT CONFIGURATION
# ─────────────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DLEVILINUX_LINUX=1)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        add_definitions(-DLEVILINUX_ARM64=1)
        set(LEVILINUX_ARCH "arm64")
    else()
        add_definitions(-DLEVILINUX_X86_64=1)
        set(LEVILINUX_ARCH "x86_64")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DLEVILINUX_WINDOWS=1)
    message(FATAL_ERROR "Windows not supported in this build. Use LeviLauncher.")
endif()

# Install paths
set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install prefix")
set(LEVILINUX_INSTALL_BINDIR "bin")
set(LEVILINUX_INSTALL_LIBDIR "lib/${PROJECT_NAME}")
set(LEVILINUX_INSTALL_DATADIR "share/${PROJECT_NAME}")

# ─────────────────────────────────────────────────────────────────────────────
# CMAKE MODULES & POLICIES
# ─────────────────────────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

cmake_policy(SET CMP0077 NEW)  # option() honors normal variables
cmake_policy(SET CMP0135 NEW)  # file(GLOB) not re-evaluated at build time

# ─────────────────────────────────────────────────────────────────────────────
# OPTIONS
# ─────────────────────────────────────────────────────────────────────────────
option(BUILD_UI "Build Qt6 UI" ON)
option(BUILD_CLI "Build CLI tool" OFF)
option(BUILD_TESTS "Build unit/integration tests" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(USE_SYSTEM_MCPelaUNCHER "Use system mcpelauncher libraries" OFF)
option(USE_SYSTEM_QT "Use system Qt6" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# ─────────────────────────────────────────────────────────────────────────────
# SANITIZERS
# ─────────────────────────────────────────────────────────────────────────────
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()
if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# DEPENDENCY FINDING
# ─────────────────────────────────────────────────────────────────────────────
# Qt6 (Required for UI)
if(BUILD_UI)
    find_package(Qt6 6.6 REQUIRED COMPONENTS
        Core Widgets Network Concurrent LinguistTools
        Gui Svg
    )
    if(Qt6Gui_FOUND)
        find_package(Qt6 6.6 COMPONENTS OpenGLWidgets)
    endif()
    set(HAVE_QT6 TRUE)
    add_definitions(-DHAVE_QT6=1)
endif()

# System libraries
find_package(PkgConfig REQUIRED)

# Required: OpenGL, Vulkan, GLFW/SDL3, mbedTLS, libcurl, zlib, FMOD
pkg_check_modules(OPENGL REQUIRED gl)
pkg_check_modules(VULKAN vulkan)
pkg_check_modules(GLFW3 glfw3)
pkg_check_modules(SDL3 sdl3)
pkg_check_modules(MBEDTLS mbedtls)
pkg_check_modules(LIBCURL libcurl)
pkg_check_modules(ZLIB zlib)
pkg_check_modules(FMOD fmod)  # Optional, may not be in pkg-config
pkg_check_modules(PIPEWIRE libpipewire-0.3)
pkg_check_modules(PULSEAUDIO libpulse)
pkg_check_modules(LIBDWARF libdwarf)  # For stack unwinding
pkg_check_modules(LIBUNWIND libunwind)  # For stack unwinding
pkg_check_modules(CAPSTONE capstone)  # For HookManager disassembly
pkg_check_modules(FMT fmt)  # For logging

# Fallback for FMOD (proprietary, manual path)
if(NOT FMOD_FOUND)
    find_library(FMOD_LIBRARY fmod PATHS /usr/local/lib /opt/fmod/lib)
    find_path(FMOD_INCLUDE_DIR fmod.hpp PATHS /usr/local/include /opt/fmod/include)
    if(FMOD_LIBRARY AND FMOD_INCLUDE_DIR)
        set(FMOD_FOUND TRUE)
        set(FMOD_LIBRARIES ${FMOD_LIBRARY})
        set(FMOD_INCLUDE_DIRS ${FMOD_INCLUDE_DIR})
    endif()
endif()

# Required dependencies check
foreach(dep OPENGL MBEDTLS LIBCURL ZLIB)
    if(NOT ${dep}_FOUND)
        message(FATAL_ERROR "Required dependency ${dep} not found")
    endif()
endforeach()

# Optional but recommended
if(NOT GLFW3_FOUND AND NOT SDL3_FOUND)
    message(WARNING "Neither GLFW3 nor SDL3 found. Window/audio backend may not work.")
endif()

if(NOT FMOD_FOUND)
    message(WARNING "FMOD not found. Audio will use PipeWire/PulseAudio fallback.")
endif()

# ─────────────────────────────────────────────────────────────────────────────
# THIRD PARTY SUBMODULES
# ─────────────────────────────────────────────────────────────────────────────
# These are expected as git submodules in third_party/
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# mcpelauncher-manifest (provides linker, core, client, common, MSA)
# This is the core engine that loads libminecraftpe.so on Linux
set(MCPelaUNCHER_DIR "${THIRD_PARTY_DIR}/mcpelauncher-manifest")
if(EXISTS "${MCPelaUNCHER_DIR}/CMakeLists.txt")
    # Check required deps for mcpelauncher (eglut needs libpng + EGL)
    find_package(PNG QUIET)
    find_package(EGL QUIET)

    if(PNG_FOUND AND EGL_FOUND)
        # mcpelauncher-manifest has its own options; disable UI to avoid conflicts
        set(BUILD_CLIENT ON CACHE BOOL "Build mcpelauncher-client" FORCE)
        set(BUILD_UI OFF CACHE BOOL "Build mcpelauncher manifest UI" FORCE)
        set(BUILD_WEBVIEW OFF CACHE BOOL "Build webview" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
        set(USE_OWN_CURL ON CACHE BOOL "Use bundled curl" FORCE)
        set(USE_SDL3_AUDIO ON CACHE BOOL "Use SDL3 for audio" FORCE)
        set(ENABLE_DEV_PATHS ON CACHE BOOL "Enable dev paths" FORCE)

        add_subdirectory("${MCPelaUNCHER_DIR}"
                         "${CMAKE_CURRENT_BINARY_DIR}/third_party/mcpelauncher-manifest")
        set(HAVE_MCPelaUNCHER_MANIFEST TRUE)

        # Add mcpelauncher include dirs to global path
        include_directories(
            "${MCPelaUNCHER_DIR}/mcpelauncher-linker"
            "${MCPelaUNCHER_DIR}/mcpelauncher-core"
            "${MCPelaUNCHER_DIR}/mcpelauncher-common"
            "${MCPelaUNCHER_DIR}/mcpelauncher-client"
            "${MCPelaUNCHER_DIR}/msa-daemon-client"
            "${MCPelaUNCHER_DIR}/game-window"
            "${MCPelaUNCHER_DIR}/libc-shim"
            "${MCPelaUNCHER_DIR}/libjnivm"
            "${MCPelaUNCHER_DIR}/android-support-headers"
            "${MCPelaUNCHER_DIR}/minecraft-imported-symbols"
            "${MCPelaUNCHER_DIR}/logger"
            "${MCPelaUNCHER_DIR}/base64"
            "${MCPelaUNCHER_DIR}/file-util"
            "${MCPelaUNCHER_DIR}/properties-parser"
            "${MCPelaUNCHER_DIR}/arg-parser"
            "${MCPelaUNCHER_DIR}/simple-ipc"
            "${MCPelaUNCHER_DIR}/daemon-utils"
            "${MCPelaUNCHER_DIR}/cll-telemetry"
            "${MCPelaUNCHER_DIR}/eglut"
        )
    else()
        message(WARNING "mcpelauncher-manifest requires libpng-dev and libegl-dev. "
                        "Install them: sudo apt install libpng-dev libegl-dev")
        message(WARNING "Building LeviLinux WITHOUT mcpelauncher-manifest (core UI only)")
    endif()
else()
    message(WARNING "mcpelauncher-manifest not found in third_party/. "
                    "Run: git submodule update --init --recursive")
endif()

# pl-mod / pl-modmenu / pl-config (from LeviLaunchroid)
if(EXISTS "${THIRD_PARTY_DIR}/pl-mod/CMakeLists.txt")
    add_subdirectory("${THIRD_PARTY_DIR}/pl-mod"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/pl-mod")
    set(HAVE_PL_MOD TRUE)
endif()

if(EXISTS "${THIRD_PARTY_DIR}/pl-config/CMakeLists.txt")
    add_subdirectory("${THIRD_PARTY_DIR}/pl-config"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/pl-config")
    set(HAVE_PL_CONFIG TRUE)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# GLOBAL INCLUDE DIRECTORIES
# ─────────────────────────────────────────────────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${OPENGL_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIRS}
    ${SDL3_INCLUDE_DIRS}
    ${MBEDTLS_INCLUDE_DIRS}
    ${LIBCURL_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${FMOD_INCLUDE_DIRS}
    ${PIPEWIRE_INCLUDE_DIRS}
    ${PULSEAUDIO_INCLUDE_DIRS}
    ${LIBDWARF_INCLUDE_DIRS}
    ${LIBUNWIND_INCLUDE_DIRS}
    ${CAPSTONE_INCLUDE_DIRS}
    ${FMT_INCLUDE_DIRS}
)

# ─────────────────────────────────────────────────────────────────────────────
# COMPILE OPTIONS
# ─────────────────────────────────────────────────────────────────────────────
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
    -Wno-missing-field-initializers
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -pthread
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wno-class-memaccess
        -Wno-deprecated-declarations
        -Wno-sign-compare
    )
endif()

# ─────────────────────────────────────────────────────────────────────────────
# SUBDIRECTORIES
# ─────────────────────────────────────────────────────────────────────────────
add_subdirectory(src/core)
add_subdirectory(src/bridge)

if(BUILD_UI)
    add_subdirectory(src/ui)
endif()

if(BUILD_CLI)
    add_subdirectory(src/cli)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# ─────────────────────────────────────────────────────────────────────────────
# INSTALLATION
# ─────────────────────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
        DESTINATION "${CMAKE_INSTALL_DATADIR}"
        FILES_MATCHING PATTERN "*.png" PATTERN "*.svg" PATTERN "*.ttf"
        PATTERN "*.qss" PATTERN "*.json")

# ─────────────────────────────────────────────────────────────────────────────
# CPACK CONFIGURATION
# ─────────────────────────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Minecraft Bedrock Launcher for Linux")
set(CPACK_PACKAGE_VENDOR "LeviLinux Team")
set(CPACK_PACKAGE_CONTACT "https://github.com/LeviLinux/LeviLinux")
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev, libglfw3, libvulkan1, libmbedtls14, libcurl4, zlib1g, libpipewire-0.3-0, libpulse0, libcapstone4, libfmt10")
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase, glfw, vulkan-loader, mbedtls, libcurl, zlib, pipewire, pulseaudio-libs, capstone, fmt")
include(CPack)

# ─────────────────────────────────────────────────────────────────────────────
# SUMMARY
# ─────────────────────────────────────────────────────────────────────────────
message(STATUS "================================================================")
message(STATUS "LeviLinux ${PROJECT_VERSION} Configuration Summary")
message(STATUS "================================================================")
message(STATUS "  Arch: ${LEVILINUX_ARCH}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Qt6 UI: ${BUILD_UI}")
message(STATUS "  CLI Tool: ${BUILD_CLI}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  ASAN: ${ENABLE_ASAN}")
message(STATUS "  TSAN: ${ENABLE_TSAN}")
message(STATUS "  UBSAN: ${ENABLE_UBSAN}")
message(STATUS "  mcpelauncher-manifest: ${HAVE_MCPelaUNCHER_MANIFEST}")
message(STATUS "  pl-mod: ${HAVE_PL_MOD}")
message(STATUS "  pl-config: ${HAVE_PL_CONFIG}")
message(STATUS "  FMOD: ${FMOD_FOUND}")
message(STATUS "  GLFW3: ${GLFW3_FOUND}")
message(STATUS "  SDL3: ${SDL3_FOUND}")
message(STATUS "  PipeWire: ${PIPEWIRE_FOUND}")
message(STATUS "  PulseAudio: ${PULSEAUDIO_FOUND}")
message(STATUS "  Capstone: ${CAPSTONE_FOUND}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "================================================================")