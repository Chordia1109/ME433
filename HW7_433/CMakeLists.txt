cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH "/Users/chordia/.pico-sdk/sdk/2.2.0")
set(PICO_TOOLCHAIN_PATH "/Users/chordia/.pico-sdk/toolchain/13_3_Rel1")

include(pico_sdk_import.cmake)

project(HW7 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW7
    HW7_433.c
)

target_link_libraries(HW7
    pico_stdlib
    hardware_spi
    m
)

pico_enable_stdio_usb(HW7 1)
pico_enable_stdio_uart(HW7 0)

pico_add_extra_outputs(HW7)