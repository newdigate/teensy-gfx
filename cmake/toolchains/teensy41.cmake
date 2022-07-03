set(TEENSY_VERSION 41 CACHE STRING "Set to the Teensy version corresponding to your board (30 or 31 allowed)" FORCE)
set(CPU_CORE_SPEED 600000000 CACHE STRING "Set to 24000000, 48000000, 72000000 or 96000000 to set CPU core speed" FORCE) # Derived variables
set(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs" CACHE INTERNAL "")
#teensy compiler options
set(COMPILERPATH "/home/runner/work/gcc-arm-none-eabi-10.3-2021.10/bin/")
#set(COMPILERPATH "/Applications/ARM_9_3/bin/")
set(DEPSPATH "/__w/deps")
#set(DEPSPATH "/Users/nicholasnewdigate/Development/github/newdigate/temp_dep")
set(COREPATH "${DEPSPATH}/cores/teensy4/")
set(BUILD_FOR_TEENSY ON)
add_definitions(-DTEENSY_VERSION=${TEENSY_VERSION})
find_package(teensy_cmake_macros)