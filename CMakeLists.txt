cmake_minimum_required(VERSION 3.14)
include(${CMAKE_SOURCE_DIR}/pico_sdk_import.cmake)

get_filename_component(SDKPATH "${PROJECT_SOURCE_DIR}../picosystem" REALPATH)
if(NOT EXISTS ${SDKPATH})
    message(FATAL_ERROR "Please clone the Picosystem SDK in a directory parallel ot this one.")
endif()

set(PICOSYSTEM_DIR ${SDKPATH})

#execute_process(COMMAND date +%s OUTPUT_VARIABLE EPOCH)
#add_compile_definitions(RANDSEED=${EPOCH})

project(phantom-slayer
        LANGUAGES C CXX ASM
        VERSION 1.1.3
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
                      utils.cpp
                      tinymt32.c)

disable_startup_logo(phantom-slayer)
no_spritesheet(phantom-slayer)

target_link_libraries(phantom-slayer
                      hardware_adc
)

pico_enable_stdio_usb(phantom-slayer 1)
pico_enable_stdio_uart(phantom-slayer 0)
