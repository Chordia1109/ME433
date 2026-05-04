cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW10 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW10
    HW10.c
)

target_link_libraries(HW10
    pico_stdlib
)

pico_enable_stdio_usb(HW10 1)
pico_enable_stdio_uart(HW10 0)

pico_add_extra_outputs(HW10)