cmake_minimum_required(VERSION 3.14)
include(../picosystem/pico_sdk_import.cmake)

project(phantom-slayer
        C CXX ASM)
project(phantom-slayer
        VERSION 1.0.0
        DESCRIPTION "Phantom Slayer for PicoSystem")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

find_package(PICOSYSTEM REQUIRED)

picosystem_executable(phantom-slayer
                      main.cpp
                      map.cpp
                      tinymt32.c
                      utils.cpp)

disable_startup_logo(phantom-slayer)
