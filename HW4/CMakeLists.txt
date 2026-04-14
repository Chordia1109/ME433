# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(HW4_433 C CXX ASM)

pico_sdk_init()

add_executable(HW4_433
    HW4_433.c
    ssd1306.c
)

pico_set_program_name(HW4_433 "HW4_433")
pico_set_program_version(HW4_433 "0.1")

pico_enable_stdio_uart(HW4_433 0)
pico_enable_stdio_usb(HW4_433 0)

target_link_libraries(HW4_433
    pico_stdlib
    hardware_i2c
    hardware_adc
)

target_include_directories(HW4_433 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(HW4_433)