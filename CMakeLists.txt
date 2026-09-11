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
)

if(WIN32)
    add_executable(mkvdc WIN32 ${MKVDC_SOURCES})
else()
    add_executable(mkvdc ${MKVDC_SOURCES})
endif()

rexglue_setup_target(mkvdc)
