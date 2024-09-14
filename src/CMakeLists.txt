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
    include(CMakeLists.teensy.cmake.in)
ELSE()
    include(CMakeLists.linux.cmake.in)
ENDIF()