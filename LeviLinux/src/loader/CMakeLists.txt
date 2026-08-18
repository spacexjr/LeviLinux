# CMakeLists.txt for loader library

set(LOADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/library_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mod_loader.cpp
)

add_library(loader ${LOADER_SOURCES})

target_include_directories(loader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/loader
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(loader dl)
