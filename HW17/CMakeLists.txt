cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW17 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW17
    HW17.c
)

target_link_libraries(HW17
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(HW17 1)
pico_enable_stdio_uart(HW17 0)

pico_add_extra_outputs(HW17)