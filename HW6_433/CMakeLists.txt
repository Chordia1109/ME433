cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW6_433 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW6_433
    HW6_433.c
    usb_descriptors.c
)

target_include_directories(HW6_433 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(HW6_433
    pico_stdlib
    hardware_i2c
    hardware_gpio
    tinyusb_device
    tinyusb_board
    m
)

pico_enable_stdio_usb(HW6_433 0)
pico_enable_stdio_uart(HW6_433 1)

pico_add_extra_outputs(HW6_433)