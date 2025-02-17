cmake_minimum_required(VERSION 3.5)
project(teensy_gfx C CXX)
set(teensy_gfx_VERSION 1.0.0)

set(SOURCE_FILES
        glcdfont.c
        View.cpp
)

set(HEADER_FILES
        ILI9341_fonts.h
        View.h
)

IF (DEFINED BUILD_FOR_TEENSY)
    message(INFO " Teensy GFX - Compile for Teensy")
    include(CMakeLists.teensy.cmake.in)
ELSE()
    message(INFO " Teensy GFX - Compile for linux")
    add_definitions(-DBUILD_FOR_LINUX)
    include(CMakeLists.linux.cmake.in)
ENDIF()