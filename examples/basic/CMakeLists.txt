cmake_minimum_required(VERSION 3.10)
project(basic_example)
set(CMAKE_CXX_STANDARD 17)
IF (DEFINED BUILD_FOR_TEENSY)
    #import_arduino_library(cores ${COREPATH})
    teensy_include_directories(../../src)
    teensy_add_executable(basic_example basic.cpp)
ELSE()
    add_definitions(-DBUILD_FOR_LINUX)
    add_definitions(-fms-extensions)
    add_definitions(-DArduino=146)
    include_directories(../../src)
    add_executable(basic_example basic.cpp)
ENDIF()

IF (DEFINED BUILD_FOR_TEENSY)
    teensy_target_link_libraries(basic_example teensy_gfx cores )
    target_link_libraries(basic_example.elf m stdc++)
ELSE()
    target_link_libraries(basic_example teensy_x86_stubs)
    target_link_libraries(basic_example teensy_gfx)
ENDIF()

