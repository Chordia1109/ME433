cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW14 C CXX ASM)

pico_sdk_init()

add_executable(HW14
    HW14.c
)

target_link_libraries(HW14
    pico_stdlib
)

pico_enable_stdio_usb(HW14 1)
pico_enable_stdio_uart(HW14 0)

pico_add_extra_outputs(HW14)