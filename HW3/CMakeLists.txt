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

# Pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(HW3_433 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(HW3_433
    HW3_433.c
)

pico_set_program_name(HW3_433 "HW3_433")
pico_set_program_version(HW3_433 "0.1")

# Enable USB serial, disable UART serial
pico_enable_stdio_uart(HW3_433 0)
pico_enable_stdio_usb(HW3_433 1)

# Add include path
target_include_directories(HW3_433 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link libraries
target_link_libraries(HW3_433
    pico_stdlib
    hardware_i2c
)

# Create additional output files
pico_add_extra_outputs(HW3_433)