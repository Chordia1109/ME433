cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hw18_current_sensor_version C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(hw18_current_sensor_version
    HW18.c
)

target_link_libraries(hw18_current_sensor_version
    pico_stdlib
    hardware_i2c
    hardware_pwm
)

pico_enable_stdio_usb(hw18_current_sensor_version 1)
pico_enable_stdio_uart(hw18_current_sensor_version 0)

pico_add_extra_outputs(hw18_current_sensor_version)