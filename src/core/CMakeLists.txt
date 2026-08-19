cmake_minimum_required(VERSION 3.22)

# ─────────────────────────────────────────────────────────────────────────────
# CORE LIBRARY (LeviLinuxCore) - Static library linking all core components
# ─────────────────────────────────────────────────────────────────────────────

# Source files for the core library
set(CORE_SOURCES
    # Launcher orchestration
    launcher/GameLauncher.cpp
    launcher/VersionManager.cpp
    launcher/VersionConfig.cpp
    launcher/MinecraftProcess.cpp
    
    # Mod injector
    mod_injector/ModLoader.cpp
    mod_injector/ModManager.cpp
    mod_injector/ModManifest.cpp
    mod_injector/NativeMod.cpp
    mod_injector/ModMenu.cpp
    mod_injector/ConfigSchema.cpp
    mod_injector/Preloader.cpp
    mod_injector/HookManager.cpp
    mod_injector/SymbolPatcher.cpp
    mod_injector/ModEntry.cpp
)

set(CORE_HEADERS
    launcher/GameLauncher.hpp
    launcher/VersionManager.hpp
    launcher/VersionConfig.hpp
    launcher/MinecraftProcess.hpp
    
    mod_injector/ModLoader.hpp
    mod_injector/ModManager.hpp
    mod_injector/ModManifest.hpp
    mod_injector/NativeMod.hpp
    mod_injector/ModMenu.hpp
    mod_injector/ConfigSchema.hpp
    mod_injector/Preloader.hpp
    mod_injector/HookManager.hpp
    mod_injector/SymbolPatcher.hpp
    mod_injector/ModEntry.hpp
)

add_library(LeviLinuxCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(LeviLinuxCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/LeviLinux>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/launcher
        ${CMAKE_CURRENT_SOURCE_DIR}/mod_injector
)

target_link_libraries(LeviLinuxCore
    PUBLIC
        ${OPENGL_LIBRARIES}
        ${VULKAN_LIBRARIES}
        ${GLFW3_LIBRARIES}
        ${SDL3_LIBRARIES}
        ${MBEDTLS_LIBRARIES}
        ${LIBCURL_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${FMOD_LIBRARIES}
        ${PIPEWIRE_LIBRARIES}
        ${PULSEAUDIO_LIBRARIES}
        ${LIBDWARF_LIBRARIES}
        ${LIBUNWIND_LIBRARIES}
        ${CAPSTONE_LIBRARIES}
        ${FMT_LIBRARIES}
        pthread
        dl
        rt
)

# mcpelauncher-manifest libraries (if available)
if(HAVE_MCPelaUNCHER_MANIFEST)
    target_link_libraries(LeviLinuxCore PUBLIC
        mcpelauncher-linker
        mcpelauncher-core
        mcpelauncher-client
        mcpelauncher-common
        msa-daemon-client
        eglut
        libc-shim
        libjnivm
        game-window
        linux-gamepad
        simple-ipc
        logger
        daemon-utils
    )
    target_compile_definitions(LeviLinuxCore PUBLIC HAVE_MCPelaUNCHER_MANIFEST=1)
else()
    message(STATUS "LeviLinux: Building without mcpelauncher-manifest (UI/bridge only)")
endif()

# pl-mod / pl-config (if available)
if(HAVE_PL_MOD)
    target_link_libraries(LeviLinuxCore PUBLIC pl-mod pl-modmenu)
endif()
if(HAVE_PL_CONFIG)
    target_link_libraries(LeviLinuxCore PUBLIC pl-config)
endif()

target_compile_definitions(LeviLinuxCore
    PUBLIC
        LEVILINUX_CORE_EXPORTS
    PRIVATE
        LEVILINUX_BUILD_CORE
)

set_target_properties(LeviLinuxCore PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

# ─────────────────────────────────────────────────────────────────────────────
# PRELOADER SHARED LIBRARY (libpreloader.so) - Injected into game process
# ─────────────────────────────────────────────────────────────────────────────

set(PRELOADER_SOURCES
    mod_injector/preloader/PreloaderMain.cpp
    mod_injector/preloader/ModContextImpl.cpp
    mod_injector/preloader/JNIHooks.cpp
    mod_injector/preloader/HookInstaller.cpp
    mod_injector/preloader/ModRegistry.cpp
)

set(PRELOADER_HEADERS
    mod_injector/preloader/PreloaderMain.hpp
    mod_injector/preloader/ModContextImpl.hpp
    mod_injector/preloader/JNIHooks.hpp
    mod_injector/preloader/HookInstaller.hpp
    mod_injector/preloader/ModRegistry.hpp
)

add_library(preloader SHARED ${PRELOADER_SOURCES} ${PRELOADER_HEADERS})

target_include_directories(preloader
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mod_injector
        ${CMAKE_CURRENT_SOURCE_DIR}/mod_injector/preloader
)

target_link_libraries(preloader
    PRIVATE
        LeviLinuxCore
        ${CAPSTONE_LIBRARIES}
        ${LIBDWARF_LIBRARIES}
        ${LIBUNWIND_LIBRARIES}
        dl
        pthread
)

# Preloader must be position independent and not hide symbols (for dlopen)
set_target_properties(preloader PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
    OUTPUT_NAME "preloader"
    PREFIX ""
    SUFFIX ".so"
    INSTALL_RPATH "\$ORIGIN/../lib/${PROJECT_NAME}"
)

# Install preloader to lib/LeviLinux/
install(TARGETS preloader
    LIBRARY DESTINATION ${LEVILINUX_INSTALL_LIBDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# ─────────────────────────────────────────────────────────────────────────────
# MCPelauncher-client wrapper (mcpelauncher-client-levi)
# ─────────────────────────────────────────────────────────────────────────────

# This is a thin wrapper that sets up environment and calls the real client
# The real mcpelauncher-client comes from mcpelauncher-manifest
# We create a launcher script / wrapper binary

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL HEADERS
# ─────────────────────────────────────────────────────────────────────────────
install(FILES ${CORE_HEADERS}
    DESTINATION include/LeviLinux
    COMPONENT dev
)

# ─────────────────────────────────────────────────────────────────────────────
# INSTALL MAIN LIBRARY
# ─────────────────────────────────────────────────────────────────────────────
install(TARGETS LeviLinuxCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    COMPONENT libraries
)

# ─────────────────────────────────────────────────────────────────────────────
# PACKAGE CONFIG
# ─────────────────────────────────────────────────────────────────────────────
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/LeviLinuxConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LeviLinuxConfig.cmake
    INSTALL_DESTINATION lib/cmake/LeviLinux
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LeviLinuxConfig.cmake
    DESTINATION lib/cmake/LeviLinux
)