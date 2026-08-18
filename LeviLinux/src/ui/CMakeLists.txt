# CMakeLists.txt for UI component

set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settings_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/version_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mod_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_manager.cpp
)

add_executable(levilinux ${UI_SOURCES})

target_include_directories(levilinux PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(levilinux
    core
    loader
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

set_target_properties(levilinux PROPERTIES
    AUTORCC ON
)