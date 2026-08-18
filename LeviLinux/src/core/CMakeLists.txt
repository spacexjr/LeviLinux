# CMakeLists.txt for core library

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform_abstraction.cpp
)

add_library(core ${CORE_SOURCES})

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(core dl)
