cmake_minimum_required(VERSION 3.22)

# ─────────────────────────────────────────────────────────────────────────────
# BRIDGE API (LeviLinuxBridge) - C API for UI ↔ Core communication
# ─────────────────────────────────────────────────────────────────────────────

set(BRIDGE_SOURCES
    levi_bridge.cpp
    BridgeAPI.cpp
)

set(BRIDGE_HEADERS
    levi_bridge.h
    types.h
    callbacks.h
)

add_library(LeviLinuxBridge SHARED ${BRIDGE_SOURCES} ${BRIDGE_HEADERS})

target_include_directories(LeviLinuxBridge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/LeviLinux>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../core
)

target_link_libraries(LeviLinuxBridge
    PUBLIC
        LeviLinuxCore
)

target_compile_definitions(LeviLinuxBridge
    PUBLIC
        LEVILINUX_BRIDGE_EXPORTS
)

set_target_properties(LeviLinuxBridge PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME "levilinux_bridge"
)

# Install
install(TARGETS LeviLinuxBridge
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${BRIDGE_HEADERS}
    DESTINATION include/LeviLinux
    COMPONENT dev
)