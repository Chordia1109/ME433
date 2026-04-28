cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW8_433 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW8_433
    HW8_433.c
)

target_link_libraries(HW8_433
    pico_stdlib
    hardware_spi
    m
)

pico_enable_stdio_usb(HW8_433 1)
pico_enable_stdio_uart(HW8_433 0)

pico_add_extra_outputs(HW8_433)