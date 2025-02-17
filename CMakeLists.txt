cmake_minimum_required(VERSION 3.5)
project(teensy_gfx_src_root C CXX)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_CXX_STANDARD 17)
IF (DEFINED BUILD_FOR_TEENSY)
    # ignore cmake macro idf_component_register
    macro(idf_component_register)
        return()
    endmacro()

    include(FetchContent)
    FetchContent_Declare(teensy_cmake_macros
            GIT_REPOSITORY https://github.com/newdigate/teensy-cmake-macros
            GIT_TAG        main
    )
    FetchContent_MakeAvailable(teensy_cmake_macros)
    include(${teensy_cmake_macros_SOURCE_DIR}/CMakeLists.include.txt)

    import_arduino_library(cores ${teensy_cores_SOURCE_DIR}/teensy4 avr util)
    import_arduino_library_git(SPI https://github.com/PaulStoffregen/SPI master "")
    import_arduino_library_git(Wire https://github.com/PaulStoffregen/Wire master "" utility)
    import_arduino_library_git(adafruit_busio https://github.com/adafruit/Adafruit_BusIO master "")
    import_arduino_library_git(adafruit_gfx https://github.com/adafruit/Adafruit-GFX-Library.git master "")
    import_arduino_library_git(adafruit_st77xx https://github.com/adafruit/Adafruit-ST7735-Library.git master "")
ELSE()
    include(cmake_declare_and_fetch.cmake.in)
    DeclareAndFetch(teensy_x86_stubs            https://github.com/newdigate/teensy-x86-stubs.git main src)
ENDIF()

add_subdirectory(src)
IF (BUILD_EXAMPLES)
    add_subdirectory(examples/basic)
ENDIF ()