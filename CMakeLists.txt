# mkvdc - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(mkvdc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(MKVDC_SOURCES
    src/main.cpp
    src/mod_manager.cpp
)

if(WIN32)
    add_executable(mkvdc WIN32 ${MKVDC_SOURCES})
else()
    add_executable(mkvdc ${MKVDC_SOURCES})
endif()

rexglue_setup_target(mkvdc)
if(WIN32)
    target_link_options(mkvdc PRIVATE -Wl,/DEBUG)
endif()

add_custom_command(TARGET mkvdc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:mkvdc>/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/mods
        $<TARGET_FILE_DIR:mkvdc>/mods
)
