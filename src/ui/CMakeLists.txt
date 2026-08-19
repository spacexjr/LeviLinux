cmake_minimum_required(VERSION 3.22)

# ─────────────────────────────────────────────────────────────────────────────
# UI APPLICATION (LeviLinuxUI) - Qt6 Application
# ─────────────────────────────────────────────────────────────────────────────

# Enable Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 already found in parent CMakeLists.txt
if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Required for UI.")
endif()

# ─────────────────────────────────────────────────────────────────────────────
# SOURCES & RESOURCES
# ─────────────────────────────────────────────────────────────────────────────

set(UI_SOURCES
    # Main
    main.cpp
    
    # Windows
    windows/MainWindow.cpp
    windows/VersionSelector.cpp
    windows/ModManagerDialog.cpp
    windows/AccountManagerDialog.cpp
    windows/SettingsDialog.cpp
    windows/LogConsole.cpp
    
    # Widgets
    widgets/ModListWidget.cpp
    widgets/VersionCard.cpp
    widgets/ProgressDialog.cpp
    widgets/ToastNotification.cpp
    
    # Models
    models/VersionModel.cpp
    models/ModModel.cpp
    models/AccountModel.cpp
    
    # Services
    services/BridgeAPI.cpp
    services/LaunchService.cpp
    services/ModService.cpp
    services/AuthService.cpp
    services/VersionService.cpp
)

set(UI_HEADERS
    windows/MainWindow.hpp
    windows/VersionSelector.hpp
    windows/ModManagerDialog.hpp
    windows/AccountManagerDialog.hpp
    windows/SettingsDialog.hpp
    windows/LogConsole.hpp
    
    widgets/ModListWidget.hpp
    widgets/VersionCard.hpp
    widgets/ProgressDialog.hpp
    widgets/ToastNotification.hpp
    
    models/VersionModel.hpp
    models/ModModel.hpp
    models/AccountModel.hpp
    
    services/BridgeAPI.hpp
    services/LaunchService.hpp
    services/ModService.hpp
    services/AuthService.hpp
    services/VersionService.hpp
)

set(UI_FORMS
    windows/MainWindow.ui
    windows/ModManagerDialog.ui
    windows/AccountManagerDialog.ui
    windows/SettingsDialog.ui
    windows/LogConsole.ui
    widgets/ModListWidget.ui
    widgets/VersionCard.ui
    widgets/ProgressDialog.ui
)

set(UI_RESOURCES
    resources/resources.qrc
)

# ─────────────────────────────────────────────────────────────────────────────
# TARGET
# ─────────────────────────────────────────────────────────────────────────────

add_executable(LeviLinuxUI
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${UI_FORMS}
    ${UI_RESOURCES}
)

target_include_directories(LeviLinuxUI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../core
        ${CMAKE_CURRENT_SOURCE_DIR}/../bridge
)

target_link_libraries(LeviLinuxUI
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Concurrent
        Qt6::Gui
        Qt6::Svg
        LeviLinuxCore
        LeviLinuxBridge
)

# Qt6 OpenGL widgets if available
if(Qt6OpenGLWidgets_FOUND)
    target_link_libraries(LeviLinuxUI PRIVATE Qt6::OpenGLWidgets)
endif()

target_compile_definitions(LeviLinuxUI
    PRIVATE
        QT_NO_DEBUG_OUTPUT
        LEVILINUX_UI_VERSION="${PROJECT_VERSION}"
)

# ─────────────────────────────────────────────────────────────────────────────
# TRANSLATIONS
# ─────────────────────────────────────────────────────────────────────────────

# Get translatable files
file(GLOB_RECURSE UI_TRANSLATABLE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ui"
)

# Languages
set(UI_TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/levilinux_en.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/levilinux_es.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/levilinux_pt_BR.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/levilinux_zh_CN.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/levilinux_ru.ts"
)

# Create/update .ts files
qt_add_lupdate(levilinux_ui_translations
    SOURCES ${UI_TRANSLATABLE_SOURCES}
    TS_FILES ${UI_TS_FILES}
)

# Compile .ts to .qm and embed
qt_add_lrelease(LeviLinuxUI
    TS_FILES ${UI_TS_FILES}
    QM_FILES_OUTPUT QM_FILES
)

# ─────────────────────────────────────────────────────────────────────────────
# ICONS & DESKTOP FILE
# ─────────────────────────────────────────────────────────────────────────────

# Application icon
set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/levilinux.svg")

# Desktop entry
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/levilinux.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/levilinux.desktop
    @ONLY
)

# ─────────────────────────────────────────────────────────────────────────────
# INSTALLATION
# ─────────────────────────────────────────────────────────────────────────────

install(TARGETS LeviLinuxUI
    RUNTIME DESTINATION ${LEVILINUX_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

install(FILES ${APP_ICON}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/
    RENAME levi-linux.svg
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/levilinux.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/
    RENAME levi-linux.desktop
)

# MIME types for .mcpack, .mcaddon, .tmc
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/mime/levilinux-minecraft.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages/
)

# ─────────────────────────────────────────────────────────────────────────────
# DESKTOP FILE VALIDATION
# ─────────────────────────────────────────────────────────────────────────────

find_program(DESKTOP_FILE_VALIDATE desktop-file-validate)
if(DESKTOP_FILE_VALIDATE)
    add_custom_target(validate_desktop_file
        COMMAND ${DESKTOP_FILE_VALIDATE} ${CMAKE_CURRENT_BINARY_DIR}/levilinux.desktop
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/levilinux.desktop
        COMMENT "Validating desktop file"
    )
    add_dependencies(LeviLinuxUI validate_desktop_file)
endif()