cmake_minimum_required(VERSION 3.14)
include(../picosystem/pico_sdk_import.cmake)

set(PICOSYSTEM_DIR /Users/smitty/GitHub/picosystem)

add_definitions(-DDEBUG)

project(phantom-slayer
        C CXX ASM)
project(phantom-slayer
        VERSION 1.1.0
        DESCRIPTION "Phantom Slayer for PicoSystem")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

find_package(PICOSYSTEM REQUIRED)

picosystem_executable(phantom-slayer
                      assets.cpp
                      gfx.cpp
                      help.cpp
                      main.cpp
                      map.cpp
                      phantom.cpp
                      tinymt32.c
                      utils.cpp)

#disable_startup_logo(phantom-slayer)
no_spritesheet(phantom-slayer)

target_link_libraries(phantom-slayer
                      hardware_adc
)

pico_enable_stdio_usb(phantom-slayer 1)
pico_enable_stdio_uart(phantom-slayer 0)
